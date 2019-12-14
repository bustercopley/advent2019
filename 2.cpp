#include "precompiled.h"
#include "intcode.h"

void part_zero(const std::vector<program_t> &programs, bool verbose) {
  for (auto program : programs) {
    std::cout << "Initial";
    for (int x : program) {
      std::cout << " " << x;
    }
    std::cout << "\n";

    run(program, {}, verbose);

    std::cout << "Final";
    for (int x : program) {
      std::cout << " " << x;
    }
    std::cout << "\n\n";
  }
}

void part_one(const program_t &program, bool verbose) {
  auto p = program;
  p[1] = 12;
  p[2] = 2;
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  run_until_output(p, pc, base, inputs, verbose);
  std::cout << "Part One, answer " << p[0] << std::endl;
}

void part_two(const program_t &program, bool verbose) {
  for (int a = 0; a != 100; ++a) {
    for (int b = 0; b != 100; ++b) {
      program_t p = program;
      p[1] = a;
      p[2] = b;
      int64_t pc = 0, base = 0;
      std::vector<int64_t> inputs;
      auto result = run_until_output(p, pc, base, inputs, verbose);
      if (verbose) {
        std::cout << a << " " << b << " -> " << result << std::endl;
      }
      if (p[0] == 19690720) {
        std::cout << "Part Two, parameters " << a << " " << b << ", answer "
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
      if (auto [p, ec] = std::from_chars(m[1].first, m[1].second, op);
          ec != std::errc{}) {
        throw "CONVERT ERROR";
      }
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
  try {
    {
      auto programs = read("2-test.data");
      part_zero(programs, true);
    }

    {
      auto programs = read("2.data");
      part_one(programs[0], false);
      part_two(programs[0], false);
    }

    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
