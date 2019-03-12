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
  void updateDetectedObjectVertexBuffers();

  void drawImage(QOpenGLFunctions* functions);
  void drawDetections(QOpenGLFunctions* functions);

  QTE_DECLARE_PUBLIC(Player)
  QTE_DECLARE_PUBLIC_PTR(Player)

  kwiver::vital::image_container_sptr image;
  kwiver::vital::detected_object_set_sptr detectedObjectSet;
  std::vector<std::unique_ptr<QOpenGLBuffer>> detectedObjectVertexBuffers;
  QMatrix3x3 homography;
  QMatrix4x4 homographyGl;
  int imageHomographyLocation;
  QMatrix4x4 viewHomography;
  int imageViewHomographyLocation;
  std::unique_ptr<QOpenGLTexture> imageTexture;
  std::unique_ptr<QOpenGLBuffer> imageVertexBuffer;
  std::unique_ptr<QOpenGLTexture> noImageTexture;
  std::unique_ptr<QOpenGLBuffer> noImageVertexBuffer;
  std::unique_ptr<QOpenGLShaderProgram> imageShaderProgram;
  std::unique_ptr<QOpenGLShaderProgram> detectionShaderProgram;
  int detectionHomographyLocation;
  int detectionViewHomographyLocation;

  QPointF center{0.0f, 0.0f};
  float zoom = 1.0f;

  QPoint dragStart;
  bool dragging = false;

  core::VideoSource* videoSource = nullptr;

  QMetaObject::Connection kwiverImageDisplayedConnection;
  QMetaObject::Connection noImageDisplayedConnection;
  QMetaObject::Connection detectedObjectSetDisplayedConnection;
  QMetaObject::Connection noDetectedObjectSetDisplayedConnection;
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
core::VideoSource* Player::videoSource() const
{
  QTE_D();
  return d->videoSource;
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
void Player::setDetectedObjectSet(
  kwiver::vital::detected_object_set_sptr const& detectedObjectSet)
{
  QTE_D();

  d->detectedObjectSet = detectedObjectSet;
  this->makeCurrent();
  d->updateDetectedObjectVertexBuffers();
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
void Player::setVideoSource(core::VideoSource* videoSource)
{
  QTE_D();

  if (d->videoSource != videoSource)
  {
    if (d->videoSource)
    {
      QObject::disconnect(d->kwiverImageDisplayedConnection);
      QObject::disconnect(d->noImageDisplayedConnection);
      QObject::disconnect(d->detectedObjectSetDisplayedConnection);
      QObject::disconnect(d->noDetectedObjectSetDisplayedConnection);
    }

    d->videoSource = videoSource;

    if (d->videoSource)
    {
      d->kwiverImageDisplayedConnection = QObject::connect(
        videoSource, &core::VideoSource::kwiverImageDisplayed,
        this, &Player::setImage);
      d->noImageDisplayedConnection = QObject::connect(
        videoSource, &core::VideoSource::noImageDisplayed,
        [this]()
      {
        this->setImage(nullptr);
      });

      d->detectedObjectSetDisplayedConnection = QObject::connect(
        videoSource, &core::VideoSource::detectedObjectSetDisplayed,
        this, &Player::setDetectedObjectSet);
      d->noDetectedObjectSetDisplayedConnection = QObject::connect(
        videoSource, &core::VideoSource::noDetectedObjectSetDisplayed,
        [this]()
      {
        this->setDetectedObjectSet(nullptr);
      });
    }

    emit this->videoSourceSet(d->videoSource);
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
  d->updateDetectedObjectVertexBuffers();

  d->noImageVertexBuffer = std::make_unique<QOpenGLBuffer>(
    QOpenGLBuffer::VertexBuffer);
  d->noImageVertexBuffer->create();
  d->noImageVertexBuffer->bind();
  d->noImageVertexBuffer->allocate(
    noImageVertexData.data(), noImageVertexData.size() * sizeof(VertexData));

  d->imageShaderProgram = std::make_unique<QOpenGLShaderProgram>(this);
  d->imageShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                 ":/PlayerVertex.glsl");
  d->imageShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                 ":/PlayerFragment.glsl");
  d->imageShaderProgram->bindAttributeLocation("a_vertexCoords", 0);
  d->imageShaderProgram->bindAttributeLocation("a_textureCoords", 1);
  d->imageShaderProgram->link();

  d->imageHomographyLocation =
    d->imageShaderProgram->uniformLocation("homography");
  d->imageViewHomographyLocation = d->imageShaderProgram->uniformLocation(
    "viewHomography");

  d->detectionShaderProgram = std::make_unique<QOpenGLShaderProgram>(this);
  d->detectionShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                     ":/DetectionVertex.glsl");
  // TODO Get the geometry shader working
  /*d->detectionShaderProgram->addShaderFromSourceFile(
    QOpenGLShader::Geometry, ":/DetectionGeometry.glsl");*/
  d->detectionShaderProgram->addShaderFromSourceFile(
    QOpenGLShader::Fragment, ":/DetectionFragment.glsl");
  d->detectionShaderProgram->bindAttributeLocation("a_vertexCoords", 0);
  d->detectionShaderProgram->link();

  d->detectionHomographyLocation =
    d->detectionShaderProgram->uniformLocation("homography");
  d->detectionViewHomographyLocation =
    d->detectionShaderProgram->uniformLocation("viewHomography");
}

