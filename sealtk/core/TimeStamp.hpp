/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_TimeStamp_hpp
#define sealtk_core_TimeStamp_hpp

#include <sealtk/core/Export.h>

#include <vital/types/timestamp.h>

namespace sealtk
{

namespace core
{

class SEALTK_CORE_EXPORT TimeStamp : public kwiver::vital::timestamp
{
public:
  TimeStamp();
  TimeStamp(kwiver::vital::time_usec_t t, kwiver::vital::frame_id_t f);
  TimeStamp(TimeStamp const& other);

  int get_time_domain() const;
  TimeStamp& set_time_domain(int time_domain);

  int get_frame_domain() const;
  TimeStamp& set_frame_domain(int frame_domain);

  bool operator==(TimeStamp const& rhs) const;
  bool operator!=(TimeStamp const& rhs) const;
  bool operator<(TimeStamp const& rhs) const;
  bool operator>(TimeStamp const& rhs) const;
  bool operator<=(TimeStamp const& rhs) const;
  bool operator>=(TimeStamp const& rhs) const;

private:
  int timeDomain;
  int frameDomain;
};

} // namespace core

} // namespace sealtk

#endif
