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
#include "SamplerModel.h"
#include "EditScene.h"
#include "DirectoryFileNameCache.h"
#include "WavePreviewBuilder.h"
#include <TeensyAudioLaunchCtrl.h>
#include <TeensyVariablePlayback.h>
#include "output_soundio.h"
#include "Sampler.h"

SDClass sd = SDClass("/Users/nicholasnewdigate/Development/sampler");

// GUItool: begin automatically generated code
AudioPlaySdResmp           playSdAudio3(sd);     //xy=334.0000457763672,459.0000305175781
AudioPlaySdResmp           playSdAudio2(sd);     //xy=338.0000305175781,382.0000305175781
AudioPlaySdResmp           playSdAudio4(sd);     //xy=339.0000457763672,536
AudioPlaySdResmp           playSdAudio1(sd);     //xy=340.0000419616699,302.0000238418579
AudioMixer4              mixerLeft;         //xy=650.0000419616699,404.0000238418579
AudioMixer4              mixerRight;         //xy=650.0000610351562,511.0000305175781
AudioOutputSoundIO       sio_out1;       //xy=958.0000610351562,466.0000305175781
AudioConnection          patchCord1(playSdAudio3, 0, mixerLeft, 2);
AudioConnection          patchCord2(playSdAudio3, 1, mixerRight, 2);
AudioConnection          patchCord3(playSdAudio2, 0, mixerLeft, 1);
AudioConnection          patchCord4(playSdAudio2, 1, mixerRight, 1);
AudioConnection          patchCord5(playSdAudio4, 0, mixerLeft, 3);
AudioConnection          patchCord6(playSdAudio4, 1, mixerRight, 3);
AudioConnection          patchCord7(playSdAudio1, 0, mixerLeft, 0);
AudioConnection          patchCord8(playSdAudio1, 1, mixerRight, 0);
AudioConnection          patchCord9(mixerLeft, 0, sio_out1, 0);
AudioConnection          patchCord10(mixerRight, 0, sio_out1, 1);
// GUItool: end automatically generated code


newdigate::SamplerModel model;

MIDI_CREATE_RTMIDI_INSTANCE(RtMidiMIDI, rtMIDI,  MIDI);

#define DEBOUNCE    150

using namespace Bounce2;
Button button = Button();
Encoder encoderLeftRight;
Encoder encoderUpDown;
typedef st7735_opengl<Encoder, Button> st7735_ogl;
typedef SceneController< st7735_opengl<Encoder, Button>, Encoder, Button> MySceneController;
polyphonic<AudioPlaySdResmp> _polyphony;

newdigate::Sampler _sampler = newdigate::Sampler(model, _polyphony);
st7735_ogl _display(true, 20, &encoderLeftRight, &encoderUpDown, &button);
MySceneController sceneController(_display, encoderLeftRight, encoderUpDown, button);

void DrawSettingsMenuItem0(View *v);

int _directionValue = 64;
newdigate::WavePreviewBuilder _wavePreviewBuilder(sd);

#define NUM_SETTINGS_MENU_ITEMS 20
TeensyMenu settingsMenu = TeensyMenu( _display, 0, 0, 128, 128, ST7735_BLUE, ST7735_BLACK );
TeensyMenuItem settingMenuItems[NUM_SETTINGS_MENU_ITEMS] = {
  TeensyMenuItem(settingsMenu, DrawSettingsMenuItem0, 20),
  TeensyMenuItem(settingsMenu, 
    [] (View *v) {
      v->drawString("direction  ", 0, 0);
      v->drawNumber(_directionValue, 100, 0);
    }, 
    8, 
    [] (bool forward) { 
      if (forward) _directionValue++; else _directionValue--; 
    },
    [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
      Serial.println("NoteDown...");
      return true;
    }),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("loop  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("pan  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("sample  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("tune  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("volume  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 8  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 9  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu10  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu11  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu12  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu13  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu14  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu15  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu16  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu17  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu18  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu19  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {
    v->drawPixel(0,0, ST7735_WHITE);
    v->drawString("Menu20  ", 0, 0);
  }, 8), 
};

TFTPianoDisplay pianoDisplay1(settingMenuItems[0], 3, 2, 0, 9); //tft, byte octaves, byte startOctave, byte x, byte y

void DrawSettingsMenuItem0(View *v) {
  pianoDisplay1.drawFullPiano();
  settingMenuItems[0].drawString("trigger pad:  ", 0, 0);
}

newdigate::DirectoryFileNameCache directoryFileNameCache(sd);

newdigate::EditScene editScene(model, _display, directoryFileNameCache, sd, _sampler);

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
                          _display.fillScreen(ST7735_BLUE); 
                          settingsMenu.NeedsUpdate = true; 
                          pianoDisplay1.displayNeedsUpdating();
                        },

                        // void buttonPressed()
                        [] {}, 

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
                        [] { _display.fillScreen(ST7735_GREEN); });                

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

  _display.fillScreen(ST7735_BLACK);

  sceneController.AddScene(&settingsScene);
  sceneController.AddScene(&editScene);
  sceneController.AddScene(&playScene);

  sceneController.SetCurrentSceneIndex(0);
  sceneController.SetActive(false);

  for (int i = 0; i < NUM_SETTINGS_MENU_ITEMS; i++) {
    settingsMenu.AddControl(&settingMenuItems[i]);
  }
  _polyphony.addVoice(playSdAudio1);
  _polyphony.addVoice(playSdAudio2);
  _polyphony.addVoice(playSdAudio3);
  _polyphony.addVoice(playSdAudio4);
  
  AudioMemory(20);
}

void loop() {
  sceneController.Process();
  MIDI.read();
  _sampler.updateProgress();
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
    bool processedMessage = sceneController.MidiNoteUpDown(true, channel, pitch, velocity);
    //if (processedMessage) return;

    _sampler.midiChannleVoiceMessage(0x90, pitch, velocity, channel);
}

void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
  sceneController.MidiNoteUpDown(false, channel, pitch, velocity);
  _sampler.midiChannleVoiceMessage(0x80, pitch, velocity, channel);
}

void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) 
{
  sceneController.MidiControlChange(channel, data1, data2);
}