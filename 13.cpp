#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include "d2d.h"

void part_one(program_t program, bool verbose, bool verbose1) {
  rectangle_t<int64_t> surface;
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  while (true) {
    int64_t x = run_until_output(program, pc, base, inputs, verbose);
    if (pc == -1) {
      break;
    }
    int64_t y = run_until_output(program, pc, base, inputs, verbose);
    int64_t z = run_until_output(program, pc, base, inputs, verbose);
    surface.set(x, y, z);
  }
  int count = 0;
  surface.put1(std::cout, [&count](bool set, int64_t value) {
    if (value == 2)
      ++count;
    return " #=-O"[value];
  });
  std::cout << count << std::endl;
}

void part_two(program_t program, bool verbose, bool verbose1) {
  rectangle_t<int64_t> surface;
  program.resize(65536);
  program[0] = 2;
  int64_t pc = 0, base = 0;
  int score = 0;
  std::vector<int64_t> inputs;
  int ballx = -1;
  int paddlex = -1;
  int frame_index = 0;
  d2d_stuff_t d2d_stuff;
  int max_width = 0, max_height = 0;
  while (true) {
    int64_t x = run_until_output(program, pc, base, inputs, verbose);
    if (pc == -1) {
      break;
    }
    int64_t y = run_until_output(program, pc, base, inputs, verbose);
    int64_t z = run_until_output(program, pc, base, inputs, verbose);

    if (x == -1 && y == 0) {
      score = z;
      std::cout << "Score " << score << std::endl;
    } else {
      surface.set(x, y, z);
      if (z == 3) {
        paddlex = x;
      }
      if (z == 4) {
        ballx = x;
        inputs = {(paddlex < ballx) - (paddlex > ballx)};
        int count = 0;
        auto [width, height] = surface.size();
        max_width = std::max(max_width, width);
        max_height = std::max(max_height, height);
        surface.put2(d2d_stuff, frame_index++, [&count](bool set, int64_t value) -> int {
          if (value == 2) {
            ++count;
          }
          return value;
        });
        if (!count)
          break;
      }
    }
  }
  d2d_stuff.render_frames(8, max_width, max_height);
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto programs = read_programs("13.data");
    // part_one(programs[0], false, false);
    part_two(programs[0], false, false);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
