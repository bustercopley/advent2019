#include "precompiled.h"
#include "com.h"
#include "d2d.h"

#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

static constexpr int limit = 100;
static constexpr int scale = 5;
static constexpr int gap = 5;
static constexpr float scalef = (float)scale;
static constexpr int row_size = 5;

using tile_t = std::pair<char, int>;
using line_t = std::vector<tile_t>;
using field_t = std::vector<line_t>;
using point_t = std::array<int, 2>;
using portals_t = std::map<point_t, point_t>;
using data_t = std::tuple<field_t, portals_t, point_t, point_t>;
using access_t = std::set<point_t>;
bool isalpha(char c) { return 'A' <= c && c <= 'Z'; }

std::istream &read_field(std::istream &stream, field_t &field) {
  std::string s;
  std::size_t width = 0;
  while (std::getline(stream, s)) {
    if (!std::empty(s)) {
      width = std::max(width, std::size(s));
      line_t line(std::size(s));
      for (int j = 0; j != (int)std::size(s); ++j) {
        line[j] = tile_t{s[j], 0};
      }
      field.push_back(std::move(line));
    }
  }
  for (std::size_t i = 0; i != std::size(field); ++i) {
    field[i].resize(width);
  }
  return stream;
}

// Normal weight, grid fit, snap, no vertical antialising.
const text_style text_style = {L"DSEG7 Classic Mini", 24.0f,
  DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_ITALIC,
  DWRITE_FONT_STRETCH_NORMAL, DWRITE_RENDERING_MODE_NATURAL,
  DWRITE_GRID_FIT_MODE_ENABLED, D2D1_DRAW_TEXT_OPTIONS_NONE};

struct BeginDrawGuard {
  BeginDrawGuard(ID2D1RenderTarget *r) : r(r) { r->BeginDraw(); }
  ~BeginDrawGuard() noexcept(false) { CHECK_HRESULT(r->EndDraw()); }
  ID2D1RenderTarget *r;
};

struct d2d_stuff_t {
  void enqueue(const std::basic_string<WCHAR> &caption,
    const std::vector<field_t> &levels);
  void render_frames(std::size_t thread_count, int max_level,
    text_style::type caption_style, int caption_color_index);

private:
  std::vector<std::vector<field_t>> work;
  std::vector<std::basic_string<WCHAR>> captions;
  void thread_function(int max_level, std::size_t begin, std::size_t end,
    text_style::type caption_style, int caption_color_index);
  int max_width = 1, max_height = 1;
};

void d2d_stuff_t::enqueue(
  const std::basic_string<WCHAR> &caption, const std::vector<field_t> &levels) {
  captions.push_back(std::move(caption));
  work.push_back(levels);
}

void d2d_stuff_t::render_frames(std::size_t thread_count, int max_level,
  text_style::type caption_style, int caption_colour_index) {
  std::vector<std::thread> threads;
  for (std::size_t i = 0; i != thread_count; ++i) {
    std::size_t begin = std::size(work) * i / thread_count;
    std::size_t end = std::size(work) * (i + 1) / thread_count;
    threads.emplace_back(thread_function, this, max_level, begin, end,
      caption_style, caption_colour_index);
  }
  for (std::size_t i = 0; i != thread_count; ++i) {
    threads[i].join();
  }
}

const D2D1::ColorF brush_colors[] = {
  {0.0f, 0.0f, 0.0f, 1.0f},       // 0: black
  {1.0f, 0.0f, 0.0f, 1.0f},       // 1: red
  {0.0f, 1.0f, 0.0f, 1.0f},       // 2: green
  {0.198f, 0.263f, 1.000f, 1.0f}, // 3: blue
  {0.0f, 1.0f, 1.0f, 1.0f},       // 4: cyan
  {1.0f, 0.0f, 1.0f, 1.0f},       // 5: magenta
  {1.0f, 1.0f, 0.0f, 1.0f},       // 6: yellow
  {1.0f, 1.0f, 1.0f, 1.0f},       // 7: white
  {0.2f, 0.0f, 0.0f, 1.0f},       // 8: dark red
  {0.600f, 0.420f, 0.216f, 1.0f}, // 9: light brown
  {0.120f, 0.1f, 0.03f, 1.0f}, // 10: dark brown
};

int tile_contents_to_color_index(char c) {
  switch (c) {
  case '#':
    return 10;
  case '\0':
    return 10;
  case ' ':
    return 10;
  case '.':
    return 9;
  case '@':
    return 3;
  case '+':
    return 7;
  case '-':
    return 0;
  default:
    return 2;
  }
}

