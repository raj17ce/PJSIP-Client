#include "MyAudioStreamer.h"

pj_status_t MyAudioStreamer::init_Audio_codecs(pjmedia_endpt *med_endpt) {
    return pjmedia_codec_register_audio_codecs(med_endpt, NULL);
}

pj_status_t MyAudioStreamer::create_Audio_stream( pj_pool_t *pool,
                                  pjmedia_endpt *med_endpt,
                                  const pjmedia_codec_info *codec_info,
                                  pjmedia_dir dir,
                                  pj_uint16_t local_port,
                                  const pj_sockaddr_in *rem_addr,
                                  pj_bool_t mcast,
                                  const pj_sockaddr_in *mcast_addr,
                                  pjmedia_stream **p_stream) {

        pjmedia_stream_info info;
    pjmedia_transport *transport = NULL;
    pj_status_t status;

    /* Reset stream info. */
    pj_bzero(&info, sizeof(info));


    /* Initialize stream info formats */
    info.type = PJMEDIA_TYPE_AUDIO;
    info.dir = dir;
    pj_memcpy(&info.fmt, codec_info, sizeof(pjmedia_codec_info));
    info.tx_pt = codec_info->pt;
    info.rx_pt = codec_info->pt;
    info.ssrc = pj_rand();

    /* Copy remote address */
    pj_memcpy(&info.rem_addr, rem_addr, sizeof(pj_sockaddr_in));

    /* If remote address is not set, set to an arbitrary address
     * (otherwise stream will assert).
     */
    if (info.rem_addr.addr.sa_family == 0) {
        const pj_str_t addr = pj_str("127.0.0.1");
        pj_sockaddr_in_init(&info.rem_addr.ipv4, &addr, 0);
    }

    pj_sockaddr_cp(&info.rem_rtcp, &info.rem_addr);
    pj_sockaddr_set_port(&info.rem_rtcp,
                         pj_sockaddr_get_port(&info.rem_rtcp)+1);

    if (mcast) {
        pjmedia_sock_info si;
        int reuse = 1;

        pj_bzero(&si, sizeof(pjmedia_sock_info));
        si.rtp_sock = si.rtcp_sock = PJ_INVALID_SOCKET;

        /* Create RTP socket */
        status = pj_sock_socket(pj_AF_INET(), pj_SOCK_DGRAM(), 0,
                                &si.rtp_sock);
        if (status != PJ_SUCCESS)
            return status;

        status = pj_sock_setsockopt(si.rtp_sock, pj_SOL_SOCKET(),
                                    pj_SO_REUSEADDR(), &reuse, sizeof(reuse));
        if (status != PJ_SUCCESS)
            return status;

        /* Bind RTP socket */
        status = pj_sockaddr_init(pj_AF_INET(), &si.rtp_addr_name,
                                  NULL, local_port);
        if (status != PJ_SUCCESS)
            return status;
    
        status = pj_sock_bind(si.rtp_sock, &si.rtp_addr_name,
                              pj_sockaddr_get_len(&si.rtp_addr_name));
        if (status != PJ_SUCCESS)
            return status;

        /* Create RTCP socket */
        status = pj_sock_socket(pj_AF_INET(), pj_SOCK_DGRAM(), 0,
                                &si.rtcp_sock);
        if (status != PJ_SUCCESS)
            return status;

        status = pj_sock_setsockopt(si.rtcp_sock, pj_SOL_SOCKET(),
                                    pj_SO_REUSEADDR(), &reuse, sizeof(reuse));
        if (status != PJ_SUCCESS)
            return status;

        /* Bind RTCP socket */
        status = pj_sockaddr_init(pj_AF_INET(), &si.rtcp_addr_name,
                                  NULL, local_port+1);
        if (status != PJ_SUCCESS)
            return status;
    
        status = pj_sock_bind(si.rtcp_sock, &si.rtcp_addr_name,
                              pj_sockaddr_get_len(&si.rtcp_addr_name));
        if (status != PJ_SUCCESS)
            return status;
 
        /* Create media transport from existing sockets */
        status = pjmedia_transport_udp_attach( med_endpt, NULL, &si, 
                                PJMEDIA_UDP_NO_SRC_ADDR_CHECKING, &transport);
        if (status != PJ_SUCCESS)
            return status;      
        
    } else {
        /* Create media transport */
        status = pjmedia_transport_udp_create(med_endpt, NULL, local_port,
                                              0, &transport);
        if (status != PJ_SUCCESS)
            return status;
    }


    /* Now that the stream info is initialized, we can create the 
     * stream.
     */

    status = pjmedia_stream_create( med_endpt, pool, &info, 
                                    transport, 
                                    NULL, p_stream);

    if (status != PJ_SUCCESS) {
        // app_perror(THIS_FILE, "Error creating stream", status);
        pjmedia_transport_close(transport);
        return status;
    }

    /* Start media transport */
    pjmedia_transport_media_start(transport, 0, 0, 0, 0);


    return PJ_SUCCESS;
                                 
}

