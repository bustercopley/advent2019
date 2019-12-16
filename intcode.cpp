#include "precompiled.h"
#include "intcode.h"

using program_t = std::vector<int64_t>;

std::map<int64_t, std::string> mnemonics = {
  {99, "HLT"},
  {1, "ADD"},
  {2, "MUL"},
  {3, "INP"},
  {4, "OUT"},
  {5, "JNZ"},
  {6, "JZ "},
  {7, "LT "},
  {8, "EQ "},
  {9, "ARB"},
};

template <int N>
std::array<int64_t, N> get_params(const program_t &program, int64_t opcode,
  int64_t &pc, int64_t &base, int out, bool verbose) {

  if (verbose) {
    std::cout << std::setw(4) << (pc) << "; " << std::setw(6) << opcode << " "
              << mnemonics[opcode % 100];
  }
  ++pc;

  std::array<int64_t, N> params;
  opcode /= 100;
  for (int i = 0; i != N; ++i) {
    if (i == out) {
      if (opcode % 10 == 2) {
        int64_t addr = program[pc++];
        int64_t value = base + addr;
        params[i] = value;
        if (verbose) {
          std::cout << " @[" << base << "+" << addr << "]";
        }
      } else {
        int64_t value = program[pc++];
        params[i] = value;
        if (verbose) {
          std::cout << " @[" << value << "]";
        }
      }
    } else {
      if (opcode % 10 == 1) {
        int64_t value = program[pc++];
        params[i] = value;
        if (verbose) {
          std::cout << " " << value;
        }
      } else if (opcode % 10 == 2) {
        int64_t addr = program[pc++];
        int64_t value = program[base + addr];
        params[i] = value;
        if (verbose) {
          std::cout << " [" << base << "+" << addr << "](" << value << ")";
        }
      } else {
        int64_t addr = program[pc++];
        int64_t value = program[addr];
        params[i] = value;
        if (verbose) {
          std::cout << " [" << addr << "](" << value << ")";
        }
      }
    }
    opcode /= 10;
  }
  if (verbose) {
    std::cout << "\n\t\t\t\t\t";
  }
  return params;
}

int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::vector<int64_t> &inputs, bool verbose) {
  return run_until_output(
    program, pc, base,
    [&inputs]() {
      if (std::empty(inputs)) {
        throw "NO INPUT";
      }
      int64_t result = inputs[0];
      inputs.erase(inputs.begin(), inputs.begin() + 1);
      return result;
    },
    verbose);
}

int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::function<int64_t()> get_input, bool verbose) {
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      throw "RANGE ERROR";
    }

    int64_t opcode = program[pc];
    switch (opcode % 100) {

    case 99: // HALT
    {
      auto p = get_params<0>(program, opcode, pc, base, -1, verbose);
      (void)p;
      if (verbose) {
        std::cout << "halt\n--------------------------------------------\n";
      }
      pc = -1;
      return 0;
    }

    case 1: { // PLUS
      auto p = get_params<3>(program, opcode, pc, base, 2, verbose);
      if (verbose) {
        std::cout << "[" << p[2] << "] = " << p[0] << " + " << p[1] << " = "
                  << (p[0] + p[1]) << std::endl;
      }
      program[p[2]] = p[0] + p[1];
      break;
    }

    case 2: { // TIMES
      auto p = get_params<3>(program, opcode, pc, base, 2, verbose);
      if (verbose) {
        std::cout << "[" << p[2] << "] = " << p[0] << " * " << p[1] << " = "
                  << (p[0] * p[1]) << std::endl;
      }
      program[p[2]] = p[0] * p[1];
      break;
    }

    case 3: { // INPUT
      auto p = get_params<1>(program, opcode, pc, base, 0, verbose);
      int64_t input = get_input();
      if (verbose) {
        std::cout << "[" << p[0] << "] = input() = " << input << std::endl;
      }
      program[p[0]] = input;
      break;
    }

    case 4: { // OUTPUT
      auto p = get_params<1>(program, opcode, pc, base, -1, verbose);
      if (verbose) {
        std::cout << "output(" << p[0] << ")" << std::endl;
      }
      return p[0];
    }

    case 5: { // JUMP-IF-TRUE
      auto p = get_params<2>(program, opcode, pc, base, -1, verbose);
      int64_t cond = p[0];
      int64_t addr = p[1];
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
      auto p = get_params<2>(program, opcode, pc, base, -1, verbose);
      int64_t cond = p[0];
      int64_t addr = p[1];
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
      auto p = get_params<3>(program, opcode, pc, base, 2, verbose);
      int64_t a = p[0];
      int64_t b = p[1];
      int64_t addr = p[2];
      if (verbose) {
        std::cout << "[" << addr << "] = (" << a << " < " << b
                  << ") = " << (a < b ? 1 : 0) << std::endl;
      }
      program[addr] = (a < b ? 1 : 0);
      break;
    }

    case 8: { // EQUALS
      auto p = get_params<3>(program, opcode, pc, base, 2, verbose);
      int64_t a = p[0];
      int64_t b = p[1];
      int64_t addr = p[2];
      if (verbose) {
        std::cout << "[" << addr << "] = (" << a << " == " << b
                  << ") = " << (a == b ? 1 : 0) << std::endl;
      }
      program[addr] = (a == b ? 1 : 0);
      break;
    }

    case 9: { // ADJUST RELATIVE BASE
      auto p = get_params<1>(program, opcode, pc, base, 2, verbose);
      if (verbose) {
        std::cout << "base = base + " << p[0] << " = " << (base + p[0])
                  << std::endl;
      }
      base += p[0];
      break;
    }

    default:
      throw "OPCODE ERROR";
    }
  }
  __builtin_unreachable();
}

void run(
  const program_t &program, const std::vector<int64_t> &inputs, bool verbose) {
  int64_t pc = 0, base = 0;
  std::vector<int64_t> outputs;
  auto p = program;
  p.resize(65536);
  auto in = inputs;
  while (true) {
    int64_t output = run_until_output(p, pc, base, in, verbose);
    if (pc == -1) {
      break;
    }
    outputs.push_back(output);
  }
  if (!std::empty(outputs)) {
    std::cout << "Output";
    for (auto output : outputs) {
      std::cout << " " << output;
    }
    std::cout << std::endl;
  }
}

std::vector<program_t> read_programs(const char *filename) {
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
      int64_t op;
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
