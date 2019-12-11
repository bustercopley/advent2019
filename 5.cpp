#include "precompiled.h"
#include "intcode.h"

void part_one(const std::vector<program_t> &programs, bool verbose) {
  auto program = programs[0];
  std::cout << "Part One\n";
  run(program, {1}, verbose);
}

void part_two_tests(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "\nPart Two tests\n";
  int inputs[] = {0, 8, 9};
  for (int input : inputs) {
    std::cout << "\nInput = " << input << std::endl;
    for (auto program : programs) {
      run(program, {input}, verbose);
    }
  }
  std::cout << "\n";
}

void part_two(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "Part Two\n";
  program_t program = programs[0];
  run(program, {5}, verbose);
}

std::vector<program_t> read(const char *filename) {
  std::ifstream in(filename);
  std::vector<program_t> programs;
  std::regex regex1("(-?\\d+)");
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
  auto test_programs = read("5-test.data");
  auto programs = read("5.data");
  bool verbose = false;
  part_one(programs, verbose);
  part_two_tests(test_programs, verbose);
  part_two(programs, verbose);
  return 0;
}
