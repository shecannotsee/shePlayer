//
// Created by shecannotsee on 23-6-13.
//

#ifndef SHEPLAYER_TEST_M4_ENCODE_VIDEO_TEST_H_
#define SHEPLAYER_TEST_M4_ENCODE_VIDEO_TEST_H_

#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

namespace m4_encode_video_test {

void encode(AVCodecContext* pCodecCtx, AVFrame *pFrame, AVPacket* pPacket, FILE* p_output_f) {
  int ret;
  ret = avcodec_send_frame(pCodecCtx, pFrame);
  while (ret >= 0) {
    ret = avcodec_receive_packet(pCodecCtx, pPacket);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return;
    }
    fwrite(pPacket->data, 1, pPacket->size, p_output_f);
    av_packet_unref(pPacket);
  }
};

void main() {
  int temp = 0;
  // 查找编码器
  const AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);/* check */ {
    // pCodec = avcodec_find_encoder_by_name("libx264");// other way
    // pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec) {
      std::cout << RED_COLOR << "avcodec_find_encoder_by_name error, codec_name=libx264\n" << RESET_COLOR;
      exit(1);
    }
  };
  // 设置编码器上下文参数
  AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);/* check and set */ {
    if (!pCodecCtx) {
      std::cout << RESET_COLOR <<"avcodec_alloc_context3 error, pCodecCtx is NULL\n" << RESET_COLOR;
      exit(1);
    }
    // 设置 AVCodecContext 参数
    pCodecCtx->bit_rate = 400000;// 比特率
    pCodecCtx->width = 720;// 宽
    pCodecCtx->height = 240;// 高
    pCodecCtx->time_base = { 1, 25 };
    pCodecCtx->framerate = { 25, 1 };
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    pCodecCtx->gop_size = 10;
    pCodecCtx->max_b_frames = 1;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;// 若编码的图像格式部位YUV420P则需要修稿
    if (pCodec->id == AV_CODEC_ID_H264) {
      av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
    }
  };

  // 用来存编码前的数据
  AVFrame* pFrame = av_frame_alloc();/* init */ {
    pFrame->format = pCodecCtx->pix_fmt;// 格式
    pFrame->width = pCodecCtx->width;// 宽
    pFrame->height = pCodecCtx->height;// 高
  };

  // 打开编码器
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    std::cout << RED_COLOR << "avcodec_open2 error.\n" << RESET_COLOR;
    exit(1);
  }

  // 为音频或视频数据分配新的缓冲区。
  if (av_frame_get_buffer(pFrame, 32) < 0) {
    std::cout << RESET_COLOR << "av_frame_get_buffer error\n" << RESET_COLOR;
    exit(1);
  }

  // 用来存编码后的数据
  AVPacket* pPacket = av_packet_alloc();
  // 打开输出文件
  std::string output_file = "./output_img/output.img";
  FILE* p_output_f = fopen(output_file.c_str(), "wb");/* check */ {
    if (!p_output_f) {
      std::cout << RED_COLOR << "open file failed.\n" << RESET_COLOR;
      exit(1);
    }
  };
  // encode 5 seconds of video
  for (int i = 0; i < 25 * 5; i++) {
    fflush(stdout);
    // 确保帧数据可写
    if (av_frame_is_writable(pFrame) < 0) {
      std::cout << RESET_COLOR << "av_frame_is_writable error.\n" << RESET_COLOR;
      exit(1);
    }

    // 将数据填到pFrame里
    // Y
    for (int y = 0; y < pCodecCtx->height; y++) {
      for (int x = 0; x < pCodecCtx->width; x++) {
        pFrame->data[0][y*pFrame->linesize[0] + x] = x + y + i * 3;
      }
    }
    //Y and V
    for (int y = 0; y < pCodecCtx->height / 2; y++) {
      for (int x = 0; x < pCodecCtx->width / 2; x++) {
        pFrame->data[1][y * pFrame->linesize[1] + x] = 128 + y + i * 2;
        pFrame->data[2][y * pFrame->linesize[2] + x] = 64 + x + i * 5;
      }
    }

    pFrame->pts = i;

    // 编码成img
    encode(pCodecCtx, pFrame, pPacket, p_output_f);
  }

  // 刷新编码器
  encode(pCodecCtx, NULL, pPacket, p_output_f);
  // 添加序列结束代码以具有真正的 MPEG 文件
  unsigned char endcode[] = { 0x0, 0x00, 0x01, 0x7b };// 控制播放的颜色
  fwrite(endcode, 1, sizeof(endcode), p_output_f);

  fclose(p_output_f);

  /* release resources  */ {
    if (pCodecCtx) {
      avcodec_free_context(&pCodecCtx);
    }
    if (pPacket) {
      av_packet_free(&pPacket);
    }
    if (pFrame) {
      av_frame_free(&pFrame);
    }
  };
  std::cout << GREEN_COLOR << "=============== encode_yuv_to_h264 done ===============\n" << std::endl << RESET_COLOR;
};

};// namespace m4_encode_video_test

#endif //SHEPLAYER_TEST_M4_ENCODE_VIDEO_TEST_H_
