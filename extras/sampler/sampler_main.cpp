#include <Arduino.h>
#include <Audio.h>

#include <MIDI.h>
#include "RtMidiMIDI.h"
#include "RtMidiTransport.h"

#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>

#include <Encoder.h>
#include <Bounce2.h>
#include <SD.h>

#include "icons.h"
#include "scenecontroller.h"
#include "teensy_controls.h"
#include "TFTPianoDisplay.h"
#include "EditScene.h"
#include "DirectoryFileNameCache.h"
#include "WavePreviewBuilder.h"
#include <TeensyAudioLaunchCtrl.h>
#include <TeensyVariablePlayback.h>
#include "output_soundio.h"
#include "MySampler.h"
#include "DevicesScene.h"

SDClass sd = SDClass("/Users/nicholasnewdigate/Development/sampler");

// GUItool: begin automatically generated code
AudioPlaySdResmp      voice1(sd);         //xy=375,182
AudioPlaySdResmp      voice2(sd);         //xy=375,221
AudioPlaySdResmp      voice3(sd);         //xy=375,263
AudioPlaySdResmp      voice4(sd);         //xy=376,304
AudioPlaySdResmp      voice5(sd);        //xy=377,357
AudioPlaySdResmp      voice6(sd);        //xy=377,401
AudioPlaySdResmp      voice7(sd);        //xy=377,439
AudioPlaySdResmp      voice8(sd);        //xy=379,477
AudioEffectEnvelope      envelope1;      //xy=520,177
AudioEffectEnvelope      envelope2;      //xy=521,221
AudioEffectEnvelope      envelope3;      //xy=519,263
AudioEffectEnvelope      envelope4;      //xy=522,305
AudioEffectEnvelope      envelope5;      //xy=522,357
AudioEffectEnvelope      envelope6;      //xy=526,401
AudioEffectEnvelope      envelope7;      //xy=526,439
AudioEffectEnvelope      envelope8;      //xy=526,477
AudioMixer4              mixer1;          //xy=736,230
AudioMixer4              mixer2;         //xy=737,395
AudioMixer4              mixer3;         //xy=895,329
AudioOutputSoundIO       i2s1;          //xy=737,395
AudioConnection          patchCord1(voice1, envelope1);
AudioConnection          patchCord2(voice2, envelope2);
AudioConnection          patchCord3(voice3, envelope3);
AudioConnection          patchCord4(voice4, envelope4);
AudioConnection          patchCord5(voice5, envelope5);
AudioConnection          patchCord6(voice6, envelope6);
AudioConnection          patchCord7(voice7, envelope7);
AudioConnection          patchCord8(voice8, envelope8);
AudioConnection          patchCord9(envelope3, 0, mixer1, 2);
AudioConnection          patchCord10(envelope1, 0, mixer1, 0);
AudioConnection          patchCord11(envelope2, 0, mixer1, 1);
AudioConnection          patchCord12(envelope4, 0, mixer1, 3);
AudioConnection          patchCord13(envelope5, 0, mixer2, 0);
AudioConnection          patchCord14(envelope6, 0, mixer2, 1);
AudioConnection          patchCord15(envelope7, 0, mixer2, 2);
AudioConnection          patchCord16(envelope8, 0, mixer2, 3);
AudioConnection          patchCord17(mixer1, 0, mixer3, 0);
AudioConnection          patchCord18(mixer2, 0, mixer3, 1);
AudioConnection          patchCord19(mixer3, 0, i2s1, 0);
AudioConnection          patchCord20(mixer3, 0, i2s1, 1);

// GUItool: end automatically generated code

AudioEffectEnvelope *envelopes[] = {&envelope1, &envelope2, &envelope3, &envelope4, &envelope5, &envelope6, &envelope7, &envelope8  };
AudioPlaySdResmp *voices[] = {&voice1, &voice2, &voice3, &voice4, &voice5, &voice6, &voice7, &voice8};

samplermodel<sdsampleplayernote> model;

MIDI_CREATE_RTMIDI_INSTANCE(RtMidiMIDI, rtMIDI,  MIDI);

#define DEBOUNCE    150

