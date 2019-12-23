// -*- C++ -*-
#ifndef intcode2_h
#define intcode2_h

#include <cstdint>
#include <istream>
#include <vector>

namespace intcode {

using ll = std::int64_t;
enum class stopcode { none, input, input_error, output, halt };

struct program : public std::vector<ll> {};

struct machine {
  machine(const program &p);
  stopcode run();
  stopcode run_insn();
  ll run_until_output();
  void insert(ll value);
  ll extract();

private:
  std::vector<ll> tape;
  std::vector<ll> inputs;
  std::vector<ll> outputs;
  ll pc = 0;
  ll base = 0;

  using arglist = std::vector<ll>;
  arglist get_args(ll opcode);
  ll range_check(ll address);
};

std::istream &operator>>(std::istream &in, program &p);

} // namespace intcode

#endif
