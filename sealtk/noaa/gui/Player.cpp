/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Player.hpp>

#include <sealtk/core/KwiverVideoSource.hpp>

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

#include <memory>
#include <vector>

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
  QAction* loadDetectionsAction;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

// ----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : sealtk::gui::Player{parent},
    d_ptr{new PlayerPrivate}
{
  QTE_D();

  d->loadDetectionsAction = new QAction{"Load Detections...", this};

  connect(d->loadDetectionsAction, &QAction::triggered,
          this, &Player::loadDetectionsTriggered);

  connect(this, &sealtk::gui::Player::videoSourceSet, this,
          [d](sealtk::core::VideoSource* videoSource){
            d->loadDetectionsAction->setEnabled(
              videoSource &&
              qobject_cast<sealtk::core::KwiverVideoSource*>(videoSource));
          });

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
void Player::contextMenuEvent(QContextMenuEvent* event)
{
  QTE_D();

  auto* menu = new QMenu;

  auto* submenu = menu->addMenu("Load Video");
  for (auto& action : d->videoSourceActions)
  {
    submenu->addAction(action);
  }

  menu->addAction(d->loadDetectionsAction);

  menu->exec(event->globalPos());
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
