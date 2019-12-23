#include "intcode2.h"

#include <charconv>
#include <istream>
#include <regex>
#include <tuple>
#include <vector>

intcode::machine::machine(const intcode::program &p) : tape(p) {
  tape.resize(8192);
}

void intcode::machine::insert(intcode::ll value) {
  inputs.push_back(value);
}

intcode::ll intcode::machine::extract() {
  if (std::empty(outputs)) {
    throw "Output empty";
  }
  auto value = outputs.front();
  outputs.erase(outputs.begin());
  return value;
}

intcode::ll intcode::machine::range_check(intcode::ll address) {
  if (address < 0 || (std::size_t)address >= std::size(tape)) {
    throw "Address range error";
  }
  return address;
}

intcode::machine::arglist intcode::machine::get_args(ll opcode) {
  static const struct param_spec {
    int param_count;
    bool is_store;
  } param_specs[] = {
    {0, false}, // 99: HALT
    {3, true},  // 1: ADD
    {3, true},  // 2: MUL
    {1, true},  // 3: INPUT
    {1, false}, // 4: OUTPUT
    {2, false}, // 5: JNZ
    {2, false}, // 6: JZ
    {3, true},  // 7: LESS
    {3, true},  // 8: EQUAL
    {1, false}, // 9: ARB
  };

  const auto spec = param_specs[(opcode % 100) % 11];
  range_check(pc + spec.param_count - 1);
  arglist args(spec.param_count);
  opcode /= 100;
  for (int i = 0; i != spec.param_count; ++i) {
    ll raw = tape[pc++];
    if (spec.is_store && i == spec.param_count - 1) {
      switch (opcode % 10) {
      case 2: // relative address
        args[i] = range_check(raw + base);
        break;
      case 0: // absolute address
        args[i] = range_check(raw);
        break;
      default:
        throw "Bad output parameter mode";
      }
    } else {
      switch (opcode % 10) {
      case 2: // relative address
        args[i] = tape[range_check(raw + base)];
        break;
      case 1: // immediate
        args[i] = raw;
        break;
      case 0: // absolute address
        args[i] = tape[range_check(raw)];
        break;
      default:
        throw "Bad parameter mode";
      }
    }
    opcode /= 10;
  }
  return args;
}

intcode::stopcode intcode::machine::run_insn() {
  if (pc == -1) {
    throw "Halted";
  }
  range_check(pc);

  ll opcode = tape[pc++];
  auto args = get_args(opcode);

  switch (opcode % 100) {

  case 99: // HALT
    pc = -1;
    return stopcode::halt;

  case 1: // ADD
    tape[args[2]] = args[0] + args[1];
    return stopcode::none;

  case 2: // MUL
    tape[args[2]] = args[0] * args[1];
    return stopcode::none;

  case 3: // INPUT
    if (std::empty(inputs)) {
      pc -= 2;
      return stopcode::input_error;
    }
    tape[args[0]] = inputs.front();
    inputs.erase(std::begin(inputs));
    return stopcode::input;

  case 4: // OUTPUT
    outputs.push_back(args[0]);
    return stopcode::output;

  case 5: // JNZ
    if (args[0]) {
      pc = args[1];
    }
    return stopcode::none;

  case 6: // JZ
    if (!args[0]) {
      pc = args[1];
    }
    return stopcode::none;

  case 7: // LESS
    tape[args[2]] = args[0] < args[1];
    return stopcode::none;

  case 8: // EQUALS
    tape[args[2]] = args[0] == args[1];
    return stopcode::none;

  case 9: // ADJUST RELATIVE BASE
    base += args[0];
    return stopcode::none;

  default:
    throw "Opcode error";
  }
}

intcode::stopcode intcode::machine::run() {
  stopcode code = stopcode::none;
  do {
    code = run_insn();
  } while (code == stopcode::none);
  return code;
}

intcode::ll intcode::machine::run_until_output() {
  if (run() != stopcode::output) {
    throw "Expecting output";
  }
  return extract();
}


std::istream &intcode::operator>>(std::istream &in, intcode::program &p) {
  p.clear();
  std::string s;
  if (std::getline(in, s)) {
    std::regex re("(-?\\d+)");
    std::cmatch m;
    const char * begin = std::data(s);
    const char * end = begin + std::size(s);
    ll value = 0;
    while (std::regex_search(begin, end, m, re)) {
      std::from_chars(m[1].first, m[1].second, value);
      p.push_back(value);
      begin = m[1].second;
    }
    if (std::empty(p)) {
      in.setstate(std::ios::failbit);
    }
  }
  return in;
}
