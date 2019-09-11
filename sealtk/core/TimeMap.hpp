/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_TimeMap_hpp
#define sealtk_core_TimeMap_hpp

#include <sealtk/core/Export.h>

#include <vital/types/timestamp.h>

#include <qtEnumerate.h>
#include <qtGlobal.h>

#include <QMap>
#include <QMetaObject>
#include <QMetaType>
#include <QSet>

namespace sealtk
{

namespace core
{

QTE_BEGIN_META_NAMESPACE(SEALTK_CORE_EXPORT, time_map_enums)

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
  SeekPrevious,
};

QTE_ENUM_NS(SeekMode)

QTE_END_META_NAMESPACE()

// ============================================================================
template <typename Value>
class TimeMap : public QMap<kwiver::vital::timestamp::time_t, Value>
{
  using Key = kwiver::vital::timestamp::time_t;

public:
  using typename QMap<Key, Value>::iterator;
  using typename QMap<Key, Value>::const_iterator;

  TimeMap() = default;

  TimeMap(TimeMap&&) = default;
  TimeMap(TimeMap const&) = default;

  using QMap<Key, Value>::QMap;

  ~TimeMap() = default;

  TimeMap& operator=(TimeMap&&) = default;
  TimeMap& operator=(TimeMap const&) = default;

  QSet<Key> keySet() const;
  TimeMap<std::nullptr_t> keyMap() const;

  using QMap<Key, Value>::find;
  using QMap<Key, Value>::constFind;

  iterator find(Key pos, SeekMode direction);
  const_iterator find(Key pos, SeekMode direction) const;
  const_iterator constFind(Key pos, SeekMode direction) const;

  using QMap<Key, Value>::insert;

  void insert(TimeMap<Value> const& other);

private:
  template <typename Iterator, typename Map>
  static Iterator find(Map& map, Key pos, SeekMode direction);
};

// ----------------------------------------------------------------------------
template <typename Value>
QSet<kwiver::vital::timestamp::time_t> TimeMap<Value>::keySet() const
{
  auto out = QSet<Key>{};
  out.reserve(this->size());

  for (auto const& item : qtEnumerate(*this))
  {
    out.insert(item.key());
  }

  return out;
}

// ----------------------------------------------------------------------------
template <typename Value>
TimeMap<std::nullptr_t> TimeMap<Value>::keyMap() const
{
  auto out = TimeMap<std::nullptr_t>{};

  for (auto const& item : qtEnumerate(*this))
  {
    out.insert(item.key(), nullptr);
  }

  return out;
}

// ----------------------------------------------------------------------------
template <typename Value>
typename TimeMap<Value>::iterator TimeMap<Value>::find(
  Key pos, SeekMode direction)
{
  return TimeMap::find<iterator>(*this, pos, direction);
}

// ----------------------------------------------------------------------------
template <typename Value>
typename TimeMap<Value>::const_iterator TimeMap<Value>::find(
  Key pos, SeekMode direction) const
{
  return TimeMap::find<const_iterator>(*this, pos, direction);
}

// ----------------------------------------------------------------------------
template <typename Value>
typename TimeMap<Value>::const_iterator TimeMap<Value>::constFind(
  Key pos, SeekMode direction) const
{
  return TimeMap::find<const_iterator>(*this, pos, direction);
}

// ----------------------------------------------------------------------------
template <typename Value>
template <typename Iterator, typename Map>
Iterator TimeMap<Value>::find(Map& map, Key pos, SeekMode direction)
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

// ----------------------------------------------------------------------------
template <typename Value>
void TimeMap<Value>::insert(TimeMap<Value> const& other)
{
  const_iterator iter, end = other.constEnd();
  for (iter = other.constBegin(); iter != end; ++iter)
  {
    this->insert(iter.key(), iter.value());
  }
}

} // namespace core

} // namespace sealtk

Q_DECLARE_METATYPE(sealtk::core::SeekMode)

#endif
