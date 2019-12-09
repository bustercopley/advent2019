#include "precompiled.h"
#include "intcode.h"

template <typename F>
void best_permutation(const int (&inputs0)[5], bool verbose, F get_score) {
  int inputs[5], best_inputs[5], best_score = -1;
  std::memcpy(inputs, inputs0, sizeof inputs);
  do {
    if (verbose) {
      std::cout << "Phase settings:";
      for (int n = 0; n != 5; ++n) {
        std::cout << " " << inputs[n];
      }
      std::cout << "\n";
    }
    int score = get_score(inputs);
    if (score > best_score) {
      best_score = score;
      std::memcpy(best_inputs, inputs, sizeof inputs);
    }
  } while (std::next_permutation(&inputs[0], &inputs[5]));
  std::cout << "Best score " << best_score << " (inputs";
  for (auto input : best_inputs) {
    std::cout << " " << input;
  }
  std::cout << ")" << std::endl;
}

void part_one(const program_t &program, bool verbose) {
  best_permutation(
    {0, 1, 2, 3, 4}, verbose, [&program, verbose](const int(&inputs)[5]) {
      int64_t output = 0;
      for (int i = 0; i != 5; ++i) {
        program_t p = program;
        int64_t pc = 0, base = 0;
        std::vector<int64_t> program_inputs = {inputs[i], output};
        output = run_until_output(p, pc, base, program_inputs, verbose);
      }
      return output;
    });
}

void part_two(const program_t &program, bool verbose) {
  best_permutation(
    {5, 6, 7, 8, 9}, verbose, [&program, verbose](const int(&inputs)[5]) {
      struct amp_t {
        int64_t pc;
        std::vector<int64_t> inputs;
        program_t program;
      };
      amp_t amps[5];
      for (int i = 0; i != 5; ++i) {
        amps[i] = {0, {inputs[i]}, program};
      }
      amps[0].inputs.push_back(0);
      int i = 0;
      while (amps[i].pc != -1) {
        int64_t base = 0;
        int64_t output = run_until_output(
          amps[i].program, amps[i].pc, base, amps[i].inputs, verbose);
        bool halted = amps[i].pc == -1;
        i = (i + 1) % 5;
        if (!halted) {
          amps[i].inputs.push_back(output);
        }
      }
      if (verbose) {
        std::cout << "Halted amp " << i << " pending input";
        for (auto input : amps[i].inputs) {
          std::cout << " " << input;
        }
        std::cout << ";" << std::endl;
      }
      if (i) {
        throw "WRONG AMP";
      }
      if (std::empty(amps[i].inputs)) {
        throw "NO INPUT";
      }
      return amps[i].inputs[0];
    });
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    bool verbose = false;
    auto test_programs = read_programs("7-test.data");
    auto test_programs_2 = read_programs("7-test2.data");
    auto programs = read_programs("7.data");
    std::cout << "Tests\n";
    for (const auto &program : test_programs) {
      std::cout << "Part One Test\n";
      part_one(program, false);
    }
    for (const auto &program : test_programs_2) {
      std::cout << "Part Two Test\n";
      part_two(program, false);
    }

    std::cout << "\nAnswers\n";
    for (const auto &program : programs) {
      std::cout << "Part One\n";
      part_one(program, false);
    }
    for (const auto &program : programs) {
      std::cout << "Part Two\n";
      part_two(program, verbose);
    }
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
