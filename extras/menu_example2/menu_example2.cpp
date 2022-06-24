#include <Arduino.h>
#include <ST7735_t3.h>
#include <st7735_opengl.h>
#include <st7735_opengl_main.h>
#include "st7735display.h"
// required libraries
#include "ILI9341_t3_Menu.h"    // custom utilities definition
#include "ST7735_t3.h"         // high speed display that ships with Teensy
#include <st7735_t3_font_Arial.h>
// required only if you are using an encoder to handle user input, mechanical buttons work as well
#include <Encoder.h>
#include <Bounce2.h>
#include "icons.h"

// pins for LED and select button on encoder
#define SE_PIN A2

#define ROW_HEIGHT          16
#define ROWS                7
#define DATA_COLUMN         80

// pins for LED and select button on encoder
#define LED_PIN A9
#define SE_PIN    A2
#define CS_PIN    10
#define DC_PIN    9
#define EN1_PIN   A0
#define EN2_PIN   A1

// easy way to include fonts but change globally
#define FONT_SMALL  Arial_10             // font for menus
#define FONT_ITEM   Arial_11             // font for menus
#define FONT_TITLE  Arial_12        // font for all headings

#define DEBOUNCE    150

int MenuOption = 0;
int AllowColorMenu = 0;

// must have variables for each menu item
// best to have these global so you can use them in processing functions
int MenuOption1 = 0, MenuOption2 = 0, MenuOption3 = 0, MenuOption4 = 0, MenuOption5 = 0;
int MenuOption6 = 0, MenuOption7 = 0, MenuOption8 = 0, MenuOption9 = 0;

int ColorOption1 = 0, ColorOption2 = 0, ColorOption3 = 0, ColorOption4 = 0, ColorOption5 = 0;
int ColorOption6 = 0, ColorOption7 = 0, ColorOption8 = 0, ColorOption9 = 0, ColorOption10 = 0, ColorOption11 = 0;

int OptionOption1 = 0, OptionOption2 = 0, OptionOption3 = 0, OptionOption4 = 0, OptionOption5 = 0;
int OptionOption6 = 0, OptionOption7 = 0, OptionOption8 = 0, OptionOption9 = 0;

int WirelessOption1 = 0, WirelessOption2 = 0, WirelessOption3 = 0, WirelessOption4 = 0, WirelessOption5 = 0;
int WirelessOption6 = 0, WirelessOption7 = 0, WirelessOption8 = 0, WirelessOption9 = 0;
int WirelessOption10 = 0, WirelessOption11 = 0, WirelessOption12 = 0, WirelessOption13 = 0;

// variables to store some of the options, you will probably want a var for each
// note the menu code ONLY supports floats
// if you want to edit an int in the menu, still pass in a float and set decimal readout to 0
// the recast the float to an int upon menu comletion processing
float Temp1Adj = 0.2, Temp2Adj = -.3, AlarmVal = 1;

// encoder stuff
long Position = 0, oldPosition = 0;
long PositionY = 0, oldPositionY = 0;


// create some selectable menu sub-items, these are lists inside a menu item
const char *ReadoutItems[] =   {"Absolute", "Deg F", "Deg C"
                               };
const char *RefreshItems[] =   {"Off", "1 second", "2 seconds", "10 seconds",
                                "30 seconds", "1 minute", "5 minutes"
                               };
const char *PrecisionItems[] =   {"10", "0", "0.0", "0.00", "0.000"
                                 };
const char *TuneItems[] =   {"Slope/Offset", "Simple", "Linear", "2nd order ", "3rd order", "Log"
                            };
const char *OffOnItems[] =   {"Off", "On"
                             };

const char *DataRateItems[] =   {"300 baud", "1.2 kbd", "2.4 kbd", "4.8 kbd", "9.6 kbd", "19.2 kbd", "56 kbd"
                                };

