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

static constexpr int limit = 44;
static constexpr int scale = 2;
static constexpr int gap = 2;
static constexpr float scalef = (float)scale;
static constexpr int row_size = 9;

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

const text_style text_styles[text_style::count] = {
  // For animated labels (semibold; no grid fit, no snap, vertical antialising).
  {L"Calibri", 24.0f, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
    DWRITE_GRID_FIT_MODE_DISABLED, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP},
  // For static labels (normal weight, grid fit, snap, no vertical antialising).
  {L"DSEG7 Classic Mini", 24.0f, DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL,
    DWRITE_RENDERING_MODE_NATURAL, DWRITE_GRID_FIT_MODE_ENABLED,
    D2D1_DRAW_TEXT_OPTIONS_NONE},
};

struct BeginDrawGuard {
  BeginDrawGuard(ID2D1RenderTarget *r) : r(r) { r->BeginDraw(); }
  ~BeginDrawGuard() noexcept(false) { CHECK_HRESULT(r->EndDraw()); }
  ID2D1RenderTarget *r;
};

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

void part_two(const data_t &data) {

  const auto &[field, portals, begin, end] = data;
  std::vector<field_t> levels = {field};
  levels[0][begin[0]][begin[1]] = {'@', 0};
  int distance = 1;
  int level = 0;

  d2d_t d2d;
  int h = (int)std::size(field);
  int w = (int)std::size(field[0]);
  float width = row_size * (w + gap) * scale;
  float height = (limit / row_size + 1) * (h + gap) * scale + 30.0f;
  d2d_thread_t d2d_thread(d2d, width, height);
  auto RenderTarget = d2d_thread.GetRenderTarget();
  ID2D1SolidColorBrushPtr Brushes[std::size(brush_colors)];

  for (std::size_t i = 0; i != std::size(brush_colors); ++i) {
    Brushes[i] = d2d_thread.CreateSolidBrush(brush_colors[i]);
  }

  int frame_index = 0;
  auto render_frame = [&](const std::basic_string<WCHAR> &caption) {
    int level_count = (int)std::size(levels);
    {
      BeginDrawGuard guard(RenderTarget);
      RenderTarget->Clear({0.0f, 0.0f, 0.0f, 1.0f});
      for (int k = 0; k != level_count; ++k) {
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
      std::basic_string<WCHAR> all_on(std::size(caption), L'8');
      d2d_thread.draw_text(all_on.c_str(), 1.0f, height - 29.0f, Brushes[8],
        text_style::segment, text_anchor::topleft);
      d2d_thread.draw_text(caption.c_str(), 1.0f, height - 29.0f, Brushes[1],
        text_style::segment, text_anchor::topleft);
    }
    std::basic_ostringstream<WCHAR> ostr;
    ostr << L".obj/frame-" << std::setfill(L'0') << std::setw(4)
         << frame_index++ << L".png";
    auto filename = ostr.str();
    d2d_thread.write_png(filename.c_str());
  };

  for (level_access_t access = {{0, begin}}; !std::empty(access); ++distance) {
    std::basic_ostringstream<WCHAR> ostr;
    ostr << distance;
    render_frame(ostr.str());

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
    render_frame(ostr.str());
    levels[level][pos[0]][pos[1]].first = '-';
    while (distance--) {
      std::cout << "Distance " << distance << ", level " << l << ", " << pos
                << std::endl;
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
          render_frame(ostr.str());
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
}

void do_it() try {
  for (auto filename : {"20.data"}) {
    std::cout << "Processing " << filename << std::endl;
    auto data = read_data(filename);
    part_two(data);
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
