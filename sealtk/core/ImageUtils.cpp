/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ImageUtils.hpp>

#include <arrows/qt/image_container.h>

#include <vital/range/iota.h>

#include <QImage>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>
#include <QtAlgorithms>

namespace sealtk
{

namespace core
{

namespace // anonymous
{

struct TextureFormat
{
  QOpenGLTexture::TextureFormat textureFormat;
  QOpenGLTexture::PixelFormat pixelFormat;
  QOpenGLTexture::PixelType pixelType;
};

// ----------------------------------------------------------------------------
bool isAligned(ptrdiff_t outer, ptrdiff_t inner, int alignment)
{
  auto const t = outer - (outer % inner);
  return (outer == ((t + alignment - 1) & ~(alignment - 1)));
}

// ----------------------------------------------------------------------------
bool checkStrides(ptrdiff_t xs, ptrdiff_t ys, ptrdiff_t cs, ptrdiff_t ss,
                  size_t ck, int alignment)
{
  // Check that X and Y strides are positive; we don't support backwards
  // packing
  if (xs < 0 || ys < 0)
    return false;

  // Check that pixels are packed in rows; we don't support column packing
  if (xs > ys)
    return false;

  // Check row stride; must be some number of (sub)pixel strides, rounded up to
  // the alignment
  if (!isAligned(ys, xs, alignment))
    return false;

  // Check for single-channel images; for single channel, no further checks are
  // needed
  if (ck == 1)
    return true;

  // Check if we are plane-packed or pixel-packed
  auto const acs = std::abs(cs);
  if (acs > xs) // Plane-packed
  {
    // Check plane stride; must be some number of row strides, rounded up to
    // the alignment
    if (!isAligned(acs, ys, alignment))
      return false;
  }
  else // Pixel-packed
  {
    // Check that there is no padding between sub-pixel values (shouldn't be!)
    // or between pixels (not supported)
    if (acs != ss || xs != (ss * static_cast<ptrdiff_t>(ck)))
      return false;
  }

  // Looks okay!
  return true;
}

// ----------------------------------------------------------------------------
TextureFormat getFormat(kwiver::vital::image_pixel_traits const& traits,
                        size_t channels /* 1 if plane packed */)
{
  using TF = QOpenGLTexture::TextureFormat;
  using PF = QOpenGLTexture::PixelFormat;
  using PT = QOpenGLTexture::PixelType;

  using PixelType = kwiver::vital::image_pixel_traits::pixel_type;

  struct
  {
    size_t const bytesPerChannel;
    size_t const channels;

    bool operator()(size_t targetBytesPerChannel, size_t targetChannels)
    {
      return (bytesPerChannel == targetBytesPerChannel) &&
             (channels == targetChannels);
    }
  } hasFormat{traits.num_bytes, channels};

  switch (traits.type)
  {
    case PixelType::SIGNED:
      if (hasFormat(1, 1)) return {TF::R8_SNorm,     PF::Red,  PT::Int8};
      if (hasFormat(1, 2)) return {TF::RG8_SNorm,    PF::RG,   PT::Int8};
      if (hasFormat(1, 3)) return {TF::RGB8_SNorm,   PF::RGB,  PT::Int8};
      if (hasFormat(1, 4)) return {TF::RGBA8_SNorm,  PF::RGBA, PT::Int8};
      if (hasFormat(2, 1)) return {TF::R16_SNorm,    PF::Red,  PT::Int16};
      if (hasFormat(2, 2)) return {TF::RG16_SNorm,   PF::RG,   PT::Int16};
      if (hasFormat(2, 3)) return {TF::RGB16_SNorm,  PF::RGB,  PT::Int16};
      if (hasFormat(2, 4)) return {TF::RGBA16_SNorm, PF::RGBA, PT::Int16};
      break;

    case PixelType::UNSIGNED:
      if (hasFormat(1, 1)) return {TF::R8_UNorm,     PF::Red,  PT::UInt8};
      if (hasFormat(1, 2)) return {TF::RG8_UNorm,    PF::RG,   PT::UInt8};
      if (hasFormat(1, 3)) return {TF::RGB8_UNorm,   PF::RGB,  PT::UInt8};
      if (hasFormat(1, 4)) return {TF::RGBA8_UNorm,  PF::RGBA, PT::UInt8};
      if (hasFormat(2, 1)) return {TF::R16_UNorm,    PF::Red,  PT::UInt16};
      if (hasFormat(2, 2)) return {TF::RG16_UNorm,   PF::RG,   PT::UInt16};
      if (hasFormat(2, 3)) return {TF::RGB16_UNorm,  PF::RGB,  PT::UInt16};
      if (hasFormat(2, 4)) return {TF::RGBA16_UNorm, PF::RGBA, PT::UInt16};
      break;

    case PixelType::FLOAT:
      if (hasFormat(4, 1)) return {TF::R32F,    PF::Red,  PT::Float32};
      if (hasFormat(4, 2)) return {TF::RG32F,   PF::RG,   PT::Float32};
      if (hasFormat(4, 3)) return {TF::RGB32F,  PF::RGB,  PT::Float32};
      if (hasFormat(4, 4)) return {TF::RGBA32F, PF::RGBA, PT::Float32};
      break;

    default:
      break;
  }

  return {TF::NoFormat, PF::NoSourceFormat, PT::NoPixelType};
}

// ----------------------------------------------------------------------------
void loadPlanePackedTexture(
  QOpenGLTexture& texture, char const* data, ptrdiff_t channelStride,
  size_t width, size_t height, size_t channels, TextureFormat const& format,
  QOpenGLPixelTransferOptions const* pixelTransferOptions)
{
  texture.setFormat(format.textureFormat);
  texture.setSize(static_cast<int>(width), static_cast<int>(height));
  texture.setLayers(static_cast<int>(channels));
  texture.setMipLevels(texture.maximumMipLevels());

  texture.allocateStorage(format.pixelFormat, format.pixelType);
  for (auto const c : kwiver::vital::range::iota(channels))
  {
    texture.setData(
      0, static_cast<int>(c), format.pixelFormat, format.pixelType,
      data, pixelTransferOptions);
    data += channelStride;
  }
}

// ----------------------------------------------------------------------------
void loadPixelPackedTexture(
  QOpenGLTexture& texture, char const* data,
  size_t width, size_t height, TextureFormat const& format,
  QOpenGLPixelTransferOptions const* pixelTransferOptions)
{
  texture.setFormat(format.textureFormat);
  texture.setSize(static_cast<int>(width), static_cast<int>(height));
  texture.setLayers(1);
  texture.setMipLevels(texture.maximumMipLevels());

  texture.allocateStorage(format.pixelFormat, format.pixelType);
  texture.setData(0, 0, format.pixelFormat, format.pixelType,
                  data, pixelTransferOptions);
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
QImage imageContainerToQImage(
  kwiver::vital::image_container_sptr const& image)
{
  auto qContainer =
    std::dynamic_pointer_cast<kwiver::arrows::qt::image_container>(image);
  if (qContainer)
  {
    return *qContainer;
  }

  return kwiver::arrows::qt::image_container::vital_to_qt(image->get_image());
}

// ----------------------------------------------------------------------------
void SEALTK_CORE_EXPORT imageToTexture(
  QOpenGLTexture& texture,
  kwiver::vital::image_container_sptr const& imageContainer)
{
  using PixelTraits = kwiver::vital::image_pixel_traits;

  if (!imageContainer)
    return;

  // Get image, pixel traits, and subpixel stride
  auto const& image = imageContainer->get_image();
  auto const& pt = image.pixel_traits();
  auto const ss = static_cast<ptrdiff_t>(pt.num_bytes);

  // Check for bitmap images; for now, these are not supported
  if (pt.type == PixelTraits::BOOL)
    return; // TODO

  // Get dimensions and strides
  auto const xk = image.width();
  auto const yk = image.height();
  auto const ck = image.depth();

  auto const xs = image.w_step();
  auto const ys = image.h_step();
  auto cs = image.d_step();

  auto* first = reinterpret_cast<char const*>(image.first_pixel());
  auto const isPlanePacked = (std::abs(cs) > std::abs(ys));

  auto const tf = getFormat(pt, isPlanePacked ? 1 : ck);
  if (tf.textureFormat == QOpenGLTexture::NoFormat)
    return; // Unsupported format

  // Get row alignment
  auto const alignment =
    std::min(1 << qCountTrailingZeroBits(static_cast<uintptr_t>(ys)), 8);

  // Check for images with a weird number of channels
  // (NOTE: Plane-packed could support N channels, but how would users render
  // that?)
  if (ck > 4)
    return;

  // Check for acceptable strides
  if (checkStrides(xs, ys, cs, ss, ck, alignment))
  {
    // Check for pixel-packed images with the channels backwards
    if (cs == -ss)
    {
      // Set swizzle to swap channels around...
      switch (ck)
      {
        case 2:
          texture.setSwizzleMask(QOpenGLTexture::GreenValue,
                                 QOpenGLTexture::RedValue,
                                 QOpenGLTexture::ZeroValue,
                                 QOpenGLTexture::OneValue);
          break;
        case 3:
          texture.setSwizzleMask(QOpenGLTexture::BlueValue,
                                 QOpenGLTexture::GreenValue,
                                 QOpenGLTexture::RedValue,
                                 QOpenGLTexture::OneValue);
          break;
        case 4:
          texture.setSwizzleMask(QOpenGLTexture::AlphaValue,
                                 QOpenGLTexture::BlueValue,
                                 QOpenGLTexture::GreenValue,
                                 QOpenGLTexture::RedValue);
          break;
        default:
          // Uh...
          break;
      }

      // ...and adjust the stride / data pointer to the real first pixel
      first += (static_cast<ptrdiff_t>(ck - 1) * cs);
      cs = -cs;
    }

    // Check for grayscale images; also, treat plane-packed as grayscale since
    // each plane (layer) has only one channel
    if (ck == 1 || isPlanePacked)
    {
      // Set swizzle to map single channel to RGB
      texture.setSwizzleMask(QOpenGLTexture::RedValue,
                             QOpenGLTexture::RedValue,
                             QOpenGLTexture::RedValue,
                             QOpenGLTexture::OneValue);
    }

    // Check that the plane stride is positive
    if (cs > 0 || isPlanePacked)
    {
      QOpenGLPixelTransferOptions pto;

      pto.setAlignment(alignment);
      pto.setRowLength(static_cast<int>(ys / xs));

      if (isPlanePacked)
      {
        loadPlanePackedTexture(texture, first, cs, xk, yk, ck, tf, &pto);
        return;
      }
      else
      {
        loadPixelPackedTexture(texture, first, xk, yk, tf, &pto);
        return;
      }
    }
  }

  // If we got here, we're going to have to repack the data
  // TODO
}

} // namespace core

} // namespace sealtk
