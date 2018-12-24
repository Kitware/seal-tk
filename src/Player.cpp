/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Player.hpp"

#include "ui_Player.h"

namespace sealtk
{

class PlayerPrivate
{
public:
  Ui::Player ui;
};

QTE_IMPLEMENT_D_FUNC(Player)

Player::Player(QWidget* parent)
  : Panel(parent),
    d_ptr{new PlayerPrivate}
{
  QTE_D();

  d->ui.setupUi(this);
}

Player::~Player()
{
}

}
