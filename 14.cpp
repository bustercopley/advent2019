#include "precompiled.h"
#include <cmath>

typedef std::pair<std::string, int> pair_t;
typedef std::map<std::string, std::pair<int, std::vector<pair_t>>> reactions_t;

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

void part_one(const reactions_t &reactions) {
  std::map<std::string, int> got = {{"FUEL", -1}, {"ORE", 1<<30}};
 loop:
  for (const auto &pair: got) {
    if (pair.second) {
      std::cout << " " << pair.second << " " << pair.first;
    }
  }
  std::cout << std::endl;
  for (auto& pair: got) {
    if (pair.second < 0) {
      if (auto iter = reactions.find(pair.first); iter != reactions.end()) {
        auto [yield, reagents] = iter->second;
        int multiplier = std::ceil((double)(-pair.second) / (double)yield);
        pair.second += multiplier * yield;
        for (const auto &pair1: reagents) {
          got[pair1.first] -= multiplier * pair1.second;
        }
        goto loop;
      }
      else {
        throw "NO REACTION";
      }
    }
  }
  std::cout << "Needed " << ((1 << 30) - got["ORE"]) << " ORE" << std::endl;
}

void do_file(const char *filename) {
  reactions_t reactions;
  std::ifstream in(filename);
  read(in, reactions);
  if (!std::empty(reactions)) {
    std::cout << "\nRead reactions from " << filename << std::endl;
    part_one(reactions);
    // part_two(moons, true);
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
