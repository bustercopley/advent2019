#include "precompiled.h"
#include <cmath>
#include <set>

void part_one(const std::map<std::string, std::string> &orbits) {
  int count = 0;
  std::set<std::string> bodies;
  std::set<std::string> leaves;
  for (auto pair : orbits) {
    bodies.insert(pair.first);
    bodies.insert(pair.second);
  }
  for (auto body : bodies) {
    while (true) {
      if (auto iter = orbits.find(body); iter == orbits.end()) {
        break;
      } else {
        body = iter->second;
        ++count;
      }
    }
  }
  std::cout << count << std::endl;
}

std::map<std::string, int> get_ancestors(
  const std::map<std::string, std::string> &orbits, std::string body) {
  int n = -1;
  std::map<std::string, int> ancestors;
  while (true) {
    if (auto iter = orbits.find(body); iter != orbits.end()) {
      ancestors[body] = n;
      body = iter->second;
      ++n;
    } else {
      break;
    }
  }
  return ancestors;
}

void part_two(const std::map<std::string, std::string> &orbits, bool verbose) {
  std::string a = "YOU", b = "SAN";
  auto a_ancestors = get_ancestors(orbits, a);
  auto b_ancestors = get_ancestors(orbits, b);
  if (verbose) {
    std::cout << "Ancestors of " << a << ":\n";
    for (auto pair : a_ancestors) {
      std::cout << pair.first << " " << pair.second << std::endl;
    }
    std::cout << "Ancestors of " << b << ":\n";
    for (auto pair : b_ancestors) {
      std::cout << pair.first << " " << pair.second << std::endl;
    }
  }
  std::string nearest;
  int n = 1 << 30;
  for (auto pair : a_ancestors) {
    std::string body = pair.first;
    if (auto iter = b_ancestors.find(body); iter != b_ancestors.end()) {
      int m = pair.second + iter->second;
      if (verbose) {
        std::cout << "Common ancestor " << body << ", total distance " << m
                  << std::endl;
      }
      if (m < n) {
        n = m;
        nearest = body;
      }
    }
  }
  std::cout << "Nearest common ancestor: " << nearest << ", total distance "
            << n << std::endl;
}

std::map<std::string, std::string> read(std::istream &in) {
  std::map<std::string, std::string> orbits;
  std::regex regex1("^(\\w+)\\)(\\w+)$");
  std::string line;
  while (std::getline(in, line)) {
    const char *begin = std::data(line);
    const char *end = begin + std::size(line);
    std::cmatch m;
    if (std::regex_match(begin, end, m, regex1)) {
      orbits[std::string(m[2].first, m[2].second)] =
        std::string(m[1].first, m[1].second);
    }
  }
  return orbits;
}

std::map<std::string, std::string> read_file(const char *filename) {
  std::cout << "Reading \"" << filename << "\"" << std::endl;
  std::ifstream in(filename);
  return read(in);
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  {
    auto orbits = read_file("6-test.data");
    part_one(orbits);
  }

  {
    auto orbits = read_file("6-test2.data");
    part_two(orbits, true);
  }

  {
    auto orbits = read_file("6.data");
    part_one(orbits);
    part_two(orbits, false);
  }

  return 0;
}
