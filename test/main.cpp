//
// Created by shecannotsee on 23-6-2.
//
#include <iostream>
#include "m1_video_decode_test.h"
#include "m2_play_with_sdl2_test.h"

const std::string RESET_COLOR   = "\033[0m";
const std::string RED_COLOR     = "\033[31m";
const std::string GREEN_COLOR   = "\033[32m";
const std::string YELLOW_COLOR  = "\033[33m";
const std::string PURPLE_COLOR  = "\033[35m";
const std::string BLUE_COLOR    = "\033[34m";

int main() {
  std::cout << "test start.\n";

  m1_video_decode_test::main();
  m2_play_with_sdl2_test::main();

  std::cout << "test end.\n";
  return 0;
};
