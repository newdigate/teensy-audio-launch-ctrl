#ifndef TEENSY_SAMPLER_DEVICESSCHENE_H
#define TEENSY_SAMPLER_DEVICESSCHENE_H

#include <Arduino.h>

#include <MIDI.h>

#include <Encoder.h>
#include <Bounce2.h>
#include "Device.h"
#include "icons.h"
#include "scenecontroller.h"
#include "teensy_controls.h"
#include "MySampler.h"

namespace newdigate {
    class MidiRouter;

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

    class DevicePreviewControl : public TeensyControl {
    public:
        DevicePreviewControl(
            View &view, 
            Device *device) : TeensyControl(view, nullptr, 128, 16, 0, 0)
        {
        }

        void Update() override {
        }

        void ValueScroll(bool forward) override { 
        }
        
        bool MidiNoteEvent(bool noteDown, uint8_t channel, uint8_t pitch, uint8_t velocity) override { 
            return false;
        }

        bool MidiCCEvent(uint8_t channel, uint8_t data1, uint8_t data2) override {
            return false;
        }

        void ButtonDown(uint8_t buttonNumber) override {
        }

        void IncreaseSelectedIndex() override {
        }

        void DecreaseSelectedIndex() override {
        }

    protected:
        std::function<void()> f_update = nullptr;
        std::vector<TeensyControl *> _children;
    };

    class DevicePreviewControlBuilder {
    public:
        DevicePreviewControlBuilder() { }

        DevicePreviewControlBuilder( const DevicePreviewControlBuilder &devicePreviewControlBuilder) = delete;
        
        virtual ~DevicePreviewControlBuilder() {}

        TeensyControl *CreatePreviewControl(Device *device, View &view) {
            TeensyControl *result;
            switch (device->GetDeviceType())
            {
                case DeviceType::SDLoopSampler: 
                    result = new DevicePreviewControl(view, device);
                    break;
            }
            return result;
        }
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
            _sceneController(sceneController),
            _devicePreviewControlBuilder()
        {
            _settingsMenu.AddControl(_settingMenuItems[0]);
            _deviceManager.SubscribeOnDeviceAddedCallback( std::bind(&DevicesScene::OnDeviceAdded, this, std::placeholders::_1));
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

        void OnDeviceAdded(Device * device) {
            switch (device->GetDeviceType()) {
                case DeviceType::SDLoopSampler : {
                    _settingsMenu.AddControl(_devicePreviewControlBuilder.CreatePreviewControl(device, _settingsMenu));
                    break;
                }
            }
        }
    private:
        View & _view;
        TeensyMenu _settingsMenu;
        std::vector<TeensyMenuItem*> _settingMenuItems;
        DeviceManager & _deviceManager;
        SelectDeviceDialog  * _selectDeviceDialog;
        SceneController<VirtualView, Encoder, Bounce2::Button> &_sceneController;
        DevicePreviewControlBuilder _devicePreviewControlBuilder;
    };

}
#endif
