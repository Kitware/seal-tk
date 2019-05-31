/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_DateUtils_hpp
#define sealtk_core_DateUtils_hpp

#include <sealtk/core/Export.h>

#include <vital/types/timestamp.h>

#include <QDateTime>

namespace sealtk
{

namespace core
{

SEALTK_CORE_EXPORT
QDateTime vitalTimeToQDateTime(kwiver::vital::timestamp::time_t time);

SEALTK_CORE_EXPORT
kwiver::vital::timestamp::time_t qDateTimeToVitalTime(
  QDateTime const& dateTime);

SEALTK_CORE_EXPORT
QString dateString(QDateTime const& dateTime);

SEALTK_CORE_EXPORT
QString dateString(kwiver::vital::timestamp::time_t time);

SEALTK_CORE_EXPORT
QString timeString(QDateTime const& dateTime);

SEALTK_CORE_EXPORT
QString timeString(kwiver::vital::timestamp::time_t time);

} // namespace core

} // namespace sealtk

#endif
