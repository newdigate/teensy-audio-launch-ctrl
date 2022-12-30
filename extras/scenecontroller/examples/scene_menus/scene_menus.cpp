#include <Arduino.h>
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "st7735display.h"
// required libraries
#include "ST7735_t3.h"         // high speed display that ships with Teensy
// required only if you are using an encoder to handle user input, mechanical buttons work as well
#include <Encoder.h>
#include <Bounce2.h>
#include "icons.h"
#include "scenecontroller.h"

#define DEBOUNCE    150

//Scene(const uint16_t * iconOn, const uint16_t * iconOff, unsigned int iconWidth, unsigned int iconHeight) 
Scene settingsScene = Scene(_bmp_settings_on, _bmp_settings_off, 16, 16);
Scene editScene = Scene(_bmp_edit_on, _bmp_edit_off, 16, 16);
Scene playScene = Scene(_bmp_play_on, _bmp_play_off, 16, 16);


// encoder stuff
long Position = 0, oldPosition = 0;
long PositionY = 0, oldPositionY = 0;




Bounce2::Button button = Bounce2::Button();


void ProcessMainMenu();
void ProcessWirelessMenu();
void ProcessOptionMenu();
void ProcessColorMenu();

Encoder encoderLeftRight;
Encoder encoderUpDown;

st7735_opengl<Encoder, Bounce2::Button> Display(true, 20, &encoderLeftRight, &encoderUpDown, &button);

SceneController< st7735_opengl<Encoder, Bounce2::Button>, Encoder > sceneController(Display, encoderLeftRight, encoderUpDown);

void updateSettingsScene() {
  Display.fillScreen(ST7735_BLUE);
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
  if (!sceneController.Active()) {
    bool enter = false;
    // attempt to debouce these darn things...
    while (Serial.available()) {
      enter = true;
      int r = Serial.read();
    }

    if (enter) {
      sceneController.SetActive(true);
      //Display.fillScreen(MENU_BACKGROUND);
    } else {
      sceneController.Update();
    }
  } else {   
    bool exit = false;
    // attempt to debouce these darn things...
    while (Serial.available()) {
      exit = true;
      int r = Serial.read();
    }
    if (exit) {
      sceneController.SetActive(false);      
      //Display.fillScreen(MENU_BACKGROUND);
    } 
  }
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