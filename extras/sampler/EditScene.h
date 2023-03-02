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
#include "Sampler.h"

namespace newdigate {

    const int NUM_EDIT_MENU_ITEMS = 6;

    class EditScene : public BaseScene {
    public:
        EditScene(SamplerModel &samplerModel, View &view, DirectoryFileNameCache &directoryFileNameCache, SDClass &sd, Sampler &sampler) : 
            BaseScene(
                _bmp_edit_on, 
                _bmp_edit_off,
                16, 16), 
            _samplerModel(samplerModel),
            _view(view),
            _settingsMenu(view, 0, 50, 128, 78, ST7735_BLUE, ST7735_BLACK),
            _triggerNoteControlNeedsUpdate(true),
            _triggerNoteControl(view, [&] () { 
                if (_triggerNoteControlNeedsUpdate) {
                    _triggerNoteControlNeedsUpdate = false;
                    _pianoDisplay.drawFullPiano();
                    _view.drawString("trigger", 0, 0);
                    _view.drawString("oct:", 0, 8);
                    _view.drawLine(0, 28, 128, 28, ST7735_WHITE);
                    if (_currentNote == nullptr) {
                        _view.drawString("n/a", 30, 9);
                    }
                    
                    _view.drawString("ch:", 0, 20);
                    if (_currentNote == nullptr)
                        return;
                        
                    _view.fillRect(64, 0, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteNumber, 64, 0); 
                    
                    _view.fillRect(32, 8, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteNumber/12, 32, 8); 
     
                    _view.fillRect(32, 20, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteChannel, 32, 20);
                }
             }, 128, 28, 0, 0),              //     TeensyControl(View &view, std::function<void()> updateFn, unsigned int width, unsigned int height, unsigned int x unsigned int y)
            _settingMenuItems {
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) {
                        v->drawString("direction", 0, 0);
                        if (_currentNote == nullptr)
                            return;
                        if (_currentNote->_playdirection == playdirection::playdirection_begin_forward) 
                            v->drawString("fwd", 100, 0);
                        else
                            v->drawString("bwd", 100, 0);
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
                        v->drawString("loop", 0, 0);
                        if (_currentNote == nullptr)
                            return;
                        switch (_currentNote->_playlooptype) {
                            case playlooptype::playlooptype_once: v->drawString("none", 100, 0); break;
                            case playlooptype::playlooptype_looping: v->drawString("loop", 100, 0); break;
                            case playlooptype::playlooptype_pingpong: v->drawString("ping", 100, 0); break;
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
                TeensyMenuItem(_settingsMenu, 
                    [&] (View *v) 
                    {   
                        v->drawString("sample  ", 0, 0);

                        if (_currentNote == nullptr)
                            return;

                        if (_currentNote->_filename != nullptr)  {
                            v->setTextSize(2);
                            v->drawString(_currentNote->_filename, 0, 8);
                            v->setTextSize(1);
                            _wavePreview.Show(_currentNote->_filename);
                        }
                    }, 
                    24,
                    [&] (bool forward) { 
                        if (_currentNote == nullptr || _directoryFileNameCache.getNumFileNames() == 0)
                            return;

                        int value = 0;
                        if (_currentNote->_filename != nullptr)  {
                            value = _directoryFileNameCache.getIndexOfFile(_currentNote->_filename);
                        }

                        if (forward) value++; else value--;
                        value %= _directoryFileNameCache.getNumFileNames();
                        _currentNote->_filename = _directoryFileNameCache.getFileNameForIndex(value);
                    }),
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("tune  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("volume  ", 0, 0);}, 8)
            },
            _currentNote(nullptr),
            _pianoDisplay(_view, 2, 0, 48, 9), //tft, byte octaves, byte startOctave, byte x, byte y
            _directoryFileNameCache(directoryFileNameCache),
            _wavePreview(_view, sd, 128, 20, 0, 28),
            _sampler(sampler),
            _playbackProgressSubscriptions()
        {
            for (int i = 0; i < NUM_EDIT_MENU_ITEMS; i++) {
                _settingsMenu.AddControl(&_settingMenuItems[i]);
            }
        }

        virtual ~EditScene() {
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
            _wavePreview.Show("SNARE.wav");
        }

        void ButtonPressed() override {
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
            for (auto && sub : _playbackProgressSubscriptions) {
                _sampler.unregisterProgressCallback(sub);
            }
            _pianoDisplay.reset();
            _pianoDisplay.keyDown(pitch);
            _pianoDisplay.setBaseOctave(pitch/12);
            _pianoDisplay.drawPiano();
            _currentNote = _samplerModel.getNoteForChannelAndKey(channel, pitch);
            if (_currentNote == nullptr) {
                _currentNote = _samplerModel.allocateNote(channel, pitch);
            }
            if (_currentNote->_filename != nullptr){
                _playbackProgressSubscriptions.push_back(
                    _sampler.registerProgressCallback(_currentNote->_filename, _view._width, [&] (unsigned index, unsigned sampleNumber) { } )
                );
            }
            _settingsMenu.NeedsUpdate = true; 
            _settingsMenu.Update();
            _triggerNoteControlNeedsUpdate = true;
            return true; 
        }

        bool HandleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) override { 
            return false; 
        }

        void DrawSettingsMenuItem0(View *v) {
   
        }

    private:
        SamplerModel& _samplerModel;
        View& _view;
        TeensyMenu _settingsMenu;
        bool _triggerNoteControlNeedsUpdate;
        TeensyControl _triggerNoteControl;
        TeensyMenuItem _settingMenuItems[NUM_EDIT_MENU_ITEMS];
        sdsampleplayernote<AudioPlaySdResmp> *_currentNote;
        TFTPianoDisplay<View> _pianoDisplay; //tft, byte octaves, byte startOctave, byte x, byte y
        DirectoryFileNameCache& _directoryFileNameCache;
        WavePreview _wavePreview;
        Sampler &_sampler;
        std::vector<unsigned int> _playbackProgressSubscriptions;

    };

}
#endif
