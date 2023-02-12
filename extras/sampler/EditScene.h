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
namespace newdigate {

    const int NUM_EDIT_MENU_ITEMS = 6;

    class EditScene : public BaseScene {
    public:
        EditScene(SamplerModel &samplerModel, View &view) : 
            BaseScene(
                _bmp_edit_on, 
                _bmp_edit_off,
                16, 16), 
            _samplerModel(samplerModel),
            _view(view),
            _settingsMenu(view, 0, 30, 128, 98, ST7735_BLUE, ST7735_BLACK),
            _triggerNoteControlNeedsUpdate(true),
            _triggerNoteControl(view, [&] () { 
                if (_triggerNoteControlNeedsUpdate) {
                    _triggerNoteControlNeedsUpdate = false;
                    _pianoDisplay.drawFullPiano();
                    _view.drawString("trigger pad", 0, 0);
                    _view.drawLine(0, 28, 128, 28, ST7735_WHITE);
                    if (_currentNote == nullptr) {
                        _view.drawString("n/a", 30, 9);
                    }
                    
                    _view.drawString("channel", 0, 20);
                    if (_currentNote == nullptr)
                        return;
                        
                    _view.fillRect(100, 0, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteNumber, 100, 0); 
                    
                    _view.fillRect(20, 8, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteNumber/12, 20, 8); 
     
                    _view.fillRect(100, 20, 28, 8, ST7735_BLACK);
                    _view.drawNumber(_currentNote->_samplerNoteChannel, 100, 20);

                    
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
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("loop  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("pan  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("sample  ", 0, 0);}, 8),
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("tune  ", 0, 0);}, 8), 
                TeensyMenuItem(_settingsMenu, [] (View *v) {v->drawString("volume  ", 0, 0);}, 8)
            },
            _currentNote(nullptr),
            _pianoDisplay(_view, 2, 0, 48, 9) //tft, byte octaves, byte startOctave, byte x, byte y
        {
            for (int i = 0; i < NUM_EDIT_MENU_ITEMS; i++) {
                _settingsMenu.AddControl(&_settingMenuItems[i]);
            }
        }

        virtual ~EditScene() {
        }

        void Update() override {
            _triggerNoteControl.Update();
            _settingsMenu.Update();
        }

        void InitScreen() override {
            _view.fillScreen(ST7735_BLACK);
            _settingsMenu.NeedsUpdate = true; 
            _triggerNoteControlNeedsUpdate = true;
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
            _pianoDisplay.reset();
            _pianoDisplay.keyDown(pitch);
            _pianoDisplay.drawPiano();
            _currentNote = _samplerModel.getNoteForChannelAndKey(channel, pitch);
            if (_currentNote == nullptr) {
                _currentNote = _samplerModel.allocateNote(channel, pitch);
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
        sdsampleplayernote *_currentNote;
        TFTPianoDisplay<View> _pianoDisplay; //tft, byte octaves, byte startOctave, byte x, byte y

    };

}
#endif
