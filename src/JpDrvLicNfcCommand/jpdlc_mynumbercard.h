#ifndef JP_DRV_LIC_NFC_COMMAND_MYNUMBERCARD_H
#define JP_DRV_LIC_NFC_COMMAND_MYNUMBERCARD_H

#include "jpdlc_base.h"

/***************/
/* マイナ免許証 */
/***************/

// マイナンバーのREAD_BINARY はカレントEF制約あるのでガード節入れること

/* AIDなどは定数宣言のためcppファイルで定義 */

class JpDrvLicNfcCommandMynumber : public JpDrvLicNfcCommandBase{
    public:
    JPDLC_ISSET_PIN_STATUS issetPin(void) override;
    bool isDrvLicCard(void) override;
    JPDLC_EXPIRATION_DATA getExpirationData(void) override;
    uint8_t getRemainingCount(void) override;
    bool executeVerify(type_PIN) override;

    //従来免許をJISX0201形式有効期限のスタブ代わりに使用
    JPDLC_EXPIRATION_DATA stub_getExpirationData(void);
};

#endif // JP_DRV_LIC_NFC_COMMAND_MYNUMBERCARD_H