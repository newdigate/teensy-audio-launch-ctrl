#include <Arduino.h>
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "st7735display.h"
#include "../ArduinoMenu/src/menu.h"
#include "../ArduinoMenu/src/menuIO/serialOut.h"
#include "../ArduinoMenu/src/menuIO/serialIn.h"
#include "../ArduinoMenu/src/menuIO/tftOut.h"
#include "../ArduinoMenu/src/menuIO/chainStream.h"

using namespace Menu;

#define LEDPIN LED_BUILTIN
#define MAX_DEPTH 1

#define TFT_Width 160
#define TFT_Height 128

//font size plus margins
#define fontX 6
#define fontY 9

st7735_opengl tft = st7735_opengl(true, 20);

unsigned int timeOn=10;
unsigned int timeOff=90;

MENU(mainMenu, "Blink menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(timeOn,"On","ms",0,1000,10,1, Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(timeOff,"Off","ms",0,10000,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,EXIT("<Back")
);


#define ST7735_GRAY RGB565(128,128,128)

const colorDef<uint16_t> colors[6] MEMMODE={
  {{ST7735_BLACK,ST7735_BLACK},{ST7735_BLACK,ST7735_BLUE,ST7735_BLUE}},//bgColor
  {{ST7735_GRAY,ST7735_GRAY},{ST7735_WHITE,ST7735_WHITE,ST7735_WHITE}},//fgColor
  {{ST7735_WHITE,ST7735_BLACK},{ST7735_YELLOW,ST7735_YELLOW,ST7735_RED}},//valColor
  {{ST7735_WHITE,ST7735_BLACK},{ST7735_WHITE,ST7735_YELLOW,ST7735_YELLOW}},//unitColor
  {{ST7735_WHITE,ST7735_GRAY},{ST7735_BLACK,ST7735_BLUE,ST7735_WHITE}},//cursorColor
  {{ST7735_WHITE,ST7735_YELLOW},{ST7735_BLUE,ST7735_RED,ST7735_RED}},//titleColor
};

//initializing output and menu nav without macros
const panel default_serial_panels[] MEMMODE={{0,0,40,10}};
navNode* default_serial_nodes[sizeof(default_serial_panels)/sizeof(panel)];
panelsList default_serial_panel_list(
  default_serial_panels,
  default_serial_nodes,
  sizeof(default_serial_panels)/sizeof(panel)
);

//define outputs controller
idx_t ucg_tops[MAX_DEPTH];
PANELS(ucgPanels,{0,0,TFT_Width/fontX,TFT_Height/fontY});
//

serialIn serial(Serial);
MENU_INPUTS(in,&serial);

const panel panels[] MEMMODE = {{0, 0, 128 / fontX, 128 / fontY}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
panelsList pList(panels, nodes, 1); //a list of panels and nodes
idx_t eSpiTops[MAX_DEPTH]={0};
TFTOut tftOut(tft,colors,ucg_tops,ucgPanels,fontX,fontY);
menuOut* constMEM outputs[] MEMMODE={&tftOut};//list of output devices
outputsList out(outputs,1);//outputs list controller


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void setup() {
  //pinMode(LEDPIN, OUTPUT);
  Serial.begin(115200);
  while(!Serial);
  tft.initR(INITR_144GREENTAB); 
  Serial.println("Menu 4.x");
  Serial.println("Use keys + - * /");
  Serial.println("to control the menu navigation");
}

bool blink(int timeOn,int timeOff) {
  return millis()%(unsigned long)(timeOn+timeOff)<(unsigned long)timeOn;
}

void loop() {
  nav.poll();
  //digitalWrite(LEDPIN, blink(timeOn,timeOff));
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