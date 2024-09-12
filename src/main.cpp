#include <iostream>
#include <pjsua2.hpp>
#include "MyAccount.h"
#include "MyCall.h"
#include "MyVideo.h"

using namespace pj;

int main() {
    Endpoint ep;
    ep.libCreate();

    // Initialize endpoint
    EpConfig ep_cfg;
    ep.libInit(ep_cfg);

    // Create SIP transport. Error handling sample is shown
    TransportConfig tcfg;
    tcfg.port = 5060;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:203@192.168.1.200";
    acfg.regConfig.registrarUri = "sip:192.168.1.200";
    AuthCredInfo cred{"digest", "*", "203", 0, "203"};
    acfg.sipConfig.authCreds.push_back(cred);

    // Default Device Setup & Call Media Setup
    acfg.videoConfig.autoShowIncoming = PJ_TRUE;
    acfg.videoConfig.autoTransmitOutgoing = PJ_TRUE;
    acfg.videoConfig.defaultCaptureDevice = -1;
    acfg.videoConfig.defaultRenderDevice = -2;

    // Create the account
    MyAccount *acc = new MyAccount;
    acc->create(acfg);

    // 3 Sec Delay
    pj_thread_sleep(3000);

    //Setting Capture device formate
    // VidDevManager &vid_mgr = Endpoint::instance().vidDevManager();
    // MediaFormatVideo format;

    // // Set the desired resolution (1280x720) and frame rate (30 FPS)
    // format.width = 1280;
    // format.height = 720;
    // format.fpsNum = 30;
    // format.fpsDenum = 1;

    // // Apply the format to the video device (0 is the default device)
    // vid_mgr.setFormat(1, format, PJ_TRUE);

    //Making Call
    std::string dest_uri{"sip:202@192.168.1.200"};
    Call *call = new MyCall(*acc);
    CallOpParam prm(true);  // Use default call settings

    // Enabling Video and Audio for Call
    prm.opt.audioCount = 1;
    prm.opt.videoCount = 1;

    try {
        call->makeCall(dest_uri, prm);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
    }

    //MyVideo::listVideoDevices();

    // End-Delay
    pj_thread_sleep(1000000);

    // Delete the account. This will unregister from server
    delete acc;

    // This will implicitly shutdown the library
    return 0;
}
