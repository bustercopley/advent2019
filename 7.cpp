#include "precompiled.h"
#include <cmath>

using program_t = std::vector<int>;

std::map<int, std::string> mnemonics = {
  {99, "HLT"},
  {1, "ADD"},
  {2, "MUL"},
  {3, "INP"},
  {4, "OUT"},
  {5, "JNZ"},
  {6, "JZ "},
  {7, "LT "},
  {8, "EQ "},
};

template <int N>
std::array<int, N> get_params(
  const program_t &program, int opcode, int &pc, int out, bool verbose) {

  if (verbose) {
    std::cout << std::setw(4) << (pc) << "; " << std::setw(6) << opcode << " "
              << mnemonics[opcode % 100];
  }
  ++pc;

  std::array<int, N> params;
  opcode /= 100;
  for (int i = 0; i != N; ++i) {
    if (i == out) {
      int value = program[pc++];
      params[i] = value;
      if (verbose) {
        std::cout << " @" << value;
      }
    } else if (opcode % 10) {
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

void put_output(int n) {}

std::pair<bool, int> run(
  program_t &program, int &pc, std::vector<int> &inputs, bool verbose) {
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      throw "RANGE ERROR";
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
      pc = -1;
      return {true, 0};
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
      if (std::empty(inputs)) {
        throw "INPUT ERROR";
      }
      auto p = get_params<1>(program, opcode, pc, 0, verbose);
      int input = inputs[0];
      inputs.erase(inputs.begin(), inputs.begin() + 1);
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
      return {false, output};
      break;
    }

    case 5: { // JUMP-IF-TRUE
      auto p = get_params<2>(program, opcode, pc, -1, verbose);
      int cond = p[0];
      int addr = p[1];
      if (verbose) {
        std::cout << "if " << cond << " jump " << addr
                  << (cond ? " (taken)" : " (not taken)") << std::endl;
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
                  << (!cond ? " (taken)" : " (not taken)") << std::endl;
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
      throw "OPCODE ERROR";
    }
  }
  __builtin_unreachable();
}

void part_one(const program_t &program, bool verbose) {
  int i[5] = {0, 1, 2, 3, 4};
  int best_i[5];
  int best_result = -1;
  while (std::next_permutation(&i[0], &i[5])) {
    if (verbose) {
      std::cout << "Phase settings:";
      for (int n = 0; n != 5; ++n) {
        std::cout << " " << i[n];
      }
      std::cout << "\n";
    }
    int result = 0;
    for (int n = 0; n != 5; ++n) {
      auto p = program;
      int pc = 0;
      std::vector<int> inputs = {i[n], result};
      auto [halt, new_result] = run(p, pc, inputs, verbose);
      result = new_result;
      if (result == -1) {
        std::cout << "Quit\n";
        break;
      }
    }
    if (result > best_result) {
      best_result = result;
      for (int n = 0; n != 5; ++n) {
        best_i[n] = i[n];
      }
    }
  }
  std::cout << "Best result " << best_result << " (input ";
  for (int n = 0; n != 5; ++n) {
    std::cout << best_i[n];
  }
  std::cout << ")" << std::endl;
}

void part_two(const program_t &program, bool verbose) {
  int i[5] = {5, 6, 7, 8, 9};
  int best_i[5];
  int best_result = -1;
  while (std::next_permutation(&i[0], &i[5])) {
    if (verbose) {
      std::cout << "Phase settings:";
      for (int n = 0; n != 5; ++n) {
        std::cout << " " << i[n];
      }
      std::cout << "\n";
    }
    std::tuple<std::vector<int>, int, program_t> amps[5];
    for (int n = 0; n != 5; ++n) {
      std::get<0>(amps[n]).push_back(i[n]);
      std::get<2>(amps[n]) = program;
    }
    std::get<0>(amps[0]).push_back(0);
    int n = 0;
    while (std::get<1>(amps[n]) != -1) {
      if (verbose) {
        std::cout << "Resuming amp " << n << std::endl;
      }
      auto [halt, result] = run(std::get<2>(amps[n]), std::get<1>(amps[n]),
        std::get<0>(amps[n]), verbose);
      n = (n + 1) % 5;
      std::get<0>(amps[n]).push_back(result);
    }
    int result = std::get<0>(amps[n])[0];
    if (n || verbose) {
      std::cout << "Sent input " << std::get<0>(amps[n])[0] << " to halted amp "
                << n << std::endl;
    }
    if (n) {
      throw "WRONG AMP";
    }
    if (result > best_result) {
      best_result = result;
      for (int n = 0; n != 5; ++n) {
        best_i[n] = i[n];
      }
    }
  }
  std::cout << "Best result " << best_result << " (input ";
  for (int n = 0; n != 5; ++n) {
    std::cout << best_i[n];
  }
  std::cout << ")" << std::endl;
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
  try {
    auto test_programs = read("7-test.data");
    auto test_programs_2 = read("7-test2.data");
    auto programs = read("7.data");
    for (const auto &program : test_programs) {
      std::cout << "Part One Test\n";
      part_one(program, false);
    }
    for (const auto &program : programs) {
      std::cout << "Part One\n";
      part_one(program, false);
    }
    for (const auto &program : test_programs_2) {
      std::cout << "Part Two Test\n";
      part_two(program, false);
    }
    for (const auto &program : programs) {
      std::cout << "Part Two\n";
      part_two(program, false);
    }
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
