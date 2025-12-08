#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class DataEEPROM
{
    public:

        DataEEPROM(const DataEEPROM&) = delete;

        DataEEPROM& operator=(const DataEEPROM&) = delete;

        static DataEEPROM* Instance(size_t size = 4096);

        void SetError(uint16_t error);

        void CleanErrors();

        uint16_t* GetErrors();

        uint16_t GetNumErrors();

        #ifdef ESP32
            void SaveChangeValue(uint8_t num, void* ptr, uint8_t type);

            uint8_t GetNumSavedValues();

            void* GetSavedValue(uint8_t num);
        #endif
       

    private:

        DataEEPROM(size_t size);

        void CycleWrite(uint16_t error);

        static const uint8_t maxErrorNum = 50;
        static const uint8_t maxChangedValNum = 30;
        static const uint8_t validMarker = 123;

        struct MyStructDataEEPROM
        {
            uint8_t marker;

            uint8_t errNum = 0;
            uint8_t cycleErrNum = 0;
            uint16_t error[maxErrorNum] = {0};

            uint8_t changedValNumLenght = 0;
            uint8_t changedValByte[maxChangedValNum * 4] = {0};
            uint8_t changedValNum[maxChangedValNum] = {0};
        };
        MyStructDataEEPROM StructDataEEPROM;

        static DataEEPROM* instance;
};