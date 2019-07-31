/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <QAbstractItemModel>

#include <QtTest>

namespace kv = kwiver::vital;

using time_us_t = kv::timestamp::time_t;

namespace sealtk
{

namespace core
{

namespace test
{

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, int row,
                   kv::track_id_t id, TimeMap<QRectF> const& boxes)
{
  auto const& index = model.index(row, 0);

  QVERIFY(model.data(index, LogicalIdentityRole).canConvert<kv::track_id_t>());
  QCOMPARE(model.data(index, LogicalIdentityRole).value<kv::track_id_t>(), id);

  QVERIFY(model.data(index, StartTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(model.index(row, 0), StartTimeRole).value<time_us_t>(),
           boxes.firstKey());

  QVERIFY(model.data(index, EndTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(model.index(row, 0), EndTimeRole).value<time_us_t>(),
           boxes.lastKey());
}

} // namespace test

} // namespace core

} // namespace sealtk
