// -*- C++ -*-
#ifndef rectangle_h
#define rectangle_h

#include "d2d_stuff.h"

// Interface.

template <typename T> struct rectangle_t {
  void set(int x, int y, T value);
  std::pair<bool, T> get(int x, int y) const;
  std::array<int, 2> size() const;
  void d2d_init(std::size_t threads);
  void d2d_wait();

  // Display bottom up, f(bool is_set, T value) -> char;
  template <typename F> std::ostream &put(std::ostream &stream, F &&f) const;

  // Display top down, f(bool is_set, T value) -> char;
  template <typename F> std::ostream &put1(std::ostream &stream, F &&f) const;

  // Display top down, f(bool is_set, T value) -> D2D1::ColorF;
  template <typename F>
  void put2(d2d_stuff_t &d2d_stuff, std::basic_string<WCHAR> &&str, F &&f);

  template <typename F>
  void put0(F &&f) const;

 private:
  typedef std::map<int, T> row_t;
  std::map<int, row_t> matrix;
  int minx = 0, miny = 0, maxx = 0, maxy = 0;
};

// Implementation.

template <typename T> inline void rectangle_t<T>::set(int x, int y, T value) {
  matrix[y][x] = value;
  minx = std::min(x, minx);
  miny = std::min(y, miny);
  maxx = std::max(x, maxx);
  maxy = std::max(y, maxy);
}

template <typename T>
inline std::pair<bool, T> rectangle_t<T>::get(int x, int y) const {
  if (auto rowiter = matrix.find(y); rowiter != std::end(matrix)) {
    auto &row = rowiter->second;
    if (auto iter = row.find(x); iter != std::end(row)) {
      return {true, iter->second};
    }
  }
  return {false, T{}};
}

template <typename T> inline std::array<int, 2> rectangle_t<T>::size() const {
  return {maxx - minx + 1, maxy - miny + 1};
}

template <typename T>
template <typename F>
inline std::ostream &rectangle_t<T>::put(std::ostream &stream, F &&f) const {
  for (int y = maxy; y != miny - 1; --y) {
    for (int x = minx; x != maxx + 1; ++x) {
      std::cout << std::apply(f, get(x, y));
    }
    stream << "\\\n";
  }
  return stream;
}

template <typename T>
template <typename F>
inline std::ostream &rectangle_t<T>::put1(std::ostream &stream, F &&f) const {
  for (int y = minx; y != maxy + 1; ++y) {
    for (int x = minx; x != maxx + 1; ++x) {
      std::cout << std::apply(f, get(x, y));
    }
    stream << "\\\n";
  }
  return stream;
}

template <typename T>
template <typename F>
inline void rectangle_t<T>::put2(
  d2d_stuff_t &d2d_stuff, std::basic_string<WCHAR> &&str, F &&f) {
  auto [width, height] = size();
  std::vector<int> colors(width * height);
  for (int y = miny; y != maxy + 1; ++y) {
    std::size_t base = width * (y - miny);
    for (int x = minx; x != maxx + 1; ++x) {
      std::size_t index = base + (x - minx);
      colors[index] = std::apply(f, get(x, y));
    }
  }
  d2d_stuff.enqueue(std::move(str), std::move(colors), width, height);
}

template <typename T>
template <typename F>
inline void rectangle_t<T>::put0(F &&f) const {
  for (int y = maxy; y != miny - 1; --y) {
    for (int x = minx; x != maxx + 1; ++x) {
      std::apply(f, x, y, get(x, y).second);
    }
  }
}


#endif
