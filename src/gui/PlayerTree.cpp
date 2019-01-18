/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/PlayerTree.hpp>
#include "ui_PlayerTree.h"

#include <sealtk/gui/Window.hpp>

namespace sealtk
{

namespace gui
{

//=============================================================================
class PlayerTreePrivate
{
public:
  Ui::PlayerTree ui;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(PlayerTree)

//-----------------------------------------------------------------------------
PlayerTree::PlayerTree(QWidget* parent)
  : Panel(parent),
    d_ptr{new PlayerTreePrivate}
{
  QTE_D();

  d->ui.setupUi(this);
}

//-----------------------------------------------------------------------------
PlayerTree::~PlayerTree()
{
}

//-----------------------------------------------------------------------------
void PlayerTree::init(Window* window)
{
}

}

}
