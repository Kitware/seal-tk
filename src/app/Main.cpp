/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <QApplication>

#include "Player.hpp"
#include "PlayerTree.hpp"
#include "Window.hpp"

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app{argc, argv};

  auto* window = new sealtk::Window;
  window->registerPanelType<sealtk::Player>("Player");
  window->registerPanelType<sealtk::PlayerTree>("Players");
  window->show();

  return app.exec();
}
