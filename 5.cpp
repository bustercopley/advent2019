#include "precompiled.h"
#include <cmath>

using program_t = std::vector<int>;

std::map <int, std::string> mnemonics = {
  { 99, "HLT" },
  { 1, "ADD" },
  { 2, "MUL" },
  { 3, "INP" },
  { 4, "OUT" },
  { 5, "JNZ" },
  { 6, "JZ " },
  { 7, "LT " },
  { 8, "EQ " },
};

template <int N>
std::array<int, N> get_params(
  const program_t &program, int opcode, int &pc, int out, bool verbose) {

  if (verbose) {
    std::cout << std::setw(4) << (pc++)
              << "; " << std::setw(6) << opcode
              << " " << mnemonics[opcode % 100];
  }

  std::array<int, N> params;
  opcode /= 100;
  for (int i = 0; i != N; ++i) {
    if (i == out) {
      int value = program[pc++];
      params[i] = value;
      if (verbose) {
        std::cout << " @" << value;
      }
    }
    else if (opcode % 10) {
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
        std::cout << " [" << addr << "](" << value << ")";
      }
    }
    opcode /= 10;
  }
  if (verbose) {
    std::cout << "\n\t\t\t\t\t";
  }
  return params;
}

void put_output(int n) { }

int run(program_t &program, int pc, std::vector<int> inputs, bool verbose) {
  int input_index = 0;
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      std::cout << "\nRANGE ERROR\n";
      return -1;
    }

    int opcode = program[pc];
    switch (opcode % 100) {

    case 99: // HALT
      {
        auto p = get_params<0>(program, opcode, pc, -1, verbose);
        (void)p;
        if (verbose) {
          std::cout << "halt\n--------------------------------------------\n";
        }
        return program[0];
      }

    case 1: { // PLUS
      auto p = get_params<3>(program, opcode, pc, 2, verbose);
      if (verbose) {
        std::cout << "[" << p[2] << "] = " << p[0] << " + " << p[1] << " = "
                  << (p[0] + p[1]) << std::endl;
      }
      program[p[2]] = p[0] + p[1];
      break;
    }

    case 2: { // TIMES
      auto p = get_params<3>(program, opcode, pc, 2, verbose);
      if (verbose) {
        std::cout << "[" << p[2] << "] = " << p[0] << " * " << p[1] << " = "
                  << (p[0] * p[1]) << std::endl;
      }
      program[p[2]] = p[0] * p[1];
      break;
    }

    case 3: { // INPUT
      auto p = get_params<1>(program, opcode, pc, 0, verbose);
      if (std::size_t(input_index) == std::size(inputs)) {
        std::cout << "\nINPUT ERROR\n";
        return -1;
      }
      int input = inputs[input_index++];
      if (verbose) {
        std::cout << "[" << p[0] << "] = input() = " << input << std::endl;
      }
      program[p[0]] = input;
      break;
    }

    case 4: { // OUTPUT
      auto p = get_params<1>(program, opcode, pc, -1, verbose);
      int output = p[0];
      if (verbose) {
        std::cout << "output(" << output << ")" << std::endl;
      }
      put_output(output);
      break;
    }

    case 5: { // JUMP-IF-TRUE
      auto p = get_params<2>(program, opcode, pc, -1, verbose);
      int cond = p[0];
      int addr = p[1];
      if (verbose) {
        std::cout << "if " << cond << " jump " << addr
                  << (cond ? " (taken)" : " (not taken)")
                  << std::endl;
      }
      if (cond) {
        pc = addr;
      }
      break;
    }

    case 6: { // JUMP-IF-FALSE
      auto p = get_params<2>(program, opcode, pc, -1, verbose);
      int cond = p[0];
      int addr = p[1];
      if (verbose) {
        std::cout << "if not " << cond << " jump " << addr
                  << (!cond ? " (taken)" : " (not taken)")
                  << std::endl;
      }
      if (!cond) {
        pc = addr;
      }
      break;
    }

    case 7: { // LESS THAN
      auto p = get_params<3>(program, opcode, pc, 2, verbose);
      int a = p[0];
      int b = p[1];
      int addr = p[2];
      if (verbose) {
        std::cout << "[" << addr << "] = (" << a << " < " << b
                  << ") = " << (a < b ? 1 : 0) << std::endl;
      }
      program[addr] = (a < b ? 1 : 0);
      break;
    }

    case 8: { // EQUALS
      auto p = get_params<3>(program, opcode, pc, 2, verbose);
      int a = p[0];
      int b = p[1];
      int addr = p[2];
      if (verbose) {
        std::cout << "[" << addr << "] = (" << a << " == " << b
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
  auto program = programs[0];
  std::cout << "Part One\n";
  run(program, 0, {1}, verbose);
}

void part_two_tests(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "\nPart Two tests\n";
  int inputs[] = { 0, 8, 9 };
  for (int input: inputs) {
    std::cout << "\nInput = " << input << std::endl;
    for (auto program : programs) {
      run(program, 0, {input}, verbose);
    }
  }
  std::cout << "\n";
}

void part_two(const std::vector<program_t> &programs, bool verbose) {
  std::cout << "Part Two\n";
  program_t program = programs[0];
  run(program, 0, {5}, verbose);
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
  auto test_programs = read("5-test.data");
  auto programs = read("5.data");
  bool verbose = true;
  part_one(programs, verbose);
  part_two_tests(test_programs, verbose);
  part_two(programs, verbose);
  return 0;
}
