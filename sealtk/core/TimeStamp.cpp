/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/TimeStamp.hpp>

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp()
  : timeDomain{},
    frameDomain{}
{
}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(kwiver::vital::time_usec_t t, kwiver::vital::frame_id_t f)
  : kwiver::vital::timestamp{t, f},
    timeDomain{},
    frameDomain{}
{
}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(TimeStamp const& other)
  : kwiver::vital::timestamp{other},
    timeDomain{other.timeDomain},
    frameDomain{other.frameDomain}
{
}

// ----------------------------------------------------------------------------
int TimeStamp::get_time_domain() const
{
  return this->timeDomain;
}

// ----------------------------------------------------------------------------
TimeStamp& TimeStamp::set_time_domain(int time_domain)
{
  this->timeDomain = time_domain;
  return *this;
}

// ----------------------------------------------------------------------------
int TimeStamp::get_frame_domain() const
{
  return this->frameDomain;
}

// ----------------------------------------------------------------------------
TimeStamp& TimeStamp::set_frame_domain(int frame_domain)
{
  this->frameDomain = frame_domain;
  return *this;
}

// ----------------------------------------------------------------------------
#define COMPARE(OP, TIME_CHECK, FALLBACK)                                     \
bool TimeStamp                                                                \
::operator OP(TimeStamp const& rhs) const                                     \
{                                                                             \
  bool const timeValid = this->has_valid_time() && rhs.has_valid_time() &&    \
    this->timeDomain == rhs.timeDomain;                                       \
  bool const frameValid = this->has_valid_frame() && rhs.has_valid_frame()    \
    && this->frameDomain == rhs.frameDomain;                                  \
                                                                              \
  if (!timeValid && !frameValid)                                              \
  {                                                                           \
    return false;                                                             \
  }                                                                           \
                                                                              \
  if (timeValid)                                                              \
  {                                                                           \
    TIME_CHECK                                                                \
  }                                                                           \
                                                                              \
  if (frameValid)                                                             \
  {                                                                           \
    return this->get_frame() OP rhs.get_frame();                              \
  }                                                                           \
                                                                              \
  return FALLBACK;                                                            \
}

COMPARE(==,
    if (this->get_time_usec() != rhs.get_time_usec())
    {
      return false;
    }
, true)

COMPARE(>=,
    return this->get_time_usec() >= rhs.get_time_usec();
, true)

COMPARE(<=,
    return this->get_time_usec() <= rhs.get_time_usec();
, true)

COMPARE(>,
    if (this->get_time_usec() > rhs.get_time_usec())
    {
      return true;
    }
    else if (this->get_time_usec() < rhs.get_time_usec())
    {
      return false;
    }
, false)

COMPARE(<,
    if (this->get_time_usec() < rhs.get_time_usec())
    {
      return true;
    }
    else if (this->get_time_usec() > rhs.get_time_usec())
    {
      return false;
    }
, false)

// ----------------------------------------------------------------------------
bool TimeStamp::operator!=(TimeStamp const& rhs) const
{
  return !this->operator==(rhs);
}

} // namespace core

} // namespace sealtk
