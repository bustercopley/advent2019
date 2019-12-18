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

std::map<char, int> get_accessible_keys(
  field_t field, const std::set<char> &keys_taken, int x, int y) {
  std::map<char, int> keys;
  std::set<std::array<int, 2>> access = {{x, y}};
  int distance = 1;
  field[y][x] = '@';
  while (!std::empty(access)) {
    std::set<std::array<int, 2>> new_access = {};
    for (auto p : access) {
      std::array<int, 2> locs[4] = {
        {p[0] - 1, p[1]},
        {p[0] + 1, p[1]},
        {p[0], p[1] - 1},
        {p[0], p[1] + 1},
      };
      for (auto loc : locs) {
        char c = field[loc[1]][loc[0]];
        if (('a' <= (c | ' ') && (c | ' ') <= 'z' &&
              keys_taken.contains(c | ' ')) ||
            c == '.') {
          field[loc[1]][loc[0]] = '@';
          new_access.insert({loc[0], loc[1]});
        } else if ('a' <= c && c <= 'z') {
          keys.try_emplace(c, distance);
        }
      }
    }
    std::swap(access, new_access);
    ++distance;
  }
  return keys;
}

std::set<char> string_to_set(const std::string &s) {
  std::set<char> result;
  for (auto c : s) {
    result.insert(c);
  }
  return result;
}

std::string set_to_string(const std::set<char> &s) {
  std::string result;
  for (auto c : s) {
    result.push_back(c);
  }
  return result;
}

void part_one(field_t field) {
  auto objects = get_objects(field);
  std::string all_keys_list;
  {
    std::set<char> all_keys;
    for (const auto &pair : objects) {
      if ('a' <= pair.first && pair.first <= 'z') {
        all_keys.insert(pair.first);
      }
    }
    all_keys_list = set_to_string(all_keys);
  }
  int key_count = std::size(all_keys_list);
  auto [x, y] = objects['@'];
  field[y][x] = '.';

  // {[keys_already_taken][current_key], current_distance}
  std::map<std::string, int> optimal = {{"@", 0}};
  for (int i = 0; i != key_count; ++i) {
    std::map<std::string, int> new_optimal;
    for (auto [keys_string, current_distance] : optimal) {
      char last_key = keys_string.back();
      auto keys_taken_list = keys_string;
      keys_taken_list.pop_back();
      auto keys_taken = string_to_set(keys_taken_list);
      auto [x, y] = objects[last_key];
      auto keys = get_accessible_keys(field, keys_taken, x, y);
      for (auto [key, distance] : keys) {
        auto keys = keys_taken;
        keys.insert(key);
        auto keys_list = set_to_string(keys);
        keys_list.push_back(key);
        auto &d = new_optimal[keys_list];
        if (!d || d > distance + current_distance) {
          d = distance + current_distance;
        }
      }
    }
    optimal = new_optimal;
  }
  int best = 1 << 30;
  for (auto [keys_taken_list, distance] : optimal) {
    if (best > distance) {
      best = distance;
    }
  }
  std::cout << "Answer " << best << std::endl;
}

void part_two(field_t field) {
  auto objects = get_objects(field);
  std::string all_keys_list;
  {
    std::set<char> all_keys;
    for (const auto &pair : objects) {
      if ('a' <= pair.first && pair.first <= 'z') {
        all_keys.insert(pair.first);
      }
    }
    all_keys_list = set_to_string(all_keys);
  }
  int key_count = std::size(all_keys_list);

  auto [x, y] = objects['@'];

  field[y - 1][x - 1] = '.';
  field[y - 1][x] = '#';
  field[y - 1][x + 1] = '.';

  field[y][x - 1] = '#';
  field[y][x] = '#';
  field[y][x + 1] = '#';

  field[y + 1][x - 1] = '.';
  field[y + 1][x] = '#';
  field[y + 1][x + 1] = '.';

  std::array<int, 2> robots[4] = {
    {x - 1, y - 1},
    {x + 1, y - 1},
    {x - 1, y + 1},
    {x + 1, y + 1},
  };

  // {[keys_already_taken][current_keys], current_distance}
  std::map<std::string, int> optimal = {{"@@@@", 0}};
  for (int i = 0; i != key_count; ++i) {
    std::map<std::string, int> new_optimal;
    for (auto [keys_string, current_distance] : optimal) {
      if (std::size(keys_string) != (std::size_t)(i + 4)) {
        throw "LENGTH ERROR";
      }
      std::string robots_list(
        std::begin(keys_string) + i, std::end(keys_string));
      auto keys_taken_list = keys_string;
      keys_taken_list.erase(
        std::begin(keys_taken_list) + i, std::end(keys_taken_list));

      for (int j = 0; j != 4; ++j) {
        char last_key = robots_list[j];
        auto keys_taken = string_to_set(keys_taken_list);
        auto [x, y] = objects[last_key];
        if (last_key == '@') {
          x = robots[j][0];
          y = robots[j][1];
        }
        auto keys = get_accessible_keys(field, keys_taken, x, y);
        for (auto [key, distance] : keys) {
          auto keys = keys_taken;
          keys.insert(key);
          robots_list[j] = key;
          auto keys_list = set_to_string(keys);
          keys_list.append(robots_list);
          auto &d = new_optimal[keys_list];
          if (!d || d > distance + current_distance) {
            d = distance + current_distance;
          }
        }
      }
      optimal = new_optimal;
    }
  }
  int best = 1 << 30;
  for (auto [keys_taken_list, distance] : optimal) {
    if (best > distance) {
      best = distance;
    }
  }
  std::cout << "Answer " << best << std::endl;
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
  try {
    for (auto filename : {"18-test-2.data", "18.data" }) {
      std::cout << "Processing " << filename << std::endl;
      auto field = read_file(filename);
      // std::cout << "Part one" << std::endl;
      // part_one(field);
      std::cout << "Part two" << std::endl;
      part_two(field);
    }
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
  return 0;
}
