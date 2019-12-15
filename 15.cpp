#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

int64_t directions[] = {4, 1, 3, 2}; // E, N, W, S
int vectors[][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

enum contents_t { unknown, wall, empty, oxygen, droid, pump, path };

struct tile_t {
  contents_t contents;
  int time;
};

void do_it(program_t program) {
  rectangle_t<tile_t> maze;
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int direction = 0;
  int x = 0, y = 0;
  int px = 0, py = 0;
  int64_t output;
  maze.set(x, y, {empty, 0});

  // Explore the maze for an arbitrary number of turns.
  // Keep the droid's right hand on a wall.
  for (int n = 0; n != 3000; ++n) {
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
      // We hit a wall. Update map, go back and turn left.
      maze.set(x, y, {wall, 0});
      x -= vectors[direction][0];
      y -= vectors[direction][1];
      direction = (direction + 1) & 3;
      continue;
    }

    // We went forward. Update map and turn right.
    // Update map.
    maze.set(x, y, {empty, 0});
    if (output == 2) {
      // We found the pump.
      px = x;
      py = y;
    }
    // Turn right.
    direction = (direction + 3) & 3;
  }

  // Fill with oxygen.
  maze.set(px, py, {oxygen, 0});
  std::set<std::array<int, 2>> oxygens = {{px, py}};
  int time = 0;
  bool finished = false;
  while (!finished) {
    ++time;
    finished = true;
    std::set<std::array<int, 2>> new_oxygens;
    for (auto tile : oxygens) {
      for (int direction = 0; direction != 4; ++direction) {
        int x = tile[0] + vectors[direction][0];
        int y = tile[1] + vectors[direction][1];
        if (auto [is_set, tile] = maze.get(x, y); tile.contents == empty) {
          maze.set(x, y, {oxygen, time});
          new_oxygens.insert({x, y});
          finished = false;
        }
      }
    }
    oxygens = new_oxygens;
  }

  std::cout << "Part one, distance " << maze.get(0, 0).second.time << "\n"
            << "Part two, time " << (time - 1) << std::endl;

  // Just for fun, mark the shortest path.
  x = 0;
  y = 0;
  time = maze.get(0, 0).second.time;
  maze.set(0, 0, {droid, -1});
  while (time) {
    // Find an adjacent square one step closer to the pump.
    for (int direction = 0; direction != 4; ++direction) {
      int nx = x + vectors[direction][0];
      int ny = y + vectors[direction][1];
      auto tile = maze.get(nx, ny).second;
      if (tile.contents == oxygen && tile.time == time - 1) {
        x = nx;
        y = ny;
        --time;
        maze.set(x, y, {path, -1});
        break;
      }
    }
  }

  // Dump map.
  maze.set(px, py, {pump, -1});
  maze.put(std::cout, [](bool set, const tile_t &tile) {
    switch (tile.contents) {
    case pump: return 'O';
    case droid: return 'D';
    case path: return '.';
    case oxygen: return ' ';
    default: return '#';
    }
  });
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    auto programs = read_programs("15.data");
    do_it(programs[0]);
    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
