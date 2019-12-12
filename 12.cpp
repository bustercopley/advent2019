#include "precompiled.h"
#include <numeric>

struct moon_t {
  int x[3];
  int v[3];
};

bool equal(const moon_t (&moons0)[4], const moon_t (&moons1)[4], int k) {
  for (int i = 0; i != 4; ++i) {
    if (moons0[i].x[k] != moons1[i].x[k])
      return false;
    if (moons0[i].v[k] != moons1[i].v[k])
      return false;
  }
  return true;
}

std::istream &read(std::istream &in, moon_t (&moons)[4]) {
  std::memset(&moons, '\0', sizeof moons);
  std::regex regex1("^<x=(-?\\d+), y=(-?\\d+), z=(-?\\d+)>$");
  std::string line;
  for (int i = 0; i != 4; ++i) {
    if (std::getline(in, line)) {
      const char *begin = std::data(line);
      const char *end = begin + std::size(line);
      std::cmatch m;
      if (std::regex_match(begin, end, m, regex1)) {
        for (int j = 0; j != 3; ++j) {
          if (auto [p, ec] =
                std::from_chars(m[j + 1].first, m[j + 1].second, moons[i].x[j]);
              ec != std::errc{}) {
            std::cout.setstate(std::ios::failbit);
          }
        }
      } else {
        std::cout.setstate(std::ios::failbit);
      }
    }
  }
  return in;
}

void step1(moon_t (&moons)[4], bool verbose) {
  for (int i = 0; i != 4; ++i) {
    for (int j = 0; j != i; ++j) {
      for (int k = 0; k != 3; ++k) {
        if (moons[i].x[k] < moons[j].x[k]) {
          ++moons[i].v[k];
          --moons[j].v[k];
        } else if (moons[i].x[k] > moons[j].x[k]) {
          --moons[i].v[k];
          ++moons[j].v[k];
        }
      }
    }
  }
  for (int i = 0; i != 4; ++i) {
    for (int k = 0; k != 3; ++k) {
      moons[i].x[k] += moons[i].v[k];
    }
  }
  if (verbose) {
    for (int i = 0; i != 4; ++i) {
      std::cout << "moons " << i << " x"
                << " " << moons[i].x[0] << " " << moons[i].x[1] << " "
                << moons[i].x[2] << " v"
                << " " << moons[i].v[0] << " " << moons[i].v[1] << " "
                << moons[i].v[2] << "\n";
    }
    std::cout << std::endl;
  }
}

void step2(moon_t (&moons)[4], int k) {
  for (int i = 0; i != 4; ++i) {
    for (int j = 0; j != i; ++j) {
      if (moons[i].x[k] < moons[j].x[k]) {
        ++moons[i].v[k];
        --moons[j].v[k];
      } else if (moons[i].x[k] > moons[j].x[k]) {
        --moons[i].v[k];
        ++moons[j].v[k];
      }
    }
  }
  for (int i = 0; i != 4; ++i) {
    moons[i].x[k] += moons[i].v[k];
  }
}

void part_one(const moon_t (&moons0)[4], int steps) {
  moon_t moons[4];
  std::memcpy(moons, moons0, sizeof moons);
  for (int i = 0; i != steps; ++i) {
    step1(moons, steps == 10);
  }
  int total = 0;
  for (int i = 0; i != 4; ++i) {
    int kin = 0, pot = 0;
    for (int k = 0; k != 3; ++k) {
      pot += std::abs(moons[i].x[k]);
      kin += std::abs(moons[i].v[k]);
    }
    total += kin * pot;
  }
  std::cout << "Total energy after " << steps << " steps is " << total
            << std::endl;
}

void part_two(const moon_t (&moons0)[4], bool verbose) {
  moon_t moons[4];
  std::memcpy(moons, moons0, sizeof moons);
  int64_t period = 1;
  for (int k = 0; k != 3; ++k) {
    int steps = 0;
    do {
      step2(moons, k);
      ++steps;
    } while (!equal(moons, moons0, k));
    period = std::lcm(period, (int64_t)steps);
    if (verbose) {
      std::cout << "Dimension " << k << " period " << steps << std::endl;
    }
  }
  std::cout << "Period " << period << std::endl;
}

void do_file(const char *filename, std::vector<int> steps) {
  moon_t moons[4];
  std::ifstream in(filename);
  int n = 0;
  while (read(in, moons)) {
    std::cout << "\nRead moons from " << filename << std::endl;
    part_one(moons, steps[n++]);
    part_two(moons, true);
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    do_file("12-test.data", {10, 100});
    do_file("12.data", {1000});
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
