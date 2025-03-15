#ifndef JP_DRV_LIC_NFC_COMMAND_CONVENTIONAL_H
#define JP_DRV_LIC_NFC_COMMAND_CONVENTIONAL_H

#include "jpdlc_base.h"

/***************/
/* 従来型免許証 */
/***************/

/* AIDなどは定数宣言のためcppファイルで定義 */

class JpDrvLicNfcCommandConventional : public JpDrvLicNfcCommandBase{
    public:
    JPDLC_ISSET_PIN_STATUS issetPin(void) override;
    bool isDrvLicCard(void) override;
    JPDLC_EXPIRATION_DATA getExpirationData(void) override;
    uint8_t getRemainingCount(void) override;
    bool executeVerify(type_PIN) override;

//従来型特化
    JPDLC_EXPIRATION_DATA getExpirationData_from_DF1_EF01(void);   //認証必要なEF側から有効期限情報を取得 

    //JIS X 6320-4 p.78 より必ずMFに飛べる "3F00" 付きを使用する
    std::vector<type_data_byte> assemblyCommandSelectFile_MF_Case3(void);

    uint8_t packedBCDtoInt(type_data_byte);


};

#endif // JP_DRV_LIC_NFC_COMMAND_CONVENTIONAL_H
