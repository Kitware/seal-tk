/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_Window_hpp
#define sealtk_noaa_gui_Window_hpp

#include <sealtk/gui/Window.hpp>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class WindowPrivate;

class Window : public sealtk::gui::Window
{
  Q_OBJECT

public:
  explicit Window(QWidget* parent = nullptr);
  ~Window() override;

public slots:
  void showAbout();
  void newDockablePanel(QMetaObject const& type);
  void newLeftPanel(QMetaObject const& type);
  void newRightPanel(QMetaObject const& type);

protected:
  QTE_DECLARE_PRIVATE(Window)

private:
  QTE_DECLARE_PRIVATE_RPTR(Window)
};

}

}

}

#endif
