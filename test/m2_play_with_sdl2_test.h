//
// Created by shecannotsee on 23-6-5.
//

#ifndef SHEPLAYER_M2_PLAY_WITH_SDL2_TEST_H
#define SHEPLAYER_M2_PLAY_WITH_SDL2_TEST_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
};
#include <SDL2/SDL.h>

namespace m2_play_with_sdl2_test {

const std::string file_path = "./xx.mpg";
int g_frame_rate = 0;// 帧率
bool g_sfp_refresh_thread_exit = false;
bool g_sfp_refresh_thread_pause = false;
#define SFM_REFRESH_EVENT (SDL_USEREVENT+1)
#define SFM_BREAK_EVENT   (SDL_USEREVENT+2)

void main() {

  /* 初始化,后续版本不再使用,也可以不写该行 */ {
    avformat_network_init();
  }

  AVFormatContext* pFormatCtx = nullptr;/* init */ {
    pFormatCtx = avformat_alloc_context();
  };
  /* 打开输入流 */ {
    if (avformat_open_input(&pFormatCtx,file_path.c_str(),NULL,NULL) != NULL) {
      std::cout << RED_COLOR << "avformat_open_input error.\n" << RESET_COLOR;
      exit(1);
    }
  }
  /* 查找流信息 */ {
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
      std::cout << RESET_COLOR << "avformat_find_stream_info errpr.\n" << RESET_COLOR;
      exit(1);
    }
  };

