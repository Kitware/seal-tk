/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/KwiverTrackModel.hpp>
#include <sealtk/core/TrackUtils.hpp>

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
kv::track_sptr createTrack(
  kv::track_id_t id, TimeMap<TrackState> const& states,
  kv::frame_id_t frameOffset = 0)
{
  auto track = kv::track::create();
  track->set_id(id);

  auto frame = frameOffset;

  for (auto const& i : states | kvr::indirect)
  {
    auto const& qbox = i.value().location;
    auto const kbox = kv::bounding_box_d{{qbox.left(), qbox.top()},
                                         qbox.width(), qbox.height()};
    auto const& qdot = i.value().classification;
    auto const& kdot = classificationToDetectedObjectType(qdot);

    auto d = std::make_shared<kv::detected_object>(kbox, 1.0, kdot);
    auto s = std::make_shared<kv::object_track_state>(++frame, i.key(),
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
  void mergeTracks();
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

  testTrackData(model, 1, data::track1);
  testTrackData(model, 2, data::track2);
  testTrackData(model, 3, data::track3);
  testTrackData(model, 4, data::track4);
  testTrackData(model, 5, data::track5);
}

// ----------------------------------------------------------------------------
void TestKwiverTrackModel::mergeTracks()
{
  using Result = KwiverTrackModel::MergeTracksResult;

  KwiverTrackModel model;

  model.addTracks(
    std::make_shared<kv::object_track_set>(
      std::vector<kv::track_sptr>{
        createTrack(1, data::track1, 0),
        createTrack(2, data::track2, 2),
        createTrack(3, data::track3, 6),
        createTrack(4, data::track4, 17),
        createTrack(5, data::track5, 10),
        createTrack(6, data::track1, 0),
      }));
  QCOMPARE(model.rowCount(), 6);

  QCOMPARE(model.mergeTracks({1}), Result::NothingToDo);
  QCOMPARE(model.mergeTracks({7}), Result::NothingToDo);
  QCOMPARE(model.mergeTracks({7, 8}), Result::NothingToDo);
  QCOMPARE(model.mergeTracks({1, 7}), Result::NothingToDo);
  QCOMPARE(model.rowCount(), 6);

  QCOMPARE(model.mergeTracks({1, 6}), Result::OverlappingStates);
  QCOMPARE(model.rowCount(), 6);

  QCOMPARE(model.mergeTracks({2, 3}), Result::Success);
  QCOMPARE(model.rowCount(), 5);

  auto expected = TimeMap<TrackState>{};
  expected.unite(data::track2);
  expected.unite(data::track3);
  testTrackData(model, 2, expected);
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverTrackModel)
#include "KwiverTrackModel.moc"
