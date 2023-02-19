#ifndef TEENSY_SAMPLER_WAVEPREVIEWBUILDER_H
#define TEENSY_SAMPLER_WAVEPREVIEWBUILDER_H

#include <Arduino.h>
#include <SD.h>

namespace newdigate {
    uint8_t const F_TRUNC = 0X40;
    uint8_t const F_WRITE = 0X02;

    class WavePreviewBuilder {
    public:

        WavePreviewBuilder(SDClass &sd) : _sd(sd) 
        {
            
        }

        virtual ~WavePreviewBuilder() {
        }

        void Process(const char *inputFilename, const char *outputFilename) {
            File wavFile = _sd.open(inputFilename);
            if (!wavFile) return;

            if (wavFile.size() <= 44) return;

            std::vector<unsigned int> amplitudes;

            int bytesRead;
            //if (!wavFile.seek(44)) return;
            uint8_t riff[44];
            bytesRead = wavFile.read(riff, 44);

            int16_t buffer[128];
            do {
                unsigned int maxAmplitude = 0;
                bytesRead = wavFile.read(buffer, 256);
                for (int i=0; i < bytesRead/2; i++) {
                    unsigned int absolute = abs(buffer[i]);
                    if ( absolute > maxAmplitude )
                        maxAmplitude = absolute;
                }
                amplitudes.push_back(maxAmplitude);
            } while (bytesRead > 0);
            wavFile.close();

            File outputFile = _sd.open(outputFilename, F_WRITE | F_TRUNC);
            for (auto && amplitude : amplitudes){
                outputFile.write( amplitude >> 8 );
            }
            outputFile.close();
        }

    private:
        SDClass &_sd;
    };

}
#endif
