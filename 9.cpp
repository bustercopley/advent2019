#include "precompiled.h"
#include "intcode.h"

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    bool verbose = false;
    auto test_programs = read_programs("9-test.data");
    auto programs = read_programs("9.data");
    std::cout << "Part One Tests\n";
    for (const auto &program : test_programs) {
      if (verbose) {
        std::cout << "\nBegin" << std::endl;
      }
      run(program, {}, verbose);
    }
    for (const auto &program : programs) {
      std::cout << "Part One\n";
      run(program, {1}, false);

      std::cout << "Part Two\n";
      run(program, {2}, false);
    }
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
