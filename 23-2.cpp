#include <charconv>
#include <cmath>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <regex>
#include <tuple>
#include <vector>

#include "intcode2.h"

using intcode::ll;

struct packet {
  ll x, y;
};

enum class netcode { idle, notidle, natset };

struct node : intcode::machine {
  ll network_address;

  node(const intcode::program &p, ll i) : machine(p), network_address(i) {
    insert(network_address);
  }

  netcode schedule(std::vector<node> &nodes, packet &nat) {
    bool idle = true;
    while (true) {
      switch (run()) {

      case intcode::stopcode::input_error:
        insert(-1);
        run_insn();
        return idle ? netcode::idle : netcode::notidle;

      case intcode::stopcode::input:
        idle = false;
        break;

      case intcode::stopcode::output: {
        idle = false;
        ll dest = extract();
        if (dest == -1) {
          return netcode::notidle;
        } else {
          ll x = run_until_output();
          ll y = run_until_output();
          if (dest == 255) {
            nat = {x, y};
            return netcode::natset;
          } else {
            std::cout << "From " << this - std::data(nodes) << " to " << dest
                      << " {" << x << ", " << y << "}" << std::endl;
            nodes[dest].insert(x);
            nodes[dest].insert(y);
          }
        }
        break;
      }

      default:
        throw "Unexpected NIC stop";
      }
    }
  }
};

void part_one(const intcode::program &program) {
  std::vector<node> nodes;
  for (int i = 0; i != 50; ++i) {
    nodes.emplace_back(program, i);
  }

  packet nat;
  while (true) {
    for (auto &n : nodes) {
      if (n.schedule(nodes, nat) == netcode::natset) {
        std::cout << "Part one answer " << nat.y << std::endl;
        return;
      }
    }
  }
}

void part_two(const intcode::program &program) {
  std::vector<node> nodes;
  for (int i = 0; i != 50; ++i) {
    nodes.emplace_back(program, i);
  }

  ll nat_last_y = -1;
  packet nat;
  while (true) {
    bool all_idle = true;
    for (auto &n : nodes) {
      auto code = n.schedule(nodes, nat);
      if (code != netcode::idle) {
        all_idle = false;
      }
    }
    if (all_idle) {
      if (nat.y == nat_last_y) {
        std::cout << "Part two answer " << nat.y << std::endl;
        return;
      }
      nodes[0].insert(nat.x);
      nodes[0].insert(nat.y);
      nat_last_y = nat.y;
    }
  }
}

void do_it() try {
  for (auto filename : {"23.data"}) {
    std::cout << "Processing " << filename << std::endl;
    std::ifstream in(filename);
    intcode::program p;
    if (in >> p) {
      part_one(p);
      part_two(p);
    }
  }
} catch (const char *e) {
  std::cout << "Error: " << e << std::endl;
}

int main() {
  do_it();
  return 0;
}

int wmain() {
  do_it();
  return 0;
}
