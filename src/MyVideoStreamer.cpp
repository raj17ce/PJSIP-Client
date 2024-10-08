#include "../include/MyVideoStreamer.h"

pj_status_t MyVideoStreamer::init_Video_codecs(pj_pool_factory *pf){
    pj_status_t status;

    /* To suppress warning about unused var when all codecs are disabled */
    PJ_UNUSED_ARG(status);
        PJ_UNUSED_ARG(pf);

    #if defined(PJMEDIA_HAS_OPENH264_CODEC) && PJMEDIA_HAS_OPENH264_CODEC != 0
        status = pjmedia_codec_openh264_vid_init(NULL, pf);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
    #endif

    #if defined(PJMEDIA_HAS_VID_TOOLBOX_CODEC) && \
        PJMEDIA_HAS_VID_TOOLBOX_CODEC != 0
        status = pjmedia_codec_vid_toolbox_init(NULL, pf);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
    #endif

    #if defined(PJMEDIA_HAS_VPX_CODEC) && PJMEDIA_HAS_VPX_CODEC != 0
        status = pjmedia_codec_vpx_vid_init(NULL, pf);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
    #endif

    #if defined(PJMEDIA_HAS_FFMPEG_VID_CODEC) && PJMEDIA_HAS_FFMPEG_VID_CODEC != 0
        status = pjmedia_codec_ffmpeg_vid_init(NULL, pf);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
    #endif

    return PJ_SUCCESS;
}


void MyVideoStreamer::deinit_Video_codecs(){
    #if defined(PJMEDIA_HAS_FFMPEG_VID_CODEC) && PJMEDIA_HAS_FFMPEG_VID_CODEC != 0
        pjmedia_codec_ffmpeg_vid_deinit();
    #endif

    #if defined(PJMEDIA_HAS_OPENH264_CODEC) && PJMEDIA_HAS_OPENH264_CODEC != 0
        pjmedia_codec_openh264_vid_deinit();
    #endif

    #if defined(PJMEDIA_HAS_VID_TOOLBOX_CODEC) && \
        PJMEDIA_HAS_VID_TOOLBOX_CODEC != 0
        pjmedia_codec_vid_toolbox_deinit();
    #endif

    #if defined(PJMEDIA_HAS_VPX_CODEC) && PJMEDIA_HAS_VPX_CODEC != 0
        pjmedia_codec_vpx_vid_deinit();
    #endif
}

pj_status_t MyVideoStreamer::create_Video_stream( pj_pool_t *pool,pjmedia_endpt *med_endpt,const pjmedia_vid_codec_info *codec_info,pjmedia_vid_codec_param *codec_param,     pjmedia_dir dir,pj_int8_t rx_pt,pj_int8_t tx_pt,pj_uint16_t local_port,const pj_sockaddr_in *rem_addr,pjmedia_vid_stream **p_stream ) {
    pjmedia_vid_stream_info info;
    pjmedia_transport *transport = NULL;
    pj_status_t status;

    /* Reset stream info. */
    pj_bzero(&info, sizeof(info));

    /* Initialize stream info formats */
    info.type = PJMEDIA_TYPE_VIDEO;
    info.dir = dir;
    info.codec_info = *codec_info;
    info.tx_pt = (tx_pt == -1)? (pj_uint8_t)codec_info->pt : tx_pt;
    info.rx_pt = (rx_pt == -1)? (pj_uint8_t)codec_info->pt : rx_pt;
    info.ssrc = pj_rand();
    if (codec_param)
        info.codec_param = codec_param;
    
    /* Copy remote address */
    pj_memcpy(&info.rem_addr, rem_addr, sizeof(pj_sockaddr_in));

    /* If remote address is not set, set to an arbitrary address
     * (otherwise stream will assert).
     */
    if (info.rem_addr.addr.sa_family == 0) {
        const pj_str_t addr = pj_str("127.0.0.1");
        pj_sockaddr_in_init(&info.rem_addr.ipv4, &addr, 0);
    }

    /* Create media transport */
    status = pjmedia_transport_udp_create(med_endpt, NULL, local_port,
                                          0, &transport);
    if (status != PJ_SUCCESS)
        return status;


    /* Now that the stream info is initialized, we can create the 
     * stream.
     */

    status = pjmedia_vid_stream_create( med_endpt, pool, &info, 
                                        transport, 
                                        NULL, p_stream);

    if (status != PJ_SUCCESS) {
        std::cout<<"Error creating stream"<<endl;
        pjmedia_transport_close(transport);
        return status;
    }

    /* Start media transport */
    pjmedia_transport_media_start(transport, 0, 0, 0, 0);

    return PJ_SUCCESS;
}

