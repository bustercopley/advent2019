#include "precompiled.h"
#include "d2d_stuff.h"

struct BeginDrawGuard {
  BeginDrawGuard(ID2D1RenderTarget *r) : r(r) { r->BeginDraw(); }
  ~BeginDrawGuard() noexcept(false) { CHECK_HRESULT(r->EndDraw()); }
  ID2D1RenderTarget *r;
};

void d2d_stuff_t::render_frames(
  std::size_t thread_count, int width, int height) {
  std::vector<std::thread> threads;
  for (std::size_t i = 0; i != thread_count; ++i) {
    std::size_t begin = std::size(work) * i / thread_count;
    std::size_t end = std::size(work) * (i + 1) / thread_count;
    threads.emplace_back(thread_function, this, width, height, begin, end);
  }
  for (std::size_t i = 0; i != thread_count; ++i) {
    threads[i].join();
  }
}

void d2d_stuff_t::thread_function(
  int width, int height, std::size_t frames_begin, std::size_t frames_end) {
  d2d_t d2d;
  d2d_thread_t d2d_thread(d2d, 10 * width, 10 * height);
  auto RenderTarget = d2d_thread.GetRenderTarget();
  const D2D1::ColorF brush_colors[] = {
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
  };
  ID2D1SolidColorBrushPtr Brushes[std::size(brush_colors)];
  for (std::size_t i = 0; i != std::size(brush_colors); ++i) {
    Brushes[i] = d2d_thread.CreateSolidBrush(brush_colors[i]);
  }

  for (std::size_t index = frames_begin; index != frames_end; ++index) {
    const std::vector<int> &pixels = work[index];
    {
      BeginDrawGuard guard(RenderTarget);
      RenderTarget->Clear({1.0f, 1.0f, 1.0f, 1.0f});
      for (int y = 0; y != height; ++y) {
        std::size_t base = width * y;
        for (int x = 0; x != width; ++x) {
          std::size_t index = base + x;
          D2D1_RECT_F rect = {
            x * 10.0f, y * 10.0f, x * 10.0f + 10.0f, y * 10.0f + 10.0f};
          RenderTarget->FillRectangle(
            &rect, Brushes[pixels[index] % std::size(brush_colors)]);
        }
      }
    }
    std::basic_ostringstream<WCHAR> ostr;
    ostr << L".obj/frame-" << std::setfill(L'0') << std::setw(4) << index
         << L".png";
    auto filename = ostr.str();
    d2d_thread.write_png(filename.c_str());
  }
}

void d2d_stuff_t::enqueue(std::vector<int> &&pixels) {
  work.push_back(std::move(pixels));
}
