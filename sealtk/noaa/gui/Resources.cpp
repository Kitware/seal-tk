/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Resources.hpp>

#include <QDir>

// ----------------------------------------------------------------------------
static void init()
{
  Q_INIT_RESOURCE(SEALTKBranding);
}

// ----------------------------------------------------------------------------
static void cleanup()
{
  Q_CLEANUP_RESOURCE(SEALTKBranding);
}

///////////////////////////////////////////////////////////////////////////////

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ----------------------------------------------------------------------------
Resources::Resources()
{
  init();
}

// ----------------------------------------------------------------------------
Resources::~Resources()
{
  cleanup();
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