void d2d_stuff_t::thread_function(int max_level, std::size_t frames_begin,
  std::size_t frames_end, text_style::type caption_style,
  int caption_color_index) {
  d2d_t d2d;
  int h = (int)std::size(work[0][0]);
  int w = (int)std::size(work[0][0][0]);
  float width = row_size * (w + gap) * scale;
  float height = ((max_level - 1) / row_size + 1) * (h + gap) * scale + 30.0f;
  d2d_thread_t d2d_thread(d2d, width, height);
  auto RenderTarget = d2d_thread.GetRenderTarget();
  ID2D1SolidColorBrushPtr Brushes[std::size(brush_colors)];
  for (std::size_t i = 0; i != std::size(brush_colors); ++i) {
    Brushes[i] = d2d_thread.CreateSolidBrush(brush_colors[i]);
  }

  for (std::size_t index = frames_begin; index != frames_end; ++index) {
    const std::vector<field_t> &levels = work[index];
    int l = std::min((int)std::size(levels), max_level);
    {
      BeginDrawGuard guard(RenderTarget);
      RenderTarget->Clear({0.0f, 0.0f, 0.0f, 1.0f});
      for (int k = 0; k != l; ++k) {
        for (int i = 0; i != h; ++i) {
          for (int j = 0; j != w; ++j) {
            int color_index =
              tile_contents_to_color_index(levels[k][i][j].first);
            float x = ((k % row_size) * (w + gap) + j) * scalef;
            float y = ((k / row_size) * (h + gap) + i) * scalef;
            D2D1_RECT_F rect = {x, y, x + scalef, y + scalef};
            RenderTarget->FillRectangle(&rect, Brushes[color_index]);
          }
        }
      }
      std::basic_string<WCHAR> all_on(std::size(captions[index]), L'8');
      d2d_thread.draw_text(all_on.c_str(), 1.0f, height - 29.0f, Brushes[8],
        caption_style, text_anchor::topleft);
      d2d_thread.draw_text(captions[index].c_str(), 1.0f, height - 29.0f,
        Brushes[1], caption_style, text_anchor::topleft);
    }
    std::basic_ostringstream<WCHAR> ostr;
    ostr << L".obj/frame-" << std::setfill(L'0') << std::setw(4) << index
         << L".png";
    auto filename = ostr.str();
    d2d_thread.write_png(filename.c_str());
  }
}

data_t read_data(const char *filename) {
  data_t data;
  std::ifstream stream(filename);
  auto &[field, portals, begin, end] = data;
  read_field(stream, field);
  std::map<std::string, std::pair<point_t, point_t>> mouths;
  for (int i = 2; i != (int)std::size(field) - 2; ++i) {
    for (int j = 2; j != (int)std::size(field[i]) - 2; ++j) {
      if (field[i][j].first == '.') {
        std::map<std::string, std::pair<point_t, point_t>> new_mouths;
        if (isalpha(field[i - 1][j].first)) {
          std::string name{field[i - 2][j].first, field[i - 1][j].first};
          new_mouths[name] = {point_t{i - 1, j}, point_t{i, j}};
        }
        if (isalpha(field[i + 1][j].first)) {
          std::string name{field[i + 1][j].first, field[i + 2][j].first};
          new_mouths[name] = {point_t{i + 1, j}, point_t{i, j}};
        }
        if (isalpha(field[i][j - 1].first)) {
          std::string name{field[i][j - 2].first, field[i][j - 1].first};
          new_mouths[name] = {point_t{i, j - 1}, point_t{i, j}};
        }
        if (isalpha(field[i][j + 1].first)) {
          std::string name{field[i][j + 1].first, field[i][j + 2].first};
          new_mouths[name] = {point_t{i, j + 1}, point_t{i, j}};
        }
        for (const auto &mouth : new_mouths) {
          const auto &[name, points] = mouth;
          const auto &[in, out] = points;
          if (name == "AA") {
            begin = out;
          } else if (name == "ZZ") {
            end = out;
          } else if (auto iter = mouths.find(name); iter != mouths.end()) {
            portals[in] = iter->second.second;
            portals[iter->second.first] = out;
          } else {
            mouths.insert(mouth);
          }
        }
      }
    }
  }
  for (auto [name, points] : mouths) {
    std::cout << name << std::endl;
  }
  return data;
}

std::ostream &operator<<(std::ostream &stream, const point_t &point) {
  auto [x, y] = point;
  return stream << "(" << x << "," << y << ")";
}