// OK i'm going crazy with examples, but this will help show more processing when an int is needed but a float returned
// from the menu code
const char *C_NAMES[] = {"White", "Black", "Grey", "Blue", "Red", "Green", "Cyan", "Magenta",
                         "Yellow", "Teal", "Orange", "Pink", "Purple", "Lt Grey", "Lt Blue", "Lt Red",
                         "Lt Green", "Lt Cyan", "Lt Magenta", "Lt Yellow", "Lt Teal", "Lt Orange", "Lt Pink", "Lt Purple",
                         "Medium Grey", "Medium Blue", "Medium Red", "Medium Green", "Medium Cyan", "Medium Magenta", "Medium Yellow", "Medium Teal",
                         "Medium Orange", "Medium Pink", "Medium Purple", "Dk Grey", "Dk Blue", "Dk Red", "Dk Green", "Dk Cyan",
                         "Dk Magenta", "Dk Yellow", "Dk Teal", "Dk Orange", "Dk Pink", "Dk Purple"
                        };

const uint16_t  C_VALUES[] = {  0XFFFF, 0X0000, 0XC618, 0X001F, 0XF800, 0X07E0, 0X07FF, 0XF81F, //7
                                0XFFE0, 0X0438, 0XFD20, 0XF81F, 0X801F, 0XE71C, 0X73DF, 0XFBAE, //15
                                0X7FEE, 0X77BF, 0XFBB7, 0XF7EE, 0X77FE, 0XFDEE, 0XFBBA, 0XD3BF, //23
                                0X7BCF, 0X1016, 0XB000, 0X0584, 0X04B6, 0XB010, 0XAD80, 0X0594, //31
                                0XB340, 0XB00E, 0X8816, 0X4A49, 0X0812, 0X9000, 0X04A3, 0X0372, //39
                                0X900B, 0X94A0, 0X0452, 0X92E0, 0X9009, 0X8012 //45
                             };
// set default colors
uint16_t MENU_TEXT = C_VALUES[1];
uint16_t MENU_BACKGROUND = C_VALUES[0];
uint16_t MENU_HIGHLIGHTTEXT = C_VALUES[1];
uint16_t MENU_HIGHLIGHT = C_VALUES[21];
uint16_t MENU_HIGHBORDER = C_VALUES[10];
uint16_t MENU_SELECTTEXT = C_VALUES[0];
uint16_t MENU_SELECT = C_VALUES[4];
uint16_t MENU_SELECTBORDER = C_VALUES[37];
uint16_t MENU_DISABLE = C_VALUES[2];
uint16_t TITLE_TEXT = C_VALUES[13];
uint16_t TITLE_BACK = C_VALUES[36];

// this next rediciously long section is merely the icon image data
// scroll down to 1560 or so for the actual code

// Website for generating icons
// https://javl.github.io/image2cpp/

// all icons created at that site and copy / pasted here
// '64_wireless', 64x64px
Bounce2::Button button = Bounce2::Button();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// real code starts here
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessMainMenu();
void ProcessWirelessMenu();
void ProcessOptionMenu();
void ProcessColorMenu();

Encoder myEnc;
Encoder myEnc2;

// you know the drill
st7735_opengl Display(true, 20, &myEnc, &myEnc2, &button);

// required, you must create either an Item menu (no inline editing) or an EditMenu (allows inline editing)
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
ItemMenu MainMenu(&Display);

// since we're showing both menu types, create an object for each where the item menu is the main and calls edit menus
// you can have an item menu call other item menus an edit menu can call an edit menu but in a round about way--not recommended
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
EditMenu OptionMenu(&Display); // default is false, need not specify
EditMenu ColorMenu(&Display, false); // or you can still call false to force mechanical input selection
EditMenu WirelessMenu(&Display);

