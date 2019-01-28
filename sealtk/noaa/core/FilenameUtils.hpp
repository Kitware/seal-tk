/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_core_FilenameUtils_hpp
#define sealtk_noaa_core_FilenameUtils_hpp

#include <QDateTime>

namespace sealtk
{

namespace noaa
{

namespace core
{

QDateTime imageFilenameToQDateTime(QString const& path);

}

}

}

#endif
