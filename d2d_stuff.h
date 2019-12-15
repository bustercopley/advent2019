// -*- C++ -*-
#ifndef d2d_stuff_h
#define d2d_stuff_h

#include "com.h"
#include "d2d.h"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct d2d_stuff_t {
  void enqueue(std::basic_string<WCHAR> &&caption, std::vector<int> &&pixels, int width, int height);
  void render_frames(std::size_t thread_count);

private:
  std::vector<std::vector<int>> work;
  std::vector<std::basic_string<WCHAR>> captions;
  void thread_function(
    int width, int height, std::size_t begin, std::size_t end);
  int max_width = 1, max_height = 1;
};

#endif
