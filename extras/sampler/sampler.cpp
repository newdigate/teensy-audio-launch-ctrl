#include <Arduino.h>
#include <MIDI.h>
#include "RtMidiMIDI.h"
#include "RtMidiTransport.h"
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "ST7735_t3.h"         // high speed display that ships with Teensy
#include <Encoder.h>
#include <Bounce2.h>
#include "icons.h"
#include "scenecontroller.h"
#include "teensy_controls.h"
#include "TFTPianoDisplay.h"

MIDI_CREATE_RTMIDI_INSTANCE(RtMidiMIDI, rtMIDI,  MIDI);
#define DEBOUNCE    150

//Scene(const uint16_t * iconOn, const uint16_t * iconOff, unsigned int iconWidth, unsigned int iconHeight) 

// MOVED to icons.h:
// this next rediciously long section is merely the icon image data
// scroll down to 1560 or so for the actual code
// Website for generating icons
// https://javl.github.io/image2cpp/
// all icons created at that site and copy / pasted here
// '64_wireless', 64x64px

using namespace Bounce2;
Button button = Button();
Encoder encoderLeftRight;
Encoder encoderUpDown;

st7735_opengl<Encoder, Button> Display(true, 20, &encoderLeftRight, &encoderUpDown, &button);
SceneController< st7735_opengl<Encoder, Button>, Encoder, Button> sceneController(Display, encoderLeftRight, encoderUpDown, button);

void DrawSettingsMenuItem0(View *v);

#define NUM_SETTINGS_MENU_ITEMS 20
TeensyMenu settingsMenu = TeensyMenu( Display, 10, 10, 108, 108, ST7735_BLUE, ST7735_BLACK );
TeensyMenuItem settingMenuItems[NUM_SETTINGS_MENU_ITEMS] = {
  TeensyMenuItem(settingsMenu, DrawSettingsMenuItem0, 16),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 2  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 3  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 4  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 5  ", 0, 0);}, 8),
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 6  ", 0, 0);}, 8), 
  TeensyMenuItem(settingsMenu, [] (View *v) {v->drawString("Menu 7  ", 0, 0);}, 8),
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

TFTPianoDisplay pianoDisplay1(settingMenuItems[0], 3, 2, 0, 0); //tft, byte octaves, byte startOctave, byte x, byte y
void DrawSettingsMenuItem0(View *v) {
  pianoDisplay1.drawFullPiano();
}

Scene settingsScene = Scene(
                        _bmp_settings_on, 
                        _bmp_settings_off, 
                        16, 16, 
                        [] { settingsMenu.Update(); pianoDisplay1.drawPiano();}, 
                        [] { Display.fillScreen(ST7735_BLUE); settingsMenu.NeedsUpdate = true; pianoDisplay1.displayNeedsUpdating();},
                        [] {} , // std::function<void()> buttonPressed = nullptr, 
                        [] (bool forward) { if (forward) settingsMenu.IncreaseSelectedIndex(); else settingsMenu.DecreaseSelectedIndex(); }, //std::function<void(bool)> rotary1Changed = nullptr, 
                        [] (bool forward) { },  //std::function<void(bool)> rotary2Changed = nullptr
                        [] (bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) -> bool {
                          Serial.println("Midi note down during settings scene...");
                          return false;
                        }
                        );

Scene editScene = Scene(
                        _bmp_edit_on, 
                        _bmp_edit_off, 
                        16, 16,
                        [] { }, 
                        [] { Display.fillScreen(ST7735_RED); });

Scene playScene = Scene(
                        _bmp_play_on, 
                        _bmp_play_off, 
                        16, 16,
                        [] { }, 
                        [] { Display.fillScreen(ST7735_GREEN); });                

void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2);

void setup() {

  Serial.begin(9600);

  // while ((millis() > 5000) || (!Serial)) {}

  // button in the encoder
  //pinMode(SE_PIN, INPUT_PULLUP);

  // I use a digital pin to control LED brightness
  //pinMode(LED_PIN, OUTPUT);

  button.attach( 10, 10 ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  button.interval(5); 

  // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  button.setPressedState(LOW); 
  
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

  // Do the same for NoteOffs
  MIDI.setHandleNoteOff(handleNoteOff);

  MIDI.setHandleControlChange(handleControlChange);
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  delay(10);
  //Display.initR(INITR_GREENTAB);
  Display.setRotation(1);

  Display.fillScreen(ST7735_BLACK);

  sceneController.AddScene(&settingsScene);
  sceneController.AddScene(&editScene);
  sceneController.AddScene(&playScene);

  sceneController.SetCurrentSceneIndex(0);
  sceneController.SetActive(false);

  //settingsScene.SetUpdateFunction(updateSettingsScene);
  //editScene.SetUpdateFunction(updateEditScene);
  //playScene.SetUpdateFunction(updatePlayScene);

  for (int i = 0; i < NUM_SETTINGS_MENU_ITEMS; i++) {
    settingsMenu.AddControl(&settingMenuItems[i]);
  }
}

void loop() {
  sceneController.Process();
  MIDI.read();
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
    sceneController.MidiNoteUpDown(true, channel, pitch, velocity);
}

void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
  sceneController.MidiNoteUpDown(false, channel, pitch, velocity);
}

void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) 
{
  sceneController.MidiControlChange(channel, data1, data2);
}