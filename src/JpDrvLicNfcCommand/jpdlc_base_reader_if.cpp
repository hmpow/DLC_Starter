#include "jpdlc_base_reader_if.h"

//カードリーダのインスタンスはmain.cppの持ち物なのでポインタで
Rcs660sAppIf *p_rcs660sInstance;

void setReaderInstance(Rcs660sAppIf * const p_reader){
    p_rcs660sInstance = p_reader;
    return;
}

std::vector<type_data_byte> _nfcTransceive(const std::vector<type_data_byte> inputCommand){
    
    std::vector<type_data_byte> command;
    std::vector<type_data_byte> retResponse;

    const uint16_t comLen = inputCommand.size();

    if(comLen > READER_MAX_COMMAND_LEN){
        //カードリーダーが対応する最大長を超えている
        return retResponse; //空のvector
    }

    for(uint16_t i = 0; i < comLen; i++){
        command.push_back( (type_data_byte)inputCommand[i] );
    }

#ifdef DLC_LAYER_DEBUG
    printf("Tx To Card : ");
    for(const auto& comByte : command){
        printf("%02X ",comByte);
    }
    printf("\r\n");
#endif

    std::vector<uint8_t> orgResponse;
    orgResponse = p_rcs660sInstance->communicateNfc(command, comLen);  

#ifdef DLC_LAYER_DEBUG
    printf("Rx From Card : ");
    for(const auto& comByte : orgResponse){
        printf("%02X ",comByte);
    }
    printf("\r\n");
    printf("\r\n");
#endif

    for(const auto& orgResByte : orgResponse){
        retResponse.push_back( (type_data_byte)orgResByte );
    }

    return retResponse;
}

