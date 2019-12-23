// -*- compile-command: "g++ -std=c++17 23-bug.cpp" -*-

#include <charconv>
#include <cmath>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <regex>
#include <tuple>
#include <vector>

// Intcode: interface.

// Caller is responsible for keeping track of 'pc' and 'base',
// and for resizing the program to some unspecified large size.

// program_t is just the tape.
using program_t = std::vector<int64_t>;

program_t read_program(const char *filename);

// If pc == -1 on return, the program encountered a HALT instruction.
// Otherwise, program encountered an OUTPUT instruction.
// The return value is -1 on HALT, or else the the OUTPUT value.
// 'get_input' is called once per INPUT instruction and supplies the INPUT
// result.
int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::function<int64_t()> get_input);

// Intcode: implementation.

template <int N>
std::array<int64_t, N> get_params(const program_t &program, int64_t opcode,
  int64_t &pc, int64_t &base, int out_param_index) {

  ++pc; // Skip opcode

  std::array<int64_t, N> params;
  opcode /= 100;
  for (int i = 0; i != N; ++i) {
    if (i == out_param_index) {
      // Destination operand.
      if (opcode % 10 == 2) {
        // Return a relative address.
        int64_t addr = program[pc++];
        int64_t value = base + addr;
        params[i] = value;
      } else {
        // Return an absolute address.
        int64_t value = program[pc++];
        params[i] = value;
      }
    } else {
      // Other operand.
      if (opcode % 10 == 1) {
        // Return the operand.
        int64_t value = program[pc++];
        params[i] = value;
      } else if (opcode % 10 == 2) {
        // Return the value at a relative address.
        int64_t addr = program[pc++];
        int64_t value = program[base + addr];
        params[i] = value;
      } else {
        // Return the value at an absolute address.
        int64_t addr = program[pc++];
        int64_t value = program[addr];
        params[i] = value;
      }
    }
    opcode /= 10;
  }
  return params;
}

int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::function<int64_t()> get_input) {
  while (true) {
    if (pc < 0 || std::size_t(pc) >= std::size(program)) {
      throw "RANGE ERROR";
    }

    int64_t opcode = program[pc];
    switch (opcode % 100) {

    case 99: // HALT
    {
      auto p = get_params<0>(program, opcode, pc, base, -1);
      (void)p;
      pc = -1;
      return 0;
    }

    case 1: { // ADD
      auto p = get_params<3>(program, opcode, pc, base, 2);
      program[p[2]] = p[0] + p[1];
      break;
    }

    case 2: { // MULTIPLY
      auto p = get_params<3>(program, opcode, pc, base, 2);
      program[p[2]] = p[0] * p[1];
      break;
    }

    case 3: { // INPUT
      auto p = get_params<1>(program, opcode, pc, base, 0);
      int64_t input = get_input();
      program[p[0]] = input;
      break;
    }

    case 4: { // OUTPUT
      auto p = get_params<1>(program, opcode, pc, base, -1);
      return p[0];
    }

    case 5: { // JUMP-IF-TRUE
      auto p = get_params<2>(program, opcode, pc, base, -1);
      int64_t cond = p[0];
      int64_t addr = p[1];
      if (cond) {
        pc = addr;
      }
      break;
    }

    case 6: { // JUMP-IF-FALSE
      auto p = get_params<2>(program, opcode, pc, base, -1);
      int64_t cond = p[0];
      int64_t addr = p[1];
      if (!cond) {
        pc = addr;
      }
      break;
    }

    case 7: { // LESS THAN
      auto p = get_params<3>(program, opcode, pc, base, 2);
      int64_t a = p[0];
      int64_t b = p[1];
      int64_t addr = p[2];
      program[addr] = (a < b ? 1 : 0);
      break;
    }

    case 8: { // EQUALS
      auto p = get_params<3>(program, opcode, pc, base, 2);
      int64_t a = p[0];
      int64_t b = p[1];
      int64_t addr = p[2];
      program[addr] = (a == b ? 1 : 0);
      break;
    }

    case 9: { // ADJUST RELATIVE BASE
      auto p = get_params<1>(program, opcode, pc, base, 2);
      base += p[0];
      break;
    }

    default:
      throw "OPCODE ERROR";
    }
  }
  __builtin_unreachable();
}

