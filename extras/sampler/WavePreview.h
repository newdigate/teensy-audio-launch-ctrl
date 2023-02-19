#ifndef TEENSY_SAMPLER_WAVEPREVIEW_H
#define TEENSY_SAMPLER_WAVEPREVIEW_H

#include <Arduino.h>
#include <SD.h>
#include "teensy_controls.h"
#include "WavePreviewBuilder.h"

namespace newdigate {

    class WavePreview : public TeensyControl {
    public:

        WavePreview(View &view, SDClass &sd, unsigned int width, unsigned int height, unsigned int x, unsigned int y) : 
            TeensyControl(
                view, 
                [&] () {
                    if (_needsUpdate) {
                        _view.fillRect(_left, _top, _width, _height, ST7735_BLACK);
                        for (int i=0; i<_width; i++) {
                            int dataIndex = i * _dataSize / _width;
                            const uint16_t Polynesian_Blue = 0x2272;
                            _view.drawLine(_left+i, _top + _height-(_height * _data[dataIndex]/128), _left+i, _top + _height, Polynesian_Blue );
                        }
                        _needsUpdate = false;
                    }
                }, 
                width, height, x, y),
            _view(view), 
            _sd(sd),
            _data(nullptr),
            _dataSize(-1),
            _needsUpdate(false)
        {        
        }

        virtual ~WavePreview() {
        } 

        void Show(const char *filename) {
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

    private:
        View &_view;
        SDClass &_sd;
        uint8_t *_data;
        int _dataSize;
        bool _needsUpdate;
    };

}
#endif
