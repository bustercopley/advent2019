#include "precompiled.h"
#include "d2d.h"
#include "intcode.h"
#include "rectangle.h"

int paint(
  d2d_stuff_t &d2d_stuff, rectangle_t<int64_t> &surface, int64_t score) {
  std::basic_ostringstream<WCHAR> ostr;
  ostr << std::setfill(L'0') << std::setw(5) << score;
  int count = 0;
  surface.put2(d2d_stuff, ostr.str(), [&count](bool set, int64_t value) -> int {
    count += value == 2;
    return value;
  });
  return count;
}

void part_two(program_t program, bool verbose, bool verbose1) {
  rectangle_t<int64_t> surface;
  program.resize(65536);
  program[0] = 2;
  int64_t pc = 0, base = 0;
  int score = 0;
  int targetx = -1;
  int paddlex = -1;
  d2d_stuff_t d2d_stuff;
  bool first_frame = true;

  auto get_input = [&]() {
    int count = paint(d2d_stuff, surface, score);
    if (first_frame) {
      std::cout << "Blocks " << count << std::endl;
      first_frame = false;
    }
    return (paddlex < targetx) - (paddlex > targetx);
  };

  while (true) {
    int64_t x = run_until_output(program, pc, base, get_input, verbose);
    if (pc == -1) {
      paint(d2d_stuff, surface, score);
      std::cout << "Score " << score << std::endl;
      break;
    }
    int64_t y = run_until_output(program, pc, base, get_input, verbose);
    int64_t z = run_until_output(program, pc, base, get_input, verbose);

    if (x == -1 && y == 0) {
      score = z;
    } else {
      surface.set(x, y, z);
      if (z == 3) {
        paddlex = x;
      }
      if (z == 4) {
        targetx = x;
      }
    }
  }
  d2d_stuff.render_frames(10, text_style::segment, 5);
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
