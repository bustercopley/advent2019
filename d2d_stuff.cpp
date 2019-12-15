#include "precompiled.h"
#include "d2d_stuff.h"

struct BeginDrawGuard {
  BeginDrawGuard(ID2D1RenderTarget *r) : r(r) { r->BeginDraw(); }
  ~BeginDrawGuard() noexcept(false) { CHECK_HRESULT(r->EndDraw()); }
  ID2D1RenderTarget *r;
};

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

void d2d_stuff_t::render_frames(std::size_t thread_count,
  text_style::type caption_style, int caption_colour_index) {
  std::vector<std::thread> threads;
  for (std::size_t i = 0; i != thread_count; ++i) {
    std::size_t begin = std::size(work) * i / thread_count;
    std::size_t end = std::size(work) * (i + 1) / thread_count;
    threads.emplace_back(thread_function, this, max_width, max_height, begin,
      end, caption_style, caption_colour_index);
  }
  for (std::size_t i = 0; i != thread_count; ++i) {
    threads[i].join();
  }
}

void d2d_stuff_t::thread_function(int width, int height,
  std::size_t frames_begin, std::size_t frames_end,
  text_style::type caption_style, int caption_color_index) {
  d2d_t d2d;
  d2d_thread_t d2d_thread(d2d, 10 * width, 10 * height + 30);
  auto RenderTarget = d2d_thread.GetRenderTarget();
  const D2D1::ColorF brush_colors[] = {
    {0.0f, 0.0f, 0.0f, 1.0f}, // 0: black
    {1.0f, 0.0f, 0.0f, 1.0f}, // 1: red
    {0.0f, 1.0f, 0.0f, 1.0f}, // 2: green
    {0.0f, 0.0f, 1.0f, 1.0f}, // 3: blue
    {0.0f, 1.0f, 1.0f, 1.0f}, // 4: cyan
    {1.0f, 0.0f, 1.0f, 1.0f}, // 5: magenta
    {1.0f, 1.0f, 0.0f, 1.0f}, // 6: yellow
    {1.0f, 1.0f, 1.0f, 1.0f}, // 7: white
    {0.2f, 0.0f, 0.0f, 1.0f}, // 8: dark red
  };
  ID2D1SolidColorBrushPtr Brushes[std::size(brush_colors)];
  for (std::size_t i = 0; i != std::size(brush_colors); ++i) {
    Brushes[i] = d2d_thread.CreateSolidBrush(brush_colors[i]);
  }

  for (std::size_t index = frames_begin; index != frames_end; ++index) {
    const std::vector<int> &pixels = work[index];
    int w = widths[index];
    int h = (int)(std::size(pixels) / (std::size_t)w);
    {
      BeginDrawGuard guard(RenderTarget);
      RenderTarget->Clear({0.0f, 0.0f, 0.0f, 1.0f});
      for (int y = 0; y != h; ++y) {
        std::size_t base = w * y;
        for (int x = 0; x != w; ++x) {
          std::size_t index = base + x;
          D2D1_RECT_F rect = {
            x * 10.0f, y * 10.0f, x * 10.0f + 10.0f, y * 10.0f + 10.0f};
          RenderTarget->FillRectangle(
            &rect, Brushes[pixels[index] % std::size(brush_colors)]);
        }
      }
      if (caption_style == text_style::segment) {
        std::basic_string<WCHAR> all_on(std::size(captions[index]), L'8');
        d2d_thread.draw_text(all_on.c_str(), 1.0f, height * 10.0f + 1.0f,
          Brushes[8], caption_style, text_anchor::topleft);
        d2d_thread.draw_text(captions[index].c_str(), 1.0f,
          height * 10.0f + 1.0f, Brushes[1], caption_style,
          text_anchor::topleft);
      } else {
        d2d_thread.draw_text(captions[index].c_str(), 1.0f,
          height * 10.0f + 1.0f, Brushes[caption_color_index], caption_style,
          text_anchor::topleft);
      }
    }

    std::basic_ostringstream<WCHAR> ostr;
    ostr << L".obj/frame-" << std::setfill(L'0') << std::setw(4) << index
         << L".png";
    auto filename = ostr.str();
    d2d_thread.write_png(filename.c_str());
  }
}

void d2d_stuff_t::enqueue(const std::basic_string<WCHAR> &caption,
  std::vector<int> &&pixels, int width, int height) {
  if (width > 0 && height > 0) {
    captions.push_back(std::move(caption));
    widths.push_back(width);
    work.push_back(std::move(pixels));
    max_width = std::max(max_width, width);
    max_height = std::max(max_height, height);
  }
}
