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
  QRectF detection;
  bool dragging = false;

  DetectionRepresentation representation;
  QOpenGLBuffer vertexBuffer{QOpenGLBuffer::VertexBuffer};
  std::vector<DetectionInfo> vertexIndices;
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
  d->vertexIndices.push_back({0, 0, 5});
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
