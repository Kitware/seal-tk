/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/CreateDetectionPlayerTool.hpp>

#include <sealtk/gui/Player.hpp>

#include <QMouseEvent>

namespace sealtk
{

namespace gui
{

// ============================================================================
class CreateDetectionPlayerToolPrivate
{
public:
  QRectF detection;
  bool dragging = false;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(CreateDetectionPlayerTool)

// ----------------------------------------------------------------------------
CreateDetectionPlayerTool::CreateDetectionPlayerTool(Player* parent)
  : PlayerTool{parent}, d_ptr{new CreateDetectionPlayerToolPrivate}
{
}

// ----------------------------------------------------------------------------
CreateDetectionPlayerTool::~CreateDetectionPlayerTool()
{
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::activate()
{
  this->PlayerTool::activate();
  this->pushProperty("cursor", QCursor{Qt::CrossCursor});
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mousePressEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() & Qt::LeftButton && this->player()->hasImage())
  {
    d->dragging = true;
    d->detection =
      QRectF{this->player()->viewToImage(event->localPos()), QSize{}};
    this->player()->update();
  }
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mouseReleaseEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() & Qt::LeftButton && this->player()->hasImage())
  {
    d->dragging = false;
    this->player()->update();
    emit this->detectionCreated(d->detection);
  }
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mouseMoveEvent(QMouseEvent* event)
{
  QTE_D();

  if (d->dragging && this->player()->hasImage())
  {
    d->detection.setBottomRight(
      this->player()->viewToImage(event->localPos()));
    this->player()->update();
  }
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::paintGL()
{
  QTE_D();

  if (d->dragging && this->player()->hasImage())
  {
    this->player()->drawPendingDetection(d->detection);
  }
}

} // namespace gui

} // namespace sealtk