// create encoder object


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
  //digitalWrite(LED_PIN, HIGH);

  // fire up the display
  Display.initR(INITR_GREENTAB);
  Display.setRotation(1);

  // initialize the MainMenu object
  // note ROW_HEIGHT is row height and needs to be larger that font height
  // the ROWS is max rows to be displayed (remember, library handles wraparound
  /*
    init(TextColor, BackgroundColor, HighLtTextColor, HighLtColor,
    ItemRowHeight, MaxRowsPerScreen, TitleText, ItemFont, TitleFont);
  */

  MainMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, 16, ROWS, "Main Menu", FONT_TITLE, FONT_TITLE);

  // now add each item, text is what's displayed for each menu item, there are no other arguements
  // there are 3 add types
  // 1. addNI for no icon
  // 2. addMono for simple 1 color  icon each item
  // 3. add565 for displaying a color icon for each item

  // note the return value for each item will be it's returned itemid an dwill be > 0
  // a return of 0 is reserved for the exit item
  // the exit item is actually the title bar--if user moves selector to the title bar
  // it's temporarily renamed to "Exit"

  /*
    addNI(ItemLabel);
    addMono(ItemLabel, MonoBitmap, BitmapWidth, BitmapHeight );
    add565(ItemLabel, ColorBitmap, uint8_t BitmapWidth, uint8_t BitmapHeight);
  */
  MenuOption1 = MainMenu.add565("Options",  _bmp_allArray[0], 16, 16);
  MenuOption2 = MainMenu.add565("Colors",  _bmp_allArray[1], 16, 16);
  MenuOption3 = MainMenu.add565("Wireless",  _bmp_allArray[2], 16, 16);
  MenuOption4 = MainMenu.add565("Servos",  _bmp_allArray[3], 16, 16);
  MenuOption5 = MainMenu.add565("Motors",  _bmp_allArray[4], 16, 16);
  MenuOption6 = MainMenu.add565("Sensors",  _bmp_allArray[5], 16, 16);



  // the remaing method calls for this menu are optional and shown as an example on what some of the things you can do
  // however, you will most likely need to set xxxMarginxxx stuff as the library does not attempt to get text bounds
  // and center, you will have to put pixel values in to control where text is display in menu items, title bars, etc.
  
  if (AllowColorMenu == 0) {
    MainMenu.disable(MenuOption2);
  }
  else {
    MainMenu.enable(MenuOption2);
  }

  // getEnableState(MenuItemID);
  // wanna know what the stat is?
  Serial.print("Enable State for MainMenu, MenuOption3: ");
  Serial.println(MainMenu.getEnableState(MenuOption3));

  // optional, but you can change title bar colors
  // setTitleColors(TitleTextColor, TitleFillColor);
  MainMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);

  // these are all optional, but you can change title bar colors, size, and margins.
  // setTitleBarSize(TitleTop, TitleLeft, TitleWith, TitleHeight);
  MainMenu.setTitleBarSize(0, 0, 128, 16);

  // optional but lets center the title text about the height but scoot it in a bit from the left
  // setTitleTextMargins(LeftMargin, TopMargin);
  MainMenu.setTitleTextMargins(16, 2);

  // optional but you can set the left margin and top to scoot the icon over and down
  // only needed if you are using icons in your menu items
  // setIconMargins(LeftMargin, TopMargin);
  MainMenu.setIconMargins(0, 0);

  // optional but you can set the left margin and top to scoot the menu highLt over and down
  // not needed but recommended so you can better control where the text is placed
  // the library will not determine font height and such and adjust--that will be on you
  // setMenuBarMargins(LeftMargin, Width, BorderRadius, BorderThickness);
  //MainMenu.setMenuBarMargins(0, 310, 10, 4);

  // optional but you can set the menu highLt special colors (disable text color and border color)
  // not needed if you dont plan on disabling menu items
  // the library will not determine font height and such and adjust--that will be on you
  // setItemColors(DisableTextColor, BorderColor);
  MainMenu.setItemColors(MENU_DISABLE, MENU_HIGHBORDER);

  // setItemTextMargins(LeftMargin, TopMargin, MarginBetweenTitleBarAndMenu);
  MainMenu.setItemTextMargins(0, 0, 0);

  // end of ItemMenu setup




  // this example includes both menu types, the first (above was a menu where items have not editing)
  // this menu type EditMenu allows changes for each items value--basically in-line editing
  // more capability? more arguements...

  /*
     init(TextColor, BackgroundColor, HighLtTextColor, HighLtColor, SelectedTextColor, SelectedColor,
      MenuColumn, ItemRowHeight,MaxRowsPerScreen, TitleText, ItemFont, TitleFont);
  */
  OptionMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                  DATA_COLUMN, ROW_HEIGHT, ROWS, "Option Menu", FONT_ITEM, FONT_TITLE);

  // now add each item, text is what's displayed for each menu item, there are no other arguements
  // there are 3 add types
  // 1. addNI for no icon
  // 2. addMono for simple 1 color  icon each item
  // 3. add565 for displaying a color icon for each item

  // note the return value for each item will be it's returned itemid an dwill be > 0
  // a return of 0 is reserved for the exit item
  // the exit item is actually the title bar--if user moves selector to the title bar
  // it's temporarily renamed to "Exit" while the selector is on the menu bar

  // the edit can be either by cycling through a range of values (low to high by some increment value
  // example, set a voltage divider calibration value 4000 to 10000 in increments of 100
  // or cycle through an array list
  // example, choose your favorite pet from a list ("cat", "dog", "bird", "fish")
  // you still enter a lower and high limit and are the array bounds (0 to 3 in the above example)
  // your Data is the initial array value so you can still have say "bird" be the initial value)
  // in either case you can have icons none, mono or color
  // the example below for OptionOption1, notice the menu value is a variable (AllowColorMenu)
  // this allows you to make sure the menu matches actual current settings. Idea: store settings in the eeprom
  // read at startup, populate the menu items, upon menu completion, store the value back in the eeprom

  /*
    addNI(ItemText, InitalDisplayData, LowLimit, HighLimit, IncrementValue, DisplayDecimalPlaces, ItemMenuText);

    addMono(ItemText, Data, LowLimit, HighLimit, Increment, DecimalPlaces, ItemMenuText,
    Bitmap, BitmapWidth, BitmapHeight);

    add565(ItemText, Data, LowLimit, HighLimit, Increment, DecimalPlaces, ItemMenuText,
    Bitmap, BitmapWidth, BitmapHeight);
  */
  OptionOption1 = OptionMenu.add565("Color", AllowColorMenu, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems, _bmp_allArray[0], 16, 16);
  OptionOption2 = OptionMenu.add565("Adj", Temp2Adj, -1.0, 1.0 , .05, 2, NULL,  _bmp_allArray[1], 16, 16);
  OptionOption3 = OptionMenu.add565("Read", 2, 0, sizeof(ReadoutItems) / sizeof(ReadoutItems[0]), 1, 0, ReadoutItems,  _bmp_allArray[2], 16, 16);
  OptionOption4 = OptionMenu.add565("Tune", 0, 0, 20 , 1, 0, NULL,  _bmp_allArray[3], 16, 16);
  OptionOption5 = OptionMenu.add565("Alarm", AlarmVal, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems,  _bmp_allArray[4], 16, 16);
  OptionOption6 = OptionMenu.add565("Prec", 0, 0, sizeof(PrecisionItems) / sizeof(PrecisionItems[0]), 1, 0, PrecisionItems,  _bmp_allArray[5], 16, 16);
  OptionOption7 = OptionMenu.add565("Rfresh", 0, 0.0, sizeof(RefreshItems) / sizeof(RefreshItems[0]), 1, 0, RefreshItems,  _bmp_allArray[0], 16, 16);

  // again all these calls are optional, but you most likely will want to set margins

  // optional but you can store a setting such as a calibration value in EEPROM, read at startup
  // and populate with setItemValue(), even though the item data was set in the add method, you can change it later
  // setItemValue(ItemID, ItemValue){
  OptionMenu.setItemValue(OptionOption1, 0.12);
  OptionMenu.setItemValue(OptionOption3, 1); // the 2nd element in the ReadoutItems array

  // optional but can can set the title colors
  // setTitleColors(TitleTextColor, TitleFillColor);
  OptionMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);

  // optional but you can set the size of the title bar
  // setTitleBarSize(TitleTop, TitleLeft, TitleWith, TitleHeight);
  OptionMenu.setTitleBarSize(0, 0, 128, 16);

  // optional but you can set the margins in how the text in the title bar is centered
  // setTitleTextMargins(LeftMargin, TopMargin);
  OptionMenu.setTitleTextMargins(0, 0);

  // optional but you can scoot the icon over and down
  // setIconMargins(LeftMargin, TopMargin);
  OptionMenu.setIconMargins(0, 0);

  // optional but you can set the margins and size of the text in the menu bar
  // setItemTextMargins(LeftMargin, TopMargin, MarginBetweenTitleBarAndMenu);
  OptionMenu.setItemTextMargins(0, 0, 0);

  // optional but you can change colors other that in the init method
  // colors such as disable text and border color (if you display a border of course)
  // setItemColors(DisableTextColor, HighLt, EditSelectBorderColor);
  OptionMenu.setItemColors(MENU_DISABLE, MENU_HIGHBORDER, MENU_SELECTBORDER);

  // i'm now just going to rip through creating the color menu, again take note or arg 2--it's the initial menu item value
  // since colors are in arrays we can't simply put MENU_TEXT for the first add method--MENU_TEXT is the corresponding hex value
  // gotta put it's index, the library knows you are using array and will take the passed index and show the correct text
  // remember...
  // C_NAMES[1] = "White"
  // C_VALUES[1] = {  0XFFFF,
  // MENU_TEXT = C_VALUES[1]; // white

  ColorMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                 DATA_COLUMN, ROW_HEIGHT, ROWS, "Color Menu", FONT_ITEM, FONT_TITLE);

  ColorOption1 = ColorMenu.addNI("Menu Text", 1, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption2 = ColorMenu.addNI("Background", 0, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption3 = ColorMenu.addNI("Selector Text", 1, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption4 = ColorMenu.addNI("Selector Bar", 21, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption5 = ColorMenu.addNI("Selector Border", 10, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption6 = ColorMenu.addNI("Edit Text", 0, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption7 = ColorMenu.addNI("Edit Bar", 4, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption8 = ColorMenu.addNI("Edit Border", 37, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption9 = ColorMenu.addNI("Disable Text", 2, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]) , 1, 0, C_NAMES);
  ColorOption10 = ColorMenu.addNI("Banner Text", 13, 0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);
  ColorOption11 = ColorMenu.addNI("Banner Fill", 36, 0.0, sizeof(C_NAMES) / sizeof(C_NAMES[0]), 1, 0, C_NAMES);

  ColorMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  ColorMenu.setTitleBarSize(0, 0, 128, 16);
  ColorMenu.setTitleTextMargins(0, 0);
  ColorMenu.setIconMargins(0, 0);
  ColorMenu.setItemTextMargins(0, 0, 0);
  ColorMenu.setItemColors(MENU_DISABLE, MENU_HIGHBORDER, MENU_SELECTBORDER);

  // optional but showing how you can have several small items and not large pretty icons
  WirelessMenu.init(MENU_TEXT, MENU_BACKGROUND, ST7735_WHITE, ST7735_BLUE, MENU_SELECTTEXT, MENU_SELECT,
                    DATA_COLUMN, ROW_HEIGHT, ROWS, "Wireless Menu", FONT_SMALL, FONT_ITEM);      
  WirelessOption1 = WirelessMenu.addNI("Channel", 0, 0, 63 , 1, 0);
  WirelessOption2 = WirelessMenu.addNI("Address", 0, 0, 255, 1, 0);
  WirelessOption3 = WirelessMenu.addNI("Data rate", 2, 0, sizeof(DataRateItems) / sizeof(DataRateItems[0]), 1, 0, DataRateItems);
  WirelessOption4 = WirelessMenu.addNI("Air rate", 2, 0, sizeof(DataRateItems) / sizeof(DataRateItems[0]), 1, 0, DataRateItems);
  WirelessOption5 = WirelessMenu.addNI("Error checking", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessOption6 = WirelessMenu.addNI("Power", 0, 0, 100, 10, 0);
  WirelessOption7 = WirelessMenu.addNI("FEC", 0, 0, 100, 10, 0);
  WirelessOption8 = WirelessMenu.addNI("Callback", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessOption9 = WirelessMenu.addNI("Use buffer", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessOption10 = WirelessMenu.addNI("Open drain", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessOption11 = WirelessMenu.addNI("Use pullups", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessOption12 = WirelessMenu.addNI("Antenna height", 0, 0, 8, 1, 0);
  WirelessOption12 = WirelessMenu.addNI("Broadband", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  WirelessMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  WirelessMenu.setTitleBarSize(0, 0, 128, 16);
  WirelessMenu.setTitleTextMargins(0, 0);
  WirelessMenu.setIconMargins(0, 0);
  WirelessMenu.setItemTextMargins(0, 0, 0);
  WirelessMenu.setItemColors(MENU_DISABLE, MENU_HIGHBORDER, MENU_SELECTBORDER);

  // you can simplay call the draw method on a menu object, but you will need to add processing
  // here's how I recommend doing that, have a function that draws the main menu and processes the selections
  // ideally you will probably have a "setting" button in your UI that will call the "ProcessMainMenu"

  //ProcessMainMenu();

  // menu code done, now proceed to your code
  Display.fillScreen(MENU_BACKGROUND);

}

void loop() {

  button.update();
  // standard encoder read
  PositionY = myEnc2.read();

  //delay(50);
      bool enter = false;

  // attempt to debouce these darn things...
  if ((PositionY - oldPositionY) > 0) {
    delay(DEBOUNCE);
    while (oldPositionY != PositionY) {
      oldPositionY = PositionY;
      PositionY = myEnc2.read();
    }

    // once encoder calms down and is done cycling, move selector up
    // since encoder reads are increasing
    // any menu wrapping is handled in the library
    enter = true;
  }

  // and example on how you can call menu while in a loop
  // of course you will probably have a button to launch ProcessMainMenu
  if (enter) {
    ProcessMainMenu();
    Display.fillScreen(MENU_BACKGROUND);
  }
}

// function to process main menu iteraction
// ideally this implementation makes it easy to launch your menu from anywhere in th
// function to process main menu iteraction
// ideally this implementation makes it easy to launch your menu from anywhere in th
void ProcessMainMenu() {

  // set an inital flag that will be used to store what menu item the user exited on
  int MainMenuOption = 1;

  // blank out the screen
  Display.fillScreen(MENU_BACKGROUND);

  // draw the main menu
  MainMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit, i.e. 0 return val)
  // and selectes it
  // note menu code can return - 1 for errors so run unitl non zero

  while (MainMenuOption != 0) {
    button.update();
    // standard encoder read
    Position = myEnc.read();
    PositionY = myEnc2.read();

    // attempt to debouce these darn things...
    if ((Position - oldPosition) < 0) {
      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      MainMenu.MoveUp();
    }
    // attempt to debouce these darn things...
    if ((Position - oldPosition) > 0) {
      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      MainMenu.MoveDown();
    }

    bool enter = false;

    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) > 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      enter = true;
    }
    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) < 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      MainMenuOption = 0;
    }

    // but wait...the user pressed the button on the encoder
    if (enter) {
      Serial.println("button pressed...");
      // get the row the selector is on
      MainMenuOption = MainMenu.selectRow();

      // here is where you process accordingly
      // remember on pressing button on title bar 0 is returned and will exit the loop

      if (MainMenuOption == MenuOption1) {
        // item 1 was the Option menu
        ProcessOptionMenu();
        // when done processing that menu, return here
        // clear display and redraw this main menu
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }
      if (MainMenuOption == MenuOption2) {
        ProcessColorMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }
      if (MainMenuOption == MenuOption3) {
        ProcessWirelessMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption4) {
        //ImTooLazyToWriteAnotherExampe();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

    }
  }

  // at this point MenuOption better be 0...

}

// menu to handle processing for a sub-menu
// since this menu will be a menu that allows edits (EditMenu object type)
// process is exactly the same as an ItemMenu
// meaning you simply use the same MoveUp, MoveDown and the library will know if you are
// wanting to move the selector or cycle through a range

void ProcessOptionMenu() {

  // the entire menu processing are basically 3 calls
  // YourMenu.MoveUp();
  // YourMenu.MoveDown();
  // EditMenuOption = YourMenu.selectRow();

  // set an inital flag that will be used to store what menu item the user exited on
  int EditMenuOption = 1;

  // blank out the screen
  Display.fillScreen(MENU_BACKGROUND);

  // draw the main menu
  OptionMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit)
  // and selectes it
  while (EditMenuOption != 0) {
    button.update();
    // standard encoder read
    Position = myEnc.read();
    PositionY = myEnc2.read();

    // attempt to debouce these darn things...
    if ((Position - oldPosition) < 0) {

      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library

      // the EditMenu object type is special cased
      // 1. if a row was NOT selected, MoveUp / MoveDown will cycle throuh menu items
      // 2. if a row IS selected, MoveUp / MoveDown will cycle throuh the range of values withing the item

      OptionMenu.MoveUp();
    }

    if ((Position - oldPosition) > 0) {
      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library

      // the EditMenu object type is special cased
      // 1. if a row was NOT selected, MoveUp / MoveDown will cycle throuh menu items
      // 2. if a row IS selected, MoveUp / MoveDown will cycle throuh the range of values withing the item
      OptionMenu.MoveDown();
    }

    bool enter = false;

    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) > 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      enter = true;
    }
    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) < 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      EditMenuOption = 0; // exit
    }

    if (enter) {

      // use the selectRow to
      // 1. select a row for editing
      // a. when a row is selected, moveup, movedown will then scroll through the editable values (values or a list)
      // 2. unselect a row when editing is done
      // 3. when selector is on the title bar annd selecRow is called a 0 is returned

      EditMenuOption = OptionMenu.selectRow();


      // this next section is purely optional and shows how data change can be used
      // the EditMenu object type is special cased where if you select a menu item
      // the library consideres that a desire to start editing the item
      // the menu bar color changes and any MoveUp / MoveDown will then be directed
      // to editing the item value itself
      // once user Selects the selected row, the library will consider editign done
      // and restore menu selection with MoveUp / MoveDown
      // watch the YouTube video for a demo
      // to modify other values
      if (EditMenuOption == OptionOption5) { // budget item
        if (OptionMenu.value[OptionOption5] == 0) {
          OptionMenu.disable(OptionOption4);
          OptionMenu.setItemText(OptionOption4, "Off");
          OptionMenu.drawRow(OptionOption4);
        }
        else {
          OptionMenu.enable(OptionOption4);
          OptionMenu.setItemText(OptionOption4, "Tune");
          OptionMenu.drawRow(OptionOption4);
        }
      }
    }
  }

  // user must have pressed the encorder select button while on the title bar (which returns 0)
  // hence exiting the loop
  // now you can process / store / display the menu selections
  // remember this is from the EditMenu and has associated value property with each menu item
  Serial.println("Option Menu Selections ");
  Serial.println("______________________________");
  Serial.print("Color Adj "); Serial.println(OptionMenu.value[OptionOption1]);
  Serial.print("Temp2 Adj "); Serial.println(OptionMenu.value[OptionOption2]);
  Serial.print("Readout "); Serial.println(ReadoutItems[(int)OptionMenu.value[OptionOption3]]);
  Serial.print("Tune "); Serial.println(TuneItems[(int)OptionMenu.value[OptionOption4]]);
  Serial.print("Alarm "); Serial.println(OffOnItems[(int)OptionMenu.value[OptionOption5]]);
  Serial.print("Precision "); Serial.println(PrecisionItems[(int)OptionMenu.value[OptionOption6]]);
  Serial.print("Refresh "); Serial.println(OptionMenu.value[OptionOption7]);

  // an example of how to set other menu values based on selection here
  // remember all menu values passed in or returned are floats
  // so recast to int if you need to
  AllowColorMenu = (int) OptionMenu.value[OptionOption1];

  if (AllowColorMenu == 0) {
    MainMenu.disable(MenuOption2);
  }
  else {
    MainMenu.enable(MenuOption2);
  }

}

void ProcessColorMenu() {

  // the entire menu processing are basically 3 calls
  // YourMenu.MoveUp();
  // YourMenu.MoveDown();
  // EditMenuOption = YourMenu.selectRow();


  // set an inital flag that will be used to store what menu item the user exited on
  int EditMenuOption = 1;

  // blank out the screen
  Display.fillScreen(MENU_BACKGROUND);

  // draw the main menu
  ColorMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit)
  // and selectes it
  while (EditMenuOption != 0) {
    button.update();
    delay(50);
    Position = myEnc.read();
    PositionY = myEnc2.read();

    if ((Position - oldPosition) < 0) {
      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }
      ColorMenu.MoveUp();
    }

    if ((Position - oldPosition) > 0) {
      delay(DEBOUNCE);
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = myEnc.read();
      }
      ColorMenu.MoveDown();
    }

    bool enter = false;

    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) > 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      enter = true;
    }
    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) < 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      EditMenuOption = 0; // exit
    }

    if (enter) {
      EditMenuOption = ColorMenu.selectRow();
    }
  }

  // out of menu now time for processing

  // set global back color
  Serial.print("back color");

  Serial.println(C_VALUES[ (int) ColorMenu.value[ColorOption2]]);


  MENU_BACKGROUND = C_VALUES[ (int) ColorMenu.value[ColorOption2]];

  // set Option menu colors

  MainMenu.setAllColors(C_VALUES[ (int) ColorMenu.value[ColorOption1]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption2]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption3]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption4]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption8]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption9]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption10]],
                        C_VALUES[ (int) ColorMenu.value[ColorOption11]]);

  OptionMenu.setAllColors(C_VALUES[ (int) ColorMenu.value[ColorOption1]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption2]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption3]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption4]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption5]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption6]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption7]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption8]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption9]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption10]],
                          C_VALUES[ (int) ColorMenu.value[ColorOption11]]);
  // set color menu colors
  ColorMenu.setAllColors(C_VALUES[ (int) ColorMenu.value[ColorOption1]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption2]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption3]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption4]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption5]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption6]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption7]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption8]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption9]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption10]],
                         C_VALUES[ (int) ColorMenu.value[ColorOption11]]);
  // set color menu colors
  WirelessMenu.setAllColors(C_VALUES[ (int) ColorMenu.value[ColorOption1]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption2]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption3]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption4]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption5]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption6]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption7]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption8]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption9]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption10]],
                            C_VALUES[ (int) ColorMenu.value[ColorOption11]]);

}

