/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/About.hpp>

#include <sealtk/gui/Panel.hpp>
#include <sealtk/gui/SplitterWindow.hpp>

#include <QDockWidget>

namespace sealtk
{

namespace noaa
{

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  Window* parent;
  Ui::Window ui;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

//-----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : gui::Window{parent},
    d_ptr{new WindowPrivate{this}}
{
  QTE_D();
  d->ui.setupUi(this);

  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
  connect(this, &gui::Window::panelTypeRegistered,
          [this, d](QString const& name, QMetaObject const& type)
  {
    auto* dockableAction = new QAction{name, this};
    d->ui.menuNewDockablePanel->addAction(dockableAction);
    connect(dockableAction, &QAction::triggered,
            this, [&type, this]() { this->newDockablePanel(type); });

    auto* leftAction = new QAction{name, this};
    d->ui.menuNewLeftPanel->addAction(leftAction);
    connect(leftAction, &QAction::triggered,
            this, [&type, this]() { this->newLeftPanel(type); });

    auto* rightAction = new QAction{name, this};
    d->ui.menuNewLeftPanel->addAction(rightAction);
    connect(rightAction, &QAction::triggered,
            this, [&type, this]() { this->newLeftPanel(type); });
  });
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
void Window::newDockablePanel(const QMetaObject& type)
{
  QTE_D();

  auto* panel = this->createPanel(type);

  auto* dock = new QDockWidget{this};
  dock->setWidget(panel);
  dock->setWindowTitle(panel->windowTitle());
  connect(panel, &QWidget::windowTitleChanged,
          dock, &QWidget::setWindowTitle);

  this->addDockWidget(Qt::LeftDockWidgetArea, dock);
  dock->show();
}

//-----------------------------------------------------------------------------
void Window::newLeftPanel(const QMetaObject& type)
{
  QTE_D();

  auto* panel = this->createPanel(type);

  auto* window = new gui::SplitterWindow{this};
  window->setCentralWidget(panel);
  window->setWindowTitle(panel->windowTitle());
  connect(panel, &QWidget::windowTitleChanged,
          window, &QWidget::setWindowTitle);

  d->ui.centralwidget->insertWidget(0, window);
  panel->show();
}

//-----------------------------------------------------------------------------
void Window::newRightPanel(const QMetaObject& type)
{
  QTE_D();

  auto* panel = this->createPanel(type);

  auto* window = new gui::SplitterWindow{this};
  window->setCentralWidget(panel);
  window->setWindowTitle(panel->windowTitle());
  connect(panel, &QWidget::windowTitleChanged,
          window, &QWidget::setWindowTitle);

  d->ui.centralwidget->addWidget(window);
  panel->show();
}

//-----------------------------------------------------------------------------
WindowPrivate::WindowPrivate(Window* parent)
  : parent{parent}
{
}

}

}
