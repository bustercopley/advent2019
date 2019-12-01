#include "precompiled.h"
#include <cmath>

void part_one(const std::vector<int> &masses, bool verbose) {
  std::cout << "Module count " << std::size(masses) << std::endl;
  int total_fuel = 0;
  for (int mass : masses) {
    int fuel = mass / 3 - 2;
    if (verbose) {
      std::cout << "Module mass " << mass << ", fuel " << fuel << std::endl;
    }
    total_fuel += fuel;
  }
  std::cout << "Total fuel " << total_fuel << "\n\n";
}

void part_two(const std::vector<int> &masses, bool verbose) {
  std::cout << "Module count " << std::size(masses) << std::endl;
  int total_fuel = 0;
  for (int mass : masses) {
    int fuel = mass / 3 - 2;
    if (verbose) {
      std::cout << "Mass " << mass << ", fuel " << fuel;
    }
    int extra_fuel = fuel;
    while ((extra_fuel = extra_fuel / 3 - 2) >= 0) {
      if (verbose) {
        std::cout << " + " << extra_fuel;
      }
      fuel = fuel + extra_fuel;
    }
    if (verbose) {
      std::cout << " = " << fuel << std::endl;
    }
    total_fuel += fuel;
  }
  std::cout << "Total fuel " << total_fuel << "\n\n";
}

void read(std::istream &in, bool verbose) {
  std::vector<int> masses;
  std::regex regex1("^(\\d+)$");
  std::string line;
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    if (std::regex_match(begin, end, m, regex1)) {
      int mass;
      std::from_chars(m[1].first, m[1].second, mass);
      masses.push_back(mass);
    }
  }
  part_one(masses, verbose);
  part_two(masses, verbose);
}

void read_file(const char *filename, bool verbose) {
  std::cout << "Reading \"" << filename << "\"" << std::endl;
  std::ifstream in(filename);
  read(in, verbose);
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  read_file("1-test.data", true);
  read_file("1.data", true);
  return 0;
}
