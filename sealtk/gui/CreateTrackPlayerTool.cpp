/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/CreateTrackPlayerTool.hpp>
#include <sealtk/gui/Player.hpp>

#include <QMouseEvent>

#include <QDebug>

namespace sealtk
{

namespace gui
{

// ============================================================================
class CreateTrackPlayerToolPrivate
{
public:
  QRectF detection;
  bool dragging = false;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(CreateTrackPlayerTool)

// ----------------------------------------------------------------------------
CreateTrackPlayerTool::CreateTrackPlayerTool(Player* parent)
  : PlayerTool{parent},
    d_ptr{new CreateTrackPlayerToolPrivate}
{
}

// ----------------------------------------------------------------------------
CreateTrackPlayerTool::~CreateTrackPlayerTool()
{
}

// ----------------------------------------------------------------------------
void CreateTrackPlayerTool::activate()
{
  this->PlayerTool::activate();
  this->pushProperty("cursor", QCursor{Qt::CrossCursor});
}

// ----------------------------------------------------------------------------
void CreateTrackPlayerTool::mousePressEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() & Qt::LeftButton && this->player()->image())
  {
    d->dragging = true;
    d->detection.setTopLeft(this->player()->viewSpaceToWorldSpace(
      event->localPos()));
    d->detection.setSize(QSizeF{});
    this->player()->update();
  }
}

// ----------------------------------------------------------------------------
void CreateTrackPlayerTool::mouseReleaseEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() & Qt::LeftButton && this->player()->image())
  {
    d->dragging = false;
    this->player()->update();
    emit this->detectionCreated(d->detection);
  }
}

// ----------------------------------------------------------------------------
void CreateTrackPlayerTool::mouseMoveEvent(QMouseEvent* event)
{
  QTE_D();

  if (d->dragging && this->player()->image())
  {
    d->detection.setBottomRight(this->player()->viewSpaceToWorldSpace(
      event->localPos()));
    this->player()->update();
  }
}

// ----------------------------------------------------------------------------
void CreateTrackPlayerTool::paintGL()
{
  QTE_D();

  if (d->dragging && this->player()->image())
  {
    this->player()->drawPendingDetection(d->detection);
  }
}

}

}
