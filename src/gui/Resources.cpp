/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Resources.hpp>

#include <QDir>

static void init()
{
  Q_INIT_RESOURCE(resources);
  Q_INIT_RESOURCE(PlayerViewer);
}

static void cleanup()
{
  Q_CLEANUP_RESOURCE(PlayerViewer);
  Q_CLEANUP_RESOURCE(resources);
}

namespace sealtk
{

namespace gui
{

Resources::Resources()
{
  init();
}

Resources::~Resources()
{
  cleanup();
}

}

}
