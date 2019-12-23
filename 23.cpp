#include "precompiled.h"
#include "intcode.h"
#include <charconv>
#include <cmath>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <regex>
#include <tuple>
#include <vector>

static const int quantum = 100;

struct yield {};

struct node_t {
  node_t(const program_t &program, int i) : address(i), program(program) {
    this->program.resize(16384);
    this->program.push_back(99);
    inputs.push_back(i);
  }

  void recv(int x, int y) {
    inputs.push_back(x);
    inputs.push_back(y);
  }

  bool schedule(std::vector<node_t> &nodes) {
    while (true) {
      bool yielded = false;
      int64_t resume = -1;

      auto get_input = [&]() -> int64_t {
        if (std::empty(inputs)) {
          // Receive -1, but yield after this instruction.
          resume = pc;
          pc = 16384;
          yielded = true;
          return -1;
        } else {
          auto result = inputs.front();
          inputs.erase(std::begin(inputs), std::begin(inputs) + 1);
          return result;
        }
      };

      int64_t destination = run_until_output(program, pc, base, get_input, false);
      if (yielded) {
        pc = resume;
        return true;
      }

      if (destination == -1) {
        std::cout << "Node " << address << " pc " << pc << ", output -1"
                  << std::endl;
        return true;
      }


      auto x = run_until_output(program, pc, base, inputs, false);
      auto y = run_until_output(program, pc, base, inputs, false);
      std::cout << "Node " << address << ", output destination "
                << destination << ", x " << x << ", y " << y << std::endl;

      if (destination == 255) {
        return false;
      }

      nodes[destination].inputs.push_back(x);
      nodes[destination].inputs.push_back(y);
    }
  }

  int64_t address = 0;
  program_t program;
  std::vector<int64_t> inputs;
  int64_t pc = 0, base = 0;
};

void part_one(const program_t &program) {
  std::vector<node_t> nodes;
  for (int n = 0; n != 50; ++n) {
    nodes.emplace_back(program, n);
  }

  int n = 0;
  while (nodes[n].schedule(nodes)) {
    n = (n + 1) % 50;
  }
}

void do_it() try {
  for (auto filename : {"23.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto programs = read_programs(filename);
    part_one(programs[0]);
    // part_two(programs[0]);
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
