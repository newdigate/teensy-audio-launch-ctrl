#ifndef TEENSY_SAMPLER_MODEL_H
#define TEENSY_SAMPLER_MODEL_H

#include "sdsampleplayernote.h"
#include <map>

namespace newdigate
{
    class SamplerModel {
    public:
        SamplerModel() : 
            _channelNotes() 
        {
        }

        virtual ~SamplerModel() {
            for (auto && channelNoteMap : _channelNotes) {
                for (auto &&  noteIndex : *(channelNoteMap.second))
                    delete noteIndex.second;
                delete channelNoteMap.second;
            }
        }

        sdsampleplayernote<AudioPlaySdResmp>* getNoteForChannelAndKey(uint8_t channel, uint8_t note) {
            if (_channelNotes.find(channel) == _channelNotes.end() ) {
                return nullptr;
            }
            std::map<uint8_t, sdsampleplayernote<AudioPlaySdResmp>*>* channelNotes = _channelNotes[channel];
            if (channelNotes->find(note) == channelNotes->end()){
                return nullptr;
            }

            return (*channelNotes)[note];
        }

        sdsampleplayernote<AudioPlaySdResmp>* allocateNote(uint8_t channel, uint8_t note) {
            if (getNoteForChannelAndKey(channel, note) != nullptr){
                // note is already allocated
                return nullptr;
            }
            std::map<uint8_t, sdsampleplayernote<AudioPlaySdResmp>*>* channelNotes = nullptr;
            if (_channelNotes.find(channel) == _channelNotes.end() ) {
                channelNotes = new std::map<uint8_t, sdsampleplayernote<AudioPlaySdResmp>*>();
                _channelNotes[channel] = channelNotes;
            } else
                channelNotes = _channelNotes[channel];

            sdsampleplayernote<AudioPlaySdResmp> *samplerNote = new sdsampleplayernote<AudioPlaySdResmp>();
            samplerNote->_samplerNoteChannel = channel;
            samplerNote->_samplerNoteNumber = note;
            (*channelNotes)[note] = samplerNote;
            return samplerNote;
        }


    private:
        std::map<uint8_t, std::map<uint8_t, sdsampleplayernote<AudioPlaySdResmp>*>*> _channelNotes;
    };

} // namespace newdigate

#endif
