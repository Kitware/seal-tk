/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/PlayerTool.hpp>

#include <sealtk/gui/Player.hpp>

#include <QByteArray>
#include <QPair>
#include <QStack>
#include <QVariant>

namespace sealtk
{

namespace gui
{

// ============================================================================
class PlayerToolPrivate
{
public:
  PlayerToolPrivate(Player* player);

  Player* player;
  QStack<QPair<QByteArray, QVariant>> pushedProperties;
};

// ----------------------------------------------------------------------------
PlayerToolPrivate::PlayerToolPrivate(Player* player)
  : player{player}
{
}

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(PlayerTool)

// ----------------------------------------------------------------------------
PlayerTool::PlayerTool(Player* parent)
  : d_ptr{new PlayerToolPrivate{parent}},
    QObject{parent}
{
}

// ----------------------------------------------------------------------------
PlayerTool::~PlayerTool()
{
}

// ----------------------------------------------------------------------------
Player* PlayerTool::player() const
{
  QTE_D();
  return d->player;
}

// ----------------------------------------------------------------------------
void PlayerTool::activate()
{
}

// ----------------------------------------------------------------------------
void PlayerTool::deactivate()
{
  QTE_D();

  while (!d->pushedProperties.empty())
  {
    auto const pair = d->pushedProperties.pop();
    auto const name = pair.first;
    d->player->setProperty(name.data(), pair.second);
  }
}

// ----------------------------------------------------------------------------
void PlayerTool::mousePressEvent(QMouseEvent* event)
{
}

// ----------------------------------------------------------------------------
void PlayerTool::mouseReleaseEvent(QMouseEvent* event)
{
}

// ----------------------------------------------------------------------------
void PlayerTool::mouseMoveEvent(QMouseEvent* event)
{
}

// ----------------------------------------------------------------------------
void PlayerTool::paintGL()
{
}

// ----------------------------------------------------------------------------
void PlayerTool::pushProperty(char const* name, QVariant const& value)
{
  QTE_D();
  d->pushedProperties.push({name, d->player->property(name)});
  d->player->setProperty(name, value);
}

} // namespace gui

} // namespace sealtk
