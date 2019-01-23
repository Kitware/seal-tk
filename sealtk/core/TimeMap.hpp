/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_TimeMap_hpp
#define sealtk_core_TimeMap_hpp

#include <QMap>

#include <vital/types/timestamp.h>

namespace sealtk
{

namespace core
{

/// Requested seek mode.
///
/// The value relates to the caller's expected result based on the input. The
/// meaning of lower/upper bound is similar to, but not exactly the same as,
/// the like-named Qt/STL container search methods.
enum SeekMode
{
  /// Direction is not specified; the result is implementation defined.
  ///
  /// Most implementations will interpret this as "choose a sensible default"
  /// (most often ::SeekNearest).
  SeekUnspecified = -1,
  /// Request the closest possible value to the input.
  ///
  /// If the difference is split, the lower value is chosen.
  SeekNearest = 0,
  /// Request nearest value that is greater than or equal to the request.
  ///
  /// The request is treated as a lower bound for permissible result values.
  /// The result value will be the same as ::SeekExact, if such a value
  /// exists. Otherwise, the result will be the same as ::SeekNext.
  SeekLowerBound,
  /// Request nearest value that is less than or equal to the request.
  ///
  /// The request is treated as an upper bound for permissible result values.
  /// The result value will be the same as ::SeekExact, if such a value
  /// exists. Otherwise, the result will be the same as ::SeekPrevious.
  SeekUpperBound,
  /// Request an exact match only.
  ///
  /// The result value will be "exactly" equal to the request. If no such
  /// value exists, no value will be returned. An implementation is permitted
  /// to interpret "exact" as "within a reasonable amount of fuzz to
  /// accommodate for floating point rounding error".
  SeekExact,
  /// Request nearest value that is strictly greater than the request.
  ///
  /// \sa ::SeekLowerBound
  SeekNext,
  /// Request nearest value that is strictly less than the request.
  ///
  /// \sa ::SeekUpperBound
  SeekPrevious
};

namespace detail
{

template<typename Map, typename Iterator>
Iterator find(Map& map, kwiver::vital::timestamp::time_t pos,
              SeekMode direction)
{
  // Check for empty map
  if (map.count() < 1)
  {
    return map.end();
  }

  Iterator iter;

  switch (direction)
  {
    case SeekExact:
      // Exact is easy, find() does what we need
      return map.find(pos);

    case SeekLowerBound:
      // Lower is easy, lowerBound() does what we need
      return map.lowerBound(pos);

    case SeekNext:
      // Next is easy, upperBound() does what we need
      return map.upperBound(pos);

    case SeekUpperBound:
      // Check first for exact match
      iter = map.find(pos);
      if (iter != map.end())
      {
        return iter;
      }

      // Check for an answer
      if (pos < map.begin().key())
      {
        return map.end();
      }

      // upperBound() gets us the position *after* the one we want, so return
      // the preceding position (we know this exists because count() > 0)
      return --map.upperBound(pos);

    case SeekPrevious:
      // Check for an answer
      if (pos <= map.begin().key())
      {
        return map.end();
      }

      // lowerBound() gets us the position *after* the one we want, so return
      // the preceding position (we know this exists because of the previous
      // test)
      return --map.lowerBound(pos);


    default: // Nearest
      // Find the first item >= pos; it will be this or the one preceding
      iter = map.upperBound(pos);
      // If upperBound() falls off the end, we want the last valid position
      if (iter == map.end())
      {
        return --iter;
      }
      // If item == pos, return that
      else if (iter == map.begin())
      {
        return iter;
      }
      else
      {
        // Check if the distance to next is less than to previous
        return ((iter.key() - pos) < (pos - (iter - 1).key()))
                 ? iter // Yes; return next
                 : --iter; // No; return previous
      }
  }
}

}

template<typename Value>
class TimeMap : public QMap<kwiver::vital::timestamp::time_t, Value>
{
public:
  typedef typename
    QMap<kwiver::vital::timestamp::time_t, Value>::iterator iterator;
  typedef typename
    QMap<kwiver::vital::timestamp::time_t, Value>::const_iterator
      const_iterator;

  TimeMap()
  {
  }

  TimeMap(TimeMap<Value> const& other)
    : QMap<kwiver::vital::timestamp::time_t, Value>{other}
  {
  }

  TimeMap(std::initializer_list<
    std::pair<kwiver::vital::timestamp::time_t, Value>> list)
    : QMap<kwiver::vital::timestamp::time_t, Value>{list}
  {
  }

  ~TimeMap()
  {
  }

  using QMap<kwiver::vital::timestamp::time_t, Value>::find;
  using QMap<kwiver::vital::timestamp::time_t, Value>::constFind;

  iterator find(kwiver::vital::timestamp::time_t pos, SeekMode direction);
  const_iterator find(kwiver::vital::timestamp::time_t pos,
                      SeekMode direction) const;
  const_iterator constFind(kwiver::vital::timestamp::time_t pos,
                           SeekMode direction) const;

  using QMap<kwiver::vital::timestamp::time_t, Value>::insert;

  void insert(TimeMap<Value> const& other);
};

template<typename Value>
typename TimeMap<Value>::const_iterator TimeMap<Value>::constFind(
  kwiver::vital::timestamp::time_t pos, SeekMode direction) const
{
  return detail::find<TimeMap<Value> const, const_iterator>(*this, pos,
                                                            direction);
}

template<typename Value>
typename TimeMap<Value>::const_iterator TimeMap<Value>::find(
  kwiver::vital::timestamp::time_t pos, SeekMode direction) const
{
  return detail::find<TimeMap<Value> const, const_iterator>(*this, pos,
                                                            direction);
}

template<typename Value>
typename TimeMap<Value>::iterator TimeMap<Value>::find(
  kwiver::vital::timestamp::time_t pos, SeekMode direction)
{
  return detail::find<TimeMap<Value>, iterator>(*this, pos, direction);
}

template<typename Value>
void TimeMap<Value>::insert(TimeMap<Value> const& other)
{
  const_iterator iter, end = other.constEnd();
  for (iter = other.constBegin(); iter != end; ++iter)
  {
    this->insert(iter.key(), iter.value());
  }
}

}

}

#endif
