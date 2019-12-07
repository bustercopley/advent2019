#include "precompiled.h"
#include <cmath>

using program_t = std::vector<int>;

template <int N>
std::array<int, N> get_params(const program_t &program, int &pc, bool verbose) {
  std::array<int, N> params;
  for (int i = 0; i != N; ++i) {
    params[i] = program[pc++];
    if (verbose) {
      std::cout << " " << params[i];
    }
  }
  return params;
}

int run(program_t &program, int pc, bool verbose) {
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      std::cout << "\nRANGE ERROR\n";
      return -1;
    }
    if (verbose) {
      std::cout << pc << "; " << program[pc];
    }
    switch (program[pc++]) {

    case 99: // 99: HALT
      if (verbose) {
        std::cout << " HALT " << program[0] << "\n";
      }
      return program[0];

    case 1: { // 1 a b c: [c] = [a] + [b]
      auto p = get_params<3>(program, pc, verbose);
      program[p[2]] = program[p[0]] + program[p[1]];
      if (verbose) {
        std::cout << "; [" << p[2] << "] <- " << program[p[2]] << "\n";
      }
      break;
    }

    case 2: { // 2 a b c: [c] = [a] * [b]
      auto p = get_params<3>(program, pc, verbose);
      program[p[2]] = program[p[0]] * program[p[1]];
      if (verbose) {
        std::cout << "; [" << p[2] << "] <- " << program[p[2]] << "\n";
      }
      break;
    }

    default:
      std::cout << "\nOPCODE ERROR\n";
      return -1;
    }
  }
  __builtin_unreachable();
}

void part_zero(const std::vector<program_t> &programs, bool verbose) {
  for (auto program : programs) {
    std::cout << "Initial:";
    for (int x : program) {
      std::cout << " " << x;
    }
    std::cout << "\n";

    run(program, 0, verbose);

    std::cout << "Final:";
    for (int x : program) {
      std::cout << " " << x;
    }
    std::cout << "\n\n";
  }
}

void part_one(const std::vector<program_t> &programs, bool verbose) {
  auto program = programs[0];
  program[1] = 12;
  program[2] = 2;
  std::cout << "Part One\n";
  run(program, 0, verbose);
}

void part_two(const std::vector<program_t> &programs, bool verbose) {
  for (int a = 0; a != 100; ++a) {
    for (int b = 0; b != 100; ++b) {
      program_t program = programs[0];
      program[1] = a;
      program[2] = b;
      int result = run(program, 0, false);
      if (verbose) {
        std::cout << a << " " << b << " -> " << result << std::endl;
      }
      if (result == 19690720) {
        std::cout << "Part Two: parameters " << a << " " << b << ", answer "
                  << (100 * a + b) << std::endl;
        break;
      }
    }
  }
}

std::vector<program_t> read(const char *filename) {
  std::ifstream in(filename);
  std::vector<program_t> programs;
  std::regex regex1("(\\d+)");
  std::string s;
  while (std::getline(in, s)) {
    program_t program;
    const char *begin = std::data(s);
    const char *end = begin + std::size(s);
    std::cmatch m;
    while (std::regex_search(begin, end, m, regex1)) {
      int op;
      std::from_chars(m[1].first, m[1].second, op);
      program.push_back(op);
      begin = m[1].second;
    }
    if (std::size(program)) {
      programs.push_back(std::move(program));
    }
  }
  return programs;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  {
    auto programs = read("2-test.data");
    part_zero(programs, true);
  }

  {
    bool verbose = true;
    auto programs = read("2.data");
    part_one(programs, verbose);
    part_two(programs, false);
  }

  return 0;
}
