#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"

int do_it(program_t program, rectangle_t<int64_t> &surface, bool verbose,
  bool verbose1) {
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int x = 0, y = 0; // Position
  int u = 0, v = 1; // Direction
  int count = 0;
  while (true) {
    {
      auto [set, value] = surface.get(x, y);
      inputs.push_back(value);
    }
    int64_t colour = run_until_output(program, pc, base, inputs, verbose);
    if (pc == -1) {
      break;
    }
    if (auto [set, value] = surface.get(x, y); !set) {
      ++count;
    }
    surface.set(x, y, colour);
    if (verbose1) {
      std::cout << "Paint " << x << "," << y << " " << colour << std::endl;
    }
    int64_t output = run_until_output(program, pc, base, inputs, verbose);
    if (pc == -1) {
      throw "UNEXPECTED HALT";
    }
    switch (output) {
    case 0: {
      // Turn left.
      // [u] = [ 0  1][u]
      // [v]   [-1  0]]v]
      int u1 = -v;
      int v1 = u;
      u = u1;
      v = v1;
      break;
    }
    case 1: {
      // Turn right.
      // [u] = [0  -1][u]
      // [v]   [1   0]]v]
      int u1 = v;
      int v1 = -u;
      u = u1;
      v = v1;
      break;
    }
    default:
      throw "BAD OUTPUT";
    }
    x += u;
    y += v;
    if (verbose1) {
      std::cout << "Move " << u << "," << v << " to " << x << "," << y
                << std::endl;
    }
  }
  return count;
}

void part_one(const program_t &program, bool verbose, bool verbose1) {
  rectangle_t<int64_t> surface;
  int count = do_it(program, surface, verbose, verbose1);
  std::cout << "Part one: " << count << std::endl;
}

void part_two(const program_t &program, bool verbose, bool verbose1) {
  rectangle_t<int64_t> surface;
  surface.set(0, 0, 1);
  do_it(program, surface, verbose, verbose1);
  std::cout << "Part two:\n";
  surface.put(std::cout, [](bool set, int64_t value) {
    return value ? '#' : ' ';
  }) << std::flush;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto programs = read_programs("11.data");
    part_one(programs[0], false, false);
    part_two(programs[0], false, false);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
