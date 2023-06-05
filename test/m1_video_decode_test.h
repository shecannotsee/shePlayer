//
// Created by shecannotsee on 23-6-5.
//

#ifndef SHEPLAYER_TEST_M1_VIDEO_DECODE_TEST_H_
#define SHEPLAYER_TEST_M1_VIDEO_DECODE_TEST_H_

extern "C" {
#include <libavcodec/avcodec.h>
};


namespace m1_video_decode_test {
const int INBUF_SIZE = 4096;

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename) {

  FILE* f = fopen(filename,"wb");
  fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
  for (int i = 0; i < ysize; i++)
    fwrite(buf + i * wrap, 1, xsize, f);
  fclose(f);
};

static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename) {

  int ret = avcodec_send_packet(dec_ctx, pkt);/* check */ {
    if (ret < 0) {
      fprintf(stderr, "Error sending a packet for decoding\n");
      exit(1);
    }
  };

  char buf[1024];
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0) {
      fprintf(stderr, "Error during decoding\n");
      exit(1);
    }

    printf("saving frame %3d\n", dec_ctx->frame_number);
    fflush(stdout);

    // 图片由解码器分配。无需释放它
    snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
    pgm_save(frame->data[0], frame->linesize[0],
             frame->width, frame->height, buf);
  }
}

void main() {

  const char* filename    = "xx.mpg";
  const char* outfilename = "output.z";

  AVPacket* pkt = av_packet_alloc();/* check */ {
    if (!pkt)
      exit(1);
  };

  uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];/* init */ {
    // 将缓冲区结束设置为 0（这可确保损坏的 MPEG 流不会发生过度读取）
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  };

  // 查找 MPEG-1 视频解码器
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);/* check */ {
    if (!codec) {
      fprintf(stderr, "Codec not found\n");
      exit(1);
    }
  };

  AVCodecParserContext* parser = av_parser_init(codec->id);/* check */{
    if (!parser) {
      fprintf(stderr, "parser not found\n");
      exit(1);
    }
  };

  AVCodecContext* c = avcodec_alloc_context3(codec);/* check */ {
    if (!c) {
      fprintf(stderr, "Could not allocate video codec context\n");
      exit(1);
    }
  };

  // 对于某些编解码器，例如 msmpeg4 和 mpeg4，必须在此处初始化宽度和高度，因为此信息在比特流中不可用。

  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }

  FILE* f= fopen(filename, "rb");/* check */ {
    if (!f) {
      fprintf(stderr, "Could not open %s\n", filename);
      exit(1);
    }
  }

  AVFrame* frame = av_frame_alloc();/* check */ {
    if (!frame) {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
    }
  };

  uint8_t *data;
  size_t   data_size;
  int ret;
  int eof;
  do {
    // 从输入文件中读取原始数据
    data_size = fread(inbuf, 1, INBUF_SIZE, f);
    if (ferror(f))
      break;
    eof = !data_size;

    // 使用解析器将数据拆分为帧
    data = inbuf;
    while (data_size > 0 || eof) {
      ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                             data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
      if (ret < 0) {
        fprintf(stderr, "Error while parsing\n");
        exit(1);
      }
      data      += ret;
      data_size -= ret;

      if (pkt->size)
        decode(c, frame, pkt, outfilename);
      else if (eof)
        break;
    }
  } while (!eof);

  // 刷新解码器
  decode(c, frame, NULL, outfilename);

  fclose(f);

  av_parser_close(parser);
  avcodec_free_context(&c);
  av_frame_free(&frame);
  av_packet_free(&pkt);
};// int main()

};// namespace m1_video_decode_test

#endif //SHEPLAYER_TEST_M1_VIDEO_DECODE_TEST_H_
