#include "MyCall.h"

void MyCall::onCallState(OnCallStateParam &prm) {
    CallInfo ci = getInfo();
    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        /* Delete the call */
        delete this;
    }
}

void MyCall::onCallMediaState(OnCallMediaStateParam &prm) {
    CallInfo ci = getInfo();

    for (unsigned i = 0; i < ci.media.size(); i++) {
        if (ci.media[i].type == PJMEDIA_TYPE_AUDIO) {
            try {
                AudioMedia aud_med = getAudioMedia(i);

                // Connect the call audio media to sound device
                AudDevManager &mgr = Endpoint::instance().audDevManager();
                aud_med.startTransmit(mgr.getPlaybackDevMedia());
                mgr.getCaptureDevMedia().startTransmit(aud_med);
            } catch (const Error &e) {
                // Handle invalid or not audio media error here
            }
        } else if (ci.media[i].type == PJMEDIA_TYPE_VIDEO) {
            try {
                
            } catch (const Error &e) {
                // Handle invalid or not audio media error here
            }
        }
    }
}