#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <cmath>

using field_t = std::vector<std::string>;

bool is_set(program_t program, int x, int y) {
  program.resize(8192);
  std::vector<int64_t> inputs = {x, y};
  int64_t pc = 0, base = 0;
  return run_until_output(program, pc, base, inputs, false);
}

void part_one(const program_t &program) {
  int64_t output = 0;
  for (int x = 0; x != 50; ++x) {
    for (int y = 0; y != 50; ++y) {
      output += is_set(program, x, y);
    }
  }
  std::cout << output << std::endl;
}

std::array<int, 2> get_onoff(program_t program, int y) {
  int on = 0;
  for (int x = 0; x != y * 1.5; ++x) {
    if (!on && is_set(program, x, y)) {
      on = x;
    } else if (on && !is_set(program, x, y)) {
      return {on, x};
    }
  }
  throw "ERROR";
}

// 1833 1989!
void paint(const program_t &program, int l0, int l1, int r0, int y0) {
  rectangle_t<char> r;
  int x0 = l0 - 10;
  for (int y = y0; y != y0 + 100; ++y) {
    for (int x = x0; x != y; ++x) {
      bool xflag = l1 <= x && x < r0;
      bool setflag = is_set(program, x, y);
      char c;
      if (xflag && setflag)
        c = 'O';
      else if (setflag)
        c = '#';
      else if (xflag)
        c = '!';
      else
        c = ' ';
      r.set(x - x0, y - y0, c);
    }
  }
  std::cout << l1 << "," << y0 << std::endl;
  r.put1(std::cout, [](bool, char c) { return c ? c : ' '; });
}

void part_two(const program_t &program) {
  int a = 0;
  while (!is_set(program, a, 100)) {
    ++a;
  }

  int y0 = (100 + a) / 0.094;
  for (int y = y0 - 200; y != y0 + 100; ++y) {
    auto [l0, r0] = get_onoff(program, y);
    auto [l1, r1] = get_onoff(program, y + 100);
    if (r0 - l1 == 100) {
      paint(program, l0, l1, r0, y);
      break;
    }
  }
}

void do_it() try {
  for (auto filename : {"19.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto programs = read_programs(filename);
    part_one(programs[0]);
    part_two(programs[0]);
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
