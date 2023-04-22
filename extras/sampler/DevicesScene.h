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
    class MidiRouter;

    class Device {
    public:
        Device() :  _outputConnections() {
        }

        Device(const Device &device) = delete;

        virtual ~Device() {
        }

        virtual void noteEvent(uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) = 0;   
        virtual void ccEvent(uint8_t ccNumber, uint8_t ccChannel, uint8_t value) = 0;

        void broadcastNoteEvent(uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) {
            for (auto && device : _outputConnections) {
                device->noteEvent(noteNumber, noteChannel, velocity);
            }
        }
    protected:
        friend class MidiRouter;
        std::vector<Device*> _outputConnections;  
    };


    class MidiRouter {
    public:
        MidiRouter() {
        }

        MidiRouter(const MidiRouter &router) = delete;

        virtual ~MidiRouter() {
        }

        void Connect(Device *input, Device *output){
            input->_outputConnections.push_back(output);
        }
    };

    class DeviceManager {
    public:
        DeviceManager() : 
            _devices() {
        }

        DeviceManager(const DeviceManager &deviceManager) = delete;

        virtual ~DeviceManager() {
        }

        int GetNumberOfAvailableDeviceTypes() {
            return _deviceTypeNames.size();
        }

        const char* GetNameOfDeviceTypeNumber(int deviceTypeNumner) {
            return _deviceTypeNames[deviceTypeNumner];
        }

    private:
        std::vector<Device *> _devices;
        static const std::vector<const char*> _deviceTypeNames; 
    };
    const std::vector<const char*> DeviceManager::_deviceTypeNames = { "SD sampler", "Pattern Sequencer", "Step Sequencer"}; 

    class SelectDeviceDialog : public TeensyMenu {
    public:
        SelectDeviceDialog(
            View &view, 
            DeviceManager & deviceManager, 
            SceneController<VirtualView, Encoder, Bounce2::Button> &sceneController,
            std::function<void()> onClose
        ) : 
            TeensyMenu(view, 100, 100, 13, 13, ST7735_BLACK, Gold),
            _selectionMenuItems(),
            _sceneController(sceneController),
            _onClose(onClose)
        {
            for (int i = 0; i < deviceManager.GetNumberOfAvailableDeviceTypes(); i++) {
                const char * deviceTypeName = deviceManager.GetNameOfDeviceTypeNumber(i);
                TeensyMenuItem *item = 
                    new TeensyMenuItem(
                        *this,
                        std::bind( &SelectDeviceDialog::menuItemDraw, this, std::placeholders::_1, deviceTypeName), 
                        8,
                        std::bind( &SelectDeviceDialog::menuValueScroll, this, std::placeholders::_1), 
                        std::bind( &SelectDeviceDialog::menuMidiNoteEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), 
                        std::bind( &SelectDeviceDialog::menuMidiCCEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 
                        std::bind( &SelectDeviceDialog::buttonDownEvent, this, std::placeholders::_1, i));
                _selectionMenuItems.push_back(item);
                AddControl(item);
            }
        } 

        SelectDeviceDialog(const SelectDeviceDialog & selectDeviceDialog) = delete;
        virtual ~SelectDeviceDialog() {
            for (auto && teensyMenuItem : _selectionMenuItems) {
                delete teensyMenuItem;
            }
            _selectionMenuItems.clear();
        }
    private:
        std::vector<TeensyMenuItem*> _selectionMenuItems;
        SceneController<VirtualView, Encoder, Bounce2::Button> &_sceneController;
        std::function<void()> _onClose;

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

        void buttonDownEvent(uint8_t buttonNumber, int selectedIndex) {
            Serial.printf("device selected: %d\n", selectedIndex);
            _sceneController.PopDialog();
            if (_onClose != nullptr) {
                _onClose();
            }
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
        DevicesScene(View &view, DeviceManager & deviceManager, SceneController<VirtualView, Encoder, Bounce2::Button> & sceneController) : 
            BaseScene(
                _bmp_devices_on, 
                _bmp_devices_off,
                16, 16), 
            _view(view),
            _settingsMenu(view, 128, 128, 0, 0, Gold, ST7735_BLACK),
            _settingMenuItems {
                new DevicesSceneAddDeviceMenuItem(
                    _settingsMenu,
                    std::bind( &DevicesScene::ShowSelectDeviceTypeDialog, this ))
            },
            _deviceManager(deviceManager),
            _selectDeviceDialog(nullptr),
            _sceneController(sceneController)
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
                _selectDeviceDialog = new SelectDeviceDialog(_view, _deviceManager, _sceneController, std::bind(&DevicesScene::SelectDeviceTypeDialogClose, this));
                _selectDeviceDialog->NeedsUpdate = true;
                _sceneController.AddDialog(_selectDeviceDialog);
            } 
        }

        void SelectDeviceTypeDialogClose() {
            delete _selectDeviceDialog;
            _selectDeviceDialog = nullptr;
        }

    private:
        View & _view;
        TeensyMenu _settingsMenu;
        std::vector<TeensyMenuItem*> _settingMenuItems;
        DeviceManager & _deviceManager;
        SelectDeviceDialog  * _selectDeviceDialog;
        SceneController<VirtualView, Encoder, Bounce2::Button> &_sceneController;
    };

}
#endif
