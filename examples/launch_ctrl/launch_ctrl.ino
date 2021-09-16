// Plays a RAW (16-bit signed) PCM audio file at slower or faster rate
// this example plays a sample stored in an array
#include <Arduino.h>
#include <Audio.h>
#include <MIDI.h>
#include <SD.h>
#include <TeensyAudioLaunchCtrl.h>
#include <ST7735_t3.h>
#include "st7735display.h"
#include "USBHost_t36.h"

#define TFT_CS   6  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define TFT_DC    2  //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define TFT_RST   255 // RST can use any pin
ST7735_t3 tft = ST7735_t3(TFT_CS, TFT_DC, TFT_RST);

#define NUM_VOICES 4
#define KEY_NOTENUMBER_C1 36

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdAudio3;     //xy=334.0000457763672,459.0000305175781
AudioPlaySdWav           playSdAudio2;     //xy=338.0000305175781,382.0000305175781
AudioPlaySdWav           playSdAudio4;     //xy=339.0000457763672,536
AudioPlaySdWav           playSdAudio1;     //xy=340.0000419616699,302.0000238418579
AudioMixer4              mixerLeft;         //xy=650.0000419616699,404.0000238418579
AudioMixer4              mixerRight;         //xy=650.0000610351562,511.0000305175781
AudioOutputTDM           tdm;           //xy=671,431
AudioConnection          patchCord1(playSdAudio3, 0, mixerLeft, 2);
AudioConnection          patchCord2(playSdAudio3, 1, mixerRight, 2);
AudioConnection          patchCord3(playSdAudio2, 0, mixerLeft, 1);
AudioConnection          patchCord4(playSdAudio2, 1, mixerRight, 1);
AudioConnection          patchCord5(playSdAudio4, 0, mixerLeft, 3);
AudioConnection          patchCord6(playSdAudio4, 1, mixerRight, 3);
AudioConnection          patchCord7(playSdAudio1, 0, mixerLeft, 0);
AudioConnection          patchCord8(playSdAudio1, 1, mixerRight, 0);
AudioConnection          patchCord9(mixerLeft, 0, tdm, 0);
AudioConnection          patchCord10(mixerRight, 0, tdm, 1);
AudioControlCS42448      audioShield;
// GUItool: end automatically generated code

USBHost myusb;
MIDIDevice midi1(myusb);

polyphonic<AudioPlaySdWav>    _sampler;
ST7735Display                 _st7735display(tft);
sdsampleplayermidicontroller  _controller(_sampler, _st7735display);

AudioPlaySdWav           *_voices[NUM_VOICES] = {&playSdAudio1, &playSdAudio2, &playSdAudio3, &playSdAudio4};

void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity);
void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2);
void printUsage();

void setup() {
    Serial.begin(9600);
    //while(!Serial) {
    //  delay(100);
    //}
    AudioMemory(20);

    while (!(SD.begin(BUILTIN_SDCARD))) {
        // stop here if no SD card, but print a message
        Serial.println("Unable to access the SD card...");
        delay(500);
    }
    audioShield.enable();
    audioShield.volume(0.5);
    myusb.begin();
    
    tft.initR(INITR_144GREENTAB); 
    tft.setRotation(3);   
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    midi1.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    midi1.setHandleNoteOff(handleNoteOff);

    midi1.setHandleControlChange(handleControlChange);
    // Initiate MIDI communications, listen to all channels
    //midi1.begin(MIDI_CHANNEL_OMNI);

    _sampler.addVoices(_voices, NUM_VOICES);
    
    _controller.begin();
    printUsage();
    //_controller.loadSamples("samples.smp");
}

void loop() {
    myusb.Task();
    midi1.read();

    _controller.update();
    if (Serial.available()) {
        String s = Serial.readString();
        if (s == "r\n") {
            Serial.println("reset...");
            _controller.initialize();
        } else if (s == "l\n") {
            Serial.println("loading samples...");
            _controller.loadSamples("samples.smp");
        } else if (s == "s\n") {
            Serial.println("saving samples...");
            _controller.writeSamples("samples.smp");
        } else {
            Serial.printf("unknown input: %s\r\n", s.c_str());
            printUsage();
        }
    }
    delay(1);
}

void printUsage() {
    Serial.println("------------------ usage ------------------");
    Serial.println("press: ");
    Serial.println("   'r' : reset control keys ");
    Serial.println("   'l' : load project ");
    Serial.println("   's' : save project ");
    Serial.println("-------------------------------------------");
    Serial.println();
}

void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
    // Do whatever you want when a note is pressed.
    // Try to keep your callbacks short (no delays ect)
    // otherwise it would slow down the loop() and have a bad impact
    // on real-time performance.
    //uint8_t pitchMapped = pitch + ((channel-1) * 5);
    //_sampler.noteEvent(pitchMapped, velocity, true, false);
    _controller.midiChannleVoiceMessage(0x90, pitch, velocity, channel);
}

void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
  _controller.midiChannleVoiceMessage(0x80, pitch, velocity, channel);
}

void handleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) {
  _controller.midiChannleVoiceMessage(0xC0, data1, data2, channel);
}