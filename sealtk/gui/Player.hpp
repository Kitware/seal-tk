/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Player_hpp
#define sealtk_gui_Player_hpp

#include <sealtk/gui/Export.h>

#include <sealtk/core/VideoDistributor.hpp>

#include <vital/types/detected_object_set.h>
#include <vital/types/image_container.h>
#include <vital/types/transform_2d.h>

#include <qtGlobal.h>

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QPointF>
#include <QRectF>

class QAbstractItemModel;
class QImage;

namespace sealtk
{

namespace gui
{

QTE_BEGIN_META_NAMESPACE(SEALTK_GUI_EXPORT, player_enums)

enum class ContrastMode
{
  Manual,
  Percentile,
};

QTE_ENUM_NS(ContrastMode)

QTE_END_META_NAMESPACE()

class PlayerTool;

class PlayerPrivate;

class SEALTK_GUI_EXPORT Player : public QOpenGLWidget
{
  Q_OBJECT

  Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
  Q_PROPERTY(QPointF center READ center WRITE setCenter NOTIFY centerChanged)
  Q_PROPERTY(QSize homographyImageSize
             READ homographyImageSize
             WRITE setHomographyImageSize)
  Q_PROPERTY(ContrastMode contrastMode READ contrastMode WRITE setContrastMode)
  Q_PROPERTY(QMatrix4x4 homography READ homography WRITE setHomography)
  Q_PROPERTY(QMatrix4x4 viewHomography READ viewHomography)

  Q_PROPERTY(QColor defaultColor READ defaultColor
             WRITE setDefaultColor NOTIFY defaultColorChanged)
  Q_PROPERTY(QColor selectionColor READ selectionColor
             WRITE setSelectionColor NOTIFY defaultColorChanged)
  Q_PROPERTY(QColor pendingColor READ pendingColor
             WRITE setPendingColor NOTIFY pendingColorChanged)

public:
  explicit Player(QWidget* parent = nullptr);
  ~Player() override;

  float zoom() const;
  QPointF center() const;
  core::VideoDistributor* videoSource() const;
  ContrastMode contrastMode() const;
  QSize homographyImageSize() const;
  QMatrix4x4 homography() const;
  QMatrix4x4 viewHomography() const;
  PlayerTool* activeTool() const;
  bool hasImage() const;
  virtual bool hasTransform() const;

  QPointF viewToImage(QPointF const& viewCoord) const;

  QColor defaultColor() const;
  QColor selectionColor() const;
  QColor pendingColor() const;

signals:
  void zoomChanged(float zoom) const;
  void centerChanged(QPointF center) const;
  void imageSizeChanged(QSize imageSize) const;
  void imageNameChanged(QString const& imageName) const;
  void activeToolChanged(PlayerTool* tool) const;

  void defaultColorChanged(QColor const& color) const;
  void selectionColorChanged(QColor const& color) const;
  void pendingColorChanged(QColor const& color) const;

  void trackPicked(qint64 id) const;

public slots:
  virtual void setImage(kwiver::vital::image_container_sptr const& image,
                        sealtk::core::VideoMetaData const& metaData);
  virtual void setTrackModel(QAbstractItemModel* model);
  virtual void setSelectedTrackIds(QSet<qint64> const& selectedIds);

  void setHomography(QMatrix4x4 const& homography);
  void setHomographyImageSize(QSize size);
  void setZoom(float zoom);
  void setCenter(QPointF center);
  void setVideoSource(core::VideoDistributor* videoSource);
  void setContrastMode(ContrastMode mode);
  void setManualLevels(float low, float high);
  void setPercentiles(double deviance, double tolerance);
  void setActiveTool(PlayerTool* tool);

  void setDefaultColor(QColor const& color);
  void setSelectionColor(QColor const& color);
  void setPendingColor(QColor const& color);

  void setCenterToTrack(qint64 id, kwiver::vital::timestamp::time_t time);

  void setShadowTrackModel(QObject* source, QAbstractItemModel* model);
  void setShadowTransform(
    QObject* source, kwiver::vital::transform_2d_sptr const& transform);

protected:
  QTE_DECLARE_PRIVATE(Player)

  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

  void paintEvent(QPaintEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  QTE_DECLARE_PRIVATE_RPTR(Player)
};

} // namespace gui

} // namespace sealtk

#endif
