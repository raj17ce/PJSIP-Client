#include <iostream>
#include <pjsua2.hpp>

using namespace pj;

class MyAccount : public Account {
   public:
    virtual void onRegState(OnRegStateParam &prm);
};