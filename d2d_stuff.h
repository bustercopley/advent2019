// -*- C++ -*-
#ifndef d2d_stuff_h
#define d2d_stuff_h

#include "com.h"
#include "d2d.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <deque>

struct d2d_stuff_t {
  void enqueue(std::vector<int> && pixels);
  void render_frames(std::size_t thread_count, int width, int height);

private:
  std::vector<std::vector<int>> work;
  void thread_function(int width, int height, std::size_t begin, std::size_t end);
};

#endif