using level_access_t = std::set<std::pair<int, point_t>>;

void part_two(const data_t &data, int limit) {
  const auto &[field, portals, begin, end] = data;
  std::vector<field_t> levels = {field};
  d2d_stuff_t d2d_stuff;
  levels[0][begin[0]][begin[1]] = {'@', 0};
  int distance = 1;
  int level = 0;
  int max_level = 0;
  for (level_access_t access = {{0, begin}}; !std::empty(access); ++distance) {
    std::basic_ostringstream<WCHAR> ostr;
    ostr << distance;
    d2d_stuff.enqueue(ostr.str(), levels);
    level_access_t new_access;
    for (const auto [l, p] : access) {
      point_t next[4] = {
        {p[0] - 1, p[1]},
        {p[0] + 1, p[1]},
        {p[0], p[1] - 1},
        {p[0], p[1] + 1},
      };
      for (auto p : next) {
        level = l;
        max_level = std::max(max_level, level);
        if (level == 0 && p == end) {
          levels[level][p[0]][p[1]].second = distance;
          goto done;
        }
        if (auto iter = portals.find(p); iter != portals.end()) {
          if (p[1] < 2 || p[1] >= (int)std::size(field[p[0]]) - 2 || p[0] < 2 ||
              p[0] >= (int)std::size(field) - 2) {
            if (level == 0) {
              continue;
            }
            --level;
          } else {
            if (level == limit) {
              continue;
            }
            ++level;
            if (level == (int)std::size(levels)) {
              levels.push_back(field);
            }
          }
          p = iter->second;
        }
        if (levels[level][p[0]][p[1]].first == '.') {
          levels[level][p[0]][p[1]] = {'@', distance};
          new_access.insert({level, p});
        }
      }
    }
    std::swap(access, new_access);
  }

  throw "UNREACHABLE";

done:

  // Just for fun, mark the shortest path.
  {
    std::cout << "Distance " << distance << std::endl;
    int l = 0;
    auto pos = end;
    if (levels[level][pos[0]][pos[1]].second != distance) {
      throw "BAD START";
    }
    levels[level][pos[0]][pos[1]].first = '+';
    std::basic_ostringstream<WCHAR> ostr;
    ostr << distance;
    d2d_stuff.enqueue(ostr.str(), levels);
    levels[level][pos[0]][pos[1]].first = '-';
    max_level = 0;
    while (distance--) {
      max_level = std::max(max_level, l);
      std::cout << "Distance " << distance << ", level " << l << ", " << pos << std::endl;
      // Find an adjacent square one step closer to the pump.
      point_t adjacents[4] = {
        {pos[0] - 1, pos[1]},
        {pos[0] + 1, pos[1]},
        {pos[0], pos[1] - 1},
        {pos[0], pos[1] + 1},
      };
      bool done_flag = false;
      for (auto p : adjacents) {
        level = l;
        if (auto iter = portals.find(p); iter != portals.end()) {
          std::cout << "Portal from level " << level << ", " << p;
          if (p[1] < 2 || p[1] >= (int)std::size(field[p[0]]) - 2 || p[0] < 2 ||
              p[0] >= (int)std::size(field) - 2) {
            if (level == 0) {
              continue;
            }
            --level;
          } else {
            if (level == limit) {
              continue;
            }
            ++level;
            if (level == (int)std::size(levels)) {
              throw "SMASH";
            }
          }
          p = iter->second;
          std::cout << " to level " << level << ", " << p << std::endl;
        }
        if (levels[level][p[0]][p[1]].second == distance) {
          levels[level][p[0]][p[1]].first = '+';
          std::basic_ostringstream<WCHAR> ostr;
          ostr << distance;
          d2d_stuff.enqueue(ostr.str(), levels);
          levels[level][p[0]][p[1]].first = '-';
          l = level;
          pos = p;
          done_flag = true;
          break;
        }
      }
      if (!done_flag) {
        throw "STUCK";
      }
    }
  }
  std::cout << "Levels " << max_level << ", ";
  std::cout << "Rendering" << std::endl;
  d2d_stuff.render_frames(max_level, 10, text_style::segment, 5);
}

void do_it() try {
  for (auto filename : {"20-test-2.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto data = read_data(filename);
    part_two(data, 100);
  }
} catch (const char *e) {
  std::cout << e << std::endl;
}

int main() {
  do_it();
  return 0;
}

int wmain() {
  do_it();
  return 0;
}