using namespace Bounce2;
Button button = Button();
Button button2 = Button();
Button button3 = Button();
Encoder encoderLeftRight;
Encoder encoderUpDown;
typedef st7735_opengl<Encoder, Button> st7735_ogl;
typedef SceneController< VirtualView, Encoder, Button> MySceneController;

audiovoicepolyphonic<AudioPlaySdResmp> _polyphony;

//newdigate::Sampler _sampler = newdigate::Sampler(model, _polyphony);
newdigate::MyLoopSampler _sampler = newdigate::MyLoopSampler(model, _polyphony);
st7735_ogl _display(true, 0, &encoderLeftRight, &encoderUpDown, &button, &button2, &button3);

VirtualView _virtualDisplay(_display, 0, 0, 128, 128);
MySceneController sceneController(_virtualDisplay, encoderLeftRight, encoderUpDown, button, button2, button3);

void DrawSettingsMenuItem0(View *v);

int _directionValue = 64;
newdigate::WavePreviewBuilder _wavePreviewBuilder(sd);

#define NUM_SETTINGS_MENU_ITEMS 2
const uint16_t Oxford_blue3 = 0x0109;
TeensyMenu settingsMenu = TeensyMenu( _virtualDisplay, 128, 128, 0, 0, Oxford_blue3, ST7735_BLACK );
TeensyMenuItem settingMenuItems[NUM_SETTINGS_MENU_ITEMS] = {
/*
            View &view, 
            std::function<void(View*)> updateWithView, 
            unsigned int height, 
            std::function<void(bool forward)> menuValueScroll = nullptr,
            std::function<bool(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity)> menuMidiNoteEvent = nullptr,
            std::function<bool(uint8_t channel, uint8_t data1, uint8_t data2)> menuMidiCCEvent = nullptr,
            std::function<void(uint8_t buttonNumber)> buttonDownEvent = nullptr
*/
  TeensyMenuItem(settingsMenu, 
    [] (View *v) {
      v->drawString("reset notes", 0, 0);
    }, 
    8, 
    [] (bool forward) { 
      if (forward) _directionValue++; else _directionValue--; 
    },
    [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
      return true;
    }, 
    nullptr,
    // button Down
    [] (uint8_t buttonNumber) {
      switch (buttonNumber) {
        case 2: _sampler.turnOffAllNotesStillPlaying(); break;
      }
    }),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("xxx", 0, 0);}, 8)
};

TFTPianoDisplay pianoDisplay1(settingMenuItems[0], 3, 2, 0, 9); //tft, byte octaves, byte startOctave, byte x, byte y

void DrawSettingsMenuItem0(View *v) {
  pianoDisplay1.drawFullPiano();
  settingMenuItems[0].drawString("trigger pad:  ", 0, 0);
}

newdigate::DeviceManager deviceManager = newdigate::DeviceManager();
newdigate::DirectoryFileNameCache directoryFileNameCache(sd);

newdigate::EditScene editScene(model, _virtualDisplay, directoryFileNameCache, sd, _sampler);

newdigate::DevicesScene * devicesScene = new newdigate::DevicesScene(_virtualDisplay, deviceManager, sceneController);

Scene settingsScene = Scene(
                        _bmp_settings_on, 
                        _bmp_settings_off, 
                        16, 16, 

                        // void update()
                        [] { 
                          settingsMenu.Update();
                          pianoDisplay1.drawPiano();
                        }, 

                        // void initScreen()
                        [] { 
                          _virtualDisplay.fillScreen(ST7735_BLUE); 
                          settingsMenu.NeedsUpdate = true; 
                          pianoDisplay1.displayNeedsUpdating();
                        },

                        // void uninitScreen() 
                        [] {
                          
                        },

                        // void buttonPressed(unsigned int index)
                        [] (unsigned index) {
                          settingsMenu.ButtonDown(index);
                        }, 

                        // void rotary1Changed(bool forward)
                        [] (bool forward) { 
                          if (forward) 
                            settingsMenu.IncreaseSelectedIndex(); 
                          else 
                            settingsMenu.DecreaseSelectedIndex(); 
                        }, 

                        // void rotary1Changed(bool forward)
                        [] (bool forward) {
                          settingsMenu.ValueScroll(forward);
                        },

                        // bool midiNoteEvent(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity)
                        [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
                          //Serial.println("Midi note down during settings scene...");
                          return settingsMenu.MidiNoteEvent(noteDown, channel, pitch, velocity);
                        });

