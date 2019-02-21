/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Player.hpp>

#include <sealtk/core/ImageUtils.hpp>

#include <QApplication>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QVector>
#include <QVector2D>
#include <QWheelEvent>

#include <QtGlobal>

#include <memory>

#include <cmath>

namespace sealtk
{

namespace gui
{

struct VertexData
{
  QVector2D vertexCoords;
  QVector2D textureCoords;
};

//=============================================================================
class PlayerPrivate
{
public:
  PlayerPrivate(Player* parent);

  void destroyResources();
  void createTexture();
  void calculateViewHomography();

  QTE_DECLARE_PUBLIC(Player)
  QTE_DECLARE_PUBLIC_PTR(Player)

  kwiver::vital::image_container_sptr image;
  QMatrix3x3 homography;
  QMatrix4x4 homographyGl;
  int homographyLocation;
  QMatrix4x4 viewHomography;
  int viewHomographyLocation;
  std::unique_ptr<QOpenGLTexture> imageTexture;
  std::unique_ptr<QOpenGLBuffer> imageVertexBuffer;
  std::unique_ptr<QOpenGLTexture> noImageTexture;
  std::unique_ptr<QOpenGLBuffer> noImageVertexBuffer;
  std::unique_ptr<QOpenGLShaderProgram> shaderProgram;

  QPointF center{0.0f, 0.0f};
  float zoom = 1.0f;

  QPoint dragStart;
  bool dragging = false;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

//-----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerPrivate{this}}
{
  QTE_D();

  connect(this, &Player::zoomSet,
          [this, d](float zoom)
  {
    d->calculateViewHomography();
    this->update();
  });
  connect(this, &Player::centerSet,
          [this, d](QPointF center)
  {
    d->calculateViewHomography();
    this->update();
  });
}

//-----------------------------------------------------------------------------
Player::~Player()
{
}

//-----------------------------------------------------------------------------
float Player::zoom() const
{
  QTE_D();
  return d->zoom;
}

//-----------------------------------------------------------------------------
QPointF Player::center() const
{
  QTE_D();
  return d->center;
}

//-----------------------------------------------------------------------------
void Player::setImage(kwiver::vital::image_container_sptr const& image)
{
  QTE_D();

  d->image = image;
  d->calculateViewHomography();
  this->makeCurrent();
  d->createTexture();
  this->doneCurrent();
  this->update();
}

//-----------------------------------------------------------------------------
void Player::setHomography(QMatrix3x3 const& homography)
{
  QTE_D();

  d->homography = homography;
  for (int row = 0; row < 3; row++)
  {
    for (int column = 0; column < 3; column++)
    {
      int glRow = row;
      if (glRow >= 2)
      {
        glRow++;
      }
      int glColumn = column;
      if (glColumn >= 2)
      {
        glColumn++;
      }
      d->homographyGl(glRow, glColumn) = d->homography(row, column);
    }
  }
  this->update();
}

//-----------------------------------------------------------------------------
void Player::setZoom(float zoom)
{
  QTE_D();
  if (!qFuzzyCompare(zoom, d->zoom))
  {
    d->zoom = zoom;
    emit this->zoomSet(zoom);
  }
}

//-----------------------------------------------------------------------------
void Player::setCenter(QPointF center)
{
  QTE_D();
  if (!(qFuzzyCompare(center.x(), d->center.x()) &&
        qFuzzyCompare(center.y(), d->center.y())))
  {
    d->center = center;
    emit this->centerSet(center);
  }
}

//-----------------------------------------------------------------------------
void Player::initializeGL()
{
  QTE_D();

  connect(this->context(), &QOpenGLContext::aboutToBeDestroyed,
          [d]() {d->destroyResources();});

  d->noImageTexture = std::make_unique<QOpenGLTexture>(QImage{
    ":/PlayerX.png"});

  float w = d->noImageTexture->width(), h = d->noImageTexture->height();
  QVector<VertexData> noImageVertexData{
    {{w, 0.0f}, {1.0f, 0.0f}},
    {{0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.0f, h}, {0.0f, 1.0f}},
    {{w, h}, {1.0f, 1.0f}},
  };

  d->createTexture();

  d->noImageVertexBuffer = std::make_unique<QOpenGLBuffer>(
    QOpenGLBuffer::VertexBuffer);
  d->noImageVertexBuffer->create();
  d->noImageVertexBuffer->bind();
  d->noImageVertexBuffer->allocate(
    noImageVertexData.data(), noImageVertexData.size() * sizeof(VertexData));

  d->shaderProgram = std::make_unique<QOpenGLShaderProgram>(this);
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/PlayerVertex.glsl");
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/PlayerFragment.glsl");
  d->shaderProgram->bindAttributeLocation("a_vertexCoords", 0);
  d->shaderProgram->bindAttributeLocation("a_textureCoords", 1);
  d->shaderProgram->link();

  d->homographyLocation = d->shaderProgram->uniformLocation("homography");
  d->viewHomographyLocation = d->shaderProgram->uniformLocation(
    "viewHomography");
}

//-----------------------------------------------------------------------------
void Player::paintGL()
{
  QTE_D();
  auto* functions = this->context()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  d->shaderProgram->bind();
  if (d->imageTexture)
  {
    d->imageTexture->bind();
    d->imageVertexBuffer->bind();
  }
  else
  {
    d->noImageTexture->bind();
    d->noImageVertexBuffer->bind();
  }

  d->shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2,
                                       sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(0);
  d->shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 2,
                                      sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(1);

  d->shaderProgram->setUniformValueArray(d->homographyLocation,
                                         &d->homographyGl, 1);
  d->shaderProgram->setUniformValueArray(d->viewHomographyLocation,
                                         &d->viewHomography, 1);

  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  if (d->imageTexture)
  {
    d->imageVertexBuffer->release();
    d->imageTexture->release();
  }
  else
  {
    d->noImageVertexBuffer->release();
    d->noImageTexture->release();
  }
  d->shaderProgram->release();
}

//-----------------------------------------------------------------------------
void Player::resizeGL(int w, int h)
{
  QTE_D();
  d->calculateViewHomography();
  this->update();
}

//-----------------------------------------------------------------------------
void Player::mousePressEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() == Qt::MiddleButton)
  {
    d->dragging = true;
    d->dragStart = event->pos();
  }
}

//-----------------------------------------------------------------------------
void Player::mouseMoveEvent(QMouseEvent* event)
{
  QTE_D();

  if (d->dragging && event->buttons() & Qt::MiddleButton)
  {
    this->setCenter(this->center() - (event->pos() - d->dragStart));
    d->dragStart = event->pos();
  }
  else
  {
    d->dragging = false;
  }
}

//-----------------------------------------------------------------------------
void Player::mouseReleaseEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() == Qt::MiddleButton)
  {
    d->dragging = false;
  }
}

