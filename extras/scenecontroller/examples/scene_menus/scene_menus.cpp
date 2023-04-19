#include <Arduino.h>
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "ST7735_t3.h"         
#include <Encoder.h>
#include <Bounce2.h>
#include "icons.h"
#include "scenecontroller.h"
#include "TFTPianoDisplay.h"
#include <Audio.h>
#include <SD.h>
#include <MIDI.h>
#include "RtMidiMIDI.h"
#include "RtMidiTransport.h"
#include "output_soundio.h"

st7735_opengl_noinput tft = st7735_opengl_noinput(true, 20);
TFTPianoDisplay<st7735_opengl_noinput> pianoDisplay1(tft, 3, 2, 0, 0); //tft, byte octaves, byte startOctave, byte x, byte y

#define DEBOUNCE    150

//Scene(const uint16_t * iconOn, const uint16_t * iconOff, unsigned int iconWidth, unsigned int iconHeight) 
Scene settingsScene = Scene(_bmp_settings_on, _bmp_settings_off, 16, 16);
Scene editScene = Scene(_bmp_edit_on, _bmp_edit_off, 16, 16);
Scene playScene = Scene(_bmp_play_on, _bmp_play_off, 16, 16);

using namespace Bounce2;

Button button = Button();
Button button2 = Button();
Button button3 = Button();

void ProcessMainMenu();
void ProcessWirelessMenu();
void ProcessOptionMenu();
void ProcessColorMenu();

Encoder encoderLeftRight;
Encoder encoderUpDown;

st7735_opengl<Encoder, Button> Display(true, 20, &encoderLeftRight, &encoderUpDown, &button);
VirtualView _virtualDisplay(Display, 0, 0, 128, 128);
SceneController< VirtualView, Encoder, Button>  sceneController(_virtualDisplay, encoderLeftRight, encoderUpDown, button, button2, button3);

void updateSettingsScene() {
  Display.fillScreen(ST7735_BLUE);
  pianoDisplay1.drawFullPiano();
}

void updateEditScene() {
  Display.fillScreen(ST7735_RED);
}

void updatePlayScene() {
  Display.fillScreen(ST7735_GREEN);
}

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
  

  delay(10);
  //Display.initR(INITR_GREENTAB);
  Display.setRotation(1);

  Display.fillScreen(ST7735_BLACK);

  sceneController.AddScene(&settingsScene);
  sceneController.AddScene(&editScene);
  sceneController.AddScene(&playScene);

  sceneController.SetCurrentSceneIndex(0);
  sceneController.SetActive(false);

  settingsScene.SetUpdateFunction(updateSettingsScene);
  editScene.SetUpdateFunction(updateEditScene);
  playScene.SetUpdateFunction(updatePlayScene);
}

void loop() {
  sceneController.Process();
}

int st7735_main(int numArgs, char **args) {
  return 0;
}