/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>
#include <sealtk/test/Config.h>

namespace sealtk
{

namespace test
{

// ----------------------------------------------------------------------------
QString testDataDir()
{
  return SEALTK_TEST_DATA_DIR;
}

// ----------------------------------------------------------------------------
QString testDataPath(QString const& path)
{
  QString dir = testDataDir();
  if (!dir.isNull())
  {
    return dir + "/" + path;
  }
  else
  {
    return path;
  }
}

}

}
