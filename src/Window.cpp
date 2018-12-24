/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Window.hpp"

#include "About.hpp"
#include "Config.h"
#include "Player.hpp"

#include "ui_Window.h"

#include <QDockWidget>

#include <memory>

namespace sealtk
{

class WindowPrivate : public QObject
{
public:
  WindowPrivate(Window* parent);

  Window* parent;
  Ui::Window ui;

public slots:
  void showAbout();
  template<typename T>
  void newPanel();
};

QTE_IMPLEMENT_D_FUNC(Window)

Window::Window(QWidget* parent)
  : QMainWindow(parent),
    d_ptr{new WindowPrivate(this)}
{
  QTE_D();
  d->ui.setupUi(this);

  this->setWindowTitle(SEALTK_Q_TITLE);

  connect(d->ui.actionAbout, &QAction::triggered,
          d, &WindowPrivate::showAbout);
  connect(d->ui.actionNewPlayer, &QAction::triggered,
          d, &WindowPrivate::newPanel<Player>);
}

Window::~Window()
{
}

WindowPrivate::WindowPrivate(Window* parent)
  : QObject(parent),
    parent(parent)
{
}

void WindowPrivate::showAbout()
{
  sealtk::About about(this->parent);
  about.exec();
}

template<typename T>
void WindowPrivate::newPanel()
{
  auto* dock = new QDockWidget(this->parent);
  auto* widget = new T(dock);
  auto flags = widget->windowFlags();
  flags &= ~Qt::Window;
  widget->setWindowFlags(flags);
  dock->setWidget(widget);
  dock->show();
}

}
