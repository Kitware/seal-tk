/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/CreateDetectionPlayerTool.hpp>

#include <sealtk/gui/DetectionRepresentation.hpp>
#include <sealtk/gui/Player.hpp>

#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLContext>

namespace sealtk
{

namespace gui
{

// ============================================================================
class CreateDetectionPlayerToolPrivate
{
public:
  QPointF startPos;
  QRectF detection;
  bool creating = false;
  bool dragging = false;

  static constexpr auto threshold = 8.0;

  DetectionRepresentation representation;
  QOpenGLBuffer vertexBuffer{QOpenGLBuffer::VertexBuffer};
  QVector<DetectionInfo> vertexIndices;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(CreateDetectionPlayerTool)

// ----------------------------------------------------------------------------
CreateDetectionPlayerTool::CreateDetectionPlayerTool(Player* parent)
  : PlayerTool{parent}, d_ptr{new CreateDetectionPlayerToolPrivate}
{
  QTE_D();

  d->representation.setColorFunction(
    [player = parent](qint64){ return player->pendingColor(); });
  d->vertexIndices.append({0, 0, 5});
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
  this->pushProperty("mouseTracking", true);
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::updateImage()
{
  QTE_D();

  if (d->creating)
  {
    d->creating = false;
    d->dragging = false;
  }
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mousePressEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() == Qt::LeftButton && this->player()->hasImage())
  {
    if (!d->creating)
    {
      d->creating = true;
      d->startPos = event->localPos();
      d->detection =
        QRectF{this->player()->viewToImage(event->localPos()), QSize{}};
      this->player()->update();
    }

    event->accept();
    return;
  }

  PlayerTool::mousePressEvent(event);
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mouseReleaseEvent(QMouseEvent* event)
{
  QTE_D();

  if (d->creating && event->button() == Qt::LeftButton)
  {
    if (d->dragging)
    {
      d->creating = false;
      d->dragging = false;
      this->player()->update();
      emit this->detectionCreated(d->detection);
    }
    else if (d->creating)
    {
      d->dragging = true;
    }
  }

  PlayerTool::mouseReleaseEvent(event);
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::mouseMoveEvent(QMouseEvent* event)
{
  QTE_D();

  if (d->creating)
  {
    if (!d->dragging)
    {
      auto const& delta = event->localPos() - d->startPos;
      if (delta.manhattanLength() > d->threshold)
      {
        d->dragging = true;
      }
    }

    d->detection.setBottomRight(
      this->player()->viewToImage(event->localPos()));
    this->player()->update();
  }

  PlayerTool::mouseMoveEvent(event);
}

// ----------------------------------------------------------------------------
void CreateDetectionPlayerTool::paintGL()
{
  QTE_D();

  if (d->creating)
  {
    if (!d->vertexBuffer.isCreated())
    {
      d->vertexBuffer.create();
    }

    QVector<float> vertexData;
    auto const minX = static_cast<float>(d->detection.left());
    auto const maxX = static_cast<float>(d->detection.right());
    auto const minY = static_cast<float>(d->detection.top());
    auto const maxY = static_cast<float>(d->detection.bottom());
    vertexData.append(minX); vertexData.append(minY);
    vertexData.append(maxX); vertexData.append(minY);
    vertexData.append(maxX); vertexData.append(maxY);
    vertexData.append(minX); vertexData.append(maxY);
    vertexData.append(minX); vertexData.append(minY);

    d->vertexBuffer.bind();
    d->vertexBuffer.allocate(
      vertexData.data(), vertexData.size() * static_cast<int>(sizeof(float)));
    d->vertexBuffer.release();

    d->representation.drawDetections(
      this->player()->context()->functions(),
      this->player()->viewHomography() * this->player()->homography(),
      d->vertexBuffer, d->vertexIndices);
  }
}

} // namespace gui

} // namespace sealtk
