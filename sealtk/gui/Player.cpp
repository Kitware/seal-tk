/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Player.hpp>

#include <sealtk/core/AutoLevelsTask.hpp>
#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/ImageUtils.hpp>
#include <sealtk/core/ScalarFilterModel.hpp>

#include <sealtk/util/unique.hpp>

#include <vital/range/iota.h>

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
namespace kvr = kwiver::vital::range;

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

// ============================================================================
class PlayerPrivate
{
public:
  PlayerPrivate(Player* parent);

  void destroyResources();
  void createTexture();
  void updateViewHomography();
  void updateDetectedObjectVertexBuffers();
  void updateDetections();

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
  QMatrix4x4 viewHomography;
  QMatrix4x4 homography;
  QSize homographyImageSize;

  QVector<QPair<int, int>> detectedObjectVertexIndices;

  QOpenGLTexture imageTexture{QOpenGLTexture::Target2DArray};
  QOpenGLBuffer imageVertexBuffer{QOpenGLBuffer::VertexBuffer};
  QOpenGLBuffer detectedObjectVertexBuffer{QOpenGLBuffer::VertexBuffer};
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

  core::VideoDistributor* videoSource = nullptr;
  core::ScalarFilterModel trackModelFilter;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

// ----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerPrivate{this}}
{
  QTE_D();

  connect(&d->trackModelFilter, &QAbstractItemModel::rowsInserted,
          this, [d]{ d->updateDetections(); });
  connect(&d->trackModelFilter, &QAbstractItemModel::rowsRemoved,
          this, [d]{ d->updateDetections(); });
  connect(&d->trackModelFilter, &QAbstractItemModel::rowsMoved,
          this, [d]{ d->updateDetections(); });
  connect(&d->trackModelFilter, &QAbstractItemModel::dataChanged,
          this, [d]{ d->updateDetections(); });
  connect(&d->trackModelFilter, &QAbstractItemModel::modelReset,
          this, [d]{ d->updateDetections(); });
}

// ----------------------------------------------------------------------------
Player::~Player()
{
  QTE_D();

  // We need to clean up our textures while our context is current...
  d->destroyResources();

  // ...and then ensure cleanup isn't called after we're halfway destroyed
  disconnect(d->destroyResourcesConnection);
}

// ----------------------------------------------------------------------------
float Player::zoom() const
{
  QTE_D();
  return d->zoom;
}

// ----------------------------------------------------------------------------
QPointF Player::center() const
{
  QTE_D();
  return d->center;
}

// ----------------------------------------------------------------------------
core::VideoDistributor* Player::videoSource() const
{
  QTE_D();
  return d->videoSource;
}

// ----------------------------------------------------------------------------
void Player::setImage(kv::image_container_sptr const& image,
                      core::VideoMetaData const& metaData)
{
  QTE_D();

  d->image = image;
  d->timeStamp = metaData.timeStamp();

  auto const t = QVariant::fromValue(d->timeStamp.get_time_usec());
  d->trackModelFilter.setUpperBound(core::StartTimeRole, t);
  d->trackModelFilter.setLowerBound(core::EndTimeRole, t);

  this->makeCurrent();
  d->createTexture();
  d->updateDetectedObjectVertexBuffers();
  this->doneCurrent();

  d->updateViewHomography();
  this->update();

  if (d->image)
  {
    emit this->imageSizeChanged({static_cast<int>(d->image->width()),
                                 static_cast<int>(d->image->height())});
  }
}

// ----------------------------------------------------------------------------
void Player::setTrackModel(QAbstractItemModel* model)
{
  QTE_D();

  connect(model, &QObject::destroyed, this,
          [d]{ d->trackModelFilter.setSourceModel(nullptr); });
  d->trackModelFilter.setSourceModel(model);

  d->updateDetections();
}

// ----------------------------------------------------------------------------
void Player::setHomography(QMatrix4x4 const& homography)
{
  QTE_D();

  d->homography = homography;
  d->updateViewHomography();
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
void Player::setVideoSource(core::VideoDistributor* videoSource)
{
  QTE_D();

  if (d->videoSource != videoSource)
  {
    if (d->videoSource)
    {
      disconnect(d->videoSource, nullptr, this, nullptr);
    }

    d->videoSource = videoSource;
    d->percentileLevels.clear();
    ++d->percentileCookie;

    if (d->videoSource)
    {
      connect(videoSource, &QObject::destroyed, this,
              [this]{ this->setVideoSource(nullptr); });

      connect(videoSource, &core::VideoDistributor::frameReady, this,
              [this](core::VideoFrame const& frame){
                this->setImage(frame.image, frame.metaData);
              });
      connect(videoSource, &core::VideoDistributor::requestDeclined, this,
              [this](){
                this->setImage(nullptr, core::VideoMetaData{});
              });
    }
  }
}

// ----------------------------------------------------------------------------
ContrastMode Player::contrastMode() const
{
  QTE_D();
  return d->contrastMode;
}

// ----------------------------------------------------------------------------
void Player::setContrastMode(ContrastMode newMode)
{
  QTE_D();
  if (d->contrastMode != newMode)
  {
    d->contrastMode = newMode;
    this->update();
  }
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
QSize Player::homographyImageSize() const
{
  QTE_D();

  return d->homographyImageSize;
}

// ----------------------------------------------------------------------------
void Player::setHomographyImageSize(QSize size)
{
  QTE_D();

  if (d->homographyImageSize != size)
  {
    d->homographyImageSize = size;
    d->updateViewHomography();
  }
}

// ----------------------------------------------------------------------------
void Player::initializeGL()
{
  QTE_D();

  d->destroyResourcesConnection = connect(
    this->context(), &QOpenGLContext::aboutToBeDestroyed,
    this, [d]{ d->destroyResources(); });

  d->createTexture();
  d->updateDetectedObjectVertexBuffers();

  if (d->initialized)
  {
    return;
  }

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
  // d->detectionShaderProgram.addShaderFromSourceFile(
  //   QOpenGLShader::Geometry, ":/DetectionGeometry.glsl");
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

// ----------------------------------------------------------------------------
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

    if (!d->detectedObjectVertexIndices.isEmpty())
    {
      d->drawDetections(functions);
    }
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

// ----------------------------------------------------------------------------
void Player::resizeGL(int w, int h)
{
  QTE_D();
  d->updateViewHomography();
}

// ----------------------------------------------------------------------------
void Player::paintEvent(QPaintEvent* event)
{
  QTE_D();

  // Handle usual GL painting
  this->QOpenGLWidget::paintEvent(event);

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

// ----------------------------------------------------------------------------
void Player::mousePressEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() == Qt::MiddleButton)
  {
    d->dragging = true;
    d->dragStart = event->pos();
  }
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
void Player::mouseReleaseEvent(QMouseEvent* event)
{
  QTE_D();

  if (event->button() == Qt::MiddleButton)
  {
    d->dragging = false;
  }
}

// ----------------------------------------------------------------------------
void Player::wheelEvent(QWheelEvent* event)
{
  auto const delta = std::pow(1.001, event->angleDelta().y());
  this->setZoom(this->zoom() * static_cast<float>(delta));
}

// ----------------------------------------------------------------------------
PlayerPrivate::PlayerPrivate(Player* parent)
  : q_ptr{parent}
{
  this->homography.setToIdentity();
  this->viewHomography.setToIdentity();
}

// ----------------------------------------------------------------------------
void PlayerPrivate::createTexture()
{
  if (this->image)
  {
    if (this->imageTexture.isCreated())
    {
      this->imageTexture.destroy();
    }

    sealtk::core::imageToTexture(this->imageTexture, this->image);

    auto const w = static_cast<float>(this->image->width());
    auto const h = static_cast<float>(this->image->height());
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

// ----------------------------------------------------------------------------
void PlayerPrivate::destroyResources()
{
  QTE_Q();

  q->makeCurrent();
  this->imageTexture.destroy();
  q->doneCurrent();
}

// ----------------------------------------------------------------------------
void PlayerPrivate::updateViewHomography()
{
  if (!this->image)
  {
    return;
  }

  QTE_Q();

  // Get image and view sizes
  auto const useHomography = !this->homography.isIdentity();
  auto const iw = (useHomography
                   ? static_cast<float>(this->homographyImageSize.width())
                   : static_cast<float>(image->width()));
  auto const ih = (useHomography
                   ? static_cast<float>(this->homographyImageSize.height())
                   : static_cast<float>(image->height()));
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

  // Compute transform
  this->viewHomography.setToIdentity();
  this->viewHomography.ortho(left, right, bottom, top, 1.0, -1.0);

  q->update();
}

// ----------------------------------------------------------------------------
void PlayerPrivate::updateDetectedObjectVertexBuffers()
{
  QVector<float> vertexData;
  static constexpr decltype(vertexData.count()) tuple_size = 2;

  // Get bounding boxes of all "active" detected objects
  this->detectedObjectVertexIndices.clear();
  for (auto const pr : kvr::iota(this->trackModelFilter.rowCount()))
  {
    auto const first = vertexData.count() / tuple_size;

    auto const& pi = this->trackModelFilter.index(pr, 0);
    for (auto const cr : kvr::iota(this->trackModelFilter.rowCount(pi)))
    {
      auto const& ci = this->trackModelFilter.index(cr, 0, pi);
      auto const& cv = this->trackModelFilter.data(ci, core::VisibilityRole);
      if (cv.toBool())
      {
        auto const& cd =
          this->trackModelFilter.data(ci, core::AreaLocationRole);
        if (cd.canConvert<QRectF>())
        {
          auto const& box = cd.toRectF();
          auto const minX = static_cast<float>(box.left());
          auto const maxX = static_cast<float>(box.right());
          auto const minY = static_cast<float>(box.top());
          auto const maxY = static_cast<float>(box.bottom());
          vertexData.append(minX); vertexData.append(minY);
          vertexData.append(maxX); vertexData.append(minY);
          vertexData.append(maxX); vertexData.append(maxY);
          vertexData.append(minX); vertexData.append(maxY);
          vertexData.append(minX); vertexData.append(minY);
        }
      }
    }

    auto const last = vertexData.count() / tuple_size;
    this->detectedObjectVertexIndices.append({first, last - first});
  }

  if (vertexData.isEmpty())
  {
    return;
  }

  // Regenerate vertex buffer
  if (!this->detectedObjectVertexBuffer.isCreated())
  {
    this->detectedObjectVertexBuffer.create();
  }

  this->detectedObjectVertexBuffer.bind();
  this->detectedObjectVertexBuffer.allocate(
    vertexData.data(), vertexData.size() * static_cast<int>(sizeof(float)));
  this->detectedObjectVertexBuffer.release();
}

// ----------------------------------------------------------------------------
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
    q, [t, cookie = this->percentileCookie, this](float low, float high){
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

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
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

  this->imageShaderProgram.setUniformValue(
    this->imageHomographyLocation, this->homography);
  this->imageShaderProgram.setUniformValue(
    this->imageViewHomographyLocation, this->viewHomography);

  this->imageShaderProgram.setUniformValue(
    this->levelShiftLocation, levelShift);
  this->imageShaderProgram.setUniformValue(
    this->levelScaleLocation, levelScale);

  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  this->imageVertexBuffer.release();
  this->imageTexture.release();
  this->imageShaderProgram.release();
}

// ----------------------------------------------------------------------------
void PlayerPrivate::drawDetections(QOpenGLFunctions* functions)
{
  this->detectionShaderProgram.bind();
  this->detectedObjectVertexBuffer.bind();

  this->detectionShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);
  this->detectionShaderProgram.enableAttributeArray(0);

  this->detectionShaderProgram.setUniformValue(
    this->detectionHomographyLocation, this->homography);
  this->detectionShaderProgram.setUniformValue(
    this->detectionViewHomographyLocation, this->viewHomography);

  for (auto const& vertexInfo : this->detectedObjectVertexIndices)
  {
    // TODO set color based on selection state

    for (auto const n : kvr::iota(vertexInfo.second / 5))
    {
      auto const offset = vertexInfo.first + (n * 5);
      functions->glDrawArrays(GL_LINE_STRIP, offset, 5);
    }
  }

  this->detectedObjectVertexBuffer.release();
  this->detectionShaderProgram.release();
}

// ----------------------------------------------------------------------------
void PlayerPrivate::updateDetections()
{
  QTE_Q();

  q->makeCurrent();
  this->updateDetectedObjectVertexBuffers();
  q->doneCurrent();

  q->update();
}

} // namespace gui

} // namespace sealtk
