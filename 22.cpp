#include "precompiled.h"
#include <set>

enum method_t { cut, stack, increment };

struct technique_t {
  method_t method;
  int64_t param;
};

using techniques_t = std::vector<technique_t>;

template <int64_t N> void part_one(const techniques_t &techniques, bool verbose) {
  using deck_t = std::array<int64_t, N>;
  deck_t deck;
  for (int64_t i = 0; i != N; ++i) {
    deck[i] = i;
  }
  for (auto [method, param] : techniques) {
    switch (method) {
    case cut:
      if (verbose) {
        std::cout << "cut " << param << std::endl;
      }
      if (param < 0) {
        param += N;
      }
      std::rotate(&deck[0], &deck[param], &deck[N]);
      break;
    case stack:
      if (verbose) {
        std::cout << "deal int64_to new stack\n";
      }
      std::reverse(&deck[0], &deck[N]);
      break;
    case increment: {
      if (verbose) {
        std::cout << "deal with increment " << param << std::endl;
      }
      deck_t new_deck;
      int64_t j = 0;
      for (int64_t i = 0; i != N; ++i) {
        new_deck[j] = deck[i];
        j = (j + param) % N;
      }
      deck = new_deck;
    } break;
    }
    if (verbose) {
      for (auto card : deck) {
        std::cout << " " << card;
      }
      std::cout << "\n";
    }
  }
  if (!verbose) {
    if (auto iter = std::find(&deck[0], &deck[N], 2019); iter != &deck[N]) {
      std::cout << (iter - &deck[0]) << std::endl;
    }
  }
}

std::vector<technique_t> read_techniques(const char *filename) {
  std::ifstream in(filename);
  std::map<std::string, std::string> orbits;
  std::regex regex1("cut (-?\\d+)|deal int64_to new stack|deal with increment (\\d+)");
  std::string line;
  techniques_t techniques;
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    technique_t technique;
    if (std::regex_match(begin, end, m, regex1)) {
      if (m[1].first != m[1].second) {
        technique.method = cut;
        if (auto [p, ec] =
              std::from_chars(m[1].first, m[1].second, technique.param);
            ec != std::errc{}) {
          std::cout.setstate(std::ios::failbit);
        }
      } else if (m[2].first != m[2].second) {
        technique.method = increment;
        if (auto [p, ec] =
              std::from_chars(m[2].first, m[2].second, technique.param);
            ec != std::errc{}) {
          std::cout.setstate(std::ios::failbit);
        }
      } else {
        technique.method = stack;
      }
      techniques.push_back(technique);
    }
  }
  return techniques;
}

void do_it() try {
  for (auto filename : {"22-test.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto techniques = read_techniques(filename);
    part_one<10>(techniques, true);
    // part_two(techniques);
  }
  for (auto filename : {"22.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto techniques = read_techniques(filename);
    part_one<10007>(techniques, false);
    //part_two<119315717514047>(techniques, false); :)
  }
} catch (const char *e) {
  std::cout << e << std::endl;
}

int main() {
  do_it();
  return 0;
}

int wmain() {
  do_it();
  return 0;
}
