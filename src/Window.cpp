/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Window.hpp"

#include "About.hpp"
#include "Config.h"

#include "ui_Window.h"

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

}
