#include "precompiled.h"
#include <cmath>

using program_t = std::vector<int>;

template <int N>
std::array<int, N> get_params(
  const program_t &program, int opcode, int &pc, bool verbose) {
  std::array<int, N> params;
  opcode /= 100;
  for (int i = 0; i != N; ++i) {
    if (opcode % 10) {
      int value = program[pc++];
      params[i] = value;
      if (verbose) {
        std::cout << " " << value;
      }
    } else {
      int addr = program[pc++];
      int value = program[addr];
      params[i] = value;
      if (verbose) {
        std::cout << " [" << addr << "]=" << value;
      }
    }
    opcode /= 10;
  }
  return params;
}

int set_immediate(int opcode, int n) {
  std::ostringstream oss;
  oss << "00000000" << opcode;
  std::string s = oss.str();
  s[std::size(s) - (3 + n)] = '1';
  std::istringstream iss(s);
  iss >> opcode;
  return opcode;
}

std::vector<int> input_stream;
int input_index = 0;

int get_input() {
  if (std::size_t(input_index) == std::size(input_stream)) {
    input_index = 0;
  }
  return input_stream[input_index++];
}

void put_output(int n) { std::cout << "Output(" << n << ")" << std::endl; }

int run(program_t &program, int pc, bool verbose) {
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      std::cout << "\nRANGE ERROR\n";
      return -1;
    }
    if (verbose) {
      std::cout << pc << "; " << program[pc];
    }

    int opcode = program[pc++];
    switch (opcode % 100) {

    case 99: // HALT
      if (verbose) {
        std::cout << " HALT " << program[0] << "\n";
      }
      return program[0];

    case 1: { // PLUS
      opcode = set_immediate(opcode, 2);
      auto p = get_params<3>(program, opcode, pc, verbose);
      if (verbose) {
        std::cout << "; [" << p[2] << "] = " << p[0] << " + " << p[1] << " = "
                  << (p[0] + p[1]) << std::endl;
      }
      program[p[2]] = p[0] + p[1];
      break;
    }

    case 2: { // TIMES
      opcode = set_immediate(opcode, 2);
      auto p = get_params<3>(program, opcode, pc, verbose);
      if (verbose) {
        std::cout << "; [" << p[2] << "] = " << p[0] << " * " << p[1] << " = "
                  << (p[0] * p[1]) << std::endl;
      }
      program[p[2]] = p[0] * p[1];
      break;
    }

    case 3: { // INPUT
      opcode = set_immediate(opcode, 0);
      auto p = get_params<1>(program, opcode, pc, verbose);
      int input = get_input();
      if (verbose) {
        std::cout << "; [" << p[0] << "] = input() = " << input << std::endl;
      }
      program[p[0]] = input;
      break;
    }

    case 4: { // OUTPUT
      auto p = get_params<1>(program, opcode, pc, verbose);
      int output = p[0];
      if (verbose) {
        std::cout << "; output(" << output << ")" << std::endl;
      }
      put_output(output);
      break;
    }

    case 5: { // JUMP-IF-TRUE
      auto p = get_params<2>(program, opcode, pc, verbose);
      int cond = p[0];
      int addr = p[1];
      if (verbose) {
        std::cout << "; if " << cond << " != 0 jump " << addr << std::endl;
      }
      if (cond != 0) {
        pc = addr;
      }
      break;
    }

    case 6: { // JUMP-IF-FALSE
      auto p = get_params<2>(program, opcode, pc, verbose);
      int cond = p[0];
      int addr = p[1];
      if (verbose) {
        std::cout << "; if " << cond << " == 0 jump " << addr << std::endl;
      }
      if (cond == 0) {
        pc = addr;
      }
      break;
    }

    // Opcode 7 is less than: if the first parameter is less than the second
    // parameter, it stores 1 in the position given by the third parameter.
    // Otherwise, it stores 0.
    case 7: { // LESS THAN
      opcode = set_immediate(opcode, 2);
      auto p = get_params<3>(program, opcode, pc, verbose);
      int a = p[0];
      int b = p[1];
      int addr = p[2];
      if (verbose) {
        std::cout << "; [" << addr << "] = (" << a << " < " << b
                  << ") = " << (a < b ? 1 : 0) << std::endl;
      }
      program[addr] = (a < b ? 1 : 0);
      break;
    }

    // Opcode 8 is equals: if the first parameter is equal to the second
    // parameter, it stores 1 in the position given by the third parameter.
    // Otherwise, it stores 0.
    case 8: { // EQUALS
      opcode = set_immediate(opcode, 2);
      auto p = get_params<3>(program, opcode, pc, verbose);
      int a = p[0];
      int b = p[1];
      int addr = p[2];
      if (verbose) {
        std::cout << "; [" << addr << "] = (" << a << " == " << b
                  << ") = " << (a == b ? 1 : 0) << std::endl;
      }
      program[addr] = (a == b ? 1 : 0);
      break;
    }

    default:
      std::cout << "\nOPCODE ERROR\n";
      return -1;
    }
  }
  __builtin_unreachable();
}

void part_one(const std::vector<program_t> &programs, bool verbose) {
  input_index = 0;
  input_stream = {1};
  auto program = programs[0];
  std::cout << "Part One\n";
  run(program, 0, verbose);
}

void part_two_tests(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "Part Two tests\n";
  input_stream = {8, 8, 8, 8};
  input_index = 0;
  for (auto program : programs) {
    run(program, 0, verbose);
  }
}

void part_two(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "Part Two\n";
  input_stream = {5};
  input_index = 0;
  program_t program = programs[0];
  run(program, 0, verbose);
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
    auto test_programs = read("5-test.data");
    auto programs = read("5.data");
    input_stream = {1};
    part_one(programs, false);
    part_two_tests(test_programs, true);
    part_two(programs, false);
  }

  return 0;
}
