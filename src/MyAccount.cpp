#include "MyAccount.h"

void MyAccount::onRegState(OnRegStateParam &prm) {
    AccountInfo ai = getInfo();
    std::cout << (ai.regIsActive ? "*** Register:" : "*** Unregister:")
              << " code=" << prm.code << std::endl;
}