//-----------------------------------------------------------------------------
void Player::paintGL()
{
  QTE_D();
  auto* functions = this->context()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  d->drawImage(functions);
  d->drawDetections(functions);
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
    this->setCenter(this->center() - (event->pos() - d->dragStart) / d->zoom);
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
  this->imageShaderProgram = nullptr;
  q->doneCurrent();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::calculateViewHomography()
{
  QTE_Q();

  float left, right, top, bottom;
  if (this->image)
  {
    left = this->center.x() + this->image->width() / 2.0f
      - q->width() / 2.0f / this->zoom;
    right = this->center.x() + this->image->width() / 2.0f
      + q->width() / 2.0f / this->zoom;
    top = this->center.y() + this->image->height() / 2.0f
      - q->height() / 2.0f / this->zoom;
    bottom = this->center.y() + this->image->height() / 2.0f
      + q->height() / 2.0f / this->zoom;
  }
  else if (this->noImageTexture)
  {
    float quotient =
      (static_cast<float>(q->width()) / static_cast<float>(q->height()))
      / (static_cast<float>(this->noImageTexture->width())
        / static_cast<float>(this->noImageTexture->height()));
    if (quotient > 1.0f)
    {
      left = this->noImageTexture->width() / 2.0f
        - this->noImageTexture->width() * quotient / 2.0f;
      right = this->noImageTexture->width() / 2.0f
        + this->noImageTexture->width() * quotient / 2.0f;
      top = 0.0f;
      bottom = this->noImageTexture->height();
    }
    else
    {
      left = 0.0f;
      right = this->noImageTexture->width();
      top = this->noImageTexture->height() / 2.0f
        - this->noImageTexture->height() / quotient / 2.0f;
      bottom = this->noImageTexture->height() / 2.0f
        + this->noImageTexture->height() / quotient / 2.0f;
    }
  }

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

//-----------------------------------------------------------------------------
void PlayerPrivate::updateDetectedObjectVertexBuffers()
{
  this->detectedObjectVertexBuffers.clear();

  if (this->detectedObjectSet)
  {
    for (auto o : *this->detectedObjectSet)
    {
      auto bbox = o->bounding_box();
      float minX = static_cast<float>(bbox.min_x());
      float minY = static_cast<float>(bbox.min_y());
      float maxX = static_cast<float>(bbox.max_x());
      float maxY = static_cast<float>(bbox.max_y());
      QVector<QVector2D> detectedObjectVertexData{
        {maxX, maxY},
        {maxX, minY},
        {minX, minY},
        {minX, maxY},
        {maxX, maxY},
        {maxX, minY},
      };
      auto buf = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
      buf->create();
      buf->bind();
      buf->allocate(
        detectedObjectVertexData.data(), detectedObjectVertexData.size() *
          sizeof(VertexData));
      this->detectedObjectVertexBuffers.push_back(std::move(buf));
    }
  }
}

//-----------------------------------------------------------------------------
void PlayerPrivate::drawImage(QOpenGLFunctions* functions)
{
  this->imageShaderProgram->bind();
  if (this->imageTexture)
  {
    this->imageTexture->bind();
    this->imageVertexBuffer->bind();
  }
  else
  {
    this->noImageTexture->bind();
    this->noImageVertexBuffer->bind();
  }

  this->imageShaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2,
                                               sizeof(VertexData));
  this->imageShaderProgram->enableAttributeArray(0);
  this->imageShaderProgram->setAttributeBuffer(1, GL_FLOAT,
                                               2 * sizeof(GLfloat), 2,
                                               sizeof(VertexData));
  this->imageShaderProgram->enableAttributeArray(1);

  this->imageShaderProgram->setUniformValueArray(this->imageHomographyLocation,
                                                 &this->homographyGl, 1);
  this->imageShaderProgram->setUniformValueArray(
    this->imageViewHomographyLocation, &this->viewHomography, 1);

  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  if (this->imageTexture)
  {
    this->imageVertexBuffer->release();
    this->imageTexture->release();
  }
  else
  {
    this->noImageVertexBuffer->release();
    this->noImageTexture->release();
  }
  this->imageShaderProgram->release();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::drawDetections(QOpenGLFunctions* functions)
{
  this->detectionShaderProgram->bind();

  for (auto const& detectionBuffer : this->detectedObjectVertexBuffers)
  {
    detectionBuffer->bind();

    this->detectionShaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    this->detectionShaderProgram->enableAttributeArray(0);

    this->detectionShaderProgram->setUniformValueArray(
      this->detectionHomographyLocation, &this->homographyGl, 1);
    this->detectionShaderProgram->setUniformValueArray(
      this->detectionViewHomographyLocation, &this->viewHomography, 1);

    functions->glDrawArrays(GL_LINE_STRIP, 0, 5);

    detectionBuffer->release();
  }
  this->imageShaderProgram->release();
}

}

}
