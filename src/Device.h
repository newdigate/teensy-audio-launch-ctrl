#ifndef TEENSY_SAMPLER_DEVICE_H
#define TEENSY_SAMPLER_DEVICE_H

#include <Arduino.h>
#include "MySampler.h"
#include "sdsampleplayernote.h"

namespace newdigate {

    enum DeviceType {
        MidiInputUSB = 0,
        MidiInputSerial = 1,
        SDLoopSampler = 2,
        PatternSequencer =  3,
        StepSequncer = 4
    };

    class DeviceTypeNames {
    public:
        static const std::vector<const char*> _deviceTypeNames;
        static const std::map<DeviceType, const char *> _deviceTypeNamesMap;
    };

    const std::vector<const char*> DeviceTypeNames::_deviceTypeNames = { 
        "MidiInput USB" ,
        "MidiInput Serial" ,
        "SD sampler", 
        "Pattern Sequencer", 
        "Step Sequencer"
    }; 

    const std::map<DeviceType, const char *> DeviceTypeNames::_deviceTypeNamesMap = { 
        { DeviceType::MidiInputUSB, DeviceTypeNames::_deviceTypeNames[0] }, 
        { DeviceType::MidiInputSerial, DeviceTypeNames::_deviceTypeNames[1] }, 
        { DeviceType::SDLoopSampler, DeviceTypeNames::_deviceTypeNames[2]}, 
        { DeviceType::PatternSequencer, DeviceTypeNames::_deviceTypeNames[3]},
        { DeviceType::StepSequncer, DeviceTypeNames::_deviceTypeNames[4]} };

    class Device {
    public:
        Device() :  _outputConnections() {
        }

        Device(const Device &device) = delete;

        virtual ~Device() {
        }

        virtual DeviceType GetDeviceType() = 0;
        
        virtual void noteEvent(uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) = 0;   
        virtual void ccEvent(uint8_t ccNumber, uint8_t ccChannel, uint8_t value) = 0;

        void broadcastNoteEvent(uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) {
            for (auto && device : _outputConnections) {
                device->noteEvent(noteNumber, noteChannel, velocity);
            }
        }

        void broadcastCCEvent(uint8_t ccNumber, uint8_t ccChannel, uint8_t value) {
            for (auto && device : _outputConnections) {
                device->ccEvent(ccNumber, ccChannel, value);
            }
        }
    protected:
        friend class DeviceManager;
        std::vector<Device*> _outputConnections;  
    };

    class LoopSamplerDevice : public Device {
    public:
        LoopSamplerDevice(MyLoopSampler& loopSampler) : Device(), _loopSampler(loopSampler) {
        }

        LoopSamplerDevice(const LoopSamplerDevice &device) = delete;

        virtual ~LoopSamplerDevice() {
        }
        
        DeviceType GetDeviceType() override {
            return DeviceType::SDLoopSampler;
        }

        void noteEvent(uint8_t noteNumber, uint8_t noteChannel, uint8_t velocity) override {
            _loopSampler.trigger(noteNumber, noteChannel, velocity, velocity != 0);
        }   

        void ccEvent(uint8_t ccNumber, uint8_t ccChannel, uint8_t value)  override {
        }

    protected:
        MyLoopSampler& _loopSampler;
    };


 
    class DeviceFactory {
    public:
        DeviceFactory(
            samplermodel<sdsampleplayernote> &model,
            audiovoicepolyphonic<AudioPlaySdResmp> &polyphony) 
        :   _model(model),
            _polyphony(polyphony)
        {
        }

        DeviceFactory(const DeviceFactory& device) = delete;

        virtual ~DeviceFactory() {
            for (auto && loopSampler : _loopSamplers) {
                delete loopSampler;
            }
            _loopSamplers.clear();
        }

        int GetNumberOfAvailableDeviceTypes() {
            return DeviceTypeNames::_deviceTypeNames.size();
        }

        const char* GetNameOfDeviceTypeNumber(int deviceTypeNumner) {
            return DeviceTypeNames::_deviceTypeNames[deviceTypeNumner];
        }

        Device * CreateDevice(int deviceTypeNumber){
            Device * newDevice = nullptr;
            switch (deviceTypeNumber) {
                case 0: 
                    MyLoopSampler *loopSampler = new MyLoopSampler(_model, _polyphony);
                    _loopSamplers.push_back(loopSampler);
                    newDevice = new LoopSamplerDevice(*loopSampler);
                    break;
            }
            return newDevice;
        }

    protected:
        samplermodel<sdsampleplayernote> &_model;
        audiovoicepolyphonic<AudioPlaySdResmp> &_polyphony;
        std::vector<MyLoopSampler *> _loopSamplers;
    };

    class DeviceManager {
    public:
        DeviceManager(DeviceFactory & deviceFactory) : 
            _devices(),
            _deviceFactory(deviceFactory),
            _onDeviceAddedCallbacks(),
            _onDeviceRemovedCallbacks()
        {
        }

        DeviceManager(const DeviceManager &deviceManager) = delete;

        virtual ~DeviceManager() {
            for (auto && device : _devices) {
                delete device;
            }
            _devices.clear();
        }

        int GetNumberOfAvailableDeviceTypes() {
            return _deviceFactory.GetNumberOfAvailableDeviceTypes();
        }

        const char* GetNameOfDeviceTypeNumber(int deviceTypeNumner) {
            return _deviceFactory.GetNameOfDeviceTypeNumber(deviceTypeNumner);
        }

        void Connect(Device *input, Device *output){
            input->_outputConnections.push_back(output);
        }

        void AddNewDevice(int deviceTypeNumber){
            Device * newDevice = _deviceFactory.CreateDevice( deviceTypeNumber );
            if (newDevice != nullptr)
                _devices.push_back(newDevice);
        }

        void SubscribeOnDeviceAddedCallback(std::function<void(Device*)> callback){
            _onDeviceAddedCallbacks.push_back(callback);
        }

        void SubscribeOnDeviceRemovedCallback(std::function<void(Device*)> callback){
            _onDeviceRemovedCallbacks.push_back(callback);
        } 

    private:
        std::vector<Device *> _devices;
        DeviceFactory &_deviceFactory;
        std::vector<std::function<void(Device*)>> _onDeviceAddedCallbacks;
        std::vector<std::function<void(Device*)>> _onDeviceRemovedCallbacks;

        void CallOnDeviceAddedCallbacks(Device *device){
            for (auto && callback : _onDeviceAddedCallbacks) {
                callback(device);
            }
        }

        void CallOnDeviceRemovedCallbacks(Device *device){
            for (auto && callback : _onDeviceRemovedCallbacks) {
                callback(device);
            }
        }
    };
}
#endif
