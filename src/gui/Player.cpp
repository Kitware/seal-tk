/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Player.hpp>
#include "ui_Player.h"

#include <sealtk/gui/PlayerViewer.hpp>
#include <sealtk/gui/Window.hpp>

#include <QFileDialog>
#include <QImage>
#include <QString>

namespace sealtk
{

namespace gui
{

//=============================================================================
class PlayerPrivate
{
public:
  Ui::Player ui;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

//-----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : Panel(parent),
    d_ptr{new PlayerPrivate}
{
  QTE_D();

  d->ui.setupUi(this);

  d->ui.video->addAction(d->ui.actionOpen);

  connect(d->ui.actionOpen, &QAction::triggered,
          this, &Player::openFile);
  connect(this, &Player::imageOpened,
          d->ui.video, &PlayerViewer::setImage);
}

//-----------------------------------------------------------------------------
Player::~Player()
{
}

//-----------------------------------------------------------------------------
void Player::init(Window* window)
{
  int counter = window->panelCounter();
  this->setWindowTitle(QStringLiteral("Untitled-%1").arg(counter));
  window->setPanelCounter(counter + 1);
}

//-----------------------------------------------------------------------------
void Player::openFile()
{
  QString filename = QFileDialog::getOpenFileName(this);
  if (!filename.isNull())
  {
    QImage image(filename);
    if (!image.isNull())
    {
      emit this->imageOpened(image);
    }
  }
}

}

}
