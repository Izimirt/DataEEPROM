#include "DataEEPROM.h"

DataEEPROM* DataEEPROM::instance = nullptr;

DataEEPROM::DataEEPROM(size_t size)
{    
    #ifdef ESP32
        EEPROM.begin(size);
    #endif
    delay(10);
    uint8_t marker = 0;

    EEPROM.get(GetMarkerOffset(),marker);

    if(marker != validMarker)
    {
        EEPROM.get(GetErrNumOffset(),errNum);
        EEPROM.get(GetErrorOffset(),ErrStruct);
        EEPROM.get(GetValOffset(),ValStruct);

        marker = validMarker;
        errNum = 0;

        ErrStruct.cycleErrNum = 0;
        for(int i = 0; i < maxErrorNum; i++)
            ErrStruct.error[i] = 0;

        for(int i = 0; i < (maxChangedValNum * 4); i++)
            ValStruct.changedValByte[i] = 0;
        for(int i = 0; i < maxChangedValNum; i++)
            ValStruct.changedValInfo[i] = 0b00000010;

        EEPROM.put(GetMarkerOffset(),marker);
        EEPROM.put(GetErrNumOffset(),errNum);
        EEPROM.put(GetErrorOffset(),ErrStruct);
        EEPROM.put(GetValOffset(),ValStruct);

        #ifdef ESP32
            EEPROM.commit();
        #endif
        delay(10);
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
    EEPROM.get(GetErrNumOffset(),errNum);
    EEPROM.get(GetErrorOffset(),ErrStruct);

    CycleWrite(error);

    EEPROM.put(GetErrNumOffset(),errNum);
    EEPROM.put(GetErrorOffset(),ErrStruct);

    delay(10);
    #ifdef ESP32 
        EEPROM.commit();
    #endif
}

void DataEEPROM::CycleWrite(uint16_t error)
{
    ErrStruct.error[ErrStruct.cycleErrNum] = error;
    ErrStruct.error[(ErrStruct.cycleErrNum + 1) % maxErrorNum] = 0;
    ErrStruct.cycleErrNum = (ErrStruct.cycleErrNum + 1) % maxErrorNum;
    if(errNum < maxErrorNum)
        errNum++;
}

inline uint16_t DataEEPROM::GetMarkerOffset()
{
    return 0;
}

inline uint16_t DataEEPROM::GetErrNumOffset()
{
    return (GetMarkerOffset() + 1);
}

uint16_t DataEEPROM::GetErrorOffset()
{
    return (GetErrNumOffset() + 1);
}

uint16_t DataEEPROM::GetValOffset()
{
    return (GetErrorOffset() + (1 + (maxErrorNum * 2)));
}

uint16_t DataEEPROM::GetFullOffset()
{
    return (GetValOffset() + (maxChangedValNum * 5));
}

void DataEEPROM::CleanErrors()
{
    EEPROM.get(GetErrNumOffset(),errNum);
    EEPROM.get(GetErrorOffset(),ErrStruct);

    errNum = 0;
    ErrStruct.cycleErrNum = 0;
    for(int i = 0; i < maxErrorNum; i++)
        ErrStruct.error[i] = 0;

    EEPROM.put(GetErrNumOffset(),errNum);
    EEPROM.put(GetErrorOffset(),ErrStruct);

    delay(10);
    #ifdef ESP32
        EEPROM.commit();
    #endif
}

uint16_t* DataEEPROM::GetErrors()
{
    EEPROM.get(GetErrorOffset(),ErrStruct);
    return ErrStruct.error;
}

uint8_t DataEEPROM::GetNumErrors()
{
    EEPROM.get(GetErrNumOffset(),errNum);
    return errNum;
}

#ifdef ESP32

    void DataEEPROM::SaveChangeValue(uint8_t amount, uint8_t* numArr, void** ptrArr, uint8_t* typeArr)
    {
        EEPROM.get(GetValOffset(),ValStruct);

        for (int i = 0; i < amount; i++)
        {
            uint8_t num = numArr[i];
            void* ptr = ptrArr[i];
            uint8_t type = typeArr[i];

            if(num > maxChangedValNum)
                continue;
            if(ptr == nullptr)
                continue;

            ValStruct.changedValInfo[num] |= 0b00000001;

            *(uint32_t*)(ValStruct.changedValByte + (num * 4)) = 0;    // for cleaning junk

            if(type == 1)
                *(int8_t*)(ValStruct.changedValByte + (num * 4)) = *(int8_t*)ptr;
            else if(type == 2)
                *(uint8_t*)(ValStruct.changedValByte + (num * 4)) = *(uint8_t*)ptr;
            else if(type == 3)
                *(int16_t*)(ValStruct.changedValByte + (num * 4)) = *(int16_t*)ptr;
            else if(type == 4)
                *(uint16_t*)(ValStruct.changedValByte + (num * 4)) = *(uint16_t*)ptr;
            else if(type == 5)
                *(int32_t*)(ValStruct.changedValByte + (num * 4)) = *(int32_t*)ptr;
            else if(type == 6)
                *(uint32_t*)(ValStruct.changedValByte + (num * 4)) = *(uint32_t*)ptr;
            else if(type == 7)
                *(float*)(ValStruct.changedValByte + (num * 4)) = *(float*)ptr;
        }

        EEPROM.put(GetValOffset(),ValStruct);

        delay(10);
            
        EEPROM.commit();
    }

    void DataEEPROM::GetSavedValues(uint8_t** infoArrPtr, uint8_t** valArrPtr)
    {
        EEPROM.get(GetValOffset(),ValStruct);

        *infoArrPtr = ValStruct.changedValInfo;
        *valArrPtr = ValStruct.changedValByte;
    }

    void DataEEPROM::DeleteChangeValue(uint8_t amount, uint8_t* ptr)
    {
        EEPROM.get(GetValOffset(),ValStruct);

        for (int i = 0; i < amount; i++)
        {
            uint8_t num = ptr[i];
            if(num > maxChangedValNum)
                continue;

            ValStruct.changedValInfo[num] &= (~0b00000001);
            *(uint32_t*)(ValStruct.changedValByte + (num * 4)) = 0;    // for cleaning junk
        }

        EEPROM.put(GetValOffset(),ValStruct);

        delay(10);
            
        EEPROM.commit();
    }

    void DataEEPROM::ChangeStartGet(uint8_t amount, uint8_t* ptr, bool en)
    {
        EEPROM.get(GetValOffset(),ValStruct);

        for (int i = 0; i < amount; i++)
        {
            uint8_t num = ptr[i];
            if(num > maxChangedValNum)
                continue;

            if (en)
                ValStruct.changedValInfo[num] |= 0b00000010;
            else
                ValStruct.changedValInfo[num] &= (~0b00000010);
        }

        EEPROM.put(GetValOffset(),ValStruct);

        delay(10);
            
        EEPROM.commit();
    }

    inline uint8_t DataEEPROM::GetMaxChangeValAmount()
    {
        return maxChangedValNum;
    }

#endif