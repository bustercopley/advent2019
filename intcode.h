// -*- C++ -*-
#ifndef intcode_h
#define intcode_h

#include "precompiled.h"

using program_t = std::vector<int64_t>;

int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::vector<int64_t> &inputs, bool verbose);

int64_t run_until_output(program_t &program, int64_t &pc, int64_t &base,
  std::function<int64_t()> get_input, bool verbose);

void run(
  const program_t &program, const std::vector<int64_t> &inputs, bool verbose);

std::vector<program_t> read_programs(const char *filename);

#endif
