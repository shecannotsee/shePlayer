//
// Created by shecannotsee on 23-6-2.
//

#ifndef SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
#define SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <iostream>

namespace m1_simple_player_test {

void main() {
  // 读取文件
  AVFormatContext* formatContext = nullptr;/* read from file and check */ {
    if (avformat_open_input(&formatContext, "xx.h265", nullptr, nullptr) != 0) {
      std::cerr << "Could not open file." << std::endl;
      return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
      std::cerr << "Could not find stream information." << std::endl;
      return;
    }
  }

  // 在文件中查找视频流
  int videoStreamIndex = -1;/* find stream */ {
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
      if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoStreamIndex = i;
        break;
      }
    }

    if (videoStreamIndex == -1) {
      std::cerr << "Could not find video stream." << std::endl;
      return;
    }
  }

  // 获取视频解码器
  const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
  if (codec == nullptr) {
    std::cerr << "Unsupported codec." << std::endl;
    return ;
  }
  // 为解码器分配上下文
  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  // 将编码器参数拷贝到解码器上下文中
  if (avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) != 0) {
    std::cerr << "Failed to copy codec parameters to decoder context." << std::endl;
    return ;
  }
  // 打开解码器
  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    std::cerr << "Failed to open codec." << std::endl;
    return ;
  }

  AVFrame* frame = av_frame_alloc();
  AVFrame* frameRGB = av_frame_alloc();

  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
  uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

  av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);

  SwsContext* swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt, codecContext->width, codecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

  AVPacket packet;
  while (av_read_frame(formatContext, &packet) >= 0) {
    if (packet.stream_index == videoStreamIndex) {
      avcodec_send_packet(codecContext, &packet);
      int ret;
      while (ret >= 0) {
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
          break;
        else if (ret < 0) {
          std::cerr << "Error during decoding." << std::endl;
          return ;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, frameRGB->data, frameRGB->linesize);
        // 在此处使用你自己的显示逻辑来显示帧（例如使用OpenGL、SDL等库）
      }
    }
    av_packet_unref(&packet);
  }


  sws_freeContext(swsContext);
  av_frame_free(&frameRGB);
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);


  std::cout << "good.\n";
};

};// namespace m1_simple_player_test

#endif //SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
