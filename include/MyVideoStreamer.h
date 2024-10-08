#include <iostream>
#include <pjsua2.hpp>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>

using namespace pj;
using namespace std;

#define DEF_RENDERER_WIDTH                  200
#define DEF_RENDERER_HEIGHT                 200

#define HAS_LOCAL_RENDERER_FOR_PLAY_FILE    1
#define PJMEDIA_HAS_FFMPEG_VID_CODEC 1

class MyVideoStreamer {
    public: 
        
        // Video Streaming
        static pj_status_t init_Video_codecs(pj_pool_factory *pf);
        
        static void deinit_Video_codecs(); 
        
        static pj_status_t create_Video_stream( pj_pool_t *pool,pjmedia_endpt *med_endpt,const pjmedia_vid_codec_info *codec_info,pjmedia_vid_codec_param *codec_param,     pjmedia_dir dir,pj_int8_t rx_pt,pj_int8_t tx_pt,pj_uint16_t local_port,const pj_sockaddr_in *rem_addr,pjmedia_vid_stream **p_stream );

        static int read_Send_Video_stream(int conf_port);
};