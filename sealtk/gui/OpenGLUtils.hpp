/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_OpenGLUtils_hpp
#define sealtk_gui_OpenGLUtils_hpp

#include <sealtk/gui/Export.h>

#include <QOpenGLTexture>

#include <vital/types/image.h>

namespace sealtk
{

namespace gui
{

void SEALTK_GUI_EXPORT setTextureDataFromKwiver(
  QOpenGLTexture& texture, kwiver::vital::image const& image);

}

}

#endif
