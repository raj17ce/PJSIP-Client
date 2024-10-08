#include <iostream>
#include <pjsua2.hpp>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>

using namespace pj;
using namespace std;

class MyAudioStreamer {
    public: 
        static pj_status_t init_Audio_codecs(pjmedia_endpt *med_endpt);

        static pj_status_t create_Audio_stream( pj_pool_t *pool,
                                  pjmedia_endpt *med_endpt,
                                  const pjmedia_codec_info *codec_info,
                                  pjmedia_dir dir,
                                  pj_uint16_t local_port,
                                  const pj_sockaddr_in *rem_addr,
                                  pj_bool_t mcast,
                                  const pj_sockaddr_in *mcast_addr,
                                  pjmedia_stream **p_stream );

        static int Send_Audio_stream(int conf_port);
 
};