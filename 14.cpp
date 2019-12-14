#include "precompiled.h"
#include <cmath>

typedef std::pair<std::string, int64_t> pair_t;
typedef std::map<std::string, std::pair<int64_t, std::vector<pair_t>>> reactions_t;

std::istream &read(std::istream &in, reactions_t &reactions) {
  std::regex regex1("([0-9]+) ([A-Z]+)");
  std::string line;
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    std::vector<pair_t> pairs;
    while (std::regex_search(begin, end, m, regex1)) {
      pair_t pair;
      std::from_chars(m[1].first, m[1].second, pair.second);
      pair.first = std::string(m[2].first, m[2].second);
      pairs.push_back(pair);
      begin = m[1].second;
    }
    if (std::size(pairs)) {
      pair_t pair = pairs.back();
      pairs.erase(pairs.end() - 1, pairs.end());
      reactions[pair.first] = {pair.second, pairs};
    }
  }
  return in;
}

int64_t ore_remaining(const reactions_t &reactions, int64_t fuel_required) {
  std::map<std::string, int64_t> got = {{"FUEL", -fuel_required}, {"ORE", 1000000000000}};
 loop:
  for (auto& pair: got) {
    if (pair.second < 0) {
      if (auto iter = reactions.find(pair.first); iter != reactions.end()) {
        auto [yield, reagents] = iter->second;
        int64_t multiplier = std::ceil((double)(-pair.second) / (double)yield);
        pair.second += multiplier * yield;
        for (const auto &pair1: reagents) {
          got[pair1.first] -= multiplier * pair1.second;
        }
        goto loop;
      }
      else {
        return -1;
      }
    }
  }
  return (1000000000000) - got["ORE"];
}

void part_one(const reactions_t &reactions) {
  int64_t n = ore_remaining(reactions, 1);
  std::cout << "Need " << n << " ORE" << std::endl;
}

void part_two(const reactions_t &reactions) {
  int64_t lo = 1;
  int64_t hi = 10000000000;
  while (hi - lo > 1) {
    int64_t mid = lo + (hi - lo) / 2;
    if (ore_remaining(reactions, mid) < 0) {
      hi = mid;
    }
    else {
      lo = mid;
    }
  }
  std::cout << "Made " << lo << " FUEL" << std::endl;
}

void do_file(const char *filename) {
  reactions_t reactions;
  std::ifstream in(filename);
  read(in, reactions);
  if (!std::empty(reactions)) {
    std::cout << "\nRead reactions from " << filename << std::endl;
    part_one(reactions);
    part_two(reactions);
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    do_file("14-test.data");
    do_file("14.data");
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
