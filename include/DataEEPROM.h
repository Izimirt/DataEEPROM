
#pragma once

#include <Arduino.h>
#include <EEPROM.h>

    //                       |---------------------------------------> EEPROM memory
    // markerOffset = 0----->|marker|errNum|{ErrStruct}|{ValStruct}|...
    //             errNumOffset = 1 |      |           |           |
    //                       |----->|      |           |           |
    //                       |             |           |           |
    //                       |errOffset    |           |           |
    //                       |------------>|           |           |
    //                       |                         |           |
    //                       |                         |           |
    //                       |       valOffset         |           |
    //                       |------------------------>|           |
    //                       |                                     |
    //                       |              fullOffset             |
    //                       |------------------------------------>|

class DataEEPROM
{
    public:

        DataEEPROM(const DataEEPROM&) = delete;

        DataEEPROM& operator=(const DataEEPROM&) = delete;

        static DataEEPROM* Instance(size_t size = 4096);



        inline uint16_t GetFullOffset();



        void SetError(uint16_t error);

        void CleanErrors();

        uint16_t* GetErrors();

        uint8_t GetNumErrors();

        

        #ifdef ESP32

            void SaveChangeValue(uint8_t amount, uint8_t* numArr, void** ptrArr, uint8_t* typeArr);

            void GetSavedValues(uint8_t** infoArrPtr, uint8_t** valArrPtr);

            void DeleteChangeValue(uint8_t amount, uint8_t* ptr);

            void ChangeStartGet(uint8_t amount, uint8_t* ptr, bool en);

            inline uint8_t GetMaxChangeValAmount();

        #endif

    private:

        DataEEPROM(size_t size);

        void CycleWrite(uint16_t error);

        inline uint16_t GetMarkerOffset();

        inline uint16_t GetErrNumOffset();

        inline uint16_t GetErrorOffset();

        inline uint16_t GetValOffset();

        static const uint8_t maxErrorNum = 30;
        static const uint8_t maxChangedValNum = 50;
        static const uint8_t validMarker = 123;

        uint8_t errNum = 0;

        typedef struct 
        {
            uint8_t cycleErrNum = 0;
            uint16_t error[maxErrorNum] {};
        } err_struct_t;
        err_struct_t ErrStruct;

        typedef struct 
        {
            uint8_t changedValByte[maxChangedValNum * 4] {};    // (array number * 4) = AppLink vector number
            uint8_t changedValInfo[maxChangedValNum] {};        // array number = AppLink vector number. [0] bit = have val, [1] bit = get val on start
        } val_struct_t;
        val_struct_t ValStruct;

        static DataEEPROM* instance;
};