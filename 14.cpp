#include "precompiled.h"

typedef std::pair<std::string, int64_t> pair_t;
typedef std::map<std::string, std::pair<int64_t, std::vector<pair_t>>>
  reactions_t;

std::istream &read(std::istream &in, reactions_t &reactions) {
  std::regex regex1("([0-9]+) ([A-Z]+)");
  std::string line;
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    std::vector<pair_t> reagents;
    while (std::regex_search(begin, end, m, regex1)) {
      auto &[reagent, reagent_consumed] = reagents.emplace_back();
      std::from_chars(m[1].first, m[1].second, reagent_consumed);
      reagent.assign(m[2].first, m[2].second);
      begin = m[1].second;
    }
    if (std::size(reagents)) {
      auto [product, product_produced] = std::move(reagents.back());
      reagents.erase(reagents.end() - 1, reagents.end());
      reactions.insert_or_assign(std::move(product),
        std::make_pair(product_produced, std::move(reagents)));
    }
  }
  return in;
}

int64_t ore_remaining(
  const reactions_t &reactions, int64_t fuel_required, bool verbose) {
  std::map<std::string, int64_t> got = {
    {"FUEL", -fuel_required}, {"ORE", 1000000000000}};

  while (true) {
    if (auto iter = std::find_if(std::begin(got), std::end(got),
          [](const auto &pair) { return pair.second < 0; });
        iter != std::end(got)) {
      auto &[product, product_available] = *iter;
      if (auto iter = reactions.find(product); iter != std::end(reactions)) {
        const auto &[yield, reagents] = iter->second;
        int64_t multiplier = (-1 - product_available) / yield + 1;
        if (verbose) {
          std::cout << "Produce " << (multiplier * yield) << " " << product
                    << ", consume";
        }
        product_available += multiplier * yield;
        for (const auto &[reagent, reagent_consumed] : reagents) {
          if (verbose) {
            std::cout << " " << (multiplier * reagent_consumed) << " "
                      << reagent;
          }
          got[reagent] -= multiplier * reagent_consumed;
        }
        if (verbose) {
          std::cout << std::endl;
        }
      } else {
        return -1;
      }
    } else {
      return (1000000000000) - got["ORE"];
    }
  }
}

void part_one(const reactions_t &reactions, bool verbose) {
  int64_t n = ore_remaining(reactions, 1, verbose);
  std::cout << "Need " << n << " ORE" << std::endl;
}

void part_two(const reactions_t &reactions) {
  int64_t lo = 1;
  int64_t hi = 10000000000;
  while (hi - lo > 1) {
    int64_t mid = lo + (hi - lo) / 2;
    if (ore_remaining(reactions, mid, false) < 0) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  std::cout << "Made " << lo << " FUEL" << std::endl;
}

void do_file(const char *filename, bool verbose) {
  reactions_t reactions;
  std::ifstream in(filename);
  read(in, reactions);
  if (!std::empty(reactions)) {
    std::cout << "\nRead reactions from " << filename << std::endl;
    part_one(reactions, verbose);
    part_two(reactions);
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    do_file("14-test.data", true);
    do_file("14.data", false);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
