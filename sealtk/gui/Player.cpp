/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Player.hpp>

#include <sealtk/core/AutoLevelsTask.hpp>
#include <sealtk/core/ImageUtils.hpp>

#include <sealtk/util/unique.hpp>

#include <QApplication>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QPainter>
#include <QVector>
#include <QVector2D>
#include <QWheelEvent>

#include <QtConcurrentRun>

#include <cmath>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace gui
{

struct LevelsPair
{
  float low;
  float high;
};

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
  void updateViewHomography();
  void updateDetectedObjectVertexBuffers();

  void drawImage(float levelShift, float levelScale,
                 QOpenGLFunctions* functions);
  void drawDetections(QOpenGLFunctions* functions);

  LevelsPair levels();
  void computeLevels(LevelsPair const& temporaryLevels);

  QTE_DECLARE_PUBLIC(Player)
  QTE_DECLARE_PUBLIC_PTR(Player)

  QMetaObject::Connection destroyResourcesConnection;

  kv::timestamp timeStamp;
  kv::image_container_sptr image;
  kv::detected_object_set_sptr detectedObjectSet;
  std::vector<std::unique_ptr<QOpenGLBuffer>> detectedObjectVertexBuffers;
  QMatrix3x3 homography;
  QMatrix4x4 homographyGl;
  QMatrix4x4 viewHomography;

  QOpenGLTexture imageTexture{QOpenGLTexture::Target2DArray};
  QOpenGLBuffer imageVertexBuffer{QOpenGLBuffer::VertexBuffer};
  QOpenGLShaderProgram imageShaderProgram;
  QOpenGLShaderProgram detectionShaderProgram;

  int imageHomographyLocation;
  int imageViewHomographyLocation;
  int detectionHomographyLocation;
  int detectionViewHomographyLocation;
  int levelShiftLocation;
  int levelScaleLocation;

  bool initialized = false;

  ContrastMode contrastMode = ContrastMode::Manual;
  LevelsPair manualLevels{0.0f, 1.0f};
  double percentileDeviance = 0.0078125;
  double percentileTolerance = 0.5;
  core::TimeMap<LevelsPair> percentileLevels;
  quint64 percentileCookie = 0;

  QPointF center{0.0f, 0.0f};
  float zoom = 1.0f;

  QPoint dragStart;
  bool dragging = false;

  core::VideoSource* videoSource = nullptr;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

//-----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerPrivate{this}}
{
}

//-----------------------------------------------------------------------------
Player::~Player()
{
  QTE_D();

  // We need to clean up our textures while our context is current...
  d->destroyResources();

  // ...and then ensure cleanup isn't called after we're halfway destroyed
  disconnect(d->destroyResourcesConnection);
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
void Player::setImage(kv::image_container_sptr const& image,
                      core::VideoMetaData const& metaData)
{
  QTE_D();

  d->image = image;
  d->timeStamp = metaData.timeStamp();

  this->makeCurrent();
  d->createTexture();
  this->doneCurrent();

  d->updateViewHomography();
}

//-----------------------------------------------------------------------------
void Player::setDetectedObjectSet(
  kv::detected_object_set_sptr const& detectedObjectSet)
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
    d->updateViewHomography();

    emit this->zoomChanged(zoom);
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
    d->updateViewHomography();

    emit this->centerChanged(center);
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
      disconnect(d->videoSource, nullptr, this, nullptr);
    }

    d->videoSource = videoSource;

    if (d->videoSource)
    {
      connect(videoSource, &core::VideoSource::imageReady,
              this, &Player::setImage);
      connect(videoSource, &core::VideoSource::detectionsReady,
              this, &Player::setDetectedObjectSet);
    }

    emit this->videoSourceChanged(d->videoSource);
  }
}

//-----------------------------------------------------------------------------
ContrastMode Player::contrastMode() const
{
  QTE_D();
  return d->contrastMode;
}

//-----------------------------------------------------------------------------
void Player::setContrastMode(ContrastMode newMode)
{
  QTE_D();
  if (d->contrastMode != newMode)
  {
    d->contrastMode = newMode;
    this->update();
  }
}

//-----------------------------------------------------------------------------
void Player::setManualLevels(float low, float high)
{
  QTE_D();

  if (d->manualLevels.low != low || d->manualLevels.high != high)
  {
    d->manualLevels.low = low;
    d->manualLevels.high = high;

    if (d->contrastMode == ContrastMode::Manual)
    {
      this->update();
    }
  }
}

//-----------------------------------------------------------------------------
void Player::setPercentiles(double deviance, double tolerance)
{
  QTE_D();

  if (d->percentileDeviance != deviance ||
      d->percentileTolerance != tolerance)
  {
    d->percentileDeviance = deviance;
    d->percentileTolerance = tolerance;
    d->percentileLevels.clear();
    ++d->percentileCookie;

    if (d->contrastMode == ContrastMode::Percentile)
    {
      this->update();
    }
  }
}

