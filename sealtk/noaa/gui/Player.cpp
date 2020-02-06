/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Player.hpp>

#include <sealtk/core/KwiverTrackModel.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>

#include <vital/algo/transform_2d_io.h>
#include <vital/exceptions/base.h>

#include <qtStlUtil.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMatrix4x4>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QTransform>

#include <memory>
#include <vector>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;

namespace sc = sealtk::core;

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ============================================================================
class PlayerPrivate
{
public:
  QMenu* contextMenu = nullptr;
  QMenu* loadVideoMenu = nullptr;

  QAction* loadTransformAction = nullptr;
  QAction* resetTransformAction = nullptr;
  QAction* loadDetectionsAction = nullptr;
  QAction* saveDetectionsAction = nullptr;
  QAction* mergeDetectionsAction = nullptr;

  QPointer<QAbstractItemModel> trackModel;
  QSet<qint64> selectedTrackIds;

  kv::transform_2d_sptr transform;
  QSizeF imageSize;

  void loadTransform(Player* q);
  void resetTransform(Player* q);
  void updateTransform(Player* q);

  void mergeSelectedTracks(Player* q);
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

// ----------------------------------------------------------------------------
Player::Player(Role role, QWidget* parent)
  : sealtk::gui::Player{parent},
    d_ptr{new PlayerPrivate}
{
  QTE_D();

  d->contextMenu = new QMenu{this};
  d->contextMenu->addSection(QStringLiteral("Video"));

  d->loadVideoMenu = d->contextMenu->addMenu("Load &Video");

  if (role == Role::Slave)
  {
    d->loadTransformAction = new QAction{"Load &Transformation...", this};
    d->resetTransformAction = new QAction{"&Reset Transformation...", this};
    d->resetTransformAction->setEnabled(false);

    connect(d->loadTransformAction, &QAction::triggered,
            this, [this, d]{ d->loadTransform(this); });
    connect(d->resetTransformAction, &QAction::triggered,
            this, [this, d]{ d->resetTransform(this); });

    d->contextMenu->addAction(d->loadTransformAction);
    d->contextMenu->addAction(d->resetTransformAction);
  }

  d->contextMenu->addSection(QStringLiteral("Detections"));

  d->loadDetectionsAction = new QAction{"&Load Detections...", this};
  d->saveDetectionsAction = new QAction{"&Save Detections...", this};
  d->mergeDetectionsAction = new QAction{"&Merge Detections", this};

  connect(d->loadDetectionsAction, &QAction::triggered,
          this, &Player::loadDetectionsTriggered);
  connect(d->saveDetectionsAction, &QAction::triggered,
          this, &Player::saveDetectionsTriggered);
  connect(d->mergeDetectionsAction, &QAction::triggered,
          this, [this, d] { d->mergeSelectedTracks(this); });

    d->contextMenu->addAction(d->loadDetectionsAction);
    d->contextMenu->addAction(d->saveDetectionsAction);
    d->contextMenu->addAction(d->mergeDetectionsAction);
}

// ----------------------------------------------------------------------------
Player::~Player()
{
}

// ----------------------------------------------------------------------------
void Player::registerVideoSourceFactory(
  QString const& name, sealtk::core::VideoSourceFactory* factory, void* handle)
{
  QTE_D();

  auto action = new QAction{name, this};
  connect(action, &QAction::triggered, factory,
          [factory, handle]{ factory->loadVideoSource(handle); });

  d->loadVideoMenu->addAction(action);
}

// ----------------------------------------------------------------------------
void Player::setImage(kv::image_container_sptr const& image,
                      sealtk::core::VideoMetaData const& metaData)
{
  QTE_D();

  sealtk::gui::Player::setImage(image, metaData);

  if (image)
  {
    auto const w = static_cast<qreal>(image->width());
    auto const h = static_cast<qreal>(image->height());
    d->imageSize = QSizeF{w, h};
    d->updateTransform(this);
  }
  else
  {
    d->imageSize = QSizeF{};
  }
}

// ----------------------------------------------------------------------------
void Player::setTrackModel(QAbstractItemModel* model)
{
  QTE_D();

  sealtk::gui::Player::setTrackModel(model);

  d->trackModel = model;
}

// ----------------------------------------------------------------------------
void Player::setSelectedTrackIds(QSet<qint64> const& selectedIds)
{
  QTE_D();

  sealtk::gui::Player::setSelectedTrackIds(selectedIds);

  d->selectedTrackIds = selectedIds;
}

// ----------------------------------------------------------------------------
void Player::contextMenuEvent(QContextMenuEvent* event)
{
  QTE_D();

  d->saveDetectionsAction->setEnabled(this->videoSource());
  d->mergeDetectionsAction->setEnabled(
    d->trackModel && d->selectedTrackIds.count() > 1);

  d->contextMenu->exec(event->globalPos());
}

// ----------------------------------------------------------------------------
void PlayerPrivate::loadTransform(Player* q)
{
  auto const& path = QFileDialog::getOpenFileName(q);
  if (!path.isEmpty())
  {
    auto config = kv::config_block::empty_config();
    config->set_value("transform_reader:type", "auto");

    try
    {
      kva::transform_2d_io_sptr ti;
      kva::transform_2d_io::set_nested_algo_configuration(
        "transform_reader", config, ti);
      if (ti)
      {
        this->transform = ti->load(stdString(path));
        this->updateTransform(q);
      }
      else
      {
        QMessageBox::warning(
          q, QStringLiteral("Failed to load transformation"),
          QStringLiteral("The transformation could not be loaded: "
                         "a required plugin was not found"));
      }
    }
    catch (kv::vital_exception const& e)
    {
      auto const& text =
        QStringLiteral("The transformation could not be loaded: ") +
        QString::fromLocal8Bit(e.what());
      QMessageBox::warning(
        q, QStringLiteral("Failed to load transformation"), text);
    }
  }
}

// ----------------------------------------------------------------------------
void PlayerPrivate::resetTransform(Player* q)
{
  this->transform.reset();
  this->updateTransform(q);
}

// ----------------------------------------------------------------------------
void PlayerPrivate::updateTransform(Player* q)
{
  if (this->imageSize.isValid() && this->transform)
  {
    auto const w = this->imageSize.width();
    auto const h = this->imageSize.height();

    QPolygonF in, out;
    in.append({0, 0});
    in.append({0, h});
    in.append({w, h});
    in.append({w, 0});

    for (auto const& point : in)
    {
      auto const mappedPoint = this->transform->map({point.x(), point.y()});
      out.append({mappedPoint.x(), mappedPoint.y()});
    }

    QTransform xf;
    if (QTransform::quadToQuad(in, out, xf))
    {
      q->setHomography(xf);
      if (this->resetTransformAction)
      {
        this->resetTransformAction->setEnabled(true);
      }
      return;
    }
  }

  // Did not successfully set homography; use identity
  q->setHomography({});
  if (this->resetTransformAction)
  {
    this->resetTransformAction->setEnabled(false);
  }
}

// ----------------------------------------------------------------------------
void PlayerPrivate::mergeSelectedTracks(Player* q)
{
  auto* const model =
    qobject_cast<sc::KwiverTrackModel*>(this->trackModel);

  if (!model)
  {
    QMessageBox::warning(
      q, QStringLiteral("Failed to merge tracks"),
      QStringLiteral("No model; are detections loaded?"));
    return;
  }

  using Result = sc::KwiverTrackModel::MergeTracksResult;
  switch (model->mergeTracks(this->selectedTrackIds))
  {
    case Result::NothingToDo:
      QMessageBox::warning(
        q, QStringLiteral("Failed to merge tracks"),
        QStringLiteral("Select two or more tracks to merge."));
      break;

    case Result::OverlappingStates:
      QMessageBox::warning(
        q, QStringLiteral("Failed to merge tracks"),
        QStringLiteral("Tracks with overlapping states cannot be merged."));
      break;

    default:
      break;
  }
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
