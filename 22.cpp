#include <tuple>
#include <vector>
#include <istream>
#include <ostream>
#include <iostream>
#include <fstream>
#include "egcd.h"

enum method_t { cut, stack, increment };

struct technique_t {
  method_t method;
  int64_t param;
};

using techniques_t = std::vector<technique_t>;

std::vector<technique_t> read_techniques(const char *filename) {
  std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "Read file \"" << filename << "\"" << std::endl;
  std::ifstream in(filename);
  std::map<std::string, std::string> orbits;
  std::regex regex1(
    "cut (-?\\d+)|deal into new stack|deal with increment (\\d+)");
  std::string line;
  techniques_t techniques;
  technique_t technique{cut, 0};
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    if (std::regex_match(begin, end, m, regex1)) {
      if (m[1].first != m[1].second) {
        technique.method = cut;
        std::from_chars(m[1].first, m[1].second, technique.param);
      } else if (m[2].first != m[2].second) {
        technique.method = increment;
        std::from_chars(m[2].first, m[2].second, technique.param);
      } else {
        technique.method = stack;
        technique.param = 0;
      }
      techniques.push_back(technique);
    }
  }
  return techniques;
}

int64_t apply_inverse(
  technique_t technique, int64_t modulus, int64_t i, bool verbose) {
  auto [method, param] = technique;
  switch (method) {
  case cut:
    if (verbose) {
      std::cout << "cut " << param << ": " << i;
    }
    i = (i + modulus + param) % modulus;
    break;
  case stack:
    if (verbose) {
      std::cout << "deal into new stack: " << i;
    }
    i = modulus - 1 - i;
    break;
  case increment: {
    if (verbose) {
      std::cout << "deal with increment " << param << ": " << i;
    }
    i = (int64_t)(((__int128_t)i * (__int128_t)modinv(param, modulus)) %
                  (__int128_t)modulus);
    break;
  }
  }
  if (verbose) {
    std::cout << " -> " << i << std::endl;
  }
  return i;
}

int64_t apply(technique_t technique, int64_t modulus, int64_t i, bool verbose) {
  auto [method, param] = technique;
  switch (method) {
  case cut:
    if (verbose) {
      std::cout << "cut " << param << ": " << i;
    }
    i = (i + modulus - param) % modulus;
    break;
  case stack:
    if (verbose) {
      std::cout << "deal into new stack: " << i;
    }
    i = modulus - 1 - i;
    break;
  case increment: {
    if (verbose) {
      std::cout << "deal with increment " << param << ": " << i;
    }
    i = (int64_t)(((__int128_t)i * (__int128_t)param) % (__int128_t)modulus);
    break;
  }
  }
  if (verbose) {
    std::cout << " -> " << i << std::endl;
  }
  return i;
}

