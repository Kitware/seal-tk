/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_test_TestTracks_hpp
#define sealtk_core_test_TestTracks_hpp

#include <sealtk/core/TimeMap.hpp>

#include <vital/types/track.h>

#include <QRectF>

class QAbstractItemModel;
class QModelIndex;

namespace sealtk
{

namespace core
{

namespace test
{

void testTrackData(
  QAbstractItemModel const& model,
  kwiver::vital::track_id_t id,
  TimeMap<QRectF> const& boxes);

void testTrackData(
  QAbstractItemModel const& model,
  QModelIndex const& parent,
  kwiver::vital::timestamp::time_t time,
  QRectF const& box);

namespace data
{

auto const track1 = TimeMap<QRectF>{
  {100, {0, 0, 10, 10}},
};

auto const track2 = TimeMap<QRectF>{
  {300, {420, 130, 30, 20}},
  {400, {440, 160, 40, 50}},
};

auto const track3 = TimeMap<QRectF>{
  {700, {160, 190, 50, 70}},
};

auto const track4 = TimeMap<QRectF>{
  {1800, {460, 410, 60, 30}},
  {1900, {450, 380, 65, 35}},
  {2000, {480, 370, 55, 35}},
  {2200, {440, 390, 50, 30}},
};

auto const track5 = TimeMap<QRectF>{
  {1100, {120, 560, 50, 20}},
  {1300, {140, 570, 40, 30}},
};

} // namespace data

} // namespace test

} // namespace core

} // namespace sealtk

#endif
