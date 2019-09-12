/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/TimeStamp.hpp>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

template <typename R, typename T> using comparator = R (*)(T, T);

enum class ComparisonResult
{
  False,
  True,
  Indeterminate
};

// ----------------------------------------------------------------------------
bool compare(
  TimeStamp const& lhs, TimeStamp const& rhs,
  comparator<ComparisonResult, kwiver::vital::time_usec_t> compareTimes,
  comparator<bool, kwiver::vital::frame_id_t> compareFrames, bool fallback)
{
  bool const timeValid =
    lhs.has_valid_time() && rhs.has_valid_time() &&
    lhs.get_time_domain() == rhs.get_time_domain();
  bool const frameValid =
    lhs.has_valid_frame() && rhs.has_valid_frame() &&
    lhs.get_frame_domain() == rhs.get_frame_domain();

  if (!timeValid && !frameValid)
  {
    return false;
  }

  if (timeValid)
  {
    switch (compareTimes(lhs.get_time_usec(), rhs.get_time_usec()))
    {
      case ComparisonResult::False:
        return false;
      case ComparisonResult::True:
        return true;
      default:
        break;
    }
  }

  if (frameValid)
  {
    return compareFrames(lhs.get_frame(), rhs.get_frame());
  }

  return fallback;
}

// ----------------------------------------------------------------------------
#define COMPARE_TIMES(LOGIC) \
  [](kv::time_usec_t lhs, kv::time_usec_t rhs){ \
    do { LOGIC } while (0); \
    return ComparisonResult::Indeterminate; \
  }
#define COMPARE_FRAMES(OP) \
  [](kv::frame_id_t lhs, kv::frame_id_t rhs){ return lhs OP rhs; }

} // namespace <anonymous>

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp()
  : timeDomain{},
    frameDomain{}
{
}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(kv::time_usec_t t, kv::frame_id_t f)
  : kv::timestamp{t, f},
    timeDomain{},
    frameDomain{}
{
}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(TimeStamp const& other)
  : kv::timestamp{other},
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
bool TimeStamp::operator==(TimeStamp const& rhs) const
{
  return compare(
    *this, rhs,
    COMPARE_TIMES(
      if (lhs != rhs)
      {
        return ComparisonResult::False;
      }),
    COMPARE_FRAMES(==),
    true);
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator>=(TimeStamp const& rhs) const
{
  return compare(
    *this, rhs,
    COMPARE_TIMES(
      return (lhs >= rhs) ? ComparisonResult::True : ComparisonResult::False;),
    COMPARE_FRAMES(>=),
    true);
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator>(TimeStamp const& rhs) const
{
  return compare(
    *this, rhs,
    COMPARE_TIMES(
      if (lhs > rhs)
      {
        return ComparisonResult::True;
      }
      if (lhs < rhs)
      {
        return ComparisonResult::False;
      }),
    COMPARE_FRAMES(>),
    false);
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator<=(TimeStamp const& rhs) const
{
  return rhs >= *this;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator<(TimeStamp const& rhs) const
{
  return rhs > *this;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator!=(TimeStamp const& rhs) const
{
  return !this->operator==(rhs);
}

} // namespace core

} // namespace sealtk
