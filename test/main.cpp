//
// Created by shecannotsee on 23-6-2.
//
#include <iostream>
#include "print_color.h"
#include "m1_video_decode_test.h"
#include "m2_play_with_sdl2_test.h"

int main() {
  std::cout << "test start.\n";

  m1_video_decode_test::main();
  m2_play_with_sdl2_test::main();

  std::cout << "test end.\n";
  return 0;
};
