#include "d2d.h"
#include "com.h"

d2d_t::d2d_t() {
  // Create WICImagingFactory.
  CHECK_HRESULT(WICImagingFactory.CreateInstance(
    CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER));

  // Create D2DFactory.
  D2D1_FACTORY_OPTIONS options = {D2D1_DEBUG_LEVEL_INFORMATION};
  CHECK_HRESULT(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                                  __uuidof(ID2D1Factory1), &options,
                                  (void **)&D2DFactory));

  IDWriteFactory1 *DWriteFactory1 = nullptr;
  IDWriteFactory2 *DWriteFactory2 = nullptr;
  // Create DWriteFactory2.
  if (SUCCEEDED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                    __uuidof(IDWriteFactory2),
                                    (IUnknown **)&DWriteFactory2))) {
    DWriteFactory = DWriteFactory1 = DWriteFactory2;
  }
  else {
    // Create DWriteFactory.
    CHECK_HRESULT(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                      __uuidof(IDWriteFactory1),
                                      (IUnknown **)&DWriteFactory1));
    DWriteFactory = DWriteFactory1;
  }
}

d2d_thread_t::d2d_thread_t(d2d_t &d2d, int width, int height) : d2d(d2d) {
  // Create WICBitmap.
  CHECK_HRESULT(d2d.GetWICImagingFactory()->CreateBitmap(
    width, height, GUID_WICPixelFormat32bppBGR, WICBitmapCacheOnDemand,
    &WICBitmap));
  // Create D2DRenderTarget.
  D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
    D2D1_RENDER_TARGET_TYPE_DEFAULT,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 0, 0,
    D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
  CHECK_HRESULT(d2d.GetD2DFactory()->CreateWicBitmapRenderTarget(
    WICBitmap, &props, &D2DRenderTarget));

  // Set TextAntiAliasMode.
  D2DRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void d2d_thread_t::write_png(const WCHAR *filename) {
  IWICStreamPtr WICStream;
  IWICBitmapEncoderPtr WICEncoder;
  IWICBitmapFrameEncodePtr WICBitmapFrameEncode;
  // Create and initialize WICStream.
  CHECK_HRESULT(d2d.GetWICImagingFactory()->CreateStream(&WICStream));
  CHECK_HRESULT(WICStream->InitializeFromFilename(filename, GENERIC_WRITE));
  // Create and initialize WICEncoder.
  CHECK_HRESULT(d2d.GetWICImagingFactory()->CreateEncoder(
    GUID_ContainerFormatPng, nullptr, &WICEncoder));
  CHECK_HRESULT(WICEncoder->Initialize(WICStream, WICBitmapEncoderNoCache));
  // Create and initialize WICBitmapFrameEncode.
  CHECK_HRESULT(WICEncoder->CreateNewFrame(&WICBitmapFrameEncode, nullptr));
  CHECK_HRESULT(WICBitmapFrameEncode->Initialize(nullptr));
  // Write frame to stream.
  CHECK_HRESULT(WICBitmapFrameEncode->WriteSource(WICBitmap, nullptr));
  CHECK_HRESULT(WICBitmapFrameEncode->Commit());
  CHECK_HRESULT(WICEncoder->Commit());
}

ID2D1SolidColorBrushPtr d2d_thread_t::CreateSolidBrush(D2D1::ColorF Color) {
  ID2D1SolidColorBrushPtr Brush;
  CHECK_HRESULT(D2DRenderTarget->CreateSolidColorBrush(Color, &Brush));
  return Brush;
}
