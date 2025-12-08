#include "DataEEPROM.h"

DataEEPROM* DataEEPROM::instance = nullptr;

DataEEPROM::DataEEPROM(size_t size)
{    
    #ifdef ESP32
        EEPROM.begin(size);
    #endif
    delay(100);
    EEPROM.get(0,StructDataEEPROM);
    if(StructDataEEPROM.marker != validMarker)
    {
        StructDataEEPROM.marker = validMarker;
        StructDataEEPROM.errNum = 0;
        StructDataEEPROM.cycleErrNum = 0;
        StructDataEEPROM.changedValNumLenght = 0;
        for(int i = 0; i < maxErrorNum; i++)
            StructDataEEPROM.error[i] = 0;
        for(int i = 0; i < maxChangedValNum * 4; i++)
            StructDataEEPROM.changedValByte[i] = 0;
        for(int i = 0; i < maxChangedValNum; i++)
            StructDataEEPROM.changedValNum[i] = 0;
        EEPROM.put(0,StructDataEEPROM);
        #ifdef ESP32
            EEPROM.commit();
        #endif
        delay(100);
    }
}

DataEEPROM *DataEEPROM::Instance(size_t size)
{
    if(instance == nullptr)
        instance = new DataEEPROM(size);
    return instance;
}

void DataEEPROM::SetError(uint16_t error)
{
    EEPROM.get(0,StructDataEEPROM);
    CycleWrite(error);
    EEPROM.put(0,StructDataEEPROM);
    delay(100);
    #ifdef ESP32 
        EEPROM.commit();
    #endif
}

void DataEEPROM::CycleWrite(uint16_t error)
{
    StructDataEEPROM.error[StructDataEEPROM.cycleErrNum] = error;
    StructDataEEPROM.error[(StructDataEEPROM.cycleErrNum + 1) % maxErrorNum] = 0;
    StructDataEEPROM.cycleErrNum = (StructDataEEPROM.cycleErrNum + 1) % maxErrorNum;
    if(StructDataEEPROM.errNum < maxErrorNum)
        StructDataEEPROM.errNum++;
}

void DataEEPROM::CleanErrors()
{
    EEPROM.get(0,StructDataEEPROM);
    StructDataEEPROM.errNum = 0;
    StructDataEEPROM.cycleErrNum = 0;
    for(int i = 0; i < maxErrorNum; i++)
        StructDataEEPROM.error[i] = 0;
    EEPROM.put(0,StructDataEEPROM);
    delay(100);
    #ifdef ESP32
        EEPROM.commit();
    #endif
}

uint16_t* DataEEPROM::GetErrors()
{
    EEPROM.get(0,StructDataEEPROM);    
    return StructDataEEPROM.error;
}

uint16_t DataEEPROM::GetNumErrors()
{
    EEPROM.get(0,StructDataEEPROM); 
    return StructDataEEPROM.errNum;
}

#ifdef ESP32

    void DataEEPROM::SaveChangeValue(uint8_t num, void* ptr, uint8_t type)
    {
        if(num > maxChangedValNum)
            return;
        if(ptr == nullptr)
            return;

        EEPROM.get(0,StructDataEEPROM);

        if(type == 1)
            *(int8_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(int8_t*)ptr;
        else if(type == 2)
            *(uint8_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(uint8_t*)ptr;
        else if(type == 3)
            *(int16_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(int16_t*)ptr;
        else if(type == 4)
            *(uint16_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(uint16_t*)ptr;
        else if(type == 5)
            *(int32_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(int32_t*)ptr;
        else if(type == 6)
            *(uint32_t*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(uint32_t*)ptr;
        else if(type == 7)
            *(float*)(StructDataEEPROM.changedValByte + (StructDataEEPROM.changedValNumLenght*4)) = *(float*)ptr;

        StructDataEEPROM.changedValNum[StructDataEEPROM.changedValNumLenght] = num;
        StructDataEEPROM.changedValNumLenght++;

        EEPROM.put(0,StructDataEEPROM);
        delay(100);
            EEPROM.commit();
    }

    uint8_t DataEEPROM::GetNumSavedValues()
    {
        EEPROM.get(0,StructDataEEPROM);
        return StructDataEEPROM.changedValNumLenght;
    }

    void* DataEEPROM::GetSavedValue(uint8_t num)
    {
        if(num > maxChangedValNum)
            return nullptr;

        EEPROM.get(0,StructDataEEPROM);
        return StructDataEEPROM.changedValByte + (num*4);
    }

#endif