int MyAudioStreamer::Send_Audio_stream(int conf_port) {
        pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    pj_pool_t *pool;
    pjmedia_port *rec_file_port = NULL, *play_file_port = NULL;
    pjmedia_master_port *master_port = NULL;
    pjmedia_snd_port *snd_port = NULL;
    pjmedia_stream *stream = NULL;
    pjmedia_port *stream_port;
    char tmp[10];
    char addr[PJ_INET_ADDRSTRLEN];
    pj_status_t status; 

    /* Default values */
    const pjmedia_codec_info *codec_info;
    pjmedia_codec_param codec_param;
    pjmedia_dir dir = PJMEDIA_DIR_ENCODING;
    pj_sockaddr_in remote_addr ;
    pj_sockaddr_in mcast_addr;
    pj_uint16_t local_port = 8000;
    char *codec_id = NULL;
    char *rec_file = NULL;
    char *play_file = NULL;
    int mcast = 0;

    int c;
    int option_index;


    pj_bzero(&remote_addr, sizeof(remote_addr));


    /* init PJLIB : */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);



    pj_str_t ip = pj_str("192.168.1.38");
    pj_uint16_t port = 7000;

    status = pj_sockaddr_in_init(&remote_addr, &ip, port);
    if (status != PJ_SUCCESS) {
        // app_perror(THIS_FILE, "Invalid remote address", status);
        return 1;
    }



    /* Verify arguments. */
    if (dir & PJMEDIA_DIR_ENCODING) {
        if (remote_addr.sin_addr.s_addr == 0) {
            printf("Error: remote address must be set\n");
            return 1;
        }
    }

   

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    /* 
     * Initialize media endpoint.
     * This will implicitly initialize PJMEDIA too.
     */
    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Create memory pool for application purpose */
    pool = pj_pool_create( &cp.factory,     /* pool factory         */
                           "app",           /* pool name.           */
                           8000,            /* init size            */
                           8000,            /* increment size       */
                           NULL             /* callback on error    */
                           );


    /* Register all supported codecs */
    status = init_Audio_codecs(med_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    if(pjmedia_endpt_get_codec_mgr(med_endpt) == NULL) {
        std::cout<<"----------------------------------------------  Does not Get Manager-------------------------------------- "<<std::endl;
    }else{
        std::cout<<"----------------------------------------------  Get Manager --------------------------------------- "<<std::endl;
    }
    /* Find which codec to use. */
    if (codec_id) {
        std::cout<<"----------------------------------------------  Codec Find--------------------------------------- "<<std::endl;
        unsigned count = 1;
        pj_str_t str_codec_id = pj_str(codec_id);
        pjmedia_codec_mgr *codec_mgr = pjmedia_endpt_get_codec_mgr(med_endpt);
        status = pjmedia_codec_mgr_find_codecs_by_id( codec_mgr,
                                                      &str_codec_id, &count,
                                                      &codec_info, NULL);
        if (status != PJ_SUCCESS) {
            printf("Error: unable to find codec %s\n", codec_id);
            return 1;
        }
    } else {
        /* Default to pcmu */
        std::cout<<"----------------------------------------------  Codec Deos not Find--------------------------------------- "<<std::endl;
        status = pjmedia_codec_mgr_get_codec_info( pjmedia_endpt_get_codec_mgr(med_endpt),
                                          0, &codec_info);
    }

    if (status != PJ_SUCCESS){
        std::cout<<"------------------------------------------------- Error In Codec --------------------------------------------------------"<<std::endl;
        goto on_exit;
    }
        
    /* Create event manager */
    status = pjmedia_event_mgr_create(pool, 0, NULL);
    if (status != PJ_SUCCESS)
        goto on_exit;


    /* Create stream based on program arguments */
    status = create_Audio_stream(pool, med_endpt, codec_info, dir, local_port,
                           &remote_addr, mcast, &mcast_addr,
                           &stream);
    if (status != PJ_SUCCESS){
        std::cout<<"-------------------------------------------------------------- Not created audio ------------------------------------------"<<endl;
        goto on_exit;
    }

    /* Get codec default param for info */
    status = pjmedia_codec_mgr_get_default_param(
                                    pjmedia_endpt_get_codec_mgr(med_endpt),
                                    codec_info,
                                    &codec_param);
    /* Should be ok, as create_Audio_stream() above succeeded */
    pj_assert(status == PJ_SUCCESS);

    /* Get the port interface of the stream */
    status = pjmedia_stream_get_port( stream, &stream_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Create sound device port. */
    if (dir == PJMEDIA_DIR_ENCODING_DECODING)
        status = pjmedia_snd_port_create(pool, -1, -1, 
                                    PJMEDIA_PIA_SRATE(&stream_port->info),
                                    PJMEDIA_PIA_CCNT(&stream_port->info),
                                    PJMEDIA_PIA_SPF(&stream_port->info),
                                    PJMEDIA_PIA_BITS(&stream_port->info),
                                    0, &snd_port);
    else if (dir == PJMEDIA_DIR_ENCODING)
        status = pjmedia_snd_port_create_rec(pool, -1, 
                                    PJMEDIA_PIA_SRATE(&stream_port->info),
                                    PJMEDIA_PIA_CCNT(&stream_port->info),
                                    PJMEDIA_PIA_SPF(&stream_port->info),
                                    PJMEDIA_PIA_BITS(&stream_port->info),
                                    0, &snd_port);
    else
        status = pjmedia_snd_port_create_player(pool, -1, 
                                    PJMEDIA_PIA_SRATE(&stream_port->info),
                                    PJMEDIA_PIA_CCNT(&stream_port->info),
                                    PJMEDIA_PIA_SPF(&stream_port->info),
                                    PJMEDIA_PIA_BITS(&stream_port->info),
                                    0, &snd_port);


    if (status != PJ_SUCCESS) {
        // app_perror(THIS_FILE, "Unable to create sound port", status);
        goto on_exit;
    }

    /* Connect sound port to stream */
    status = pjmedia_snd_port_connect( snd_port, stream_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Start streaming */
    pjmedia_stream_start(stream);

    pjsua_conf_port_id id;
    status = pjsua_conf_add_port(pool , stream_port , &id);
    status = pjsua_conf_connect(id , conf_port);

    /* Done */

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

        puts("");
        puts("Commands:");
        puts("  s     Display media statistics");
        puts("  q     Quit");
        puts("");

        printf("Command: "); fflush(stdout);

        if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (tmp[0] == 's'){}
            // print_stream_stat(stream, &codec_param);
        else if (tmp[0] == 'q')
            break;

    }

    /* Start deinitialization: */
on_exit:

    /* Destroy sound device */
    if (snd_port) {
        pjmedia_snd_port_destroy( snd_port );
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }

    /* Destroy master port */
    if (master_port) {
        pjmedia_master_port_destroy(master_port, PJ_FALSE);
    }

    /* Destroy stream */
    if (stream) {
        pjmedia_transport *tp;

        tp = pjmedia_stream_get_transport(stream);
        pjmedia_stream_destroy(stream);
        
        pjmedia_transport_media_stop(tp);
        pjmedia_transport_close(tp);
    }

    /* Destroy file ports */
    if (play_file_port)
        pjmedia_port_destroy( play_file_port );
    if (rec_file_port)
        pjmedia_port_destroy( rec_file_port );

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


    return (status == PJ_SUCCESS) ? 0 : 1;
}