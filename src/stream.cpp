// #include <pjmedia.h>
// #include <iostream>
// #include <pjsua2.hpp>
// #include "MyVideoMedia.h"

// using namespace pj;

// int main() {
//     Step 1: Initialize PJSUA
//     Endpoint ep;
//     ep.libCreate();

//     Initialize PJSUA configuration
//     EpConfig ep_cfg;
//     ep.libInit(ep_cfg);

//     std::cout << "Step-1 Completed\n";

//     Step 2: Create RTP transport
//     TransportConfig rtp_transport_cfg;
//     rtp_transport_cfg.port = 5004;  // Port for receiving RTP stream (must match FFmpeg port)
//     try {
//         ep.transportCreate(PJSIP_TRANSPORT_UDP, rtp_transport_cfg);
//     } catch (Error &err) {
//         std::cout << "Error creating RTP transport: " << err.info() << std::endl;
//         return 1;
//     }

//     std::cout << "Step-2 Completed\n";

//     ep.libStart();

//     char error_buf[256];

//     Create a memory pool factory
//     pj_caching_pool caching_pool;
//     pj_caching_pool_init(&caching_pool, NULL, 1024);

//     Create a pool for SDP parsing
//     pj_pool_t *pool = pj_pool_create(&caching_pool.factory, "sdp_pool", 4096, 4096, NULL);
//     if (!pool) {
//         std::cout << "Error creating pool" << std::endl;
//         return 1;
//     }

//     Step 3: Parse SDP file (assuming you have already obtained the SDP from FFmpeg)
//     pjmedia_sdp_session *remote_sdp{nullptr};
//     pj_str_t sdp_str_remote = pj_str(
//         "v=0\r\n"
//         "o=- 0 0 IN IP4 127.0.0.1\r\n"
//         "s=No Name\r\n"
//         "c=IN IP4 127.0.0.1\r\n"
//         "t=0 0\r\n"
//         "a=tool:libavformat LIBAVFORMAT_VERSION\r\n"
//         "m=video 5004 RTP/AVP 96\r\n"
//         "a=rtpmap:96 H264/90000\r\n"
//         "a=fmtp:96 packetization-mode=1\r\n");  // Example SDP content

//     pj_status_t sdp_status = pjmedia_sdp_parse(pool, sdp_str_remote.ptr, sdp_str_remote.slen, &remote_sdp);
//     if (sdp_status != PJ_SUCCESS) {
//         pj_strerror(sdp_status, error_buf, sizeof(error_buf));
//         std::cout << "Error parsing SDP file: " << error_buf << std::endl;
//         return 1;
//     }

//     pjmedia_sdp_session *local_sdp{nullptr};
//     pj_str_t sdp_str_local = pj_str(
//         "v=0\n"
//         "o=- 0 0 IN IP4 0.0.0.0\n"
//         "c=IN IP4 0.0.0.0\n"
//         "t=0 0\n"
//         "m=video 5010 RTP/AVP 96\n"  // Use the same format as the remote SDP
//         "a=rtpmap:96 H264/90000\n"
//         "a=fmtp:96 packetization-mode=1\n");

//     sdp_status = pjmedia_sdp_parse(pool, sdp_str_local.ptr, sdp_str_local.slen, &local_sdp);
//     if (sdp_status != PJ_SUCCESS) {
//         pj_strerror(sdp_status, error_buf, sizeof(error_buf));
//         std::cout << "Error parsing SDP file: " << error_buf << std::endl;
//         return 1;
//     }

//     std::cout<< "Step-3 Completed\n";

//     Step 4: Create media stream from SDP
//     pjmedia_stream_info stream_info;
//     pj_status_t status = pjmedia_stream_info_from_sdp(&stream_info, pool, pjsua_get_pjmedia_endpt(), local_sdp, remote_sdp, 0);
//     if (status != PJ_SUCCESS) {
//         pj_strerror(status, error_buf, sizeof(error_buf));
//         std::cout << "Error creating stream info from SDP" << error_buf << std::endl;
//         return 1;
//     }

//     std::cout << "Step-4 Completed\n";

//     Step 5: Create the video stream transport
//     pjmedia_transport* media_transport;
//     status = pjmedia_transport_udp_create(pjsua_get_pjmedia_endpt(), NULL, stream_info.rem_addr.ipv4.sin_port, 0, &media_transport);
//     if (status != PJ_SUCCESS) {
//         pj_strerror(status, error_buf, sizeof(error_buf));
//         std::cout << "Error creating media transport: " << pj_strerror(status, error_buf, 0).ptr << std::endl;
//         return 1;
//     }

//     std::cout << "Step-5 Completed\n";
    
//     Step 6: Create the media stream and link it to the transport
//     pjmedia_stream *stream;
//     status = pjmedia_stream_create(pjsua_get_pjmedia_endpt(), pool, &stream_info, media_transport, NULL, &stream);
//     if (status != PJ_SUCCESS) {
//         pj_strerror(status, error_buf, sizeof(error_buf));
//         std::cout << "Error creating media stream: " << error_buf << std::endl;
//         return 1;
//     }

//     std::cout << "Step-6 Completed\n";

//     Step 7: Start the media stream to receive the RTP stream
//     status = pjmedia_stream_start(stream);
//     if (status != PJ_SUCCESS) {
//         std::cout << "Error starting media stream" << std::endl;
//         return 1;
//     }

//     std::cout << "Step-7 Completed\n";

//     Step 8: Getting the Video Media Port
//     pjmedia_port *stream_port;
//     status = pjmedia_stream_get_port(stream, &stream_port);
//     if (status != PJ_SUCCESS) {
//         std::cout << "Error getting media stream port" << std::endl;
//         return 1;
//     } 

//     std::cout << "Step-8 Completed\n";

//     Keep the application running
//     std::cout << "Streaming video. Press Ctrl+C to exit..." << std::endl;
//     while (true) {
//         pj_thread_sleep(1000);  // Wait to keep the stream active
//     }

//     pj_pool_release(pool);
//     pj_caching_pool_destroy(&caching_pool);
    
//     Cleanup
//     ep.libDestroy();

//     return 0;
// }
