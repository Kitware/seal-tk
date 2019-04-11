/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <QApplication>
#include <QDir>

#include <sealtk/noaa/gui/Window.hpp>

#include <sealtk/gui/Resources.hpp>

#include <vital/plugin_loader/plugin_manager.h>

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  sealtk::gui::Resources r;
  Q_INIT_RESOURCE(SEALTKBranding);

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication app{argc, argv};

  sealtk::noaa::gui::Window window;
  window.show();

  return app.exec();
}
