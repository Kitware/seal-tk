/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/OpenGLUtils.hpp>

#include <QOpenGLContext>

namespace sealtk
{

namespace gui
{

void setTextureDataFromKwiver(QOpenGLTexture& texture,
                              kwiver::vital::image const& image)
{
  auto* context = QOpenGLContext::currentContext();
  if (!context)
  {
    qWarning("setTextureDataFromKwiver() requires a valid current context");
    return;
  }

  auto traits = image.pixel_traits();

  texture.setSize(image.width(), image.height());
  texture.setMipLevels(1);

  if (traits.type == kwiver::vital::image_pixel_traits::UNSIGNED)
  {
    if (traits.num_bytes == 1)
    {
      switch (image.depth())
      {
        case 3:
          texture.setFormat(QOpenGLTexture::RGB8_UNorm);
          texture.allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
          texture.setData(0, QOpenGLTexture::RGB, QOpenGLTexture::UInt8,
                          image.first_pixel());
          break;
        default:
          qWarning("Unsupported image format");
          return;
      }
    }
    else
    {
      qWarning("Unsupported image format");
      return;
    }
  }
  else
  {
    qWarning("Unsupported image format");
    return;
  }
}

}

}