void ProcessWirelessMenu() {

  int EditMenuOption = 1;

  Display.fillScreen(MENU_BACKGROUND);

  WirelessMenu.draw();

  while (EditMenuOption != 0) {
    button.update();
    delay(50);
    Position = myEnc.read();
    PositionY = myEnc2.read();

    if ((Position - oldPosition) < 0) {
      oldPosition = Position;
      WirelessMenu.MoveUp();
    }

    if ((Position - oldPosition) > 0) {
      oldPosition = Position;
      WirelessMenu.MoveDown();
    }
    
    bool enter = false;

    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) > 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      enter = true;
    }
    // attempt to debouce these darn things...
    if ((PositionY - oldPositionY) < 0) {
      delay(DEBOUNCE);
      while (oldPositionY != PositionY) {
        oldPositionY = PositionY;
        PositionY = myEnc2.read();
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      EditMenuOption = 0; // exit
    }

    if (enter) {

      EditMenuOption = WirelessMenu.selectRow();

    }
  }
}

// menu to handle processing for a sub-menu
// since this menu will be a menu that allows edits (EditMenu object type)
// process is exactly the same as an ItemMenu
// meaning you simply use the same MoveUp, MoveDown and the library will know if you are
// wanting to move the selector or cycle through a range

///////////////////////////////////////////////////////////////////////////////////////////////////
// end of example
//////////////////////////////////////////////////////////////////////////////////////////////////

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