//-----------------------------------------------------------------------------
void Player::initializeGL()
{
  QTE_D();

  d->destroyResourcesConnection = connect(
    this->context(), &QOpenGLContext::aboutToBeDestroyed,
    this, [d] { d->destroyResources(); });

  d->createTexture();
  d->updateDetectedObjectVertexBuffers();

  if (d->initialized)
    return;

  d->imageTexture.setWrapMode(QOpenGLTexture::ClampToEdge);

  d->imageShaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Vertex, ":/PlayerVertex.glsl");
  d->imageShaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Fragment, ":/PlayerFragment.glsl");
  d->imageShaderProgram.bindAttributeLocation("a_vertexCoords", 0);
  d->imageShaderProgram.bindAttributeLocation("a_textureCoords", 1);
  d->imageShaderProgram.link();

  d->imageHomographyLocation =
    d->imageShaderProgram.uniformLocation("homography");
  d->imageViewHomographyLocation =
    d->imageShaderProgram.uniformLocation("viewHomography");
  d->levelShiftLocation =
    d->imageShaderProgram.uniformLocation("levelShift");
  d->levelScaleLocation =
    d->imageShaderProgram.uniformLocation("levelScale");

  d->detectionShaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Vertex, ":/DetectionVertex.glsl");
  // TODO Get the geometry shader working
  /*d->detectionShaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Geometry, ":/DetectionGeometry.glsl");*/
  d->detectionShaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Fragment, ":/DetectionFragment.glsl");
  d->detectionShaderProgram.bindAttributeLocation("a_vertexCoords", 0);
  d->detectionShaderProgram.link();

  d->detectionHomographyLocation =
    d->detectionShaderProgram.uniformLocation("homography");
  d->detectionViewHomographyLocation =
    d->detectionShaderProgram.uniformLocation("viewHomography");

  d->initialized = true;
}

//-----------------------------------------------------------------------------
void Player::paintGL()
{
  QTE_D();
  auto* const functions = this->context()->functions();

  if (d->image)
  {
    functions->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto const levels = d->levels();
    auto const levelShift = levels.low;
    auto const levelScale = 1.0f / (levels.high - levels.low);

    d->drawImage(levelShift, levelScale, functions);
    d->drawDetections(functions);
  }
  else
  {
    auto const& bg = palette().color(QPalette::Background);
    auto const r = static_cast<float>(bg.redF());
    auto const g = static_cast<float>(bg.greenF());
    auto const b = static_cast<float>(bg.blueF());

    functions->glClearColor(0.5f * r, 0.5f * g, 0.5f * b, 0.0f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}

//-----------------------------------------------------------------------------
void Player::resizeGL(int w, int h)
{
  QTE_D();
  d->updateViewHomography();
}

//-----------------------------------------------------------------------------
void Player::paintEvent(QPaintEvent* event)
{
  QTE_D();

  // Handle usual GL painting
  QOpenGLWidget::paintEvent(event);

  // Paint text overlay, if needed
  if (!d->image)
  {
    static auto const noVideo = QStringLiteral("Right-click to load imagery");
    static auto const noFrame = QStringLiteral("(NO IMAGE)");

    auto const& text = (d->videoSource ? noFrame : noVideo);

    QPainter painter{this};
    painter.setPen(Qt::white);
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text);
    painter.end();
  }
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
    if (this->imageTexture.isCreated())
      this->imageTexture.destroy();

    sealtk::core::imageToTexture(this->imageTexture, this->image);

    float w = this->image->width(), h = this->image->height();
    QVector<VertexData> imageVertexData{
      {{w, 0.0f}, {1.0f, 0.0f}},
      {{0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.0f, h}, {0.0f, 1.0f}},
      {{w, h}, {1.0f, 1.0f}},
    };

    this->imageVertexBuffer.create();
    this->imageVertexBuffer.bind();
    this->imageVertexBuffer.allocate(
      imageVertexData.data(),
      imageVertexData.size() * static_cast<int>(sizeof(VertexData)));
  }
}