program_t read_program(const char *filename) {
  std::ifstream in(filename);
  std::vector<program_t> programs;
  std::regex regex1("(-?\\d+)");
  std::string s;
  program_t program;
  while (std::getline(in, s)) {
    const char *begin = std::data(s);
    const char *end = begin + std::size(s);
    std::cmatch m;
    while (std::regex_search(begin, end, m, regex1)) {
      // 'from_chars' always succeeds, but the compiler doesn't know that, so
      // initialize 'op' to avoid a warning
      int64_t op = 0;
      std::from_chars(m[1].first, m[1].second, op);
      program.push_back(op);
      begin = m[1].second;
    }
  }
  return program;
}

// Intcode implementation ends.

// Day 23 Part One

// node_t: an individual Network Interface Controller
struct node_t {

  int64_t network_address = 0;

  // Intcode housekeeping.
  program_t program;
  std::vector<int64_t> inputs;
  int64_t pc = 0, base = 0;

  node_t(const program_t &program, int i)
    : network_address(i), program(program) {
    this->program.resize(16384);

    // Annoyingly, the implementation doesn't (yet) support stopping at an
    // input instruction.

    // We can stop *after* an input instruction by saving 'pc' and jumping to a
    // halt instruction, from within the input generator (the function
    // 'get_input' passed to 'run_until_output'). Insert the halt instruction
    // here, at program location 16384.
    this->program.push_back(99);

    // Send the NIC its network address.
    inputs.push_back(i);
  }

  void recv(int64_t x, int64_t y) {
    inputs.push_back(x);
    inputs.push_back(y);
  }

  // Return { output_flag, destination, x, y }
  // * If 'output_flag' is true, the NIC asked to send a packet { x, y }
  //   to 'destination'.
  // * If 'output_flag' is false, the NIC emitted '-1' (no packet) or tried
  //   to read input from an empty queue (and received '-1').
  std::tuple<bool, int64_t, int64_t, int64_t> schedule() {

    bool yielded = false;
    int64_t resume = -1;

    auto get_input = [&]() -> int64_t {
      if (std::empty(inputs)) {
        // No input available. The input instruction receives value '-1',
        // then yields (by jumping the the HALT instruction we inserted
        // at program address 16384). When the HALT instruction is
        // executed, 'run_until_output' returns, and we restore 'pc' to
        // its previous value, just after the input instruction.
        resume = pc;
        pc = 16384;
        yielded = true;
        std::cout << "Node " << network_address << " input -1 (and yield)"
                  << std::endl;
        return -1;
      } else {
        // Input available. Consume and discard one value from the
        // input queue.
        auto result = inputs.front();
        inputs.erase(std::begin(inputs), std::begin(inputs) + 1);
        std::cout << "Node " << network_address << " input " << result
                  << std::endl;
        return result;
      }
    };

    int64_t destination = run_until_output(program, pc, base, get_input);
    if (yielded) {
      pc = resume;
      return {false, 0, 0, 0};
    }

    if (destination == -1) {
      std::cout << "Node " << network_address << " output -1" << std::endl;
      return {false, 0, 0, 0};
    }

    auto x = run_until_output(program, pc, base, get_input);
    auto y = run_until_output(program, pc, base, get_input);
    std::cout << "Node " << network_address << " send {" << x << ", " << y
              << "} to " << destination << std::endl;
    return {true, destination, x, y};
  }
};

void part_one(const program_t &program) {
  std::vector<node_t> nodes;
  for (int n = 0; n != 50; ++n) {
    nodes.emplace_back(program, n);
  }

  int n = 0;
  while (true) {
    auto [flag, addr, x, y] = nodes[n].schedule();
    if (flag) {
      if (addr == 255) {
        // Day one answer.
        std::cout << "Broadcast " << x << ", " << y << std::endl;
        return;
      }
      nodes[addr].recv(x, y);
      n = addr;
    } else {
      n = (n + 1) % 50;
    }
  }
}

void do_it() try {
  for (auto filename : {"23.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto program = read_program(filename);
    if (std::empty(program)) {
      std::cout << "Couldn't read program from \"" << filename << "\""
                << std::endl;
    }
    part_one(program);
  }
} catch (const char *e) {
  std::cout << e << std::endl;
}

int main() {
  do_it();
  return 0;
}

int wmain() {
  do_it();
  return 0;
}
