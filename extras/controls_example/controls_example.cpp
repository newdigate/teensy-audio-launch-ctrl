#include <Arduino.h>
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "st7735display.h"
#include <ILI9341_t3_Controls.h>
#include <st7735_t3_font_Arial.h>
#define FONT_TITLE Arial_16
#define FONT_DATA Arial_10

// defines for pin connection
#define TFT_CS        10
#define TFT_DC        9
#define LED_PIN       A9

// defines for locations
#define BXCENTER 32
#define BYCENTER 32
#define BDIAMETER 32
#define BLOWSCALE 0
#define BHIGHSCALE 1200
#define BSCALEINC 200
#define BSWEEPANGLE 300

#define VXCENTER 96
#define VYCENTER 32
#define VDIAMETER 32
#define VLOWSCALE 0
#define VHIGHSCALE 2
#define VSCALEINC .5
#define VSWEEPANGLE 300

// defines for colors
#define BNEEDLECOLOR C_ORANGE
#define BDIALCOLOR C_DKBLUE
#define BTEXTCOLOR C_WHITE
#define BTICCOLOR C_GREY

#define VNEEDLECOLOR C_YELLOW
#define VDIALCOLOR C_DKBLUE
#define VTEXTCOLOR C_WHITE
#define VTICCOLOR C_GREY

// create the display object
//ILI9341_t3 Display(TFT_CS, TFT_DC);
st7735_opengl Display(true, 20);

int bBits;
float bVolts;
float bData;

// create the dial object(s)
Dial Bits(&Display, BXCENTER, BYCENTER, BDIAMETER, BLOWSCALE , BHIGHSCALE, BSCALEINC, BSWEEPANGLE);
Dial Volts(&Display, VXCENTER, VYCENTER, VDIAMETER, VLOWSCALE , VHIGHSCALE, VSCALEINC, VSWEEPANGLE);

void setup() {

  Serial.begin(9600);

  // you know the drill
  //pinMode(LED_PIN, OUTPUT);
  Display.initR(INITR_GREENTAB);


  Display.setRotation(1);
  Display.fillScreen(C_BLACK);
  //digitalWrite(LED_PIN, 255);

  // initialize the dials
  Bits.init(BNEEDLECOLOR, BDIALCOLOR, BTEXTCOLOR, BTICCOLOR, "Bits", FONT_TITLE, FONT_DATA);
  Volts.init(VNEEDLECOLOR, VDIALCOLOR, VTEXTCOLOR, VTICCOLOR, "Volts", FONT_TITLE, FONT_DATA);

}

void loop() {
  // get some data
  bBits = (bBits + 2) % 1024;
  bVolts = bBits * 3.3 / 4096;

  // update the dials
  Bits.draw(bBits);
  Volts.draw(bVolts);

  delay(15);
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
}