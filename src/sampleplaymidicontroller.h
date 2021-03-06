/* Audio Library for Teensy
 * Copyright (c) 2021, Nic Newdigate
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLER_H
#define TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLER_H

#include <Arduino.h>
#include <Audio.h>
#include <vector>
#include <map>
#include <SD.h>
#include "sampleplaymidicontrollerenums.h"
#include "abstractdisplay.h"
#include "sdsampleplayernote.h" 
#include "polyphonic.h"

class playcontrollerconfig {
public:
    const char *prompts[3] = {"mode", "ctrl select", "value select"}; 
    uint8_t midinotenum_changeMode,        midichannelnum_changeMode,
            midiccnum_ctrlSelector,        midichannelnum_ctrlSelector,
            midiccnum_valueSelector,       midichannelnum_valueSelector;
    uint8_t num_ctrl_fns_configured = 0;
  
    void prompt() {
        if (num_ctrl_fns_configured < 1 ) {
            Serial.printf("%d: please select a button to configure ctrl key for '%s'.\n", num_ctrl_fns_configured, prompts[num_ctrl_fns_configured]);
        } else if (num_ctrl_fns_configured < 3) {
            Serial.printf("%d: please change a custom control (cc) to select a '%s'.\n", num_ctrl_fns_configured, prompts[num_ctrl_fns_configured]);
        }else 
            Serial.printf("%d: no more functions to configure...\n", num_ctrl_fns_configured);
    }

    void configure(uint8_t noteOrCcNum, uint8_t channel) {
        if (num_ctrl_fns_configured >= sizeof(prompts)/sizeof(prompts[0]) ) {
            Serial.printf("WARN: no more functions to configure!!! (%d)\n", num_ctrl_fns_configured);
            return;
        }
        switch (num_ctrl_fns_configured) {
            case 0: {
                midinotenum_changeMode = noteOrCcNum;
                midichannelnum_changeMode = channel;
                Serial.printf("[configured: button '%s' as note=%d, channel=%d]\n", prompts[num_ctrl_fns_configured], noteOrCcNum, channel);
                num_ctrl_fns_configured++;
                break;
            }

            case 1: {
                midiccnum_ctrlSelector = noteOrCcNum;
                midichannelnum_ctrlSelector = channel;
                Serial.printf("[configured: rotary '%s' as cc=%d, channel=%d]\n", prompts[num_ctrl_fns_configured], noteOrCcNum, channel);
                num_ctrl_fns_configured++;
                break;     
            }

            case 2: {
                if (!iscontrolcc(noteOrCcNum, channel)) {
                    midiccnum_valueSelector = noteOrCcNum;
                    midichannelnum_valueSelector = channel;
                    Serial.printf("[configured: rotary '%s' as cc=%d, channel=%d]\n", prompts[num_ctrl_fns_configured], noteOrCcNum, channel);
                    num_ctrl_fns_configured++;
                }
                break;     

            }               
        }
        if (!complete())
            prompt();
        else
            saveToFile("device.ctl");
    }

    void reset() {
        num_ctrl_fns_configured = 0;
        midinotenum_changeMode = 0;
        midichannelnum_changeMode = 0;
        midiccnum_ctrlSelector = 0;
        midichannelnum_ctrlSelector = 0;
        midiccnum_valueSelector = 0; 
        midichannelnum_valueSelector = 0;
    }

    bool complete() {
        return num_ctrl_fns_configured >= 3;
    }

    bool readReadFromFile(const char * filename) {
        File file = SD.open(filename);
        if (file) {
            playcontrollerconfig config;
            size_t uint8_tsRead = file.read((char*)&config, sizeof(playcontrollerconfig));
            file.close();
            
            if (uint8_tsRead >= sizeof(playcontrollerconfig)) {
                midinotenum_changeMode              = config.midinotenum_changeMode;
                midichannelnum_changeMode           = config.midichannelnum_changeMode;
                midiccnum_ctrlSelector              = config.midiccnum_ctrlSelector;    
                midichannelnum_ctrlSelector         = config.midichannelnum_ctrlSelector;
                midiccnum_valueSelector             = config.midiccnum_valueSelector;    
                midichannelnum_valueSelector        = config.midichannelnum_valueSelector;

                Serial.printf("loaded: change mode: midinote=%d channel=%d\n", midinotenum_changeMode, midichannelnum_changeMode);
                Serial.printf("loaded: control selector: midicc=%d channel=%d\n", midiccnum_ctrlSelector, midichannelnum_ctrlSelector);
                Serial.printf("loaded: value selector: midicc=%d channel=%d\n", midiccnum_valueSelector, midichannelnum_valueSelector);

                return true;
            }  
            Serial.printf("failed to load config - uint8_ts read: %d\n", uint8_tsRead);
            return false;
        } else {
            return false;
        }
    }
    
    bool saveToFile(const char * filename) {
        File file = SD.open(filename, O_WRITE);
        if (file) {
            size_t uint8_tsWritten = file.write((const unsigned char*)this, sizeof(playcontrollerconfig));
            bool success = uint8_tsWritten == sizeof(playcontrollerconfig);
            if (success)
                Serial.printf("config uint8_ts written: %d\n", uint8_tsWritten);
            else 
                Serial.printf("failed to write config: %d\n", uint8_tsWritten);
            file.close();
            return success;
        } 
        Serial.printf("failed config uint8_ts written - not able to open file: %s\n", filename);
        return false;
    }

    bool iscontrolkey(uint8_t noteNum, uint8_t channel) {
        return getfunctionkey(false, noteNum, channel) != ctrlkeyorcc::ctrlkeyorcc_none;
    }

    bool iscontrolcc(uint8_t ccNum, uint8_t channel) {
        return getfunctionkey(true, ccNum, channel) != ctrlkeyorcc::ctrlkeyorcc_none;
    }

    ctrlkeyorcc getfunctionkey(bool isCC, uint8_t noteNum, uint8_t channel) {
        if (!isCC) {
            if (note_is_ctrl_function_key(noteNum, channel, ctrlkeyorcc::ctrlkeyorcc_mode))
                return ctrlkeyorcc::ctrlkeyorcc_mode;
        } else {
            // midi cc
            if (ccnum_is_ctrl_special_controller(noteNum, channel, ctrlkeyorcc::ctrlkeyorcc_ctrlsel ))
                return ctrlkeyorcc::ctrlkeyorcc_ctrlsel;
            else if (ccnum_is_ctrl_special_controller(noteNum, channel, ctrlkeyorcc::ctrlkeyorcc_valsel ))
                return ctrlkeyorcc::ctrlkeyorcc_valsel;
        }
        return ctrlkeyorcc::ctrlkeyorcc_none;
    }

private:
    bool note_is_ctrl_function_key(uint8_t note, uint8_t channel, ctrlkeyorcc funct) {
        switch (funct) {
            case ctrlkeyorcc::ctrlkeyorcc_mode:
                return (note == midinotenum_changeMode && channel == midichannelnum_changeMode);
            default:{
                Serial.printf("Not sure about ctrl function #%d\n", (int)funct);
                return false;
            }
        }
    }

    bool ccnum_is_ctrl_special_controller(uint8_t ccnum, uint8_t channel, ctrlkeyorcc funct) {
        switch (funct) {
            case ctrlkeyorcc::ctrlkeyorcc_ctrlsel:
                return (ccnum == midiccnum_ctrlSelector && channel == midichannelnum_ctrlSelector);    
            case ctrlkeyorcc::ctrlkeyorcc_valsel:
                return (ccnum == midiccnum_valueSelector && channel == midichannelnum_valueSelector);               
            default:{
                Serial.printf("Not sure about cc ctrl function #%d\n", (int)funct);
                return false;
            }
        }
    }

};

class sdsampleplayermidicontroller {
public:
    sdsampleplayermidicontroller(polyphonic<AudioPlaySdWav> &polyphony, AbstractDisplay &display) : _polyphony(polyphony), _display(display) {

    }

    void initialize() {
        _config.reset();
        _state = playcontrollerstate::playcontrollerstate_initialising;
        _display.switchMode(_state);
        _config.prompt();
    }

    void begin(const char* directoryname = nullptr) {
        populateFilenames(directoryname);
        
        if (_config.readReadFromFile("device.ctl")) {
            Serial.printf("loaded settings from file.\n");
            _state = playcontrollerstate::playcontrollerstate_performing;
            _display.switchMode(_state);
        } else {
            _config.prompt();
        }
        if (loadSamples("samples.smp")){
            Serial.println("loaded sample config");
        }
    }

    void midiChannleVoiceMessage(uint8_t status, uint8_t data1, uint8_t data2, uint8_t channel) {
        bool isNoteOn = (status & 0xf0) == 0x90;
        bool isCC = (status & 0xf0) == 0xc0;
        switch (_state) {

            case playcontrollerstate_initialising : {
                // we are learning the ctrl function keys 
                if (isNoteOn || isCC ) { // midi key-down or cc status nibble
                    _config.configure(data1, channel);
                    if (_config.complete()) {
                        _state = playcontrollerstate::playcontrollerstate_performing;
                        _display.switchMode(_state);
                    }
                }
                break;
            }
            
            case playcontrollerstate_performing: {

                ctrlkeyorcc potentialFnKey = 
                    (isCC || isNoteOn)? _config.getfunctionkey(isCC, data1, channel) : ctrlkeyorcc::ctrlkeyorcc_none;

                switch (potentialFnKey) {
                    case ctrlkeyorcc::ctrlkeyorcc_none: {
                        // not a function key - regular performance note...
                        // try find a sample mapping for this fellow
                        sdsampleplayernote *sample = getSamplerNoteForNoteNum(data1, channel);
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
                        break;
                    }
                    case ctrlkeyorcc::ctrlkeyorcc_mode : {
                        if (isNoteOn) {
                            _state = playcontrollerstate::playcontrollerstate_editing;
                            _display.switchMode(_state);
                            if (_selected_target == nullptr) {
                                _display.prompt("select a note");
                            } else 
                                _display.editNote(_selected_target);
                        }
                        break;
                    }
                }
                break;
            }

            case playcontrollerstate_editing: {
                ctrlkeyorcc fntype = _config.getfunctionkey(isCC, data1, channel);

                switch (fntype) {
                    case ctrlkeyorcc::ctrlkeyorcc_none : {
                        if (isNoteOn) {
                            // midi key-down status
                            _selected_target = getSamplerNoteForNoteNum(data1, channel);
                            if (_selected_target == nullptr) {
                                _selected_target = new sdsampleplayernote();
                                _selected_target->_samplerNoteNumber = data1;
                                _selected_target->_samplerNoteChannel = channel;                        
                                _samples.push_back(_selected_target);
                            }
                            _display.editNote(_selected_target);
                            Serial.printf("[selected note=%d, channel=%d]\n", data1, channel);
                        }
                        break;
                    }
                    case ctrlkeyorcc::ctrlkeyorcc_valsel : {

                        switch (_selected_ctrl_function) {
                            case triggerctrlfunction::triggerctrlfunction_trigger: {
                                triggertype triggerType = (triggertype)round(data2 * 2.0 / 128.0); 
                                sdsampleplayernote *sample = getSamplerNoteForNoteNum(_selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel);
                                if (sample) {
                                    if (triggerType != sample->_triggertype){
                                        sample->_triggertype = triggerType;
                                        Serial.printf("sample %d, %d changed trigger type to %d: %s\n", _selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel, (int)triggerType, getTriggerTypeName(triggerType));
                                    }
                                    _display.editNote(sample);
                                }
                                break;
                            }

                            case triggerctrlfunction::triggerctrlfunction_selectsample: {
                                size_t num_samples = _filenames.size();
                                size_t sampleIndex = round(data2 * (num_samples-1) / 128.0);
                                if (_selected_target->_filename == nullptr || strcmp(_selected_target->_filename, _filenames[sampleIndex]) != 0) {
                                    if (_selected_target->_filename) 
                                        delete [] _selected_target->_filename;
                                    if (sampleIndex >= num_samples )
                                        sampleIndex = num_samples-1;
                                    size_t filename_length = strlen(_filenames[sampleIndex])+1;
                                    _selected_target->_filename = new char[filename_length] {0};
                                    memcpy(_selected_target->_filename, _filenames[sampleIndex], strlen(_filenames[sampleIndex]) );
                                    _display.displayFileName(_selected_target->_filename);
                                }
                                break;
                            }

                            case triggerctrlfunction::triggerctrlfunction_direction:{
                                playdirection playDirection = (playdirection) round( data2 / 128.0); 
                                sdsampleplayernote *sample = getSamplerNoteForNoteNum(_selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel);
                                if (sample) {
                                    if (playDirection != sample->_playdirection){
                                        sample->_playdirection = playDirection;
                                        _display.editNote(sample);
                                        //Serial.printf("sample %d, %d changed play dir to %s\n", _selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel, getPlayDirectionName(playDirection));
                                    }
                                }
                                break;
                            }

                            case triggerctrlfunction::triggerctrlfunction_looptype: {
                                playlooptype playLoopType = (playlooptype)(round(data2 * 2.0 / 128.0)); 
                                sdsampleplayernote *sample = getSamplerNoteForNoteNum(_selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel);
                                if (sample) {
                                    if (playLoopType != sample->_playlooptype){
                                        sample->_playlooptype = playLoopType;
                                        _display.editNote(sample);
                                        //Serial.printf("sample %d, %d changed loop type to %s\n", _selected_target->_samplerNoteNumber, _selected_target->_samplerNoteChannel, getPlayLoopTypeName(playLoopType));
                                    }
                                }
                                break;
                            }
                        
                            default: {
                                Serial.printf("not implemented\n");
                                break;
                            }
                        }

                        break;
                    }
                    case ctrlkeyorcc::ctrlkeyorcc_ctrlsel : {
                        triggerctrlfunction ctrlfn = (triggerctrlfunction)(round(data2 * 6.0 / 127.0) + 1); 
                        if (ctrlfn != _selected_ctrl_function) {
                            _selected_ctrl_function = ctrlfn;
                            Serial.printf("control function changed to %s\n", getCtrlFunctionName(_selected_ctrl_function));
                            _display.switchCtrlFunction(_selected_ctrl_function);
                        }
                        break;
                    }         
                    case ctrlkeyorcc::ctrlkeyorcc_mode : {
                        if (isNoteOn) {
                            // the 'change mode' key was pressed - go back to performance mode
                            writeSamples("samples.smp");
                            _state = playcontrollerstate_performing;
                            _display.switchMode(_state);
                        }
                        break;
                    }
                }

                break;
            }

            default:
                break;
        }
    }

    bool loadSamples(const char* filename) {
        File file = SD.open(filename);
        if (file) {
            sdsampleplayernote *note;
            while((note = read_sample(file)) != nullptr) {
                _samples.push_back(note);
                Serial.printf("loaded sample %s\n", note->_filename);
            }
            file.close();
            return true;
        }
        Serial.printf("Not able to open %s\n", filename);
        return false;
    }

    void writeSamples(const char* filename) {
        File file = SD.open(filename, O_WRITE);
        for (auto && note : _samples) {
            write_sample(file, note);
        }
        file.close();
    }

    void update() {
        for (auto && note : _samples) {
            if (note->_voice != nullptr && !note->isPlaying && note->_voice->isPlaying())
            {
                note->isPlaying = true;
            } else
            if (note->_voice != nullptr && note->isPlaying && !note->_voice->isPlaying()) {
                _polyphony.freeVoice(note->_voice);
                note->_voice = nullptr;
                note->isPlaying = false;
            }
        }
    }

    static const char* getCtrlFunctionName(triggerctrlfunction fn){
        switch (fn) {
            case triggerctrlfunction_direction: return "direction";
            case triggerctrlfunction_looptype: return "loop";
            case triggerctrlfunction_none: return "n/a";
            case triggerctrlfunction_pan: return "pan";
            case triggerctrlfunction_selectsample: return "sample";
            case triggerctrlfunction_trigger: return "trigger";
            case triggerctrlfunction_tune: return "tune";
            case triggerctrlfunction_volume: return "volume";
            default :
                return "not sure?";
        }
    }
    static const char* getTriggerTypeName(triggertype tt){
        switch (tt) {
            case triggertype_play_until_end: 
                return "play until end";
            case triggertype_play_until_subsequent_notedown:
                return "toggle";
            case triggertype_play_while_notedown: 
                return "play while notedown";
            default:
                return "not sure?";
        }
    }   
    static const char* getPlayDirectionName(playdirection pd){
        switch (pd) {
            case playdirection_begin_forward: 
                return "begin forwards";
            case playdirection_begin_backward:
                return "begin backwards";
            default:
                return "not sure?";
        }
    }
    static const char* getPlayLoopTypeName(playlooptype pd){
        switch (pd) {
            case playlooptype_once: return "play once";
            case playlooptype_looping: return "looping";
            case playlooptype_pingpong: return "ping-pong";
            default:
                return "not sure?";
        }
    }

private:
    polyphonic<AudioPlaySdWav> &_polyphony;
    AbstractDisplay &_display;
    playcontrollerconfig _config;
    playcontrollerstate _state = playcontrollerstate::playcontrollerstate_initialising;
    sdsampleplayernote *_selected_target = nullptr;
    triggerctrlfunction _selected_ctrl_function = triggerctrlfunction::triggerctrlfunction_selectsample;

    std::vector<sdsampleplayernote*> _samples;
    std::vector<char *> _filenames;

    void unloadFilenames() {
        //_loopsampler.removeAllSamples();
        for (auto && filename : _filenames) {
            delete [] filename;
        }
        _filenames.clear();
    }
    void unloadSamples() {
        for (auto && sample : _samples) {
            if (sample->_filename)
                delete sample->_filename;
            delete sample;
        }
        _samples.clear();
    }
    void populateFilenames(const char *directory) {
        unloadFilenames();
        File dir = directory? SD.open(directory) : SD.open("/");
        unsigned int index = 0;
        while (true) { 

            File files =  dir.openNextFile();
            if (!files) {
                //If no more files, break out.
                break;
            }

            String curfile = files.name(); //put file in string
            int m = curfile.lastIndexOf(".WAV");
            int a = curfile.lastIndexOf(".wav");
            int underscore = curfile.indexOf("_");
            int dot = curfile.indexOf(".");

            // if returned results is more then 0 add them to the list.
            if ((m > 0 || a > 0) && (underscore != 0) && (dot != 0)) {  
                char *filename = new char[curfile.length()+1] {0};
                memcpy(filename, curfile.c_str(), curfile.length());
                _filenames.push_back(filename);
                //Serial.printf("filename: %d: %s\n", index, filename);
                index++;
            } // else 
              //  Serial.printf("INGORE: filename: %d: %s\n", index, curfile.c_str());
            files.close();
        }
        // close 
        dir.close();
    }
    sdsampleplayernote* getSamplerNoteForNoteNum(uint8_t noteNum, uint8_t channel) {
        for (auto && note : _samples) {
            if (note->_samplerNoteNumber == noteNum && note->_samplerNoteChannel == channel)
                return note;
        }
        return nullptr;
    }


    void sampletrigger_received(sdsampleplayernote *sample) {        

        switch (sample->_triggertype) {
            case triggertype_play_until_end :
            case triggertype_play_while_notedown : {
                AudioPlaySdWav *voice = _polyphony.useVoice();
                if (voice != nullptr) {
                    voice->stop();
                    if (sample->_filename != nullptr) {
                        voice->play(sample->_filename);
                        sample->_voice = voice;
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
                    AudioPlaySdWav *voice = _polyphony.useVoice();
                    if (voice != nullptr && sample->_filename != nullptr) {
                        voice->play(sample->_filename);
                        sample->_voice = voice;
                    } 
                }
                break;
            }

            default: break;
        }
    }
    
    void sampletriggerstop_received(sdsampleplayernote *sample) {        
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

    sdsampleplayernote* read_sample(File file) {

        uint16_t progress = 0;
        uint32_t filename_length = varfieldRead(file, progress);

        if (progress == 0)
            return nullptr;

        sdsampleplayernote *samplernote = new sdsampleplayernote();
        samplernote->_filename = new char[filename_length + 1] {0};
        
        size_t uint8_tsRead = file.read( samplernote->_filename, filename_length);      
        if (uint8_tsRead != filename_length) {
            Serial.printf("WARN: could not load sample '%s' (1).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        }
            
        uint8_tsRead = file.read(&(samplernote->_samplerNoteNumber), 1 );
        if (uint8_tsRead != 1) {
            Serial.printf("WARN: could not load sample '%s' (2).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        } 
        
        uint8_tsRead = file.read(&(samplernote->_samplerNoteChannel), 1 );
        if (uint8_tsRead != 1) {
            Serial.printf("WARN: could not load sample '%s' (3).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        } 
        
        uint8_tsRead = file.read(&(samplernote->_playdirection), 1 );
        if (uint8_tsRead != 1) {
            Serial.printf("WARN: could not load sample '%s' (4).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        } 

        uint8_tsRead = file.read(&(samplernote->_triggertype), 1 );
        if (uint8_tsRead != 1) {
            Serial.printf("WARN: could not load sample '%s' (4).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        } 

        uint8_tsRead = file.read(&(samplernote->_playlooptype), 1 );
        if (uint8_tsRead != 1) {
            Serial.printf("WARN: could not load sample '%s' (4).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        } 

        int indexOfFile = getIndexOfSDWaveFile(samplernote->_filename);
        if (indexOfFile == -1) {
            Serial.printf("WARN: could not load sample '%s' (5).\n", samplernote->_filename);
            delete samplernote->_filename;
            delete samplernote;
            return nullptr;
        }
        //Serial.printf("loaded sample: %d: %s\n", indexOfFile, samplernote->_filename);
        return samplernote;
    }

    int getIndexOfSDWaveFile(const char *filenameToMatch) {
        int index = 0;
        for(auto && filename : _filenames) {
            if (strcmp(filename, filenameToMatch) == 0) 
                return index;
            index++;
        }
        return -1;
    }

    void write_sample(File file, sdsampleplayernote *samplernote) {
        if (samplernote->_filename != nullptr) {
            size_t filename_length = strlen(samplernote->_filename);
            varfieldWrite(file, filename_length);
            file.write((const unsigned char *)samplernote->_filename, filename_length);
        } else 
        {
            const size_t filename_length = 0;
            varfieldWrite(file, filename_length);            
        }
        file.write( &(samplernote->_samplerNoteNumber), 1);
        file.write( &(samplernote->_samplerNoteChannel), 1);
        file.write( (const uint8_t *) &(samplernote->_playdirection), 1);
        file.write( (const uint8_t *) &(samplernote->_triggertype), 1);
        file.write( (const uint8_t *) &(samplernote->_playlooptype), 1);
    }

    uint32_t varfieldRead(File &file, uint16_t &progress)
    {
        uint32_t ret = 0;
        uint8_t uint8_t_in;

        for (;;)
        {
            if (file.available()) {
                // return error
                uint8_t_in = file.read();
                progress++;
            } else return 0;
            ret = (ret << 7) | (uint8_t_in & 0x7f);
            if (!(uint8_t_in & 0x80))
                return ret;
        }
    }
    void varfieldWrite(File &file, unsigned int deltaticks) {
        if (deltaticks < 128) {
            file.write((const unsigned char *)&deltaticks, 1);
            return;
        } 

        uint16_t lengthFieldSize = 0;
        uint8_t b[4];
        
        // read least signficicant uint8_ts first
        for (int i = 3; i >= 0; i--) {
            b[i] = (uint8_t)(deltaticks & 0x7f);
            if(i < 3) // set the bit that indicates another uint8_t still to follow... except on the least significant uint8_t
                b[i] |= 0x80;
            deltaticks >>= 7;
            lengthFieldSize++;
            if (deltaticks < 1)
                break;
        }

        for( int i=0; i < lengthFieldSize; i++) {  
            file.write(&(b[4-lengthFieldSize+i]), 1);
        }
    }

};

#endif // TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLER_H