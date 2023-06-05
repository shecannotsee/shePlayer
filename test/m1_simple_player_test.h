//
// Created by shecannotsee on 23-6-2.
//

#ifndef SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
#define SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <iostream>

namespace m1_simple_player_test {

void main() {
  // 读取文件
  AVFormatContext* formatContext = nullptr;/* read from file */ {
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


  std::cout << "good.\n";
};

};// namespace m1_simple_player_test

#endif //SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