int MyVideoStreamer::read_Send_Video_stream(int conf_port){
    pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    pj_pool_t *pool;
    pjmedia_vid_stream *stream = NULL;
    pjmedia_port *enc_port, *dec_port;
    char addr[PJ_INET_ADDRSTRLEN];
    pj_status_t status; 

    pjmedia_vid_port *capture=NULL, *renderer=NULL;
    pjmedia_vid_port_param vpp;

    const pjmedia_vid_codec_info *codec_info;
    pjmedia_vid_codec_param codec_param;
    pjmedia_dir dir = PJMEDIA_DIR_DECODING;
    pj_sockaddr_in remote_addr;
    pj_uint16_t local_port = 6000;
    char *codec_id = "H264";
    pjmedia_rect_size tx_size = {0};
    pj_int8_t rx_pt = 100, tx_pt = -1;

    // play_file_data play_file = { NULL };
    pjmedia_port *play_port = NULL;
    pjmedia_vid_codec *play_decoder = NULL;
    pjmedia_clock *play_clock = NULL;



    int c;
    int option_index;

    pj_bzero(&remote_addr, sizeof(remote_addr));

    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pj_optind = 0;


    if (dir & PJMEDIA_DIR_ENCODING) {
        if (remote_addr.sin_addr.s_addr == 0) {
            printf("Error: remote address must be set\n");
            return 1;
        }
    }

    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

     pool = pj_pool_create( &cp.factory,     /* pool factory         */
                           "app",           /* pool name.           */
                           4000,            /* init size            */
                           4000,            /* increment size       */
                           NULL             /* callback on error    */
                           );

    /* Init video format manager */
    pjmedia_video_format_mgr_create(pool, 64, 0, NULL);

    /* Init video converter manager */
    pjmedia_converter_mgr_create(pool, NULL);

    /* Init event manager */
    pjmedia_event_mgr_create(pool, 0, NULL);

    /* Init video codec manager */
    pjmedia_vid_codec_mgr_create(pool, NULL);

    /* Init video subsystem */
    pjmedia_vid_dev_subsys_init(&cp.factory);

    /* Register all supported codecs */
    status = init_Video_codecs(&cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    if (codec_id) {
        unsigned count = 1;
        pj_str_t str_codec_id = pj_str(codec_id);

        status = pjmedia_vid_codec_mgr_find_codecs_by_id(NULL,
                                                         &str_codec_id, &count,
                                                         &codec_info, NULL);
        if (status != PJ_SUCCESS) {
            printf("Error: unable to find codec %s\n", codec_id);
            return 1;
        }
    } else {
        static pjmedia_vid_codec_info info[1];
        unsigned count = PJ_ARRAY_SIZE(info);

        /* Default to first codec */
        pjmedia_vid_codec_mgr_enum_codecs(NULL, &count, info, NULL);
        codec_info = &info[0];
    }

    /* Get codec default param for info */
    status = pjmedia_vid_codec_mgr_get_default_param(NULL, codec_info, 
                                                     &codec_param);
    pj_assert(status == PJ_SUCCESS);
    
    /* Set outgoing video size */
    if (tx_size.w && tx_size.h)
        codec_param.enc_fmt.det.vid.size = tx_size;

#if DEF_RENDERER_WIDTH && DEF_RENDERER_HEIGHT
    /* Set incoming video size */
    if (DEF_RENDERER_WIDTH > codec_param.dec_fmt.det.vid.size.w)
        codec_param.dec_fmt.det.vid.size.w = DEF_RENDERER_WIDTH;
    if (DEF_RENDERER_HEIGHT > codec_param.dec_fmt.det.vid.size.h)
        codec_param.dec_fmt.det.vid.size.h = DEF_RENDERER_HEIGHT;
#endif


    pjmedia_vid_port_param_default(&vpp);

        /* Set as active for all video devices */
        vpp.active = PJ_TRUE;

        /* Create video device port. */
        if (dir & PJMEDIA_DIR_ENCODING) {
            /* Create capture */
            status = pjmedia_vid_dev_default_param(
                                        pool,
                                        PJMEDIA_VID_DEFAULT_CAPTURE_DEV,
                                        &vpp.vidparam);
            if (status != PJ_SUCCESS){}
                // goto on_exit;

            pjmedia_format_copy(&vpp.vidparam.fmt, &codec_param.enc_fmt);
            vpp.vidparam.fmt.id = codec_param.dec_fmt.id;
            vpp.vidparam.dir = PJMEDIA_DIR_CAPTURE;
            
            status = pjmedia_vid_port_create(pool, &vpp, &capture);
            if (status != PJ_SUCCESS){}
                // goto on_exit;
        }
        
        if (dir & PJMEDIA_DIR_DECODING) {
            /* Create renderer */
            status = pjmedia_vid_dev_default_param(
                                        pool,
                                        PJMEDIA_VID_DEFAULT_RENDER_DEV,
                                        &vpp.vidparam);
            if (status != PJ_SUCCESS){}
                // goto on_exit;

            pjmedia_format_copy(&vpp.vidparam.fmt, &codec_param.dec_fmt);
            vpp.vidparam.dir = PJMEDIA_DIR_RENDER;
            vpp.vidparam.disp_size = vpp.vidparam.fmt.det.vid.size;
            vpp.vidparam.flags |= PJMEDIA_VID_DEV_CAP_OUTPUT_WINDOW_FLAGS;
            vpp.vidparam.window_flags = PJMEDIA_VID_DEV_WND_BORDER |
                                        PJMEDIA_VID_DEV_WND_RESIZABLE;

            status = pjmedia_vid_port_create(pool, &vpp, &renderer);
            if (status != PJ_SUCCESS){}
                // goto on_exit;
        }

        codec_param.ignore_fmtp = PJ_TRUE;

        status = create_Video_stream(pool, med_endpt, codec_info, &codec_param,
                           dir, rx_pt, tx_pt, local_port, &remote_addr,&stream);

        if (status != PJ_SUCCESS){}
        // goto on_exit;

    /* Get the port interface of the stream */
    status = pjmedia_vid_stream_get_port(stream, PJMEDIA_DIR_ENCODING,
                                         &enc_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status = pjmedia_vid_stream_get_port(stream, PJMEDIA_DIR_DECODING,
                                         &dec_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Start streaming */
    status = pjmedia_vid_stream_start(stream);
    if (status != PJ_SUCCESS){}
        // goto on_exit;

    pjsua_conf_port_id id;
    status = pjsua_vid_conf_add_port(pool , dec_port , NULL , &id);
    status = pjsua_vid_conf_connect(id , conf_port , NULL );
    status = pjsua_vid_conf_disconnect(3,conf_port);

    if (status != PJ_SUCCESS)
        std::cout<<"------------------------------------------Can not connect to conference call -------------------------------------------------------"<<endl;
    /* Start renderer */
    // if (dest_port) {
    //     status = pjmedia_vid_port_connect(dest_port, dec_port, PJ_FALSE);
    //     if (status != PJ_SUCCESS)
    //         goto on_exit;
    //     status = pjmedia_vid_port_start(dest_port);
    //     if (status != PJ_SUCCESS)
    //         goto on_exit;
    // }

    if (dir == PJMEDIA_DIR_DECODING)
        printf("Stream is active, dir is recv-only, local port is %d\n",
               local_port);
    else if (dir == PJMEDIA_DIR_ENCODING)
        printf("Stream is active, dir is send-only, sending to %s:%d\n",
               pj_inet_ntop2(pj_AF_INET(), &remote_addr.sin_addr, addr,
                             sizeof(addr)),
               pj_ntohs(remote_addr.sin_port));
    else
        printf("Stream is active, send/recv, local port is %d, "
               "sending to %s:%d\n",
               local_port,
               pj_inet_ntop2(pj_AF_INET(), &remote_addr.sin_addr, addr,
                             sizeof(addr)),
               pj_ntohs(remote_addr.sin_port));

    for (;;) {
        char tmp[10];

        puts("");
        puts("Commands:");
        puts("  q     Quit");
        puts("");

        printf("Command: "); fflush(stdout);

        if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (tmp[0] == 'q')
            break;

    }

// on_exit:

    /* Stop video devices */
    if (capture)
        pjmedia_vid_port_stop(capture);
    if (renderer)
        pjmedia_vid_port_stop(renderer);

    /* Stop and destroy file clock */
    if (play_clock) {
        pjmedia_clock_stop(play_clock);
        pjmedia_clock_destroy(play_clock);
    }

    /* Destroy file reader/player */
    if (play_port)
        pjmedia_port_destroy(play_port);

    /* Destroy file decoder */
    if (play_decoder) {
        play_decoder->op->close(play_decoder);
        pjmedia_vid_codec_mgr_dealloc_codec(NULL, play_decoder);
    }

    /* Destroy video devices */
    if (capture)
        pjmedia_vid_port_destroy(capture);
    if (renderer)
        pjmedia_vid_port_destroy(renderer);

    /* Destroy stream */
    if (stream) {
        pjmedia_transport *tp;

        tp = pjmedia_vid_stream_get_transport(stream);
        pjmedia_vid_stream_destroy(stream);
        
        pjmedia_transport_media_stop(tp);
        pjmedia_transport_close(tp);
    }

    /* Deinit codecs */
    deinit_Video_codecs();

    /* Shutdown video subsystem */
    pjmedia_vid_dev_subsys_shutdown();

    /* Destroy event manager */
    pjmedia_event_mgr_destroy(NULL);

    /* Release application pool */
    pj_pool_release( pool );

    /* Destroy media endpoint. */
    pjmedia_endpt_destroy( med_endpt );

    /* Destroy pool factory */
    pj_caching_pool_destroy( &cp );

    /* Shutdown PJLIB */
    pj_shutdown();
}