// -*- C++ -*-
#ifndef rectangle_h
#define rectangle_h

// Interface.

template <typename T> struct rectangle_t {
  void set(int x, int y, T value);
  std::pair<bool, T> get(int x, int y) const;

  // Display bottom up, f(bool is_set, int64_t value) -> char;
  template <typename F> std::ostream &put(std::ostream &stream, F &&f) const;

  // Display top down, f(bool is_set, int64_t value) -> char;
  template <typename F> std::ostream &put1(std::ostream &stream, F &&f) const;

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
  for (int y = maxy; y != miny - 1; --y) {
    for (int x = minx; x != maxx + 1; ++x) {
      std::cout << std::apply(f, get(x, y));
    }
    stream << "\\\n";
  }
  return stream;
}

#endif
