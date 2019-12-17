#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

void part_one(program_t program) {
  rectangle_t<char> view;
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int64_t output;

  int x = 0;
  int y = 0;
  int width = 0, height;
  while (pc != -1) {
    output = run_until_output(program, pc, base, inputs, false);
    if (output == '\n') {
      width = std::max(width, x);
      x = 0;
      ++y;
    } else {
      view.set(x++, y, (char)output);
    }
  }
  height = y;
  view.put1(std::cout, [](bool, char x) -> char { return x ? x : ' '; });
  std::cout << "Width " << width << ", height " << height << std::endl;
  int64_t a = 0;
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      if (view.get(x, y).second != '#')
        continue;
      if (view.get(x - 1, y).second != '#')
        continue;
      if (view.get(x, y - 1).second != '#')
        continue;
      if (view.get(x + 1, y).second != '#')
        continue;
      if (view.get(x, y + 1).second != '#')
        continue;
      std::cout << x << "," << y << std::endl;
      a += x * y;
    }
  }

  d2d_stuff_t d2d_stuff;
  view.put2(d2d_stuff, L"", [](bool, char z) -> int64_t { return z % 8; });
  d2d_stuff.render_frames(10, text_style::segment, 1);

  std::cout << "Alignment " << a << std::endl;
}

void part_two_test() {
  static const int L = -1;
  static const int R = -2;

  std::vector<int> A = {R,4,L,10,L,10};
  std::vector<int> B = {L,8,R,12,R,10,R,4};
  std::vector<int> C = {L,8,L,8,R,10,R,4};
  std::vector<std::vector<int>> P = {A,B,A,B,A,C,B,C,A,C};

  rectangle_t<char> view;
  int x = 0, y = 0; // Position
  int u = 0, v = -1; // Direction

  for (auto V : P) {
    for (auto I : V) {
      switch (I) {
      case L: {
        int u1 = v;
        int v1 = -u;
        u = u1;
        v = v1;
      } break;
      case R: {
        int u1 = -v;
        int v1 = u;
        u = u1;
        v = v1;
      } break;
      default:
        for (int i = 0; i != I; ++i) {
          x += u;
          y += v;
          view.set(x, y, 1);
        }
        break;
      }
    }
  }

  view.set(0, 0, 2);
  d2d_stuff_t d2d_stuff;
  view.put2(d2d_stuff, L"", [](bool, char z) -> int64_t { return z % 8; });
  d2d_stuff.render_frames(10, text_style::segment, 1);
}

void part_two(program_t program) {
  // By hand :(
  std::string s = R"(A,B,A,B,A,C,B,C,A,C
R,4,L,10,L,10
L,8,R,12,R,10,R,4
L,8,L,8,R,10,R,4
n
)";

  program.resize(65536);
  program[0] = 2;
  int64_t pc = 0, base = 0;
  int64_t output;

  int i = 0;
  while (pc != -1) {
    output = run_until_output(program, pc, base, [&i, &s](){
      return s[i++];
    }, false);
    if (output < 128) {
      std::cout << (char)output;
    }
    else {
      std::cout << "\nDust: " << output << std::endl;
    }
  }
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto programs = read_programs("17.data");
    part_one(programs[0]);
    part_two_test();
    part_two(programs[0]);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
