#ifndef TEENSY_SAMPLER_H
#define TEENSY_SAMPLER_H

#include "sdsampleplayernote.h"
#include "polyphonic.h"
#include <TeensyVariablePlayback.h>
#include <tuple>
#include <sampler.h>
#include "loopsampler.h"
#include "audiovoicepolyphonic.h"

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


    class MyLoopSampler : public audiosampler<AudioPlaySdResmp, sdsampleplayernote> {
    public:
        MyLoopSampler(
            samplermodel<sdsampleplayernote> &samplermodel, 
            audiovoicepolyphonic<AudioPlaySdResmp> &polyphony
        ): 
            audiosampler<AudioPlaySdResmp, sdsampleplayernote>(
                polyphony, 
                [&] (uint8_t noteNumber, uint8_t noteChannel) -> triggertype {
                    sdsampleplayernote *sample = _samplermodel.getNoteForChannelAndKey(noteChannel, noteNumber);
                    if (sample == nullptr)
                        return triggertype::triggertype_play_while_notedown;
                    return sample->_triggertype;
                }),
            _samplermodel(samplermodel) 
        {
        }
        
        MyLoopSampler(const MyLoopSampler&) = delete;

        virtual ~MyLoopSampler() {
        }

        void voiceOnEvent(AudioPlaySdResmp *voice, sdsampleplayernote *sample, uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) override {
            if (sample == nullptr || sample->_filename == nullptr)
                return;

            AudioPlaySdResmp &audioplay = *voice;
            switch (sample->_playdirection) {
                case playdirection_begin_forward: audioplay.setPlaybackRate(1.0); break; 
                case playdirection_begin_backward: audioplay.setPlaybackRate(-1.0); break;
            }

            switch (sample->_playlooptype) {
                case playlooptype_once: audioplay.setLoopType(loop_type::looptype_none); break;
                case playlooptype_looping: audioplay.setLoopType(loop_type::looptype_repeat); break;
                case playlooptype_pingpong: audioplay.setLoopType(loop_type::looptype_pingpong); break;
            }

            audioplay.playWav(sample->_filename);
            updateRegistrations(sample->_filename, voice);

            //sample->_voice = voice;
        }

        void voiceOffEvent(AudioPlaySdResmp *voice, sdsampleplayernote *sample, uint8_t noteNumber, uint8_t noteChannel) override {

        }

        void voiceRetriggerEvent(AudioPlaySdResmp *voice, sdsampleplayernote *sample, uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) override {

        }


        sdsampleplayernote* findSample(uint8_t noteNumber, uint8_t noteChannel) override {
            return _samplermodel.findNearestSampleForNoteAndChannel(noteNumber, noteChannel);
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
                std::vector<ProgressRegistration*>::iterator position = 
                    std::find_if(
                        regsForFileName->begin(), 
                        regsForFileName->end(), 
                        [registrationId] 
                            (ProgressRegistration* pr) -> bool { 
                                return pr->_id == registrationId; 
                            });
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
                updateProgressRegistration(reg);
                reg->_voice = voice;
            }
        }

        void updateProgress() {
            for (auto && reg : _progressRegistrationsById) {
                if (reg.second == nullptr)
                    return;

                updateProgressRegistration(reg.second);
            }
        }

        void updateProgressRegistration(ProgressRegistration *reg) {
            if (reg->_voice != nullptr)
            { 
                if  (reg->_voice->isPlaying() ) {
                    int bufferPosition = reg->_voice->getBufferPosition1();
                    int progress = (bufferPosition * reg->_resolution) / reg->_voice->getLoopFinish();
                    if (progress != reg->_lastValue) {
                        // call callback
                        reg->_callback(reg->_id, progress);
                        reg->_lastValue = progress;
                    }
                };
            }
        }


    protected:
        samplermodel<sdsampleplayernote> &_samplermodel;
        unsigned int _currentId;
        std::map<char*, std::vector<ProgressRegistration*>*> _progressRegistrations;
        std::map<unsigned int, ProgressRegistration*> _progressRegistrationsById;

        static float calcPitchFactor(uint8_t note, uint8_t rootNoteNumber) {
            float result = powf(2.0, (note-rootNoteNumber) / 12.0);
            return result;
        }
    };
 /*
//unpitchedsdwavsampler
    class MyLoopSampler : public loopsampler {
    public:
//    loopsampler(samplermodel<sdloopaudiosample> &samplermodel, polyphonic<audiovoice<AudioPlaySdResmp>> &polyphony) : 

        MyLoopSampler(samplermodel<sdsampleplayernote<AudioPlaySdResmp>> &model, polyphonic<AudioPlaySdResmp> &polyphony) : 
            loopsampler(model, polyphony), 
            _currentId(0),
            _progressRegistrations(),
            _progressRegistrationsById()
        {
        }
        
        virtual ~MyLoopSampler() {

        }

        void removeAllSamples() {
            for (auto && sample : _audiosamples) {
                //if (sample->_filename)
                //    delete [] sample->_filename;
                delete sample;
            }
            _audiosamples.clear();
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
                std::vector<ProgressRegistration*>::iterator position = 
                    std::find_if(
                        regsForFileName->begin(), 
                        regsForFileName->end(), 
                        [registrationId] 
                            (ProgressRegistration* pr) -> bool { 
                                return pr->_id == registrationId; 
                            });
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
                updateProgressRegistration(reg);
                reg->_voice = voice;
            }
        }

        void updateProgress() {
            for (auto && reg : _progressRegistrationsById) {
                if (reg.second == nullptr)
                    return;

                updateProgressRegistration(reg.second);
            }
        }

        void updateProgressRegistration(ProgressRegistration *reg) {
            if (reg->_voice != nullptr)
            { 
                if  (reg->_voice->isPlaying() ) {
                    int bufferPosition = reg->_voice->getBufferPosition1();
                    int progress = (bufferPosition * reg->_resolution) / reg->_voice->getLoopFinish();
                    if (progress != reg->_lastValue) {
                        // call callback
                        reg->_callback(reg->_id, progress);
                        reg->_lastValue = progress;
                    }
                };
            }
        }

    protected:
        unsigned int _currentId;
        std::map<char*, std::vector<ProgressRegistration*>*> _progressRegistrations;
        std::map<unsigned int, ProgressRegistration*> _progressRegistrationsById;


        void noteEventCallback(uint8_t voice, uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity, bool isNoteOn, bool retrigger)    
        {
            uint8_t numVoices = __base::_numVoices;
            if (voice < numVoices) {
                audiovoice<AudioPlaySdResmp> *audio_voice = __base::_voices[voice];
                sdsampleplayernote<AudioPlaySdResmp> *sample = _model.getNoteForChannelAndKey(noteChannel, noteNumber);
                if (sample != nullptr && sample->_filename != nullptr) {
                    if (isNoteOn) {                
                        if (audio_voice->_audiomixer != nullptr) {                        
                            audio_voice->_audiomixer->gain( audio_voice->_mixerChannel, velocity / 255.0);
                        }
                        if (audio_voice->_audiomixer2 != nullptr) {                        
                            audio_voice->_audiomixer2->gain(audio_voice->_mixerChannel, velocity / 255.0);
                        }

                        if (audio_voice->_audioenvelop != nullptr) {
                            audio_voice->_audioenvelop->noteOn();
                        }
                        play(noteNumber, audio_voice, sample);
                        updateRegistrations(sample->_filename, audio_voice->_audioplayarray);
                    } else 
                    {
    //                    if (sample->_triggertype == triggertype_play_while_notedown)
                        audio_voice->_audioplayarray->stop();
                    }
                }
            }
        }


        static void play(uint8_t noteNumber, audiovoice<AudioPlaySdResmp> *voice, sdsampleplayernote<AudioPlaySdResmp> *sample) {

            AudioPlaySdResmp &audioplay = *(voice->_audioplayarray);
            switch (sample->_playdirection) {
                case playdirection_begin_forward: audioplay.setPlaybackRate(1.0); break; 
                case playdirection_begin_backward: audioplay.setPlaybackRate(-1.0); break;
            }

            switch (sample->_playlooptype) {
                case playlooptype_once: audioplay.setLoopType(loop_type::looptype_none); break;
                case playlooptype_looping: audioplay.setLoopType(loop_type::looptype_repeat); break;
                case playlooptype_pingpong: audioplay.setLoopType(loop_type::looptype_pingpong); break;
            }

            audioplay.playWav(sample->_filename);
            sample->_voice = voice->_audioplayarray;
        }


    };
*/

} // namespace newdigate

#endif
