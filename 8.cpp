#include "precompiled.h"
#include <cmath>

constexpr int width = 25;
constexpr int height = 6;

using line_t = std::array<int, width>;
using layer_t = std::array<line_t, height>;
using image_t = std::vector<layer_t>;

void part_one(const image_t& image) {
  int best_score = 1 << 30, answer = -1;

  for (int i = 0; std::size_t(i) != std::size(image); ++i) {
    int counts[3] = { 0, 0, 0 };
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        const int & pixel = image[i][y][x];
        if (pixel < 3) {
          ++counts[pixel];
        }
      }
    }
    if (best_score > counts[0]) {
      best_score = counts[0];
      answer = counts[1] * counts[2];
    }
  }
  std::cout << "Best score " << best_score << ", answer " << answer << std::endl;
}

void part_two(const image_t& image) {
  layer_t layer;
  std::memset(&layer, '\0', sizeof layer);

  for (int i = 0; std::size_t(i) != std::size(image); ++i) {
    int j = std::size(image) - 1 - i;
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        const int & pixel = image[j][y][x];
        if (pixel != 2) {
          layer[y][x] = pixel;
        }
      }
    }
  }
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      const int & pixel = layer[y][x];
      switch(pixel) {
      case 0: std::cout << ' '; break;
      case 1: std::cout << '#'; break;
      default: std::cout << '!'; break;
      }
    }
    std::cout << "\\\n";
  }
}

image_t read(const char *filename) {
  std::ifstream in(filename);
  std::vector<image_t> images;
  std::string s;
  if (std::getline(in, s)) {
    auto layer_count = std::size(s) / (width * height);
    image_t image (layer_count);
    int n = 0;
    for (std::size_t i = 0; i != layer_count; ++i) {
      for (int y = 0; y != height; ++y) {
        for (int x = 0; x != width; ++x) {
          image[i][y][x] = s[n++] - '0';
        }
      }
    }
    return image;
  }
  throw "stream failed";
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto image = read("8.data");
    part_one(image);
    part_two(image);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
