/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Window.hpp"
#include "ui_Window.h"

#include "About.hpp"
#include "Config.h"

#include <QDockWidget>

namespace sealtk
{

class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  Window* parent;
  Ui::Window ui;
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
          this, &Window::showAbout);
}

Window::~Window()
{
}

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

void Window::showAbout()
{
  sealtk::About about(this);
  about.exec();
}

void Window::newDockablePanel(const QMetaObject& type)
{
  auto* dock = new QDockWidget(this);
  auto* widget = qobject_cast<QWidget*>(
    type.newInstance(Q_ARG(QWidget*, dock)));
  dock->setWidget(widget);

  this->addDockWidget(Qt::LeftDockWidgetArea, dock);
  dock->show();
}

void Window::newLeftPanel(const QMetaObject& type)
{
  QTE_D();

  auto* widget = qobject_cast<QWidget*>(
    type.newInstance(Q_ARG(QWidget*, this)));

  d->ui.centralwidgetLayout->insertWidget(0, widget);
  widget->show();
}

void Window::newRightPanel(const QMetaObject& type)
{
  QTE_D();

  auto* widget = qobject_cast<QWidget*>(
    type.newInstance(Q_ARG(QWidget*, this)));

  d->ui.centralwidgetLayout->addWidget(widget);
  widget->show();
}

WindowPrivate::WindowPrivate(Window* parent)
  : parent(parent)
{
}

}
