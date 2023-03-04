#ifndef TEENSY_SAMPLER_WAVEPREVIEW_H
#define TEENSY_SAMPLER_WAVEPREVIEW_H

#include <Arduino.h>
#include <SD.h>
#include "teensy_controls.h"
#include "WavePreviewBuilder.h"

namespace newdigate {

    // wave foreground color
    const uint16_t Polynesian_Blue = 0x2272;
    // wave background color
    const uint16_t Oxford_blue = 0x0109;

    // wave progress indicator foreground
    const uint16_t Neon_Blue = 0x4A7F;
    // wave progress indicator background
    const uint16_t Ultramarine_blue = 0x433E;

    class ProgressIndicator {
    public:
        ProgressIndicator(unsigned index, unsigned progress) :
            _index(index),
            _progress(progress),
            _previousProgress(0),
            _hasPreviousProgress(false)
        {
        }
        virtual ~ProgressIndicator() {
        } 

        unsigned _index;
        unsigned _progress;
        unsigned _previousProgress;
        bool _hasPreviousProgress;
    };

    class WavePreview : public TeensyControl {
    public:

        WavePreview(View &view, SDClass &sd, unsigned int width, unsigned int height, unsigned int x, unsigned int y) : 
            TeensyControl(
                view, 
                [&] () {
                    if (_needsUpdate && _data != nullptr) {
                        ClearBackground();
                        for (auto && prog : _progressIndicators) {
                            if (prog.second != nullptr) {
                                DrawProgressIndicator(prog.second);
                            }
                        }

                        for (int i=0; i<_width; i++) {
                            int dataIndex = i * _dataSize / _width;
                            _view.drawLine(_left+i, _top + _height-(_height * _data[dataIndex]/128), _left+i, _top + _height, Polynesian_Blue );
                        }
                        _needsUpdate = false;
                        _progressNeedsUpdate = false;
                        _progressIndicatorsToUpdate.clear();
                    } else if (_progressNeedsUpdate && _data != nullptr) {
                        for (auto && prog : _progressIndicatorsToUpdate) {
                            ProgressIndicator *ind = prog.second;
                            if (ind != nullptr) {
                                UndrawProgressIndicator(ind);
                                DrawProgressIndicator(ind);
                            }
                        }
                        _progressIndicatorsToUpdate.clear();
                    }
                }, 
                width, height, x, y),
            _view(view), 
            _sd(sd),
            _data(nullptr),
            _dataSize(-1),
            _needsUpdate(false),
            _progressNeedsUpdate(false),
            _progressIndicators(),
            _progressIndicatorsToUpdate()
        {        
        }

        virtual ~WavePreview() {
            if (_data != nullptr)
                delete _data;
            DeleteProgressIndicators();
        } 

        void Show(const char *filename) {
            DeleteProgressIndicators();
            unsigned len = strlen(filename);
            char *previewFilename = new char[len+1] { 0 };
            int indexOfDot = -1;
            for (int i=0; i<len; i++) {
                previewFilename[i] = filename[i];
                if (filename[i] == '.') {
                    indexOfDot = i;
                    break;
                }
            }
            previewFilename[indexOfDot+1] = 'w';
            previewFilename[indexOfDot+2] = 'a';
            previewFilename[indexOfDot+3] = 'a';
            previewFilename[indexOfDot+4] = '\0';

            File previewFile = _sd.open(previewFilename);

            if (!previewFile) {
                WavePreviewBuilder builder(_sd);
                builder.Process(filename, previewFilename);
                previewFile = _sd.open(previewFilename);
            }

            if (_data != nullptr) {
                delete [] _data;
            }

            _dataSize = previewFile.size();
            _data = new uint8_t[_dataSize];
            unsigned int bytesRead = 0;
            unsigned int offset = 0;
            do {
                bytesRead = previewFile.read(&_data[offset], 512);
                offset += bytesRead;
            } while (bytesRead > 0);

            delete [] previewFilename;
            _needsUpdate = true;
        }
        void DeleteProgressIndicators() {
            for (auto && prog : _progressIndicators) {
                delete prog.second;
            }
            _progressIndicators.clear();
            _progressIndicatorsToUpdate.clear();
        }

        void CreateOrUpdateProgressIndicator(unsigned index, unsigned progress) {
            ProgressIndicator *indicator = _progressIndicators[index];
            if (indicator == nullptr) {
                indicator = new ProgressIndicator(index, progress);
                _progressIndicators[index] = indicator;                
            } else {
                indicator->_previousProgress = indicator->_progress;    
                indicator->_hasPreviousProgress = true;
                //UndrawProgressIndicator(indicator);
                indicator->_progress = progress;
            }
            _progressIndicatorsToUpdate[index] = indicator;
            _progressNeedsUpdate = true;
            //DrawProgressIndicator(indicator);
        }
        void UndrawProgressIndicator(ProgressIndicator *indicator) {
            if (indicator == nullptr)
                return;
            if (!indicator->_hasPreviousProgress)
                return;

            unsigned x = indicator->_previousProgress;
            unsigned dataIndex = x * _dataSize  / _width;
            uint8_t data = _data[dataIndex];
            _view.drawLine(_left+x, _top, _left+x, _top + _height-(_height * data/128), Oxford_blue );
            _view.drawLine(_left+x, _top + _height-(_height * data/128), _left+x, _top + _height, Polynesian_Blue );
        }
        void DrawProgressIndicator(ProgressIndicator *indicator) {
            if (indicator == nullptr)
                return;

            unsigned x = indicator->_progress;
            unsigned dataIndex = x * _dataSize  / _width;
            uint8_t data = _data[dataIndex];
            _view.drawLine(_left+x, _top, _left+x, _top + _height-(_height * data/128), Ultramarine_blue );
            _view.drawLine(_left+x, _top + _height-(_height * data/128), _left+x, _top + _height, Neon_Blue );
        }

        void ClearBackground() {
            _view.fillRect(_left, _top, _width, _height, Oxford_blue);
        }

        void Reset() {
            ClearBackground();
            if (_data != nullptr) {
                delete _data;
                _dataSize = 0;
                _data = nullptr;
            }
            DeleteProgressIndicators();
            _needsUpdate = false;
            _progressNeedsUpdate = false;
        }

    private:
        View &_view;
        SDClass &_sd;
        uint8_t *_data;
        int _dataSize;
        bool _needsUpdate;
        bool _progressNeedsUpdate;
        std::map<unsigned, ProgressIndicator*> _progressIndicators;
        std::map<unsigned, ProgressIndicator*> _progressIndicatorsToUpdate;
    };

}
#endif
