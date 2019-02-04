/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>

#include <sealtk/core/VideoController.hpp>

#include <sealtk/gui/SplitterWindow.hpp>

#include <QDockWidget>

#include <memory>

namespace sealtk
{

namespace noaa
{

namespace gui
{

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)

  Ui::Window ui;

  std::unique_ptr<sealtk::core::VideoController> videoController;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

//-----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : QMainWindow{parent},
    d_ptr{new WindowPrivate{this}}
{
  QTE_D();
  d->ui.setupUi(this);

  d->videoController = std::make_unique<sealtk::core::VideoController>(this);
  d->ui.control->setVideoController(d->videoController.get());

  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
}

//-----------------------------------------------------------------------------
Window::~Window()
{
}

//-----------------------------------------------------------------------------
void Window::showAbout()
{
  About about{this};
  about.exec();
}

//-----------------------------------------------------------------------------
WindowPrivate::WindowPrivate(Window* parent)
  : q_ptr{parent}
{
}

}

}

}
