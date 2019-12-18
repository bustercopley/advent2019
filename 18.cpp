#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

using field_t = std::vector<std::string>;

std::map<char, std::array<int, 2>> get_objects(const field_t &field) {
  std::map<char, std::array<int, 2>> objects;
  int y = 0;
  for (const auto &line : field) {
    int x = 0;
    for (char c : line) {
      if (('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
        objects.try_emplace(c, std::array<int, 2>{x, y});
      }
      ++x;
    }
    ++y;
  }
  return objects;
}

std::map<char, int> get_accessible_keys(field_t field, const std::set<char> &keys_taken, int x, int y) {
  std::map<char, int> keys;
  std::set<std::array<int, 2>> access = {{x, y}};
  int distance = 1;
  field[y][x] = '@';
  while (!std::empty(access)) {
    std::set<std::array<int, 2>> new_access = {};
    for (auto p : access) {
      std::array<int, 2>locs[4] = {
        {p[0] - 1, p[1]},
        {p[0] + 1, p[1]},
        {p[0], p[1] - 1},
        {p[0], p[1] + 1},
      };
      for (auto loc: locs) {
        char c = field[loc[1]][loc[0]];
        if (('a' <= (c|' ') && (c|' ') <= 'z' && keys_taken.contains(c|' ')) || c == '.') {
          field[loc[1]][loc[0]] = '@';
          new_access.insert({loc[0], loc[1]});
        }
        else if ('a' <= c && c <= 'z') {
          keys.try_emplace(c, distance);
        }
      }
    }
    std::swap(access, new_access);
    ++distance;
  }
  return keys;
}

void part_one(field_t field) {
  auto objects = get_objects(field);
  auto [x, y] = objects['@'];
  field[y][x] = '.';
  std::set<char> keys_taken;
  auto keys = get_accessible_keys(field, keys_taken, x, y);
  for (auto [c, distance] : keys) {
    std::cout << "Key " << c << " distance " << distance << std::endl;
  }

  {
    std::cout << "Take keys a,b\n";
    keys_taken.insert('a');
    keys_taken.insert('b');
    auto [x, y] = objects['b'];
    auto keys = get_accessible_keys(field, keys_taken, x, y);
    for (auto [c, distance] : keys) {
      std::cout << "Key " << c << " distance " << distance << std::endl;
    }
  }
}

void part_two(field_t field) {
  (void)field;
  int64_t result = 0;
  std::cout << result << std::endl;
}

std::istream &read(std::istream &stream, field_t &field) {
  std::string line;
  while (std::getline(stream, line)) {
    if (!std::empty(line)) {
      field.push_back(std::move(line));
    }
  }
  return stream;
}

field_t read_file(const char *filename) {
  std::cout << "Reading \"" << filename << "\"" << std::endl;
  std::ifstream stream(filename);
  field_t field;
  read(stream, field);
  return field;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  for (auto filename : {"18-test.data", "18.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto field = read_file(filename);
    std::cout << "Part one" << std::endl;
    part_one(field);
    std::cout << "Part two" << std::endl;
    part_two(field);
  }
  return 0;
}