#include "precompiled.h"
#include <cmath>

struct stretch_t {
  int x, y;
};

using wire_t = std::vector<stretch_t>;

void part_one(const std::array<wire_t, 2> &wires, bool verbose) {
  int minx = 0, miny = 0, maxx = 0, maxy = 0;
  for (int n = 0; n != 2; ++n) {
    int x = 0, y = 0;
    for (const auto & stretch: wires[n]) {
      x += stretch.x;
      y += stretch.y;
      minx = std::min(x, minx);
      miny = std::min(y, miny);
      maxx = std::max(x, maxx);
      maxy = std::max(y, maxy);
      // std::cout << "+(" << stretch.x << " " << stretch.y << ") to ("
      //           << x << " " << y << "), minx "
      //           << minx << " miny " << miny << " maxy "
      //           << maxy << " maxx " << maxx << std::endl;
    }
  }
  // std::cout << "minx " << minx << " miny " << miny << " maxy " << maxy << " maxx " << maxx << std::endl;
  std::vector<std::vector<int>> grid(maxx - minx + 1);
  for (auto & column: grid) {
    column.resize(maxy - miny + 1);
  }
  for (int n = 0; n != 2; ++n) {
    // std::cout << "Wire " << n << std::endl;
    int x = -minx, y = -miny;
    grid[x][y] |= (1 << n);
    for (const auto & stretch: wires[n]) {
      int d = std::abs(stretch.x) + std::abs(stretch.y);
      int u = (stretch.x > 0 ? 1 : stretch.x < 0 ? -1 : 0);
      int v = (stretch.y > 0 ? 1 : stretch.y < 0 ? -1 : 0);
      // std::cout << "Stretch length " << d << " direction " << u << " " << v << std::endl;
      for (int i = 0; i != d; ++ i) {
        x += u;
        y += v;
        //std::cout << x << " " << y << std::endl;
        grid[x][y] |= (1 << n);
      }
    }
  }
  int mind = 1 << 30;
  for (int x = 0; x != maxx - minx + 1; ++x) {
    for (int y = 0; y != maxy - miny + 1; ++y) {
      int d = std::abs(x + minx) + std::abs(y + miny);
      if (d && grid[x][y] == 3) {
        mind = std::min(d, mind);
      }
    }
  }
  std::cout << "Manhattan distance " << mind << std::endl;
}

void part_two(const std::array<wire_t, 2> &wires, bool verbose) {
  int minx = 0, miny = 0, maxx = 0, maxy = 0;
  for (int n = 0; n != 2; ++n) {
    int x = 0, y = 0;
    for (const auto & stretch: wires[n]) {
      x += stretch.x;
      y += stretch.y;
      minx = std::min(x, minx);
      miny = std::min(y, miny);
      maxx = std::max(x, maxx);
      maxy = std::max(y, maxy);
      // std::cout << "+(" << stretch.x << " " << stretch.y << ") to ("
      //           << x << " " << y << "), minx "
      //           << minx << " miny " << miny << " maxy "
      //           << maxy << " maxx " << maxx << std::endl;
    }
  }
  // std::cout << "minx " << minx << " miny " << miny << " maxy " << maxy << " maxx " << maxx << std::endl;
  std::vector<std::vector<std::array<int, 2>>> grid(maxx - minx + 1);
  for (auto & column: grid) {
    column.resize(maxy - miny + 1);
  }
  for (int n = 0; n != 2; ++n) {
    // std::cout << "Wire " << n << std::endl;
    int x = -minx, y = -miny, t = 0;
    for (const auto & stretch: wires[n]) {
      int d = std::abs(stretch.x) + std::abs(stretch.y);
      int u = (stretch.x > 0 ? 1 : stretch.x < 0 ? -1 : 0);
      int v = (stretch.y > 0 ? 1 : stretch.y < 0 ? -1 : 0);
      // std::cout << "Stretch length " << d << " direction " << u << " " << v << std::endl;
      for (int i = 0; i != d; ++ i) {
        x += u;
        y += v;
        ++t;
        //std::cout << x << " " << y << std::endl;
        if (!grid[x][y][n]) {
          grid[x][y][n] = t;
        }
      }
    }
  }
  int mind = 1 << 30;
  for (int x = 0; x != maxx - minx + 1; ++x) {
    for (int y = 0; y != maxy - miny + 1; ++y) {
      if (grid[x][y][0] && grid[x][y][1]) {
        int d = grid[x][y][0] + grid[x][y][1];
        mind = std::min(d, mind);
      }
    }
  }
  std::cout << "Timing delay " << mind << std::endl;
}



std::pair<std::array<wire_t, 2>, bool> read(std::istream & stream) {
  std::pair<std::array<wire_t, 2>, bool> result;
  std::regex regex1("([LRUD])(\\d+)");
  std::string s;
  for (int n = 0; n != 2; ++ n) {
    // std::cout << "Wire " << n << std::endl;
    if (!std::getline(stream, s)) {
      result.second = false;
      return result;
    }
    wire_t & wire = result.first[n];
    const char *begin = std::data(s);
    const char *end = begin + std::size(s);
    std::cmatch m;

    while (std::regex_search(begin, end, m, regex1)) {
      auto & stretch = wire.emplace_back();
      int direction, length;
      direction = m[1].first[0];
      length = 0; // !
      if (auto [p, ec] = std::from_chars(m[2].first, m[2].second, length); ec != std::errc()) {
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
      // std::cout << char(direction) << length << ", x " << stretch.x << " y " << stretch.y << std::endl;

      begin = m[1].second;
    }
  }
  result.second = true;
  return result;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  {
    std::ifstream in("3-test.data");
    while (true) {
      if (auto [wire, ok] = read(in); ok) {
        part_one(wire, true);
        part_two(wire, true);
      }
      else {
        break;
      }
    }
  }

  {
    std::ifstream in("3.data");
    while (true) {
      if (auto [wire, ok] = read(in); ok) {
        part_one(wire, true);
        part_two(wire, true);
      }
      else {
        break;
      }
    }
  }

  // {
  //   bool verbose = true;
  //   auto wires = read("3.data");
  //   part_one(wires, verbose);
  //   part_two(wires, false);
  // }

  return 0;
}
