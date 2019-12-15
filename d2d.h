// -*- C++ -*-
#ifndef d2d_h
#define d2d_h

#include "com.h"

#include <comdef.h>
#include <d2d1_1.h>
#include <dwrite_2.h>
#include <wincodec.h>

#define COM_SMARTPTR_TYPEDEF(a) _COM_SMARTPTR_TYPEDEF(a, __uuidof(a))

COM_SMARTPTR_TYPEDEF(IWICImagingFactory);
COM_SMARTPTR_TYPEDEF(IWICBitmap);
COM_SMARTPTR_TYPEDEF(IWICStream);
COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder);
COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode);
COM_SMARTPTR_TYPEDEF(ID2D1Brush);
COM_SMARTPTR_TYPEDEF(ID2D1RenderTarget);
COM_SMARTPTR_TYPEDEF(ID2D1Factory1);
COM_SMARTPTR_TYPEDEF(ID2D1SolidColorBrush);
COM_SMARTPTR_TYPEDEF(ID2D1PathGeometry);
COM_SMARTPTR_TYPEDEF(ID2D1GeometrySink);
COM_SMARTPTR_TYPEDEF(ID2D1Geometry);
COM_SMARTPTR_TYPEDEF(ID2D1GeometryGroup);
COM_SMARTPTR_TYPEDEF(ID2D1EllipseGeometry);
COM_SMARTPTR_TYPEDEF(ID2D1TransformedGeometry);
COM_SMARTPTR_TYPEDEF(IDWriteFactory);
COM_SMARTPTR_TYPEDEF(IDWriteFactory1);
COM_SMARTPTR_TYPEDEF(IDWriteFactory2);
COM_SMARTPTR_TYPEDEF(IDWriteTextLayout);
COM_SMARTPTR_TYPEDEF(IDWriteTextFormat);
COM_SMARTPTR_TYPEDEF(IDWriteRenderingParams);
COM_SMARTPTR_TYPEDEF(IDWriteRenderingParams1);
COM_SMARTPTR_TYPEDEF(IDWriteRenderingParams2);

enum class text_anchor { topleft, centre };

struct text_style {
  enum type { label, legend, count };
  const wchar_t *fontFamilyName;
  float fontSize;
  DWRITE_FONT_WEIGHT fontWeight;
  DWRITE_FONT_STYLE fontStyle;
  DWRITE_FONT_STRETCH fontStretch;
  DWRITE_RENDERING_MODE renderingMode;
  DWRITE_GRID_FIT_MODE gridFitMode;
  D2D1_DRAW_TEXT_OPTIONS drawTextOptions;
};

extern const text_style text_styles[text_style::count];

struct d2d_t {
  struct text_style_data {
    IDWriteRenderingParamsPtr RenderingParams;
    IDWriteTextFormatPtr TextFormat;
  };

  d2d_t();

  ID2D1Factory1 *GetD2DFactory() { return D2DFactory; }
  IWICImagingFactory *GetWICImagingFactory() { return WICImagingFactory; }
  IDWriteFactory *GetDWriteFactory() { return DWriteFactory; }
  decltype(auto) GetTextStyleData(text_style::type type) {
    return styles_data[type];
  }

private:
  com_t com;
  IWICImagingFactoryPtr WICImagingFactory;
  ID2D1Factory1Ptr D2DFactory;
  IDWriteFactoryPtr DWriteFactory;
  text_style_data styles_data[text_style::type::count];
};

struct d2d_thread_t {
  d2d_thread_t(d2d_t &d2d, int width, int height);
  void write_png(const WCHAR *filename);
  void draw_text(const WCHAR *text, float x, float y, ID2D1Brush *Brush,
                 text_style::type type, text_anchor anchor);
  ID2D1RenderTarget *GetRenderTarget() { return D2DRenderTarget; }
  ID2D1SolidColorBrushPtr CreateSolidBrush(D2D1::ColorF Color);

private:
  com_t com;
  d2d_t &d2d;
  IWICBitmapPtr WICBitmap;
  ID2D1RenderTargetPtr D2DRenderTarget;
};

#endif
