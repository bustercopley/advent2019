#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

int64_t directions[] = {4, 1, 3, 2}; // E, N, W, S
int vectors[][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

enum contents_t { unknown, wall, empty, oxygen, droid };

struct tile_t {
  contents_t contents;
  int distance;
};

void do_it(program_t program) {
  rectangle_t<tile_t> maze;
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int direction = 0;
  int x = 0, y = 0;
  int ox = 0, oy = 0;
  int distance = 0, shortest = 0;
  int64_t output;
  maze.set(x, y, {empty, 0});

  for (int n = 0; n != 10000; ++n) {
    if (auto [is_set, tile] =
          maze.get(x + vectors[direction][0], y + vectors[direction][1]);
        tile.contents == wall) {
      // There is a wall ahead. Turn left.
      direction = (direction + 1) & 3;
      continue;
    }

    // Try to go forward.
    inputs = {directions[direction]};
    output = run_until_output(program, pc, base, inputs, false);
    x += vectors[direction][0];
    y += vectors[direction][1];

    if (output == 0) {
      // We hit a wall. Go back and turn left.
      maze.set(x, y, {wall, 0});
      x -= vectors[direction][0];
      y -= vectors[direction][1];
      direction = (direction + 1) & 3;
      continue;
    }

    // We went forward.
    ++distance;
    if (auto [is_set, tile] = maze.get(x, y); is_set) {
      distance = std::min(distance, tile.distance);
    }
    // Update map.
    maze.set(x, y, {empty, distance});
    if (output == 2) {
      ox = x;
      oy = y;
      shortest = distance;
    }
    // Turn right.
    direction = (direction + 3) & 3;
  }

  // Dump map.
  maze.set(0, 0, {droid, 0});
  maze.set(ox, oy, {oxygen, 0});
  maze.put(std::cout, [](bool set, const tile_t &tile) {
    switch (tile.contents) {
    case empty:
      return ' ';
    case droid:
      return 'D';
    case oxygen:
      return 'O';
    default:
      return '#';
    }
  });
  std::cout << "Distance " << shortest << std::endl;

  // Fill with oxygen.
  maze.set(0, 0, {empty, 0});
  std::set<std::array<int, 2>> oxygens = {{ox, oy}};
  int time = -1;
  bool finished = false;
  while (!finished) {
    finished = true;
    std::set<std::array<int, 2>> new_oxygens;
    for (auto tile : oxygens) {
      for (int direction = 0; direction != 4; ++direction) {
        int x = tile[0] + vectors[direction][0];
        int y = tile[1] + vectors[direction][1];
        if (auto [is_set, tile] = maze.get(x, y); tile.contents == empty) {
          maze.set(x, y, {oxygen, 0});
          new_oxygens.insert({x, y});
          finished = false;
        }
      }
    }
    oxygens = new_oxygens;
    ++time;
  }
  std::cout << "Time " << time << std::endl;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto programs = read_programs("15.data");
    do_it(programs[0]);
    // part_two(programs[0], false, false);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
