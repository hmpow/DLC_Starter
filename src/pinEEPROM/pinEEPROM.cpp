#include "pinEEPROM.h"

PinEEPROM::PinEEPROM() : EEPROM_SIZE(EEPROM.length()), DRIVER_NUM(DRIVER_LIST_NUM){ 
    return;
}

void PinEEPROM::clearPin(const uint8_t indexNum){
    updatePin(indexNum, {EEPROM_UNSET,EEPROM_UNSET,EEPROM_UNSET,EEPROM_UNSET});
    return;
}

void PinEEPROM::updatePin(const uint8_t indexNum, const type_EEPROM_PIN pin){

    uint16_t startPos = indexNum * EEPROM_PIN_LENGTH;

    if(EEPROM_SIZE < startPos + EEPROM_PIN_LENGTH){
        return;
    }

    for(int i = 0; i < EEPROM_PIN_LENGTH; i++){
        EEPROM.update(startPos + i, (char)pin[i]);
    }

    return;
}

type_EEPROM_PIN PinEEPROM::getPin(const uint8_t indexNum){
    
    type_EEPROM_PIN pin = {EEPROM_UNSET,EEPROM_UNSET,EEPROM_UNSET,EEPROM_UNSET};
    
    uint16_t startPos = indexNum * EEPROM_PIN_LENGTH;

    if(EEPROM_SIZE < startPos + EEPROM_PIN_LENGTH){
        return pin;
    }

    for(int i = 0; i < EEPROM_PIN_LENGTH; i++){
        pin[i] = (uint8_t)EEPROM.read(startPos + i);
    }

    return pin;
}

bool PinEEPROM::isSetPin(const uint8_t indexNum){
    
    uint16_t startPos = indexNum * EEPROM_PIN_LENGTH;

    if(EEPROM_SIZE < startPos + EEPROM_PIN_LENGTH){
        return false;
    }

    for(int i = 0; i < EEPROM_PIN_LENGTH; i++){
        if(EEPROM.read(startPos + i) == EEPROM_UNSET){
            return false;
        }
    }

    return true;
}

void PinEEPROM::debugPrintEEPROM(const unsigned int printLength){

    Serial.println("EEPROM.length(): " );
    Serial.println(EEPROM_SIZE);

    unsigned int pLen = 0;

    if(printLength < EEPROM_SIZE){
        pLen = printLength;
    }else{
        pLen = EEPROM_SIZE;
    }

    for(int i = 0; i < pLen; i++){

        printf("EEPROM[%2d]: ",i);
        printf("%02X",EEPROM.read(i));
        if(i % EEPROM_PIN_LENGTH == 3){
            printf("\r\n");
        }else{
            printf(" ");
        }
    }
    printf("\r\n");

    return;
}