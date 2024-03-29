#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

using field_t = std::vector<std::string>;
using point_t = std::array<int, 2>;
using portals_t = std::map<point_t, point_t>;
using data_t = std::tuple<field_t, portals_t, point_t, point_t>;
using access_t = std::set<point_t>;
bool isalpha(char c) { return 'A' <= c && c <= 'Z'; }

std::istream &read_field(std::istream &stream, field_t &field) {
  std::string line;
  std::size_t width = 0;
  while (std::getline(stream, line)) {
    if (!std::empty(line)) {
      width = std::max(width, std::size(line));
      field.push_back(std::move(line));
    }
  }
  for (std::size_t i = 0; i != std::size(field); ++i) {
    field[i].resize(width);
  }
  return stream;
}

data_t read_data(const char *filename) {
  data_t data;
  std::ifstream stream(filename);
  auto &[field, portals, begin, end] = data;
  read_field(stream, field);
  std::map<std::string, std::pair<point_t, point_t>> mouths;
  for (int i = 2; i != (int)std::size(field) - 2; ++i) {
    for (int j = 2; j != (int)std::size(field[i]) - 2; ++j) {
      if (field[i][j] == '.') {
        std::map<std::string, std::pair<point_t, point_t>> new_mouths;
        if (isalpha(field[i - 1][j])) {
          std::string name{field[i - 2][j], field[i - 1][j]};
          new_mouths[name] = {point_t{i - 1, j}, point_t{i, j}};
        }
        if (isalpha(field[i + 1][j])) {
          std::string name{field[i + 1][j], field[i + 2][j]};
          new_mouths[name] = {point_t{i + 1, j}, point_t{i, j}};
        }
        if (isalpha(field[i][j - 1])) {
          std::string name{field[i][j - 2], field[i][j - 1]};
          new_mouths[name] = {point_t{i, j - 1}, point_t{i, j}};
        }
        if (isalpha(field[i][j + 1])) {
          std::string name{field[i][j + 1], field[i][j + 2]};
          new_mouths[name] = {point_t{i, j + 1}, point_t{i, j}};
        }
        for (const auto &mouth : new_mouths) {
          const auto &[name, points] = mouth;
          const auto &[in, out] = points;
          if (name == "AA") {
            begin = out;
          } else if (name == "ZZ") {
            end = out;
          } else if (auto iter = mouths.find(name); iter != mouths.end()) {
            portals[in] = iter->second.second;
            portals[iter->second.first] = out;
          } else {
            mouths.insert(mouth);
          }
        }
      }
    }
  }

  return data;
}

std::ostream &operator<<(std::ostream &stream, const point_t &point) {
  auto [x, y] = point;
  return stream << "(" << x << "," << y << ")";
}

int part_one(data_t data) {
  auto &[field, portals, begin, end] = data;

  field[begin[0]][begin[1]] = '@';
  int distance = 0;
  for (access_t access = {begin}; !std::empty(access); ++distance) {
    access_t new_access;
    for (auto p : access) {
      point_t next[4] = {
        {p[0] - 1, p[1]},
        {p[0] + 1, p[1]},
        {p[0], p[1] - 1},
        {p[0], p[1] + 1},
      };
      for (auto p : next) {
        if (p == end) {
          return distance + 1;
        }
        if (auto iter = portals.find(p); iter != portals.end()) {
          p = iter->second;
        }
        if (field[p[0]][p[1]] == '.') {
          field[p[0]][p[1]] = '@';
          new_access.insert(p);
        }
      }
    }
    std::swap(access, new_access);
  }

  throw "UNREACHABLE";
}

using level_access_t = std::set<std::pair<int, point_t>>;

int part_two(const data_t &data, int limit) {
  const auto &[field, portals, begin, end] = data;
  std::vector<field_t> levels = {field};

  levels[0][begin[0]][begin[1]] = '@';
  int distance = 0;
  int level = 0;
  for (level_access_t access = {{0, begin}}; !std::empty(access); ++distance) {
    level_access_t new_access;
    for (const auto [l, p] : access) {
      point_t next[4] = {
        {p[0] - 1, p[1]},
        {p[0] + 1, p[1]},
        {p[0], p[1] - 1},
        {p[0], p[1] + 1},
      };
      for (auto p : next) {
        level = l;
        if (level == 0 && p == end) {
          return distance + 1;
        }
        if (auto iter = portals.find(p); iter != portals.end()) {
          if (p[1] < 2 || p[1] >= (int)std::size(field[p[0]]) - 2 || p[0] < 2 ||
              p[0] >= (int)std::size(field) - 2) {
            if (level == 0) {
              continue;
            }
            --level;
          } else {
            if (level == limit) {
              continue;
            }
            ++level;
            if (level == (int)std::size(levels)) {
              levels.push_back(field);
            }
          }
          p = iter->second;
        }
        if (levels[level][p[0]][p[1]] == '.') {
          levels[level][p[0]][p[1]] = '@';
          new_access.insert({level, p});
        }
      }
    }
    std::swap(access, new_access);
  }

  throw "UNREACHABLE";
}

void do_it() try {
  for (auto filename : {"20-test.data", "20.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto data = read_data(filename);
    std::cout << "Part one answer " << part_one(data) << std::endl;
  }
  for (auto filename : {"20-test-2.data", "20.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto data = read_data(filename);
    std::cout << "Part two answer " << part_two(data, 100) << std::endl;
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
