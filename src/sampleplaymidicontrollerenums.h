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

#ifndef TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLERENUMS_H
#define TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLERENUMS_H

enum playcontrollerstate {
    playcontrollerstate_initialising = 0, // need to ascertain which midi notes and channels correspond to which control functions
    playcontrollerstate_performing = 1,
    playcontrollerstate_editing = 2,
};

enum ctrlkeyorcc {
    ctrlkeyorcc_none = 0,
    ctrlkeyorcc_mode = 1,
    ctrlkeyorcc_ctrlsel = 2,
    ctrlkeyorcc_valsel = 3
};

enum triggerctrlfunction {
    triggerctrlfunction_none = 0,
    triggerctrlfunction_selectsample = 1,
    triggerctrlfunction_tune = 2,
    triggerctrlfunction_trigger = 3,
    triggerctrlfunction_looptype = 4,
    triggerctrlfunction_direction = 5,
    triggerctrlfunction_volume = 6,
    triggerctrlfunction_pan = 7
};

enum triggertype {
    triggertype_play_until_end = 0,
    triggertype_play_until_subsequent_notedown = 1,
    triggertype_play_while_notedown = 2
};

enum playlooptype {
    playlooptype_once = 0,
    playlooptype_looping = 1,
    playlooptype_pingpong = 2
};

enum playdirection {
    playdirection_begin_forward = 0,
    playdirection_begin_backward = 1
};

#endif // TEENSY_AUDIO_SAMPLER_SAMPLEPLAYMIDICONTROLLERENUMS_H