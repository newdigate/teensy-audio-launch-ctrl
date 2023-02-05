#ifndef TEENSY_SAMPLER_MODEL_H
#define TEENSY_SAMPLER_MODEL_H

#include "sdsampleplayernote.h"
namespace newdigate
{
    class SamplerModel {
    public:
        SamplerModel() : 
            _samplerNotes() 
        {
        }

        virtual ~SamplerModel() {
        }

    private:
        std::vector<sdsampleplayernote*> _samplerNotes;
    };

} // namespace newdigate


#endif TEENSY_SAMPLER_MODEL_H