void naive(const techniques_t &techniques, int64_t modulus, int64_t value,
  bool verbose) {
  std::cout << "------------------------------------------------------------\n"
            << "Naive, modulus " << modulus << ", value " << value << std::endl;
  using deck_t = std::vector<int64_t>;
  deck_t deck(modulus);
  for (int64_t i = 0; i != modulus; ++i) {
    deck[i] = i;
  }
  if (verbose) {
    for (auto card : deck) {
      std::cout << " " << card;
    }
    std::cout << "\n";
  }
  for (auto [method, param] : techniques) {
    switch (method) {
    case cut:
      if (verbose) {
        std::cout << "cut " << param << std::endl;
      }
      if (param < 0) {
        param += modulus;
      }
      std::rotate(&deck[0], &deck[param], &deck[modulus]);
      break;
    case stack:
      if (verbose) {
        std::cout << "deal into new stack\n";
      }
      std::reverse(&deck[0], &deck[modulus]);
      break;
    case increment: {
      if (verbose) {
        std::cout << "deal with increment " << param << std::endl;
      }
      deck_t new_deck(modulus);
      int64_t j = 0;
      for (int64_t i = 0; i != modulus; ++i) {
        new_deck[j] = deck[i];
        j = (j + param) % modulus;
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
}

int64_t apply_inverse(
  const techniques_t &techniques, int64_t modulus, int64_t i, bool verbose) {
  for (auto iter = std::rbegin(techniques); iter != std::rend(techniques);
       ++iter) {
    i = apply_inverse(*iter, modulus, i, verbose);
  }
  return i;
}

int64_t apply(
  const techniques_t &techniques, int64_t modulus, int64_t i, bool verbose) {
  for (const auto &technique : techniques) {
    i = apply(technique, modulus, i, verbose);
  }
  return i;
}

void part_one(const techniques_t &techniques, int64_t modulus, int64_t value,
  bool verbose) {
  std::cout << "------------------------------------------------------------\n"
            << "Part one, modulus " << modulus << ", value " << value
            << std::endl;
  int64_t i = apply(techniques, modulus, value, verbose);
  std::cout << "Position of card " << value << " is " << i << std::endl;
}

void part_two_exponent_one(const techniques_t &techniques, int64_t modulus,
  int64_t position, bool verbose) {
  std::cout << "------------------------------------------------------------\n"
            << "Part two, modulus " << modulus << ", position " << position
            << ", exponent 1" << std::endl;
  int64_t i = apply_inverse(techniques, modulus, position, verbose);
  std::cout << "Card in position " << position << " is " << i << std::endl;
}

std::pair<int64_t, int64_t> reduce_one(
  const technique_t &technique, int64_t modulus) {
  auto [method, param] = technique;
  switch (method) {
  case cut:
    // i = (i + modulus + param) % modulus;
    return {1, (modulus + param) % modulus};
  case stack:
    // i = modulus - 1 - i;
    return {modulus - 1, modulus - 1};
  case increment: {
    return {modinv(param, modulus), 0};
  }
  }
  __builtin_unreachable();
}

using reduced_t = std::pair<int64_t, int64_t>;

reduced_t compose(reduced_t a, reduced_t b, __int128_t modulus) {
  __int128_t mul1 = a.first;
  __int128_t add1 = a.second;
  __int128_t mul2 = b.first;
  __int128_t add2 = b.second;

  return {(int64_t)((mul1 * mul2) % modulus),
    (int64_t)((add1 * mul2 + add2) % modulus)};
}

reduced_t reduce(const techniques_t &techniques, int64_t modulus) {
  reduced_t result = {1, 0};
  for (auto iter = std::rbegin(techniques); iter != std::rend(techniques);
       ++iter) {
    result = compose(result, reduce_one(*iter, modulus), modulus);
  }
  return result;
}

void part_two(const techniques_t &techniques, int64_t modulus, int64_t position,
  int64_t exponent) {
  std::cout << "============================================================\n"
            << "Part two, modulus " << modulus << ", position " << position
            << ", exponent " << exponent << std::endl;

  // Tests.
  if (true) {
    // Card in position after all techniques, reduced i = i * m + a ? OK!
    part_two_exponent_one(techniques, 119315717514047, 2020, false);
    std::cout << "Test reduce:" << std::endl;
    const auto [mul, add] = reduce(techniques, modulus);
    techniques_t techniques_reduced{{increment, mul}, {cut, modulus - add}};
    int64_t i = apply(techniques_reduced, modulus, position, false);
    std::cout << position << " * " << mul << " + " << add << " = " << i
              << std::endl;

    // Card in position after all techniques twice, reduced i = i * (m * m) + (m
    // * a + a) ? OK!
    auto techniques2 = techniques;
    techniques2.insert(
      std::end(techniques2), std::begin(techniques), std::end(techniques));
    part_two_exponent_one(techniques2, 119315717514047, 2020, false);
    std::cout << "Test reduce squared:" << std::endl;
    int64_t mul2 =
      (int64_t)(((__int128_t)mul * (__int128_t)mul) % (__int128_t)modulus);
    int64_t add2 =
      (int64_t)(((__int128_t)add * (__int128_t)mul + (__int128_t)add) %
                (__int128_t)modulus);
    techniques_t techniques2_reduced{{increment, mul2}, {cut, modulus - add2}};
    i = apply(techniques2_reduced, modulus, position, false);
    std::cout << position << " * " << mul2 << " + " << add2 << " = " << i
              << std::endl;
  }

  std::cout << "------------------------------------------------------------\n"
            << "Repeated squaring" << std::endl;

  reduced_t power = reduce(techniques, modulus);
  reduced_t result = {1, 0};
  int64_t e = exponent;
  while (e) {
    if (e & 1) {
      result = compose(result, power, modulus);
    }
    power = compose(power, power, modulus);
    e = e >> 1;
  }
  auto [mul, add] = result;
  techniques_t techniques_result = {{increment, mul}, {cut, modulus - add}};
  int64_t i = apply(techniques_result, modulus, position, false);
  std::cout << position << " * " << mul << " + " << add << " = " << i
            << std::endl;
}

void do_it() try {
  for (auto filename :
    {"22-test-1.data", "22-test-2.data", "22-test-3.data", "22-test-4.data"}) {
    auto techniques = read_techniques(filename);
    naive(techniques, 10, 3, true);
    part_one(techniques, 10, 3, true);
    part_two_exponent_one(techniques, 10, 3, true);
  }
  {
    auto techniques = read_techniques("22.data");
    // Part one, modulus 10007, value 2019, position ?
    // part_one(techniques, 10007, 2019, false);

    // Part two,
    // modulus 119315717514047, prime, 2 ** 46.76177743085247
    // position 2020
    // exponent 101741582076661, prime, 2 ** 46.53190276174851
    part_two(techniques, 119315717514047, 2020, 101741582076661);
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