Scene playScene = Scene(
                        _bmp_play_on, 
                        _bmp_play_off, 
                        16, 16,
                        [] { }, 
                        [] { _virtualDisplay.fillScreen(ST7735_GREEN); });                

void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2);

void setup() {

  Serial.begin(9600);

  while (!(sd.begin(10))) {
        // stop here if no SD card, but print a message
        Serial.println("Unable to access the SD card...");
        delay(500);
    }
  // button in the encoder
  //pinMode(SE_PIN, INPUT_PULLUP);

  // I use a digital pin to control LED brightness
  //pinMode(LED_PIN, OUTPUT);

  button.attach( 10, 10 ); // USE EXTERNAL PULL-UP
  button.interval(5); 
  button.setPressedState(LOW); 
  
  MIDI.setHandleNoteOn(handleNoteOn); 
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  delay(10);
  //Display.initR(INITR_GREENTAB);
  _display.setRotation(1);

  _virtualDisplay.fillScreen(ST7735_BLACK);

  sceneController.AddScene(&settingsScene);
  sceneController.AddScene(devicesScene);
  sceneController.AddScene(&editScene);
  sceneController.AddScene(&playScene);

  sceneController.SetCurrentSceneIndex(0);
  sceneController.SetActive(false);

  for (int i = 0; i < NUM_SETTINGS_MENU_ITEMS; i++) {
    settingsMenu.AddControl(&settingMenuItems[i]);
  }
  _polyphony.addVoice( &voice1, &envelope1, &mixer1, 0);
  _polyphony.addVoice( &voice2, &envelope2, &mixer1, 1);
  _polyphony.addVoice( &voice3, &envelope3, &mixer1, 2);
  _polyphony.addVoice( &voice4, &envelope4, &mixer1, 3);
  _polyphony.addVoice( &voice5, &envelope5, &mixer2, 0);
  _polyphony.addVoice( &voice6, &envelope6, &mixer2, 1);
  _polyphony.addVoice( &voice7, &envelope7, &mixer2, 2);
  _polyphony.addVoice( &voice8, &envelope8, &mixer2, 3); 

  mixer3.gain(0, 1.0);
  mixer3.gain(1, 1.0);

  for (int i=0; i<8; i++) {
    voices[i]->enableInterpolation(true);
    envelopes[i]->attack(0);
  }
  AudioMemory(20);
}

void loop() {
  sceneController.Process();
  MIDI.read();
  _sampler.updateProgress();
  _sampler.update();
}

int st7735_main(int numArgs, char **args) {
    /*
    if (numArgs < 2)
    {
        std::cout << "usage: " << args[0] << " <path-to-SDCard>" << std::endl;
        arduino_should_exit = true;
        exit(0);
    }
    std::cout << args[1] << std::endl;
    SD.setSDCardFolderPath(args[1]); */
    return 0;
}

void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
    Serial.printf("MIDI IN: handleNoteOn note:%d; channel:%d; \r\n", pitch, channel);
 
    bool processedMessage = sceneController.MidiNoteUpDown(true, channel, pitch, velocity);
    //if (processedMessage) return;
    _sampler.trigger(pitch, channel, velocity, true);/*

    sdsampleplayernote* sample = model.getNoteForChannelAndKey(channel, pitch);
    if (sample != nullptr) {
      switch (sample->_triggertype) {
          case triggertype_play_until_end :
          case triggertype_play_while_notedown : _sampler.trigger(pitch, channel, velocity, true); break;
          case triggertype_play_until_subsequent_notedown: {
              if (sample->_voice != nullptr) {
                  sample->_voice->stop();

                  sample->_voice = nullptr;
                  sample->isPlaying = false;
              } else {
                _sampler.trigger(pitch, channel, velocity, false);      
              }
          }
          break;

          default: break;
      }
    } */
}

void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity)
{

  if (sceneController.MidiNoteUpDown(false, channel, pitch, velocity)){
    return;
  }

  Serial.printf("MIDI IN: handleNoteOff note:%d; channel:%d; \r\n", pitch, channel);
  _sampler.trigger(pitch, channel, velocity, false);
}

void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) 
{
  sceneController.MidiControlChange(channel, data1, data2);
}