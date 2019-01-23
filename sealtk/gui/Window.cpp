/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Window.hpp>

#include <sealtk/gui/Panel.hpp>

namespace sealtk
{

namespace gui
{

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  Window* parent;
  int panelCounter;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

//-----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : QMainWindow{parent},
    d_ptr{new WindowPrivate{this}}
{
}

//-----------------------------------------------------------------------------
Window::~Window()
{
}

//-----------------------------------------------------------------------------
void Window::registerPanelType(QString const& name, QMetaObject const& type)
{
  emit this->panelTypeRegistered(name, type);
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
Panel* Window::createPanel(const QMetaObject& type)
{
  auto* panel = qobject_cast<Panel*>(
    type.newInstance(Q_ARG(QWidget*, this)));
  panel->init(this);

  return panel;
}

//-----------------------------------------------------------------------------
WindowPrivate::WindowPrivate(Window* parent)
  : parent{parent},
    panelCounter{1}
{
}

}

}
