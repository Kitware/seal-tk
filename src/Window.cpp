/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Window.hpp"
#include "ui_Window.h"

#include "About.hpp"
#include "Config.h"
#include "Panel.hpp"
#include "SplitterWindow.hpp"

#include <QDockWidget>

namespace sealtk
{

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  Panel* createPanel(const QMetaObject& type);

  Window* parent;
  Ui::Window ui;
  int panelCounter;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

//-----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : QMainWindow(parent),
    d_ptr{new WindowPrivate(this)}
{
  QTE_D();
  d->ui.setupUi(this);

  this->setWindowTitle(SEALTK_Q_TITLE);

  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
}

//-----------------------------------------------------------------------------
Window::~Window()
{
}

//-----------------------------------------------------------------------------
void Window::registerPanelType(const QString& name, const QMetaObject& type)
{
  QTE_D();

  auto* dockableAction = new QAction(name, this);
  d->ui.menuNewDockablePanel->addAction(dockableAction);
  connect(dockableAction, &QAction::triggered,
          this, [&type, this]() { this->newDockablePanel(type); });

  auto* leftAction = new QAction(name, this);
  d->ui.menuNewLeftPanel->addAction(leftAction);
  connect(leftAction, &QAction::triggered,
          this, [&type, this]() { this->newLeftPanel(type); });

  auto* rightAction = new QAction(name, this);
  d->ui.menuNewRightPanel->addAction(rightAction);
  connect(rightAction, &QAction::triggered,
          this, [&type, this]() { this->newRightPanel(type); });
}

//-----------------------------------------------------------------------------
int Window::panelCounter() const
{
  QTE_D();

  return d->panelCounter;
}

//-----------------------------------------------------------------------------
void Window::setPanelCounter(int counter)
{
  QTE_D();

  d->panelCounter = counter;
}

//-----------------------------------------------------------------------------
void Window::showAbout()
{
  About about(this);
  about.exec();
}

//-----------------------------------------------------------------------------
void Window::newDockablePanel(const QMetaObject& type)
{
  QTE_D();

  auto* panel = d->createPanel(type);

  auto* dock = new QDockWidget(this);
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

  auto* panel = d->createPanel(type);

  auto* window = new SplitterWindow(this);
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

  auto* panel = d->createPanel(type);

  auto* window = new SplitterWindow(this);
  window->setCentralWidget(panel);
  window->setWindowTitle(panel->windowTitle());
  connect(panel, &QWidget::windowTitleChanged,
          window, &QWidget::setWindowTitle);

  d->ui.centralwidget->addWidget(window);
  panel->show();
}

//-----------------------------------------------------------------------------
WindowPrivate::WindowPrivate(Window* parent)
  : parent(parent),
    panelCounter(1)
{
}

//-----------------------------------------------------------------------------
Panel* WindowPrivate::createPanel(const QMetaObject& type)
{
  auto* panel = qobject_cast<Panel*>(
    type.newInstance(Q_ARG(QWidget*, this->parent)));
  panel->init(this->parent);

  return panel;
}

}
