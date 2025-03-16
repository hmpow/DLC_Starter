#ifndef PINEEPROM_H
#define PINEEPROM_H

#include <Arduino.h>
#include <EEPROM.h>
#include <array>

#define EEPROM_PIN_LENGTH 4
#define EEPROM_UNSET 0xFF
extern const uint8_t DRIVER_LIST_NUM;

typedef std::array<uint8_t, EEPROM_PIN_LENGTH> type_EEPROM_PIN;

class PinEEPROM
{
    public:
        PinEEPROM();
        
        void clearPin(const uint8_t);
        void updatePin(const uint8_t, const type_EEPROM_PIN);
        type_EEPROM_PIN getPin(const uint8_t);
        bool isSetPin(const uint8_t);
        void debugPrintEEPROM(const unsigned int);

    private:
        const uint16_t EEPROM_SIZE;
        const uint16_t DRIVER_NUM; 
};

#endif // PINEEPROM_H