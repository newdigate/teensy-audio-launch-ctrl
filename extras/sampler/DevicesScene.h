#ifndef TEENSY_SAMPLER_DEVICESSCHENE_H
#define TEENSY_SAMPLER_DEVICESSCHENE_H

#include <Arduino.h>

#include <MIDI.h>

#include <Encoder.h>
#include <Bounce2.h>

#include "icons.h"
#include "scenecontroller.h"
#include "teensy_controls.h"

namespace newdigate {

    class DeviceManager {
    public:


        int GetNumberOfAvailableDeviceTypes() {
            return _deviceTypeNames.size();
        }

        const char* GetNameOfDeviceTypeNumber(int deviceTypeNumner) {
            return _deviceTypeNames[deviceTypeNumner];
        }

    private:
        static const std::vector<const char*> _deviceTypeNames; 
    };
    const std::vector<const char*> DeviceManager::_deviceTypeNames = { "SD sampler", "Pattern Sequencer", "Step Sequencer"}; 

    class SelectDeviceDialog : public TeensyMenu {
    public:
        SelectDeviceDialog(View &view, DeviceManager & deviceManager) : 
            TeensyMenu(view, 100, 100, 13, 13, ST7735_BLACK, Gold),
            _selectionMenuItems() 
        {
            for (int i = 0; i < deviceManager.GetNumberOfAvailableDeviceTypes(); i++) {
                const char * deviceTypeName = deviceManager.GetNameOfDeviceTypeNumber(i);
                TeensyMenuItem *item = 
                    new TeensyMenuItem(
                        view,
                        std::bind( &SelectDeviceDialog::menuItemDraw, this, std::placeholders::_1, deviceTypeName), 
                        8,
                        std::bind( &SelectDeviceDialog::menuValueScroll, this, std::placeholders::_1), 
                        std::bind( &SelectDeviceDialog::menuMidiNoteEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), 
                        std::bind( &SelectDeviceDialog::menuMidiCCEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 
                        std::bind( &SelectDeviceDialog::buttonDownEvent, this, std::placeholders::_1));
                _selectionMenuItems.push_back(item);
            }
        } 

        SelectDeviceDialog(const SelectDeviceDialog & selectDeviceDialog) = delete;
        virtual ~SelectDeviceDialog() {}
    private:
        std::vector<TeensyMenuItem*> _selectionMenuItems;

        void menuItemDraw(View *v, const char * label){
            v->drawString(label, 0, 0);
        } 

        void menuValueScroll(bool forward) {

        }   

        bool menuMidiNoteEvent(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) {
            return false;
        }  

        bool menuMidiCCEvent(uint8_t channel, uint8_t data1, uint8_t data2) {
            return false;
        }  

        void buttonDownEvent(uint8_t buttonNumber) {

        }

    };

    class DevicesSceneAddDeviceMenuItem : public TeensyMenuItem {
    public:
        DevicesSceneAddDeviceMenuItem(View & view, std::function<void()> fnMenuItemClicked) : 
            TeensyMenuItem(
                view, 
                std::bind( &DevicesSceneAddDeviceMenuItem::updateWithView, this, std::placeholders::_1), 
                8,
                std::bind( &DevicesSceneAddDeviceMenuItem::menuValueScroll, this, std::placeholders::_1), 
                std::bind( &DevicesSceneAddDeviceMenuItem::menuMidiNoteEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), 
                std::bind( &DevicesSceneAddDeviceMenuItem::menuMidiCCEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 
                std::bind( &DevicesSceneAddDeviceMenuItem::buttonDownEvent, this, std::placeholders::_1)),
            _fnMenuItemClicked(fnMenuItemClicked)
        {
        }

        DevicesSceneAddDeviceMenuItem( const DevicesSceneAddDeviceMenuItem &item) = delete;
        virtual ~DevicesSceneAddDeviceMenuItem() {}

        void updateWithView(View *v){
            v->drawString("Add device", 0, 0);
        } 

        void menuValueScroll(bool forward) {

        }   

        bool menuMidiNoteEvent(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) {
            return false;
        }  

        bool menuMidiCCEvent(uint8_t channel, uint8_t data1, uint8_t data2) {
            return false;
        }  

        void buttonDownEvent(uint8_t buttonNumber) {
            _fnMenuItemClicked();
        }

    private:
        std::function<void()> _fnMenuItemClicked;
    };

    class DevicesScene : public BaseScene {
    public:
        DevicesScene(View &view, DeviceManager & deviceManager) : 
            BaseScene(
                _bmp_devices_on, 
                _bmp_devices_off,
                16, 16), 
            _view(view),
            _settingsMenu(view, 0, 0, 128, 128, Gold, ST7735_BLACK),
            _settingMenuItems {
                new DevicesSceneAddDeviceMenuItem(
                    _settingsMenu,
                    std::bind( &DevicesScene::ShowSelectDeviceTypeDialog, this ))
            },
            _deviceManager(deviceManager),
            _selectDeviceDialog(nullptr)
        {
            _settingsMenu.AddControl(_settingMenuItems[0]);
        }

        virtual ~DevicesScene() {
        }

        void Update() override {
            if (_selectDeviceDialog == nullptr)
                _settingsMenu.Update();
            else
                _selectDeviceDialog->Update();
        }

        void InitScreen() override {
            _view.fillScreen(ST7735_BLACK);
            _settingsMenu.NeedsUpdate = true; 
        }

        void UninitScreen() override {
        }

        void ButtonPressed(unsigned index) override {
            if (_selectDeviceDialog == nullptr)
                _settingsMenu.ButtonDown(index);
            else
                _selectDeviceDialog->ButtonDown(index);
        }

        void Rotary1Changed(bool forward) override {
        }

        void Rotary2Changed(bool forward) override {
        }

        bool HandleNoteOnOff(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) override { 
            return false;
        }

        bool HandleControlChange(uint8_t channel, uint8_t data1, uint8_t data2) override { 
            return false; 
        }

        void ShowSelectDeviceTypeDialog() {
            if (_selectDeviceDialog == nullptr) {
                _selectDeviceDialog = new SelectDeviceDialog(_view, _deviceManager);
                _selectDeviceDialog->NeedsUpdate = true;
            } 
        }

    private:
        View & _view;
        TeensyMenu _settingsMenu;
        std::vector<TeensyMenuItem*> _settingMenuItems;
        DeviceManager & _deviceManager;
        SelectDeviceDialog  * _selectDeviceDialog;
    };

}
#endif
