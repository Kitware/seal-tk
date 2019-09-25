/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_test_TestTracks_hpp
#define sealtk_core_test_TestTracks_hpp

#include <sealtk/core/TimeMap.hpp>

#include <vital/types/track.h>

#include <QRectF>
#include <QVariantHash>

class QAbstractItemModel;
class QModelIndex;

namespace sealtk
{

namespace core
{

namespace test
{

struct TrackState
{
  QRectF location;
  QVariantHash classification;
};

void testTrackData(
  QAbstractItemModel const& model,
  kwiver::vital::track_id_t id,
  TimeMap<TrackState> const& states);

void testTrackData(
  QAbstractItemModel const& model,
  QModelIndex const& parent,
  kwiver::vital::timestamp::time_t time,
  TrackState const& state);

namespace data
{

auto const track1 = TimeMap<TrackState>{
  {100, {{0, 0, 10, 10}, {{"Dab", 0.2}}}},
};

auto const track2 = TimeMap<TrackState>{
  {300, {{420, 130, 30, 20}, {{"Eel", 0.3}, {"Gar", 0.4}}}},
  {400, {{440, 160, 40, 50}, {{"Eel", 0.2}, {"Gar", 0.6}}}},
};

auto const track3 = TimeMap<TrackState>{
  {700, {{160, 190, 50, 70}, {{"Cod", 0.7}}}},
};

auto const track4 = TimeMap<TrackState>{
  {1800, {{460, 410, 60, 30}, {}}},
  {1900, {{450, 380, 65, 35}, {}}},
  {2000, {{480, 370, 55, 35}, {}}},
  {2200, {{440, 390, 50, 30}, {}}},
};

auto const track5 = TimeMap<TrackState>{
  {1100, {{120, 560, 50, 20}, {{"Koi", 0.2}, {"Sar", 0.3}}}},
  {1300, {{140, 570, 40, 30}, {{"Koi", 0.9}, {"Sar", 0.1}}}},
};

} // namespace data

} // namespace test

} // namespace core

} // namespace sealtk

#endif
