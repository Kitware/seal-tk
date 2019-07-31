/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/KwiverTrackModel.hpp>

#include <vital/types/object_track_set.h>

#include <vital/range/indirect.h>

#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
kv::track_sptr createTrack(kv::track_id_t id, TimeMap<QRectF> const& boxes)
{
  auto track = kv::track::create();
  track->set_id(id);

  auto frame = kv::frame_id_t{0};

  for (auto const& box : boxes | kvr::indirect)
  {
    auto const& qbox = box.value();
    auto const kbox = kv::bounding_box_d{{qbox.left(), qbox.top()},
                                         qbox.width(), qbox.height()};

    auto d = std::make_shared<kv::detected_object>(kbox);
    auto s = std::make_shared<kv::object_track_state>(++frame, box.key(),
                                                      std::move(d));

    track->append(std::move(s));
  }

  return track;
}

} // namespace <anonymous>

// ============================================================================
class TestKwiverTrackModel : public QObject
{
  Q_OBJECT

private slots:
  void operations();
  void data();
};

// ----------------------------------------------------------------------------
void TestKwiverTrackModel::operations()
{
  KwiverTrackModel model;

  QCOMPARE(model.rowCount(), 0);

  auto const ts1 = std::make_shared<kv::object_track_set>(
    std::vector<kv::track_sptr>{
      createTrack(1, data::track1),
    });

  model.addTracks(ts1);
  QCOMPARE(model.rowCount(), 1);

  auto const ts2 = std::make_shared<kv::object_track_set>(
    std::vector<kv::track_sptr>{
      createTrack(2, data::track2),
      createTrack(3, data::track3),
    });

  model.addTracks(ts2);
  QCOMPARE(model.rowCount(), 3);

  auto const ts3 = std::make_shared<kv::object_track_set>(
    std::vector<kv::track_sptr>{
      createTrack(4, data::track4),
      createTrack(5, data::track5),
    });

  model.setTracks(ts3);
  QCOMPARE(model.rowCount(), 2);

  model.clear();
  QCOMPARE(model.rowCount(), 0);
}

// ----------------------------------------------------------------------------
void TestKwiverTrackModel::data()
{
  KwiverTrackModel model;
  model.setTracks(
    std::make_shared<kv::object_track_set>(
      std::vector<kv::track_sptr>{
        createTrack(1, data::track1),
        createTrack(2, data::track2),
        createTrack(3, data::track3),
        createTrack(4, data::track4),
        createTrack(5, data::track5),
      }));
  QCOMPARE(model.rowCount(), 5);

  testTrackData(model, 0, 1, data::track1);
  testTrackData(model, 1, 2, data::track2);
  testTrackData(model, 2, 3, data::track3);
  testTrackData(model, 3, 4, data::track4);
  testTrackData(model, 4, 5, data::track5);
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverTrackModel)
#include "KwiverTrackModel.moc"