//-----------------------------------------------------------------------------
void Player::wheelEvent(QWheelEvent* event)
{
  this->setZoom(this->zoom() * std::pow(1.001f, event->angleDelta().y()));
}

//-----------------------------------------------------------------------------
PlayerPrivate::PlayerPrivate(Player* parent)
  : q_ptr{parent}
{
  this->homography.setToIdentity();
  this->homographyGl.setToIdentity();
  this->viewHomography.setToIdentity();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::createTexture()
{
  if (this->image)
  {
    this->imageTexture = std::make_unique<QOpenGLTexture>(
      sealtk::core::imageContainerToQImage(this->image));

    float w = this->image->width(), h = this->image->height();
    QVector<VertexData> imageVertexData{
      {{w, 0.0f}, {1.0f, 0.0f}},
      {{0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.0f, h}, {0.0f, 1.0f}},
      {{w, h}, {1.0f, 1.0f}},
    };

    this->imageVertexBuffer = std::make_unique<QOpenGLBuffer>(
      QOpenGLBuffer::VertexBuffer);
    this->imageVertexBuffer->create();
    this->imageVertexBuffer->bind();
    this->imageVertexBuffer->allocate(
      imageVertexData.data(), imageVertexData.size() * sizeof(VertexData));
  }
  else
  {
    this->imageTexture = nullptr;
    this->imageVertexBuffer = nullptr;
  }
}

//-----------------------------------------------------------------------------
void PlayerPrivate::destroyResources()
{
  QTE_Q();

  q->makeCurrent();
  this->imageTexture = nullptr;
  this->imageVertexBuffer = nullptr;
  this->noImageTexture = nullptr;
  this->noImageVertexBuffer = nullptr;
  this->shaderProgram = nullptr;
  q->doneCurrent();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::calculateViewHomography()
{
  QTE_Q();

  float width, height;

  if (this->image)
  {
    width = this->image->width();
    height = this->image->height();
  }
  else if (this->noImageTexture)
  {
    width = this->noImageTexture->width();
    height = this->noImageTexture->height();
  }

  float left, right, top, bottom;
  float aspectRatio =
    static_cast<float>(q->width()) / static_cast<float>(q->height());
  left = this->center.x() + width / 2.0f - q->width() / 2.0f / this->zoom;
  right = this->center.x() + width / 2.0f + q->width() / 2.0f / this->zoom;
  top = this->center.y() + height / 2.0f - q->height() / 2.0f / this->zoom;
  bottom = this->center.y() + height / 2.0f + q->height() / 2.0f / this->zoom;

  float invX = 1.0f / (right - left);
  float invY = 1.0f / (top - bottom);

  this->viewHomography = QMatrix4x4{
    2.0f * invX,
          0.0f, 0.0f, -(right + left) * invX,
    0.0f, 2.0f * invY,
                0.0f, -(top + bottom) * invY,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
  };
}

}

}
