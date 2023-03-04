#ifndef TEENSY_SAMPLER_H
#define TEENSY_SAMPLER_H

#include "sdsampleplayernote.h"
#include "SamplerModel.h"
#include "polyphonic.h"
#include <TeensyVariablePlayback.h>
#include <tuple>
namespace newdigate
{
    typedef std::function<void(unsigned index, unsigned sampleNumber)> CallbackFn;

    class ProgressRegistration {
        public:
            ProgressRegistration(unsigned int id, unsigned int resolution, CallbackFn callback, char *filename) :
                _id(id),
                _resolution(resolution),
                _callback(callback),
                _filename(filename)
            {
            }

            virtual ~ProgressRegistration() { }
            unsigned int _id;
            unsigned int _resolution;
            CallbackFn _callback;
            unsigned int _lastValue = 0;
            AudioPlaySdResmp *_voice = nullptr;
            char *_filename;
    };

    class Sampler {
    public:
        Sampler(SamplerModel &model, polyphonic<AudioPlaySdResmp> &polyphony) : 
            _model(model),
            _polyphony(polyphony),
            _progressRegistrations()
        {
        }

        virtual ~Sampler() {
            //destroy _progressRegistrations !!!!s
        }


        void midiChannleVoiceMessage(uint8_t status, uint8_t data1, uint8_t data2, uint8_t channel) {
            bool isNoteOn = (status & 0xf0) == 0x90;
            bool isNoteOff = (status & 0xf0) == 0x80;
            bool isCC = (status & 0xf0) == 0xc0;
            if (isCC)
                return;

            sdsampleplayernote<AudioPlaySdResmp> *sample = _model.getNoteForChannelAndKey(channel, data1);
            if (sample) {
                if (isNoteOn) {
                    sampletrigger_received(sample);
                } else  
                {
                    bool isNoteOff = (status & 0xf0) == 0x80;
                    if (isNoteOff)
                        sampletriggerstop_received(sample);
                }
            }
        }

        void sampletrigger_received(sdsampleplayernote<AudioPlaySdResmp> *sample) {        

            switch (sample->_triggertype) {
                case triggertype_play_until_end :
                case triggertype_play_while_notedown : {
                    AudioPlaySdResmp *voice = _polyphony.useVoice();
                    if (voice != nullptr) {
                        voice->stop();
                        if (sample->_filename != nullptr) {

                            std::vector<ProgressRegistration*> *progressForFilename = _progressRegistrations[sample->_filename];
                            if (progressForFilename != nullptr){
                                for (auto && reg : *progressForFilename) {
                                    reg->_voice = voice;
                                }
                            }

                            voice->playWav(sample->_filename);
                            sample->_voice = voice;
                            updateRegistrations(sample->_filename, voice);
                        }
                    }
                    break;
                }

                case triggertype_play_until_subsequent_notedown: {
                    if (sample->_voice != nullptr) {
                        sample->_voice->stop();
                        _polyphony.freeVoice(sample->_voice);
                        sample->_voice = nullptr;
                        sample->isPlaying = false;
                    } else {
                        AudioPlaySdResmp *voice = _polyphony.useVoice();
                        if (voice != nullptr && sample->_filename != nullptr) {
                            voice->playWav(sample->_filename);
                            sample->_voice = voice;
                            
                            std::vector<ProgressRegistration*> *progressForFilename = _progressRegistrations[sample->_filename];
                            if (progressForFilename != nullptr){
                                for (auto && reg : *progressForFilename) {
                                    reg->_voice = voice;
                                }
                            }
                        } 
                    }
                    break;
                }

                default: break;
            }
        }
        
        void sampletriggerstop_received(sdsampleplayernote<AudioPlaySdResmp> *sample) {        
            if (!sample) {
                return;
            }

            switch (sample->_triggertype) {
                case triggertype_play_until_end :
                case triggertype_play_until_subsequent_notedown :
                break;
                
                case triggertype_play_while_notedown :
                {   
                    if (sample->_voice != nullptr) {
                        sample->_voice->stop();
                        sample->_voice = nullptr;
                    }
                    break;
                }

                default: break;
            }
        }

        unsigned int registerProgressCallback(char *filename, unsigned int resolution, CallbackFn callback) {
            if (_progressRegistrations.find(filename) == _progressRegistrations.end()) {
                _progressRegistrations[filename] = new std::vector<ProgressRegistration*>();
            }
            ProgressRegistration *progReg = new ProgressRegistration(_currentId++, resolution, callback, filename);
            _progressRegistrations[filename]->push_back(progReg);
            _progressRegistrationsById[progReg->_id] = progReg;
            return progReg->_id;
        }

        void unregisterProgressCallback(unsigned int registrationId){
            ProgressRegistration *progReg = _progressRegistrationsById[registrationId];
            if (progReg == nullptr)
                return;
            _progressRegistrationsById.erase(registrationId);

            std::vector<ProgressRegistration*>* regsForFileName = _progressRegistrations[progReg->_filename];
            if (regsForFileName != nullptr) {
                std::vector<ProgressRegistration*>::iterator position = std::find_if(regsForFileName->begin(), regsForFileName->end(), [registrationId] (ProgressRegistration* pr) -> bool { return pr->_id == registrationId; });
                if (position != regsForFileName->end())
                    regsForFileName->erase(position);

            }
            delete progReg;
        }
    
        void updateRegistrations(char *filename, AudioPlaySdResmp *voice) {
            std::vector<ProgressRegistration*>* listOfProgressForFileName = _progressRegistrations[filename];
            if (listOfProgressForFileName == nullptr)
                return;

            for (auto && reg : *listOfProgressForFileName) {
                
            }
        }

        void updateProgress() {
            _updateCount++;
            //_updateCount %= 1;
            //if (_updateCount != 0)
            //    return;

            for (auto && reg : _progressRegistrationsById) {
                if (reg.second == nullptr)
                    return;

                if (reg.second->_voice != nullptr)
                { 
                    if  (reg.second->_voice->isPlaying() ) {
                        int bufferPosition = reg.second->_voice->getBufferPosition1();
                        int progress = (bufferPosition * reg.second->_resolution) / reg.second->_voice->getLoopFinish();
                        if (progress != reg.second->_lastValue) {
                            // call callback
                            reg.second->_callback(reg.second->_id, progress);
                            reg.second->_lastValue = progress;
                        }
                    };
                }
            }
        }
        
    private:
        SamplerModel &_model;
        polyphonic<AudioPlaySdResmp> &_polyphony;
        std::map<char*, std::vector<ProgressRegistration*>*> _progressRegistrations;
        std::map<unsigned int, ProgressRegistration*> _progressRegistrationsById;
        unsigned int _updateCount = 0;
        unsigned int _currentId = 0;
    };



} // namespace newdigate

#endif
