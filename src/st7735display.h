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

#ifndef TEENSY_AUDIO_SAMPLER_ST7735DISPLAY_H
#define TEENSY_AUDIO_SAMPLER_ST7735DISPLAY_H

#include <Arduino.h>
#include <ST7735_t3.h>
#include "sampleplaymidicontrollerenums.h"
#include "abstractdisplay.h"
#include "extracolors.h"

class ST7735Display : public AbstractDisplay {
public:
    ST7735Display(ST7735_t3 &tft) : _tft(tft) {
        tft.setTextWrap(false);
    }

    void switchMode(playcontrollerstate newstate) override {
        _tft.setCursor(0,0);

        switch (newstate) {
            case playcontrollerstate::playcontrollerstate_initialising: {
                clearDisplay(ST77XX_BLACK);
                clearTopArea(ST77XX_BLUE);
                _tft.setTextColor(ST77XX_WHITE);
                _tft.println("initializing...");
                break;
            }
            case playcontrollerstate::playcontrollerstate_performing: {
                clearDisplay(ST77XX_BLACK);
                clearTopArea(ST77XX_COLOR_BRITISHRACINGGREEN);
                _tft.setTextColor(ST77XX_WHITE);
                _tft.println("performing...");
                break;
            }
            case playcontrollerstate::playcontrollerstate_editing: {
                clearDisplay(ST77XX_BLACK);
                clearTopArea(ST77XX_RED);
                _tft.setTextColor(ST77XX_WHITE);
                _tft.println("editing...");
                break;
            }
            default: {
                clearDisplay(ST77XX_BLACK);
                clearTopArea(ST77XX_BLACK);
                _tft.setTextColor(ST77XX_BLACK);
                _tft.println("not sure...");
                break;
            }
        }
    }
    
    void switchCtrlFunction(triggerctrlfunction newctrl) override {
        if (_selectedCtrl != -1) {
            _tft.drawRoundRect(_ctrlhighlight_x1, _ctrlhighlight_y1, _ctrlhighlight_x2, _ctrlhighlight_height, 3, ST77XX_BLACK);
        }
        _ctrlhighlight_x1 = 0;
        _ctrlhighlight_x2 = 127;

        _selectedCtrl = newctrl;
        switch(newctrl) {
            case triggerctrlfunction::triggerctrlfunction_selectsample: {
                _ctrlhighlight_y1 = row4_y1;
                _ctrlhighlight_height = row4_height;
                break;
            }
            case triggerctrlfunction::triggerctrlfunction_tune: {
                _ctrlhighlight_y1 = row6_y1;
                _ctrlhighlight_height = row6_height;
                break;
            }
            case triggerctrlfunction::triggerctrlfunction_trigger: {
                _ctrlhighlight_y1 = row7_y1;
                _ctrlhighlight_height = row7_height;
                break;
            }
            case triggerctrlfunction::triggerctrlfunction_looptype: {
                _ctrlhighlight_y1 = row8_y1;
                _ctrlhighlight_height = row8_height;
                break;
            }
            default:
                break;
        }
        _tft.drawRoundRect(_ctrlhighlight_x1, _ctrlhighlight_y1, _ctrlhighlight_x2, _ctrlhighlight_height, 3, ST77XX_WHITE);
    }

    void editNote(sdsampleplayernote *note) override {
        if (!note) return;
        clearPromptArea(ST7735_BLACK);
        _tft.setTextColor(ST77XX_WHITE);

        _tft.setCursor(0, row2_y1);
        _tft.print("note: ");
        _tft.print(note->_samplerNoteNumber);

        _tft.setCursor(0, row3_y1);
        _tft.print("channel: ");
        _tft.print(note->_samplerNoteChannel);
        
        _tft.setCursor(0, row4_y1);
        _tft.print("sample:");

        // row 5
        displayFileName(note->_filename);
        
        _tft.setCursor(0, row6_y1);
        _tft.print("tune: 1.0");

        _tft.setCursor(0, row7_y1);
        _tft.print("trigger: ");
        _tft.print(note->_indexOfNoteToPlay);

        _tft.setCursor(0, row8_y1);
        _tft.print("loop: ");
        _tft.print(note->_indexOfNoteToPlay);
    }

    void prompt(const char *text) override {
        clearPromptArea(ST77XX_BLACK);
        _tft.setCursor(0,10);
        _tft.setTextColor(ST77XX_WHITE);
        _tft.println(text);
    }

    void displayFileName(const char *text) override {
        clearFilenameArea(ST77XX_BLACK);
        if (!text) return;
        _tft.setCursor(0,row5_y1);
        _tft.setTextColor(ST77XX_WHITE);
        _tft.setTextSize(2);
        _tft.println(text);
        _tft.setTextSize(1);    
    }

private:
    ST7735_t3 &_tft;
    int8_t _selectedCtrl = -1;
    uint8_t _ctrlhighlight_x1, _ctrlhighlight_y1, _ctrlhighlight_x2, _ctrlhighlight_height = 0;

    constexpr static uint8_t   
        row1_y1 = 0, 
        row1_height = 8, 
        row2_y1 = row1_y1 + row1_height + 1,
        row2_height = 8,
        row3_y1 = row2_y1 + row2_height + 1, 
        row3_height = 8, 
        row4_y1 = row3_y1 + row3_height + 1,
        row4_height = 8,
        row5_y1 = row4_y1 + row4_height + 1, 
        row5_height = 16, 
        row6_y1 = row5_y1 + row5_height + 1,
        row6_height = 8,
        row7_y1 = row6_y1 + row6_height + 1, 
        row7_height = 8, 
        row8_y1 = row7_y1 + row7_height + 1,
        row8_height = 8;

    void clearDisplay(uint16_t color) {
        _tft.fillRect(0, 0, 127, 127, color);
    }

    void clearTopArea(uint16_t color) {
        _tft.fillRect(0, row1_y1, 127, row1_height, color);
    }

    void clearPromptArea(uint16_t color) {
        _tft.fillRect(0, row2_y1, 127, row2_height + row2_height, color);
    }

    void clearFilenameArea(uint16_t color) {
        _tft.fillRect(0, row5_y1, 127, row5_height, color);
    }
 
};

#endif // TEENSY_AUDIO_SAMPLER_ST7735DISPLAY_H