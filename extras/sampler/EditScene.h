#ifndef TEENSY_SAMPLER_EDITSCHENE_H
#define TEENSY_SAMPLER_EDITSCHENE_H

#include <Arduino.h>

#include <MIDI.h>
#include "RtMidiMIDI.h"
#include "RtMidiTransport.h"

#include <Encoder.h>
#include <Bounce2.h>

#include "icons.h"
#include "scenecontroller.h"
#include "teensy_controls.h"
#include "TFTPianoDisplay.h"
#include "DirectoryFileNameCache.h"
#include "WavePreview.h"
#include "MySampler.h"

namespace newdigate {

    const int NUM_EDIT_MENU_ITEMS = 7;
    const uint16_t UA_blue = 0x01B5;
    const uint16_t Gold = 0xFEA0;
    const uint16_t Canary = 0xFFF3;
    const uint16_t Royal_blue_dark2 = 0x012C;
    const uint16_t Oxford_blue2 = 0x0109;

    class EditScene : public BaseScene {
    public:
        EditScene(samplermodel<sdsampleplayernote> &samplerModel, View &view, DirectoryFileNameCache &directoryFileNameCache, SDClass &sd, MyLoopSampler &sampler) : 
            BaseScene(
                _bmp_edit_on, 
                _bmp_edit_off,
                16, 16), 
            _samplerModel(samplerModel),
            _view(view),
            _settingsMenu(view, 0, 34, 128, 128-34, Oxford_blue2, ST7735_BLACK),
            _triggerNoteControlNeedsUpdate(true),
            _triggerNoteControl(view, [&] () { 
                if (_triggerNoteControlNeedsUpdate) {
                    _triggerNoteControlNeedsUpdate = false;
                    _triggerNoteControl.fillRect(0, 0, 128, 28, Royal_blue_dark2);

                    _triggerNoteControl.drawLine(0, 0, 128, 0, UA_blue);
                    _triggerNoteControl.drawLine(0, _triggerNoteControl.Height()-2, 127, _triggerNoteControl.Height()-2, UA_blue);
                    
                    if (_currentNote == nullptr) {
                        _triggerNoteControl.setTextColor(Canary);
                        _triggerNoteControl.drawString("Select a note...", 8, 4);
                        _triggerNoteControl.setTextColor(RGB565_WHITE);
                        return;
                    }

                    _pianoDisplay.drawFullPiano();
                    int noteNumber = _currentNote->_samplerNoteNumber % 12;
                    switch (noteNumber) {
                        case 0: case 1:     _triggerNoteControl.drawString("c", 0, 4); break;
                        case 2: case 3:     _triggerNoteControl.drawString("d", 0, 4); break;
                        case 4:             _triggerNoteControl.drawString("e", 0, 4); break;
                        case 5: case 6:     _triggerNoteControl.drawString("f", 0, 4); break;
                        case 7: case 8:     _triggerNoteControl.drawString("g", 0, 4); break;
                        case 9: case 10:    _triggerNoteControl.drawString("a", 0, 4); break;
                        case 11:            _triggerNoteControl.drawString("b", 0, 4); break;   
                        default: break;                     
                    }

                    if (noteNumber == 1 || noteNumber == 3 || noteNumber == 6 || noteNumber == 8 || noteNumber == 10)
                        _triggerNoteControl.drawString("#", 8, 4);
                    _triggerNoteControl.drawNumber(_currentNote->_samplerNoteNumber/12, 16, 4); 
                    _triggerNoteControl.setTextColor(Gold);
                    _triggerNoteControl.drawString(":", 24, 4);
                    _triggerNoteControl.drawNumber(_currentNote->_samplerNoteChannel, 32, 4);
                    _triggerNoteControl.setTextColor(RGB565_WHITE);
                    //_pianoDisplay.displayNeedsUpdating();
                }
             }, 128, 15, 0, 19),              //     TeensyControl(View &view, std::function<void()> updateFn, unsigned int width, unsigned int height, unsigned int x unsigned int y)
            _settingMenuItems {
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) 
                    {   
                        v->drawString("sample  ", 0, 0);

                        if (_currentNote == nullptr) {
                            return;
                        }

                        if (_currentNote->_filename != nullptr)  {
                            v->setTextSize(2);
                            v->drawString(_currentNote->_filename, 0, 8);
                            v->setTextSize(1);
                            _wavePreview.Show(_currentNote->_filename);
                        }
                    }, 
                    24,
                    [&] (bool forward) { 
                        if (_currentNote == nullptr || _directoryFileNameCache.getNumFileNames() == 0) {
                            return;
                        }
                        RemoveAllProgressSubscriptions();
                        int value = 0;
                        if (_currentNote->_filename != nullptr)  {
                            value = _directoryFileNameCache.getIndexOfFile(_currentNote->_filename);
                        }

                        if (forward) value++; else value--;
                        value %= _directoryFileNameCache.getNumFileNames();
                        _currentNote->_filename = _directoryFileNameCache.getFileNameForIndex(value);
                        if (_currentNote->_filename != nullptr)  {
                            AddProgressSubscription(_currentNote->_samplerNoteNumber, _currentNote->_samplerNoteChannel, _currentNote->_filename);
                        } 
                    }),
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) {
                        v->drawString("direction", 0, 0);
                        if (_currentNote == nullptr)
                            return;
                        if (_currentNote->_playdirection == playdirection::playdirection_begin_forward) 
                            v->drawString("fwd", 64, 0);
                        else
                            v->drawString("bwd", 64, 0);
                    }, 
                    8, 
                    [&] (bool forward) { 
                        if (_currentNote == nullptr)
                            return;
                        long value =  (long)(_currentNote->_playdirection);
                        if (forward) value++; else value++; // there are only two values; 
                        value %= 2;
                        _currentNote->_playdirection = (playdirection)value;
                    },
                    [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
                        Serial.println("NoteDown...");
                    return true;
                    }),
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) {
                        v->drawString("trigger", 0, 0);
                        if (_currentNote == nullptr)
                            return;
                        switch (_currentNote->_triggertype) {
                            case triggertype::triggertype_play_until_end: v->drawString("to end", 64, 0); break;
                            case triggertype::triggertype_play_until_subsequent_notedown: v->drawString("until next", 64, 0); break;
                            case triggertype::triggertype_play_while_notedown : v->drawString("while down", 64, 0); break;
                            default: v->drawString("---", 64, 0); break;
                        } 
                    }, 
                    8, 
                    [&] (bool forward) { 
                        if (_currentNote == nullptr)
                            return;
                        long value =  (long)(_currentNote->_triggertype);
                        if (forward) value++; else value--; // there are only two values; 
                        if (value < 0) value = 2;
                        value %= 3;
                        _currentNote->_triggertype = (triggertype)value;
                    },
                    [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
                        //Serial.println("NoteDown...");
                        return true;
                    }),
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) {
                        v->drawString("loop", 0, 0);
                        if (_currentNote == nullptr)
                            return;
                        switch (_currentNote->_playlooptype) {
                            case playlooptype::playlooptype_once: v->drawString("none", 64, 0); break;
                            case playlooptype::playlooptype_looping: v->drawString("loop", 64, 0); break;
                            case playlooptype::playlooptype_pingpong: v->drawString("ping", 64, 0); break;
                            default: break;
                        }
                    }, 
                    8,
                    [&] (bool forward) { 
                        if (_currentNote == nullptr)
                            return;
                        long value =  (long)(_currentNote->_playlooptype);
                        if (forward) value++; else value--; // there are only two values; 
                        value %= 3;
                        _currentNote->_playlooptype = (playlooptype)value;
                    }), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("pan  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("tune  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("volume  ", 0, 0);}, 8)
            },
            _currentNote(nullptr),
            _pianoDisplay(_triggerNoteControl, 2, 0, 48, 2), //tft, byte octaves, byte startOctave, byte x, byte y
            _directoryFileNameCache(directoryFileNameCache),
            _wavePreview(_view, sd, 128, 20, 0, 0),
            _sampler(sampler),
            _playbackProgressSubscriptions()
        {
            for (int i = 0; i < NUM_EDIT_MENU_ITEMS; i++) {
                _settingsMenu.AddControl(&_settingMenuItems[i]);
            }
        }

        virtual ~EditScene() {
            _wavePreview.Reset();
        }

        void Update() override {
            _triggerNoteControl.Update();
            _wavePreview.Update();
            _settingsMenu.Update();
        }

        void InitScreen() override {
            _view.fillScreen(ST7735_BLACK);
            _settingsMenu.NeedsUpdate = true; 
            _triggerNoteControlNeedsUpdate = true;
            _wavePreview.ClearBackground();
            if (_currentNote != nullptr && _currentNote->_filename != nullptr) {
                _wavePreview.Show(_currentNote->_filename);
                _wavePreview.Update();
                AddProgressSubscription(_currentNote->_samplerNoteNumber, _currentNote->_samplerNoteChannel, _currentNote->_filename);
            } 
        }

        void UninitScreen() override {
            RemoveAllProgressSubscriptions();
        }

        void ButtonPressed(unsigned index) override {
            if (_currentNote == nullptr || _currentNote->_filename == nullptr)
                return;
            
            _sampler.trigger(_currentNote->_samplerNoteNumber, _currentNote->_samplerNoteChannel, 127, true);

        }

        void Rotary1Changed(bool forward) override {
            if (forward) 
                _settingsMenu.IncreaseSelectedIndex(); 
            else 
                _settingsMenu.DecreaseSelectedIndex(); 
        }

        void Rotary2Changed(bool forward) override {
            _settingsMenu.ValueScroll(forward);
        }

        bool HandleNoteOnOff(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) override { 
            if (noteDown == false)
                return true;
            
            if (_currentNote != nullptr && channel == _currentNote->_samplerNoteChannel && pitch == _currentNote->_samplerNoteNumber)
            {
                // _currentNote didnt change
                return false;
            }

            RemoveAllProgressSubscriptions();
            _pianoDisplay.reset();
            _pianoDisplay.keyDown(pitch);
            _pianoDisplay.setBaseOctave(pitch/12);
            _pianoDisplay.drawPiano();

            char *oldFileName = nullptr;
            if (_currentNote != nullptr) {
                oldFileName = _currentNote->_filename;
            }

            _currentNote = _samplerModel.getNoteForChannelAndKey(channel, pitch);
            if (_currentNote == nullptr) {
                _currentNote = new sdsampleplayernote();
                _currentNote->_samplerNoteChannel = channel;
                _currentNote->_samplerNoteNumber = pitch;
                _currentNote = _samplerModel.allocateNote(channel, pitch, _currentNote);
            }
            
            if (_currentNote->_filename != nullptr){
                if  (oldFileName != nullptr && strcmp(oldFileName, _currentNote->_filename) != 0)
                {
                    AddProgressSubscription(pitch, channel, _currentNote->_filename);
                    _wavePreview.Reset();
                }
            } else {
                _wavePreview.ClearBackground();
            }

            _settingsMenu.NeedsUpdate = true; 
            _settingsMenu.Update();
            _triggerNoteControlNeedsUpdate = true;
            return true; 
        }

        void AddProgressSubscription(uint8_t noteNumber, uint8_t noteChannel, char *filename) {
            _playbackProgressSubscriptions.push_back(
                _sampler.registerProgressCallback(
                    noteNumber,
                    noteChannel,
                    filename, 
                    _view._width, 
                    [&] (unsigned index, unsigned progress) {
                        this->ProgressUpdateReceived(index, progress);
                    })
            );
        }
        void RemoveAllProgressSubscriptions() {
            for (auto && sub : _playbackProgressSubscriptions) {
                _sampler.unregisterProgressCallback(sub);
            }
            _playbackProgressSubscriptions.clear();
        }

        void ProgressUpdateReceived(unsigned index, unsigned progress) {
            _wavePreview.CreateOrUpdateProgressIndicator(index, progress);
        }

        bool HandleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) override { 
            return false; 
        }

    private:
        samplermodel<sdsampleplayernote>& _samplerModel;
        View& _view;
        TeensyMenu _settingsMenu;
        bool _triggerNoteControlNeedsUpdate;
        TeensyControl _triggerNoteControl;
        TeensyMenuItem _settingMenuItems[NUM_EDIT_MENU_ITEMS];
        sdsampleplayernote *_currentNote;
        TFTPianoDisplay<View> _pianoDisplay; //tft, byte octaves, byte startOctave, byte x, byte y
        DirectoryFileNameCache& _directoryFileNameCache;
        WavePreview _wavePreview;
        MyLoopSampler &_sampler;
        std::vector<unsigned int> _playbackProgressSubscriptions;

    };

}
#endif
