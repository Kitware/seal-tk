/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/iota.h>

#include <QAbstractItemModel>

#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using time_us_t = kv::timestamp::time_t;

namespace sealtk
{

namespace core
{

namespace test
{

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, QModelIndex const& index,
                   TimeMap<QRectF> const& boxes)
{
  QVERIFY(model.data(index, StartTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(index, StartTimeRole).value<time_us_t>(),
           boxes.firstKey());

  QVERIFY(model.data(index, EndTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(index, EndTimeRole).value<time_us_t>(),
           boxes.lastKey());
}

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, kv::track_id_t id,
                   TimeMap<QRectF> const& boxes)
{
  for (auto const row : kvr::iota(model.rowCount()))
  {
    auto const& index = model.index(row, 0);

    QVERIFY(model.data(index, LogicalIdentityRole).canConvert<qint64>());
    if (model.data(index, LogicalIdentityRole).value<qint64>() == id)
    {
      testTrackData(model, index, boxes);
      return;
    }
  }

  QFAIL(qPrintable(QStringLiteral("Did not find id %1 in model").arg(id)));
}

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, int row,
                   kv::track_id_t id, TimeMap<QRectF> const& boxes)
{
  auto const& index = model.index(row, 0);

  QVERIFY(model.data(index, LogicalIdentityRole).canConvert<kv::track_id_t>());
  QCOMPARE(model.data(index, LogicalIdentityRole).value<kv::track_id_t>(), id);

  testTrackData(model, index, boxes);
}

} // namespace test

} // namespace core

} // namespace sealtk
