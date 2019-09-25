/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/indirect.h>
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
void testTrackData(QAbstractItemModel const& model,
                   QModelIndex const& index, TrackState const& state)
{
  QVERIFY(model.data(index, AreaLocationRole).canConvert<QRectF>());
  QCOMPARE(model.data(index, AreaLocationRole).toRectF(),
           state.location);

  auto const modelHasClassification =
    model.data(index, ClassificationRole).isValid();
  if (!modelHasClassification)
  {
    QVERIFY(state.classification.isEmpty());
  }
  else
  {
    QVERIFY(model.data(index, ClassificationRole).canConvert<QVariantHash>());
    QCOMPARE(model.data(index, ClassificationRole).toHash(),
             state.classification);
  }
}

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, QModelIndex const& parent,
                   time_us_t time, TrackState const& state)
{
  for (auto const row : kvr::iota(model.rowCount(parent)))
  {
    auto const& index = model.index(row, 0, parent);

    QVERIFY(model.data(index, StartTimeRole).canConvert<time_us_t>());
    if (model.data(index, StartTimeRole).value<time_us_t>() == time)
    {
      QVERIFY(model.data(index, EndTimeRole).canConvert<time_us_t>());
      QCOMPARE(model.data(index, EndTimeRole).value<time_us_t>(), time);

      testTrackData(model, index, state);
      return;
    }
  }

  static auto const failTemplate =
    QStringLiteral("Did not find state with time %1 in model");
  QFAIL(qPrintable(failTemplate.arg(time)));
}

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, QModelIndex const& index,
                   TimeMap<TrackState> const& states)
{
  QVERIFY(model.data(index, StartTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(index, StartTimeRole).value<time_us_t>(),
           states.firstKey());

  QVERIFY(model.data(index, EndTimeRole).canConvert<time_us_t>());
  QCOMPARE(model.data(index, EndTimeRole).value<time_us_t>(),
           states.lastKey());

  QCOMPARE(model.rowCount(index), states.count());
  for (auto const& iter : states | kvr::indirect)
  {
    testTrackData(model, index, iter.key(), iter.value());
  }
}

// ----------------------------------------------------------------------------
void testTrackData(QAbstractItemModel const& model, kv::track_id_t id,
                   TimeMap<TrackState> const& states)
{
  for (auto const row : kvr::iota(model.rowCount()))
  {
    auto const& index = model.index(row, 0);

    QVERIFY(model.data(index, LogicalIdentityRole).canConvert<qint64>());
    if (model.data(index, LogicalIdentityRole).value<qint64>() == id)
    {
      testTrackData(model, index, states);
      return;
    }
  }

  static auto const failTemplate =
    QStringLiteral("Did not find id %1 in model");
  QFAIL(qPrintable(failTemplate.arg(id)));
}

} // namespace test

} // namespace core

} // namespace sealtk