  const AVCodec* pCodec = nullptr;
  AVCodecContext* pCodecCtx = nullptr;
  int video_index = -1;
  /* 查找编码解码器 */ {
    for (int i=0; i < pFormatCtx->nb_streams; i++) {
      AVStream* pStream = pFormatCtx->streams[i];
      if (pStream->codecpar->codec_type == /*流为视频流*/AVMEDIA_TYPE_VIDEO) {
        pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);// H264等
        pCodecCtx = avcodec_alloc_context3(pCodec);
        avcodec_parameters_to_context(pCodecCtx,pStream->codecpar);
        video_index = i;
        g_frame_rate = pStream->avg_frame_rate.num / pStream->avg_frame_rate.den;
      }
    };
    if (!pCodecCtx) {
      std::cout << RESET_COLOR << "can not find video codecCtx.\n" <<  RESET_COLOR;
      exit(1);
    }
  };

  /* 打开解码器 */ {
    if (avcodec_open2(pCodecCtx, pCodec, NULL)) {
      std::cout << RESET_COLOR << "avcodec_open2 error.\n" << RESET_COLOR;
      exit(1);
    }
  };

  SwsContext* pSwsCtx = nullptr;
  /* 变换,将解码后的数据转换成指定格式,将原始数据(可能是RGB24,YUV其他格式)转为YUV420P */ {
    pSwsCtx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
                             pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_YUV420P,
                             SWS_BICUBIC,NULL,NULL,NULL);
  };

  unsigned char* out_buffer = nullptr;
  int out_buffer_size = 0;
  AVFrame* pFrameYUV = nullptr;/* init */ {
    pFrameYUV = av_frame_alloc();
  };
  /* 用buffer来存储转换成目标格式的数据 */ {
    out_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    out_buffer = (unsigned char*) av_malloc(out_buffer_size);
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height,1);
  };

  // sdl param
  SDL_Rect sdl_rect;
  /* 初始化sdl */ {
    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER)!=0) {
      std::cout << RED_COLOR << "SDL init error.\n" << SDL_GetError() << RESET_COLOR;
      exit(1);
    }
  };
  SDL_Window* p_sdl_window = nullptr;
  SDL_Renderer* p_sdl_renderer = nullptr;
  SDL_Texture* p_sdl_texture = nullptr;
  /* 创建窗口 */ {
    p_sdl_window = SDL_CreateWindow("shePlayer",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    pCodecCtx->width,pCodecCtx->height,
                                    SDL_WINDOW_OPENGL);
    // 创建渲染器
    p_sdl_renderer = SDL_CreateRenderer(p_sdl_window, -1, 0);
    // 创建纹理
    p_sdl_texture = SDL_CreateTexture(p_sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                      pCodecCtx->width,pCodecCtx->height);
    // 窗口大小
    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = pCodecCtx->width;
    sdl_rect.h = pCodecCtx->height;
  };

  SDL_Thread* p_sdl_thread = nullptr;
  /* 刷新线程处理 */ {
    p_sdl_thread = SDL_CreateThread([](void*)->int {
      g_sfp_refresh_thread_exit = false;
      g_sfp_refresh_thread_pause = false;
      while (!g_sfp_refresh_thread_exit) {
        if (!g_sfp_refresh_thread_pause) {
          SDL_Event sdl_event;
          sdl_event.type = SFM_REFRESH_EVENT;
          SDL_PushEvent(&sdl_event);
        }
        SDL_Delay(1000 / g_frame_rate);
      };
      g_sfp_refresh_thread_exit = false;
      g_sfp_refresh_thread_pause = false;
      SDL_Event  sdl_event;
      sdl_event.type = SFM_BREAK_EVENT;
      SDL_PushEvent(&sdl_event);
      return 0;
    }, NULL, NULL);
  };

  AVPacket* packet = nullptr;/* init */ {
    packet = av_packet_alloc();
  };
  AVFrame* pFrame = nullptr;/* init */ {
    pFrame = av_frame_alloc();
  };
  SDL_Event sdl_event;
  /* show */ {
    while (true) {
      int ret = 0;
      SDL_WaitEvent(&sdl_event);
      // 用户自定义事件:刷新
      /*z*/if (sdl_event.type == SFM_REFRESH_EVENT) {
        while (true) {
          if (av_read_frame(pFormatCtx, packet) < 0) {
            g_sfp_refresh_thread_exit = true;
            break;
          }
          if (packet->stream_index == video_index) {
            break;
          }
        };
        if (avcodec_send_packet(pCodecCtx, packet)) {
          g_sfp_refresh_thread_exit = true;
        }
        int temp_ret = 0;
        do {
          temp_ret = avcodec_receive_frame(pCodecCtx, pFrame);/* check */ {
            if (temp_ret == AVERROR_EOF) {
              g_sfp_refresh_thread_exit = true;
            }
            if (temp_ret == 0) {
              // 将原始数据转换到YUV420P中
              sws_scale(pSwsCtx,
                        (const unsigned char* const*)pFrame->data,
                        pFrame->linesize,
                        NULL,
                        pCodecCtx->height,
                        pFrameYUV->data,
                        pFrameYUV->linesize);
              // 显示
              SDL_UpdateTexture(p_sdl_texture,
                                &sdl_rect,
                                pFrameYUV->data[0],
                                pFrameYUV->linesize[0]);
              SDL_RenderClear(p_sdl_renderer);
              SDL_RenderCopy(p_sdl_renderer, p_sdl_texture, NULL, &sdl_rect);
              SDL_RenderPresent(p_sdl_renderer);
            }
          };
        } while (temp_ret != AVERROR(EAGAIN));
      }
      // 用户自定义事件:跳出
      else if (sdl_event.type == SFM_BREAK_EVENT) {
        break;
      }
      // 键盘消息
      else if (sdl_event.type == SDL_KEYDOWN) {
        if (sdl_event.key.keysym.sym == SDLK_SPACE) {
          g_sfp_refresh_thread_pause = !g_sfp_refresh_thread_pause;
        }
        if (sdl_event.key.keysym.sym == SDLK_q) {
          g_sfp_refresh_thread_exit = true;
        }
      }
      else if (sdl_event.type == SDL_QUIT) {
        g_sfp_refresh_thread_exit = true;
      }
    };
  };

end:
  /* 释放申请的资源 */ {
    if (pCodecCtx) {
      avcodec_close(pCodecCtx);
    }
    if (pCodecCtx) {
      avcodec_free_context(&pCodecCtx);
    }
    if (pFormatCtx) {
      avformat_close_input(&pFormatCtx);
    }
    if (pFormatCtx) {
      avformat_free_context(pFormatCtx);
    }
    av_packet_free(&packet);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameYUV);

    SDL_DestroyTexture(p_sdl_texture);
    SDL_DestroyRenderer(p_sdl_renderer);
    SDL_DestroyWindow(p_sdl_window);
    SDL_Quit();
  };

};

};// namespace m2_play_with_sdl2_test

#endif //SHEPLAYER_M2_PLAY_WITH_SDL2_TEST_H
