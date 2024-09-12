#include <iostream>
#include <pjsua2.hpp>

using namespace pj;

class MyCall : public Call {
   public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID)
        : Call(acc, call_id) {}

    ~MyCall() {}

    // Notification when call's state has changed.
    virtual void onCallState(OnCallStateParam &prm);

    // Notification when call's media state has changed.
    virtual void onCallMediaState(OnCallMediaStateParam &prm);
};