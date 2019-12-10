#include "precompiled.h"
#include <cmath>

struct asteroid_t {
  int x, y;
};

using field_t = std::vector<asteroid_t>;

field_t read(std::istream & stream) {
  field_t field;
  int y = 0;
  std::string s;
  while (std::getline(stream, s)) {
    int x = 0;
    for (char c: s) {
      if (c == '#') {
        field.push_back({x, y});
      }
      ++x;
    }
    ++y;
  }
  return field;
}

bool between (asteroid_t a, asteroid_t b, asteroid_t c) {
  if (a.x == b.x && a.x == c.x) {
    return (a.y < b.y && b.y < c.y) || (c.y < b.y && b.y < a.y);
  }
  else if ((b.x - a.x) * (c.y - a.y) != (c.x - a.x) * (b.y - a.y)) {
    return false;
  }
  else {
    return (a.x < b.x && b.x < c.x) || (a.x > b.x && b.x > c.x);
  }
}

bool intervisible (field_t field, asteroid_t a, asteroid_t b) {
  if (b.x == a.x && b.y == a.y) {
    return false;
  }
  for (asteroid_t c: field) {
    if (between(a, c, b)) {
      return false;
    }
  }
  return true;
}

asteroid_t part_one(field_t field) {
  int best_score = 0;
  asteroid_t best;
  for (auto a : field) {
    int score = 0;
    for (auto b : field) {
      if (intervisible(field, a, b)) {
        ++score;
      }
    }
    if (score > best_score) {
      best_score = std::max(score, best_score);
      best = a;
    }
  }
  std::cout << best_score << ", (" << best.x << "," << best.y << ")"
            << std::endl;
  return best;
}

void part_two(field_t field) {
  asteroid_t a = part_one(field);
  int n = 0;
  while (std::size(field) > 1) {
    std::map<double, std::size_t> order;
    for (std::size_t i = 0; i != std::size(field); ++i) {
      auto b = field[i];
      if (intervisible(field, a, b)) {
        order[-std::atan2((double)(b.x - a.x), (double)(b.y - a.y))] = i;
      }
    }
    for (auto pair: order) {
      auto b = field[pair.second];
      if (++n == 200) {
        std::cout << "(" << b.x << "," << b.y << ")" << std::endl;
      }
    }
    std::vector<std::size_t> destroyed;
    for (auto pair: order) {
      destroyed.push_back(pair.second);
    }
    std::sort(destroyed.begin(), destroyed.end(), std::greater());
    for (auto i: destroyed) {
      field.erase(field.begin() + i, field.begin() + i + 1);
    }
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  for (auto filename: { "10-test.data", "10.data" }) {
    std::ifstream in(filename);
    auto field = read(in);
    part_two(field);
  }
  return 0;
}
