// -*- C++ -*-
#ifndef com_h
#define com_h

#include "comdef.h"

#define CHECK_HRESULT(CH_e)                                                    \
  do {                                                                         \
    HRESULT CH_hr = (CH_e);                                                    \
    if (!SUCCEEDED(CH_hr)) {                                                   \
      std::fprintf(stderr, "%s:%d: bad HRESULT\n", __FILE__, __LINE__);        \
      std::fflush(stderr);                                                     \
      throw _com_error(CH_hr);                                                 \
    }                                                                          \
  } while (false)

struct com_t {
  com_t() {
    CHECK_HRESULT(
      CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE));
  }
  ~com_t() { CoUninitialize(); }
  com_t(const com_t &com) = delete;
  com_t &operator=(const com_t &com) = delete;
};

#endif
