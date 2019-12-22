#include "precompiled.h"
#include "com.h"
#include "d2d.h"
#include "intcode.h"

constexpr int starti = 19;
constexpr int startj = 21;
constexpr int width = 41;
constexpr int height = 41;
constexpr int scale = 8;
constexpr int line_width = 3;
constexpr int explore_speed = 4;
constexpr int snake_length = 8;
constexpr int oxygen_speed = 3;
constexpr int oxygen_length = 5;
constexpr float scalef = (float)scale;

enum contents_t { unknown, wall, home, empty, oxygen, droid, pump, path };

struct tile_t {
  contents_t contents;
  int distance;
};

using line_t = std::array<tile_t, width>;
using field_t = std::array<line_t, height>;
using point_t = std::array<int, 2>;
using oxygens_t = std::vector<point_t>;

constexpr int64_t directions[] = {4, 1, 3, 2}; // E, N, W, S
constexpr point_t vectors[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

const D2D1::ColorF brush_colors[] = {
  {0.000f, 0.000f, 0.000f, 1.000f}, // 0: black
  {1.000f, 0.000f, 0.000f, 1.000f}, // 1: red
  {0.000f, 1.000f, 0.000f, 1.000f}, // 2: green
  {0.227f, 0.455f, 1.000f, 1.000f}, // 3: blue
  {0.000f, 1.000f, 1.000f, 1.000f}, // 4: cyan
  {1.000f, 0.000f, 1.000f, 1.000f}, // 5: magenta
  {1.000f, 1.000f, 0.000f, 1.000f}, // 6: yellow
  {1.000f, 1.000f, 1.000f, 1.000f}, // 7: white
  {0.200f, 0.000f, 0.000f, 1.000f}, // 8: dark red
  {0.600f, 0.420f, 0.220f, 1.000f}, // 9: light brown
  {0.380f, 0.275f, 0.122f, 1.000f}, // 10: dark brown
  {0.400f, 0.400f, 0.400f, 1.000f}, // 11: grey
};

namespace colors {
constexpr int unknown = 0;
constexpr int wall = 10;
constexpr int home = 2;
constexpr int oxygen = 3;
constexpr int new_oxygen = 4;
constexpr int droid = 6;
constexpr int pump = 4;
constexpr int path = 9;
constexpr int empty = 9;
constexpr int snake = 6;
} // namespace colors

std::ostream &operator<<(std::ostream &stream, const point_t &point) {
  auto [x, y] = point;
  return stream << "(" << x << "," << y << ")";
}

struct BeginDrawGuard {
  BeginDrawGuard(ID2D1RenderTarget *r) : r(r) { r->BeginDraw(); }
  ~BeginDrawGuard() noexcept(false) { CHECK_HRESULT(r->EndDraw()); }
  ID2D1RenderTarget *r;
};

void do_it(program_t program) {
  program.resize(65536);
  int64_t pc = 0, base = 0;
  std::vector<int64_t> inputs;
  int64_t output;

  field_t maze;
  int direction = 0;
  const point_t home_pos = {starti, startj};
  point_t pos = home_pos;
  point_t snake[snake_length];
  point_t pump_pos = {-1, -1};
  int time = 0;
  int distance = 0;
  int pump_distance = 0;

  for (auto &snake_pos : snake) {
    snake_pos = pos;
  }

  d2d_t d2d;
  d2d_thread_t d2d_thread(d2d, width * scale, height * scale + 30);
  auto RenderTarget = d2d_thread.GetRenderTarget();
  ID2D1SolidColorBrushPtr Brushes[std::size(brush_colors)];
  int frame_index = 0;
  for (std::size_t i = 0; i != std::size(brush_colors); ++i) {
    Brushes[i] = d2d_thread.CreateSolidBrush(brush_colors[i]);
  }

  auto paint_oxygens = [&](const oxygens_t &oxygens, int color_index) {
    for (const auto [i, j] : oxygens) {
      float x = j * scalef;
      float y = i * scalef;
      D2D1_RECT_F rect = {x, y, x + scalef, y + scalef};
      RenderTarget->FillRectangle(&rect, Brushes[color_index]);
    }
  };

  auto paint_point = [&](const point_t &point, int color_index) {
    auto [i, j] = point;
    float x = j * scalef;
    float y = i * scalef;
    D2D1_RECT_F rect = {x, y, x + scalef, y + scalef};
    RenderTarget->FillRectangle(&rect, Brushes[color_index]);
  };

  auto paint_wall = [&](const point_t &point, int direction, int color_index) {
    auto [i, j] = point;
    float x = j * scalef;
    float y = i * scalef;
    D2D1_RECT_F rect;
    switch (direction) {
    case 0:
      // right
      rect = {x + scale, y, x + scale + line_width, y + scale + line_width};
      break;
    case 1:
      // down
      rect = {x - line_width, y + scale, x + scale, y + scale + line_width};
      break;
    case 2:
      // left
      rect = {x - line_width, y - line_width, x, y + scale};
      break;
    case 3:
      // up
      rect = {x, y - line_width, x + scale + line_width, y};
      break;
    }
    RenderTarget->FillRectangle(&rect, Brushes[color_index]);
  };

  auto emit_frame = [&](int distance) {
    {
      BeginDrawGuard guard(RenderTarget);
      auto caption = std::to_wstring(distance);
      std::basic_string<WCHAR> all_on(std::size(caption), L'8');
      D2D1_RECT_F rect = {
        1.0f, height * scalef, 121.0f, height * scalef + 30.0f};
      RenderTarget->FillRectangle(&rect, Brushes[0]);

      d2d_thread.draw_text(all_on.c_str(), 1.0f, height * scalef + 1.0f,
        Brushes[8], text_style::segment, text_anchor::topleft);
      d2d_thread.draw_text(caption.c_str(), 1.0f, height * scalef + 1.0f,
        Brushes[1], text_style::segment, text_anchor::topleft);
    }
    std::basic_ostringstream<WCHAR> ostr;
    ostr << L".obj/frame-" << std::setfill(L'0') << std::setw(4)
         << frame_index++ << L".png";
    auto filename = ostr.str();
    d2d_thread.write_png(filename.c_str());
  };

  std::memset(&maze, '\0', sizeof maze);
  maze[pos[0]][pos[1]] = {empty, 0};

  {
    BeginDrawGuard guard(RenderTarget);
    D2D1_RECT_F rect = {0, 0, width * scale, height * scale};
    RenderTarget->Clear({0.0f, 0.0f, 0.0f, 1.0f});
    RenderTarget->FillRectangle(&rect, Brushes[colors::unknown]);
    paint_point(pos, colors::droid);
  }
  emit_frame(pump_distance ? pump_distance : distance);

  // Explore the maze for an arbitrary number of turns.
  // Keep the droid's right hand on a wall.

  while (true) {
    point_t newpos = pos;
    newpos[0] += vectors[direction][0];
    newpos[1] += vectors[direction][1];

    if (maze[newpos[0]][newpos[1]].contents == wall) {
      // There is a wall ahead. Turn right.
      {
        BeginDrawGuard guard(RenderTarget);
        paint_wall(pos, direction, colors::wall);
      }
      direction = (direction + 1) & 3;
      // Quit after we return to the origin.
      if (pos == home_pos && direction == 0) {
        break;
      }
      continue;
    }

    // Try to go forward.
    output = run_until_output(
      program, pc, base, [&]() { return directions[direction]; }, false);

    if (output == 0) {
      // We hit a wall. Update map and turn right.
      {
        BeginDrawGuard guard(RenderTarget);
        paint_wall(pos, direction, colors::wall);
      }
      maze[newpos[0]][newpos[1]].contents = wall;
      direction = (direction + 1) & 3;
      continue;
    }

    // We went forward.
    pos = newpos;

    // Emit a frame.
    {
      BeginDrawGuard guard(RenderTarget);
      paint_point(snake[0], colors::empty);
      for (int i = 1; i != snake_length; ++i) {
        snake[i - 1] = snake[i];
        paint_point(snake[i], colors::snake);
      }
      snake[snake_length - 1] = pos;
      paint_point(home_pos, colors::home);
      paint_point(pos, colors::droid);
    }
    if (time++ % explore_speed == 0) {
      emit_frame(pump_distance ? pump_distance : distance);
    }
    {
      BeginDrawGuard guard(RenderTarget);
      paint_point(pos, colors::empty);
      paint_point(home_pos, colors::home);
      paint_point(pump_pos, colors::pump);
    }

    // Update map.
    ++distance;
    if (tile_t tile = maze[pos[0]][pos[1]]; tile.contents == unknown) {
      if (output == 2) {
        // We found the pump.
        pump_distance = distance;
        pump_pos = pos;
        maze[pos[0]][pos[1]] = tile_t{pump, distance};
      } else {
        maze[pos[0]][pos[1]] = tile_t{empty, distance};
      }
    } else {
      distance = std::min(distance, tile.distance);
    }
    // Turn left.
    direction = (direction + 3) & 3;
  }

  {
    BeginDrawGuard guard(RenderTarget);
    for (int i = 0; i != snake_length; ++i) {
      paint_point(snake[i], colors::empty);
    }
    paint_point(home_pos, colors::home);
    paint_point(pos, colors::droid);
  }

  // Fill with oxygen.
  oxygens_t oxygens[oxygen_length + 1];
  auto & new_oxygens = oxygens[oxygen_length];
  new_oxygens.push_back(pump_pos);
  time = 0;
  bool finished = false;
  while (true) {
    maze[pump_pos[0]][pump_pos[1]] = {pump, 0};
    {
      BeginDrawGuard guard(RenderTarget);
      paint_oxygens(oxygens[0], colors::oxygen);
      paint_oxygens(new_oxygens, colors::new_oxygen);
      paint_point(home_pos, colors::home);
      paint_point(pump_pos, colors::pump);
    }
    if (time % oxygen_speed == 0) {
      emit_frame(time);
    }
    if (finished) {
      break;
    }
    std::rotate(&oxygens[0], &oxygens[1], &oxygens[oxygen_length + 1]);
    new_oxygens.resize(0);
    ++time;
    finished = true;
    for (auto point : oxygens[oxygen_length - 1]) {
      for (int direction = 0; direction != 4; ++direction) {
        int i = point[0] + vectors[direction][0];
        int j = point[1] + vectors[direction][1];
        if (auto tile = maze[i][j];
            tile.contents == empty || tile.contents == home) {
          maze[i][j] = {tile.contents == home ? home : oxygen, time};
          new_oxygens.push_back({i, j});
          finished = false;
        }
      }
    }
  }
  for (const auto & oxygen: oxygens) {
    BeginDrawGuard guard(RenderTarget);
    paint_oxygens(oxygen, colors::oxygen);
    paint_point(home_pos, colors::home);
    paint_point(pump_pos, colors::pump);
  }

  int oxygenation_time = time - 1;
  std::cout << "Part one, distance " << maze[starti][startj].distance << "\n"
            << "Part two, time " << oxygenation_time << std::endl;

  // Just for fun, mark the shortest path.
  pos = home_pos;
  direction = 0;
  distance = maze[starti][startj].distance;
  maze[starti][startj] = {droid, -1};
  while (distance) {
    // Find an adjacent square one step closer to the pump.
    point_t newpos = {
      pos[0] + vectors[direction][0], pos[1] + vectors[direction][1]};
    auto tile = maze[newpos[0]][newpos[1]];
    if ((tile.contents == oxygen || tile.contents == pump) &&
        tile.distance == distance - 1) {
      pos = newpos;
      --distance;
      maze[pos[0]][pos[1]] = {path, -1};
    } else {
      direction = (direction + 1) & 3;
      continue;
    }
    {
      BeginDrawGuard guard(RenderTarget);
      paint_point(snake[0], colors::empty);
      for (int i = 1; i != snake_length; ++i) {
        snake[i - 1] = snake[i];
        paint_point(snake[i], colors::snake);
      }
      snake[snake_length - 1] = pos;
      paint_point(pos, colors::droid);
      paint_point(home_pos, colors::home);
      paint_point(pump_pos, colors::pump);
    }
    if (time++ % explore_speed == 0) {
      emit_frame(oxygenation_time);
    }
    {
      BeginDrawGuard guard(RenderTarget);
      paint_point(pos, colors::path);
      paint_point(home_pos, colors::home);
      paint_point(pump_pos, colors::pump);
    }
  }
  {
    BeginDrawGuard guard(RenderTarget);
    for (const auto & point: snake) {
      paint_point(point, colors::empty);
    }
    paint_point(home_pos, colors::home);
    paint_point(pos, colors::droid);
  }
  emit_frame(oxygenation_time);
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
