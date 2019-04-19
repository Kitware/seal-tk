/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Player.hpp>

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
#include <QTransform>

#include <memory>
#include <vector>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;

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
  std::vector<QAction*> videoSourceActions;
  QAction* loadTransformAction;
  QAction* loadDetectionsAction;

  kv::transform_2d_sptr transform;
  QSizeF imageSize;

  void loadTransform(Player* q);
  void updateTransform(Player* q);
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

// ----------------------------------------------------------------------------
Player::Player(Role role, QWidget* parent)
  : sealtk::gui::Player{parent},
    d_ptr{new PlayerPrivate}
{
  QTE_D();

  if (role == Role::Slave)
  {
    d->loadTransformAction = new QAction{"Load Transformation...", this};

    connect(d->loadTransformAction, &QAction::triggered,
            this, [this, d]{ d->loadTransform(this); });
  }
  else
  {
    d->loadTransformAction = nullptr;
  }

  d->loadDetectionsAction = new QAction{"Load Detections...", this};

  connect(d->loadDetectionsAction, &QAction::triggered,
          this, &Player::loadDetectionsTriggered);

  /* TODO
  connect(this, &sealtk::gui::Player::videoSourceChanged, this,
          [d](sealtk::core::VideoSource* videoSource){
            d->loadDetectionsAction->setEnabled(
              videoSource &&
              qobject_cast<sealtk::core::KwiverVideoSource*>(videoSource));
          });
  */

  d->loadDetectionsAction->setEnabled(false);
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

  d->videoSourceActions.emplace_back(action);
}

// ----------------------------------------------------------------------------
void Player::setImage(kv::image_container_sptr const& image,
                      core::VideoMetaData const& metaData)
{
  QTE_D();

  sealtk::gui::Player::setImage(image, metaData);

  if (image)
  {
    auto const w = static_cast<qreal>(image->height());
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
void Player::contextMenuEvent(QContextMenuEvent* event)
{
  QTE_D();

  auto* menu = new QMenu;

  auto* submenu = menu->addMenu("Load Video");
  for (auto& action : d->videoSourceActions)
  {
    submenu->addAction(action);
  }

  if (d->loadTransformAction)
  {
    menu->addAction(d->loadTransformAction);
  }
  menu->addAction(d->loadDetectionsAction);

  menu->exec(event->globalPos());
}

// ----------------------------------------------------------------------------
void PlayerPrivate::loadTransform(Player* q)
{
  auto const& path = QFileDialog::getOpenFileName(q);
  if (!path.isEmpty())
  {
    auto config = kv::config_block::empty_config();
    config->set_value("transform_reader:type", "itk");

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
      return;
    }
  }

  // Did not successfully set homography; use identity
  q->setHomography({});
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
