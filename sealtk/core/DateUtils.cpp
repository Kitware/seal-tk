/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/DateUtils.hpp>

namespace sealtk
{

namespace core
{

QDateTime vitalTimeToQDateTime(kwiver::vital::timestamp::time_t time)
{
  qint64 ms = time / 1000;
  return QDateTime::fromMSecsSinceEpoch(ms, Qt::UTC);
}

kwiver::vital::timestamp::time_t qDateTimeToVitalTime(
  QDateTime const& dateTime)
{
  return dateTime.toMSecsSinceEpoch() * 1000;
}

}

}