//-----------------------------------------------------------------------------
void PlayerPrivate::destroyResources()
{
  QTE_Q();

  q->makeCurrent();
  this->imageTexture.destroy();
  q->doneCurrent();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::updateViewHomography()
{
  if (!this->image)
  {
    return;
  }

  QTE_Q();

  // Get image and view sizes
  auto const iw = static_cast<float>(image->width());
  auto const ih = static_cast<float>(image->height());
  auto const vw = static_cast<float>(q->width());
  auto const vh = static_cast<float>(q->height());

  // Compute values for "fit" image
  auto const left =
    static_cast<float>(this->center.x() + (0.5 * (iw - (vw / zoom))));
  auto const right =
    static_cast<float>(this->center.x() + (0.5 * (iw + (vw / zoom))));
  auto const top =
    static_cast<float>(this->center.y() + (0.5 * (ih - (vh / zoom))));
  auto const bottom =
    static_cast<float>(this->center.y() + (0.5 * (ih + (vh / zoom))));

  // Compute matrix scale and translation coefficients for x and y
  auto const invX = 1.0f / (right - left);
  auto const invY = 1.0f / (top - bottom);

  auto const mxs = 2.0f * invX;
  auto const mys = 2.0f * invY;
  auto const mxt = -(right + left) * invX;
  auto const myt = -(top + bottom) * invY;

  this->viewHomography = QMatrix4x4{
    mxs,  0.0f, 0.0f, mxt,
    0.0f, mys,  0.0f, myt,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
  };

  q->update();
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
      auto buf = make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
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
void PlayerPrivate::computeLevels(LevelsPair const& temporaryLevels)
{
  QTE_Q();

  auto const t = this->timeStamp.get_time_usec();

  // Set up task to compute percentile levels
  auto const pd = this->percentileDeviance;
  auto const pt = this->percentileTolerance;
  auto* const task = new core::AutoLevelsTask{this->image, pd, pt};

  // Hook up receipt of results from task
  QObject::connect(
    task, &core::AutoLevelsTask::levelsUpdated,
    q, [t, cookie=this->percentileCookie, this](float low, float high){
      if (this->percentileCookie == cookie)
      {
        // Update the entry in the map
        this->percentileLevels.insert(t, {low, high});

        // If the image whose levels we are updating is the currently displayed
        // image, issue a repaint
        if (this->timeStamp.get_time_usec() == t)
        {
          QTE_Q();
          q->update();
        }
      }
    });

  // Run task
  QtConcurrent::run(
    [task]{
      task->execute();
      task->deleteLater();
    });

  // Insert a placeholder entry so we don't fire off the task twice
  this->percentileLevels.insert(t, temporaryLevels);
}

//-----------------------------------------------------------------------------
LevelsPair PlayerPrivate::levels()
{
  switch (this->contrastMode)
  {
    case ContrastMode::Percentile:
      if (this->percentileLevels.isEmpty())
      {
        // No entries yet; schedule new task to compute levels for this image
        this->computeLevels(this->manualLevels);
        return this->manualLevels;
      }
      else
      {
        // Look up entry in levels map
        auto const t = this->timeStamp.get_time_usec();
        auto const i = this->percentileLevels.find(t, core::SeekNearest);
        Q_ASSERT(i != this->percentileLevels.end());

        if (i.key() != t)
        {
          // Inexact match; schedule new task to compute levels for this image
          this->computeLevels(i.value());
        }

        return i.value();
      }
    default:
      return this->manualLevels;
  }
}

//-----------------------------------------------------------------------------
void PlayerPrivate::drawImage(float levelShift, float levelScale,
                              QOpenGLFunctions* functions)
{
  this->imageShaderProgram.bind();
  this->imageTexture.bind();
  this->imageVertexBuffer.bind();

  this->imageShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2,
                                              sizeof(VertexData));
  this->imageShaderProgram.enableAttributeArray(0);
  this->imageShaderProgram.setAttributeBuffer(1, GL_FLOAT,
                                              2 * sizeof(GLfloat), 2,
                                              sizeof(VertexData));
  this->imageShaderProgram.enableAttributeArray(1);

  this->imageShaderProgram.setUniformValueArray(this->imageHomographyLocation,
                                                &this->homographyGl, 1);
  this->imageShaderProgram.setUniformValueArray(
    this->imageViewHomographyLocation, &this->viewHomography, 1);

  this->imageShaderProgram.setUniformValue(
    this->levelShiftLocation, levelShift);
  this->imageShaderProgram.setUniformValue(
    this->levelScaleLocation, levelScale);

  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  this->imageVertexBuffer.release();
  this->imageTexture.release();
  this->imageShaderProgram.release();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::drawDetections(QOpenGLFunctions* functions)
{
  this->detectionShaderProgram.bind();

  for (auto const& detectionBuffer : this->detectedObjectVertexBuffers)
  {
    detectionBuffer->bind();

    this->detectionShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);
    this->detectionShaderProgram.enableAttributeArray(0);

    this->detectionShaderProgram.setUniformValueArray(
      this->detectionHomographyLocation, &this->homographyGl, 1);
    this->detectionShaderProgram.setUniformValueArray(
      this->detectionViewHomographyLocation, &this->viewHomography, 1);

    functions->glDrawArrays(GL_LINE_STRIP, 0, 5);

    detectionBuffer->release();
  }
  this->imageShaderProgram.release();
}

}

}
