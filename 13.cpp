#include "precompiled.h"
#include "d2d.h"
#include "intcode.h"
#include "rectangle.h"

void paint(
  d2d_stuff_t &d2d_stuff, rectangle_t<int64_t> &surface, int64_t score) {
  std::basic_ostringstream<WCHAR> ostr;
  ostr << L"Score " << score;
  surface.put2(d2d_stuff, ostr.str(),
    [](bool set, int64_t value) -> int { return value; });
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
  d2d_stuff_t d2d_stuff;
  while (true) {
    int64_t x = run_until_output(program, pc, base, inputs, verbose);
    if (pc == -1) {
      paint(d2d_stuff, surface, score);
      std::cout << "Halt" << std::endl;
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
        paint(d2d_stuff, surface, score);
      }
    }
  }
  d2d_stuff.render_frames(8);
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
