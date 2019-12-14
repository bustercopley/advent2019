#include "precompiled.h"
#include "rectangle.h"

struct stretch_t {
  int x, y;
};

using wire_t = std::vector<stretch_t>;

template <typename T> int signum(T val) { return (T(0) < val) - (val < T(0)); }

void part_one(const wire_t (&wires)[2], bool verbose) {
  int best = 1 << 30;
  rectangle_t<int> grid;
  for (int n = 0; n != 2; ++n) {
    int x = 0, y = 0;
    grid.set(x, y, grid.get(x, y).second | (1 << n));
    for (const auto &stretch : wires[n]) {
      int x1 = x + stretch.x;
      int y1 = y + stretch.y;
      while (x != x1 || y != y1) {
        x = x + signum(stretch.x);
        y = y + signum(stretch.y);
        int value = grid.get(x, y).second | (1 << n);
        if (value == 3) {
          best = std::min(best, std::abs(x) + std::abs(y));
        }
        grid.set(x, y, value);
      }
    }
  }
  std::cout << "Manhattan distance " << best << std::endl;
}

void part_two(const wire_t (&wires)[2], bool verbose) {
  int best = 1 << 30;
  rectangle_t<std::pair<int, int>> grid;
  for (int n = 0; n != 2; ++n) {
    int x = 0, y = 0;
    auto [set, value] = grid.get(x, y);
    auto [flags, delay] = value;
    grid.set(x, y, {flags | (1 << n), delay});
    int new_delay = 0;
    for (const auto &stretch : wires[n]) {
      int x1 = x + stretch.x;
      int y1 = y + stretch.y;
      while (x != x1 || y != y1) {
        ++new_delay;
        x = x + signum(stretch.x);
        y = y + signum(stretch.y);
        auto [set, value] = grid.get(x, y);
        auto [flags, delay] = value;
        if ((flags & (1 << n)) == 0) {
          flags |= (1 << n);
          delay += new_delay;
          grid.set(x, y, {flags, delay});
          if (flags == 3) {
            best = std::min(best, delay);
          }
        }
      }
    }
  }
  if (verbose) {
    grid.put(std::cout, [](bool is_set, const std::pair<int, int> &value) {
      switch (value.first) {
      case 1:
        return 'X';
      case 2:
        return '+';
      case 3:
        return 'o';
      default:
        return ' ';
      }
    });
  }
  std::cout << "Signal delay " << best << std::endl;
}

std::istream &read(std::istream &stream, wire_t (&wires)[2]) {
  std::regex regex1("([LRUD])(\\d+)");
  std::string s;
  wires[0] = {};
  wires[1] = {};
  for (int n = 0; n != 2; ++n) {
    if (!std::getline(stream, s)) {
      return stream;
    }
    wire_t &wire = wires[n];
    const char *begin = std::data(s);
    const char *end = begin + std::size(s);
    std::cmatch m;

    while (std::regex_search(begin, end, m, regex1)) {
      auto &stretch = wire.emplace_back();
      int direction, length;
      direction = m[1].first[0];
      length = 0; // !
      if (auto [p, ec] = std::from_chars(m[2].first, m[2].second, length);
          ec != std::errc()) {
        std::cout << "Error! " << std::make_error_code(ec) << std::endl;
        std::exit(1);
      }
      switch (direction) {
      case 'L':
        stretch.x = -length;
        stretch.y = 0;
        break;
      case 'R':
        stretch.x = length;
        stretch.y = 0;
        break;
      case 'U':
        stretch.x = 0;
        stretch.y = length;
        break;
      case 'D':
        stretch.x = 0;
        stretch.y = -length;
        break;
      }
      begin = m[1].second;
    }
  }
  return stream;
}

void do_file(const char *filename, bool verbose) {
  std::ifstream in(filename);
  wire_t wires[2];
  while (read(in, wires)) {
    part_one(wires, verbose);
    part_two(wires, verbose);
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  do_file("3-test.data", true);
  do_file("3.data", false);
  return 0;
}
