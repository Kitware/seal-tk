/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_DetectionRepresentation_hpp
#define sealtk_gui_DetectionRepresentation_hpp

#include <sealtk/gui/Export.h>

#include <qtGlobal.h>

#include <QScopedPointer>

#include <functional>
#include <vector>

class QColor;
class QMatrix4x4;
class QOpenGLBuffer;
class QOpenGLFunctions;

namespace sealtk
{

namespace gui
{

class DetectionRepresentationPrivate;

// ============================================================================
struct DetectionInfo
{
  qint64 id;
  int first; // first index in vertex buffer of this detection
  int count; // number of indices used for this detection
};

// ============================================================================
class SEALTK_GUI_EXPORT DetectionRepresentation
{
public:
  DetectionRepresentation();
  ~DetectionRepresentation();

  void setColorFunction(std::function<QColor (qint64)> const& function);

  void drawDetections(
    QOpenGLFunctions* functions, QMatrix4x4 const& transform,
    QOpenGLBuffer& vertexBuffer, std::vector<DetectionInfo> const& indices);

protected:
  QTE_DECLARE_PRIVATE(DetectionRepresentation)

private:
  QTE_DECLARE_PRIVATE_RPTR(DetectionRepresentation)
};

} // namespace gui

} // namespace sealtk

#endif
