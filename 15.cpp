#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

int64_t directions[] = {4, 1, 3, 2}; // E, N, W, S
int vectors[][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

enum contents_t { unknown, wall, home, empty, oxygen, droid, pump, path };

struct tile_t {
  contents_t contents;
  int time;
};

void paint(d2d_stuff_t &d2d_stuff, rectangle_t<tile_t> &maze,
  bool unknown_is_wall, int time) {
  std::basic_ostringstream<WCHAR> ostr;
  ostr << time;
  maze.put2(
    d2d_stuff, ostr.str(), [unknown_is_wall](bool is_set, tile_t tile) -> int {
      switch (is_set ? tile.contents : unknown) {
      case unknown:
        [[fallthrough]];
      case wall:
        return 10;
      case home:
        return 2;
      case oxygen:
        return 3;
      case droid:
        return 6;
      case pump:
        return 4;
      case path:
        return 9;
      default:
        return 9;
      }
    });
}

void do_it(program_t program) {
  rectangle_t<tile_t> maze;
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int direction = 0;
  int x = 0, y = 0;
  int px = 0, py = 0;
  int time = 0;
  int pump_time = 0;
  int64_t output;
  d2d_stuff_t d2d_stuff;
  maze.set(x, y, {empty, 0});

  // Explore the maze for an arbitrary number of turns.
  // Keep the droid's right hand on a wall.

  while (true) {
    if (auto [is_set, tile] =
          maze.get(x + vectors[direction][0], y + vectors[direction][1]);
        tile.contents == wall) {
      // There is a wall ahead. Turn left.
      direction = (direction + 1) & 3;
      continue;
    }

    // Try to go forward.
    output = run_until_output(
      program, pc, base, [&]() { return directions[direction]; }, false);

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

    // We went forward. Emit a frame.
    tile_t tile = maze.get(x, y).second;
    maze.set(0, 0, {home, 0});
    maze.set(x, y, {droid, 0});
    paint(d2d_stuff, maze, false, pump_time ? pump_time : time);
    maze.set(x, y, tile);

    // Quit after we return to the origin.
    if (x == 0 && y == 0) {
      break;
    }

    // Update map.
    ++time;
    if (tile_t tile = maze.get(x, y).second; tile.contents == unknown) {
      if (output == 2) {
        // We found the pump.
        pump_time = time;
        px = x;
        py = y;
        maze.set(x, y, {pump, time});
      } else {
        maze.set(x, y, {empty, time});
      }
    } else {
      time = std::min(time, tile.time);
    }
    // Turn right.
    direction = (direction + 3) & 3;
  }

  // Fill with oxygen.
  maze.set(px, py, {oxygen, 0});
  std::set<std::array<int, 2>> oxygens = {{px, py}};
  time = 0;
  bool finished = false;
  while (!finished) {
    maze.set(px, py, {pump, 0});
    paint(d2d_stuff, maze, true, time);
    ++time;
    finished = true;
    std::set<std::array<int, 2>> new_oxygens;
    for (auto tile : oxygens) {
      for (int direction = 0; direction != 4; ++direction) {
        int x = tile[0] + vectors[direction][0];
        int y = tile[1] + vectors[direction][1];
        if (auto [is_set, tile] = maze.get(x, y);
            tile.contents == empty || tile.contents == home) {
          maze.set(x, y, {tile.contents == home ? home : oxygen, time});
          new_oxygens.insert({x, y});
          finished = false;
        }
      }
    }
    oxygens = new_oxygens;
  }
  int oxygenation_time = time - 1;
  std::cout << "Part one, distance " << maze.get(0, 0).second.time << "\n"
            << "Part two, time " << oxygenation_time << std::endl;

  // Just for fun, mark the shortest path.
  x = 0;
  y = 0;
  direction = 0;
  time = maze.get(0, 0).second.time;
  maze.set(0, 0, {droid, -1});
  while (time) {
    // Find an adjacent square one step closer to the pump.
    int nx = x + vectors[direction][0];
    int ny = y + vectors[direction][1];
    auto tile = maze.get(nx, ny).second;
    if ((tile.contents == oxygen || tile.contents == pump) &&
        tile.time == time - 1) {
      x = nx;
      y = ny;
      --time;
      maze.set(x, y, {path, -1});
    } else {
      direction = (direction + 1) & 3;
      continue;
    }
    maze.set(0, 0, {home, -1});
    maze.set(x, y, {droid, -1});
    maze.set(px, py, {pump, 0});
    paint(d2d_stuff, maze, true, oxygenation_time);
    maze.set(x, y, {path, -1});
    maze.set(px, py, {pump, 0});
  }

  d2d_stuff.render_frames(10, text_style::segment, 5);

  // Dump map.

  maze.put1(std::cout, [](bool set, const tile_t &tile) {
    switch (tile.contents) {
    case pump:
      return 'O';
    case droid:
      return 'D';
    case path:
      return '.';
    case oxygen:
      return ' ';
    default:
      return '#';
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
