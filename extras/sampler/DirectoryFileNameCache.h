#ifndef TEENSY_SAMPLER_DIRECTORYFILENAMECACHE_H
#define TEENSY_SAMPLER_DIRECTORYFILENAMECACHE_H

#include <Arduino.h>
#include <SD.h>

namespace newdigate {

    class DirectoryFileNameCache {
    public:
        DirectoryFileNameCache(SDClass &sd) : 
            _sd(sd),
            _filenames()
        {
            populateFilenames();
        }

        virtual ~DirectoryFileNameCache() {
            unloadFilenames();
        }

        void unloadFilenames() {
            for (auto && filename : _filenames) {
                delete [] filename;
            }
            _filenames.clear();
        }

        void populateFilenames(const char *directory = nullptr) {
            unloadFilenames();
            File dir = directory? _sd.open(directory) : _sd.open("/");
            unsigned int index = 0;
            while (true) { 

                File files =  dir.openNextFile();
                if (!files) {
                    //If no more files, break out.
                    break;
                }

                String curfile = files.name(); //put file in string
                int m = curfile.lastIndexOf(".WAV");
                int a = curfile.lastIndexOf(".wav");
                int underscore = curfile.indexOf("_");
                int dot = curfile.indexOf(".");

                // if returned results is more then 0 add them to the list.
                if ((m > 0 || a > 0) && (underscore != 0) && (dot != 0)) {  
                    char *filename = new char[curfile.length()+1] {0};
                    memcpy(filename, curfile.c_str(), curfile.length());
                    _filenames.push_back(filename);
                    //Serial.printf("filename: %d: %s\n", index, filename);
                    index++;
                } // else 
                //  Serial.printf("INGORE: filename: %d: %s\n", index, curfile.c_str());
                files.close();
            }
            // close 
            dir.close();
        }
        
        int getIndexOfFile(const char *filename){
            auto it = find(_filenames.begin(), _filenames.end(), filename);
            if (it != _filenames.end())   {
                int index = it - _filenames.begin();
                return index;
            }
            return -1;
        }

        int getNumFileNames() {
            return _filenames.size();
        }

        char *getFileNameForIndex(const uint index){
            if (index < _filenames.size()) {
                return _filenames[index];
            }
            return nullptr;
        }


    private:
        SDClass& _sd;
        std::vector<char *> _filenames;
    };
}
#endif
