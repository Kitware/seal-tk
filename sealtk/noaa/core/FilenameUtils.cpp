/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/core/FilenameUtils.hpp>

#include <QRegularExpression>

namespace sealtk
{

namespace noaa
{

namespace core
{

QDateTime imageFilenameToQDateTime(QString const& path)
{
  static QRegularExpression const regex{
    "([0-9]{2})([0-9]{2})([0-9]{2})_([0-9]{2})([0-9]{2})([0-9]{2})\\."
    "([0-9]{3})_[^_]*\\.[^_.]+$"
  };

  auto matches = regex.globalMatch(path);

  if (matches.hasNext())
  {
    auto match = matches.next();
    if (!matches.hasNext())
    {
      return QDateTime{
        {
          match.captured(1).toInt() + 2000, // Y2K all over again...
          match.captured(2).toInt(),
          match.captured(3).toInt()
        },
        {
          match.captured(4).toInt(),
          match.captured(5).toInt(),
          match.captured(6).toInt(),
          match.captured(7).toInt()
        },
        Qt::UTC
      };
    }
  }

  return QDateTime{};
}

}

}

}
