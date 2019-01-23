/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_test_TestCore_hpp
#define sealtk_test_TestCore_hpp

#include <QString>

#define SEALTK_TEST_DATA_PATH(f) QStringLiteral(SEALTK_TEST_DATA_DIR "/" f)

namespace sealtk
{

namespace test
{

static inline QString testDataPath(QString const& path)
{
  return QStringLiteral(SEALTK_TEST_DATA_DIR "/") + path;
}

}

}

#endif
