//
// Created by shecannotsee on 23-6-2.
//

#ifndef SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
#define SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_

#include <string>
#include <chrono>
#include <thread>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace m1_simple_player_test {

int main() {
  std::string inputFilename = "xx.mp4";

  avformat_network_init();

  // Open input file
  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(&formatContext, inputFilename.c_str(), nullptr, nullptr) != 0) {
    std::cerr << "Failed to open input file" << std::endl;
    return 1;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    std::cerr << "Failed to retrieve stream information" << std::endl;
    avformat_close_input(&formatContext);
    return 1;
  }

  // Find the first video stream
  int videoStreamIndex = -1;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      break;
    }
  }

  if (videoStreamIndex == -1) {
    std::cerr << "Failed to find a video stream in the input file" << std::endl;
    avformat_close_input(&formatContext);
    return 1;
  }

  // Get a pointer to the codec context for the video stream
  AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;
  const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
  if (!codec) {
    std::cerr << "Failed to find video codec" << std::endl;
    avformat_close_input(&formatContext);
    return 1;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    std::cerr << "Failed to allocate codec context" << std::endl;
    avformat_close_input(&formatContext);
    return 1;
  }

  if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
    std::cerr << "Failed to initialize codec context" << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  // Open the codec
  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    std::cerr << "Failed to open codec" << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  // Allocate video frame and initialize buffer for decoded frame
  AVFrame* frame = av_frame_alloc();
  AVFrame* frameRGB = av_frame_alloc();
  if (!frame || !frameRGB) {
    std::cerr << "Failed to allocate frames" << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
  uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
  if (!buffer) {
    std::cerr << "Failed to allocate RGB buffer" << std::endl;
    av_frame_free(&frame);
    av_frame_free(&frameRGB);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);

  // Initialize SwsContext for color conversion
  SwsContext* swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                          codecContext->width, codecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!swsContext) {
    std::cerr << "Failed to initialize SwsContext" << std::endl;
    av_freep(&buffer);
    av_frame_free(&frame);
    av_frame_free(&frameRGB);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  // Read frames from the input file and decode
  AVPacket packet;
  av_init_packet(&packet);

  while (av_read_frame(formatContext, &packet) >= 0) {
    if (packet.stream_index == videoStreamIndex) {
      // Decode video packet
      int response = avcodec_send_packet(codecContext, &packet);
      if (response < 0) {
        std::cerr << "Failed to decode video packet" << std::endl;
        break;
      }

      while (response >= 0) {
        response = avcodec_receive_frame(codecContext, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
          break;
        else if (response < 0) {
          std::cerr << "Failed to receive video frame" << std::endl;
          break;
        }

        // Convert the frame to RGB
        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, frameRGB->data, frameRGB->linesize);

        // Display the frame
        std::cout << "Displaying frame: " << frame->pts << std::endl;

        // Delay for a short period to display the frame
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
      }
    }

    av_packet_unref(&packet);
  }

  // Clean up resources
  av_freep(&buffer);
  av_frame_free(&frame);
  av_frame_free(&frameRGB);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);
  sws_freeContext(swsContext);

};

};// namespace m1_simple_player_test

#endif //SHEPLAYER_TEST_M1_SIMPLE_PLAYER_TEST_H_
