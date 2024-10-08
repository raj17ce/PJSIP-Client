#include <iostream>
#include <thread>
#include <pjsua2.hpp>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>
#include "MyAccount.h"
#include "MyCall.h"
#include "MyVideo.h"
#include "MyVideoStreamer.h"
#include "MyAudioStreamer.h"

using namespace pj;
using namespace std;

int main()
{
    Endpoint ep;
    ep.libCreate();

    // Initialize endpoint
    EpConfig ep_cfg;
    ep.libInit(ep_cfg);

    // Create SIP transport. Error handling sample is shown
    TransportConfig tcfg;
    tcfg.port = 5060;
    try
    {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    }
    catch (Error &err)
    {
        std::cout << err.info() << std::endl;
        return 1;
    }

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;
    // pjsua_set_snd_dev(1, -1);

    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:103@192.168.1.200";
    acfg.regConfig.registrarUri = "sip:192.168.1.200";
    AuthCredInfo cred{"digest", "*", "103", 0, "103"};
    acfg.sipConfig.authCreds.push_back(cred);

    // Default Device Setup & Call Media Setup
    acfg.videoConfig.autoShowIncoming = PJ_TRUE;
    acfg.videoConfig.autoTransmitOutgoing = PJ_TRUE;
    acfg.videoConfig.defaultCaptureDevice = -1;
    acfg.videoConfig.defaultRenderDevice = -2;

    MyVideo::listVideoDevices();
    // Create the account
    MyAccount *acc = new MyAccount;
    acc->create(acfg);

    // 3 Sec Delay
    pj_thread_sleep(3000);

    // Setting Capture device formate
    //  VidDevManager &vid_mgr = Endpoint::instance().vidDevManager();
    //  MediaFormatVideo format;

    // // Set the desired resolution (1280x720) and frame rate (30 FPS)
    // format.width = 1280;
    // format.height = 720;
    // format.fpsNum = 30;
    // format.fpsDenum = 1;

    // // Apply the format to the video device (0 is the default device)
    // vid_mgr.setFormat(1, format, PJ_TRUE);
    MyVideo::listVideoDevices();
    // MyVideo::listAudioDevices();

    // int aud , cap;
    // int *p1=&aud , *p2=&cap;
    // pjsua_get_snd_dev(p1 , p2);

    // std::cout<<"-------------------------------------------------------------------------------------------------- "<<aud<<" --------------------------- "<<cap<<std::endl;

    // Making Call
    std::string dest_uri{"sip:104@192.168.1.200"};
    Call *call1 = new MyCall(*acc);
    CallOpParam prm(true); // Use default call settings

    // Enabling Video and Audio for Call
    prm.opt.audioCount = 1;
    prm.opt.videoCount = 1;

    try
    {
        call1->makeCall(dest_uri, prm);
    }
    catch (Error &err)
    {
        std::cout << err.info() << std::endl;
    }   

    pj_thread_sleep(5000);
    AudioMedia media = call1->getAudioMedia(-1);


    MyAudioStreamer::Send_Audio_stream(media.getPortId());

    // pj_pool_t *pool;
    // pool = pj_pool_create(pjsua_get_pool_factory(), NULL, 4000, 4000, NULL);
    // // struct thread_param tparam = {0};
    // pj_thread_t *audio_thread = NULL;

    // pj_status_t status = pj_thread_create(pool, "audio", &StreamAudio, pool,
    //                                0, 0, &audio_thread);
    

    // VideoMedia vid_enc_med1 = call1->getEncodingVideoMedia(-1);

    // int port;

    // port = vid_enc_med1.getPortInfo().portId;

    // std::cout<<"Port Id : "<<port<<std::endl;
    // MyVideoStreamer::read_Send_Video_stream(port);



    // 3 Sec Delay
    //    dest_uri = "sip:102@192.168.1.200";
    //    Call *call2 = new MyCall(*acc);

    //    try {
    //        call2->makeCall(dest_uri, prm);
    //    } catch (Error &err) {
    //        std::cout << err.info() << std::endl;
    //    }

    // pj_thread_sleep(15000);

    //   VideoMedia vid_enc_med1 = call1->getEncodingVideoMedia(-1);
    //   VideoMedia vid_dec_med1 = call1->getDecodingVideoMedia(-1);

    //    VideoMedia vid_enc_med2 = call2->getEncodingVideoMedia(-1);
    //    VideoMedia vid_dec_med2 = call2->getDecodingVideoMedia(-1);

    //    vid_dec_med1.startTransmit(vid_enc_med2, VideoMediaTransmitParam{});
    //    vid_dec_med2.startTransmit(vid_enc_med1, VideoMediaTransmitParam{});

    //    AudioMedia aud_med1 = call1->getAudioMedia(-1);
    //    AudioMedia aud_med2 = call2->getAudioMedia(-1);

    //    aud_med1.startTransmit(aud_med2);
    //    aud_med2.startTransmit(aud_med1);

    // MyVideo::listVideoDevices();

    // End-Delay
    pj_thread_sleep(1000000);


    // Delete the account. This will unregister from server
    delete acc;

    // This will implicitly shutdown the library
    return 0;
}
