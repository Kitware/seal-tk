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

// ============================================================================
enum SignalType
{
  RowsAboutToBeInserted,
  RowsInserted,
  DataChanged,
};

// ============================================================================
class ModelSignalCounter : public QObject
{
public:
  struct Event
  {
    SignalType type;
    int first;
    int last;
  };

  ModelSignalCounter(QAbstractItemModel* model);

  int count() const { return this->events.count(); }
  void reset() { this->events.clear(); }

  Event const& operator[](int i) const { return this->events[i]; }

private:
  QList<Event> events;
};

// ----------------------------------------------------------------------------
ModelSignalCounter::ModelSignalCounter(QAbstractItemModel* model)
{
  connect(
    model, &QAbstractItemModel::rowsAboutToBeInserted, this,
    [this](QModelIndex const&, int first, int last){
      this->events.append({RowsAboutToBeInserted, first, last});
    });
  connect(
    model, &QAbstractItemModel::rowsInserted, this,
    [this](QModelIndex const&, int first, int last){
      this->events.append({RowsInserted, first, last});
    });
  connect(
    model, &QAbstractItemModel::dataChanged, this,
    [this](QModelIndex const& first, QModelIndex const& last){
      this->events.append({DataChanged, first.row(), last.row()});
    });
}

// ----------------------------------------------------------------------------
kv::track_state_sptr createState(
  kv::frame_id_t frame, kv::timestamp::time_t time, TrackState const& state)
{
  auto detection = createDetection(state.location, state.classification);
  return createTrackState(frame, time, std::move(detection));
}

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
    track->append(createState(++frame, i.key(), i.value()));
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
  void updateTrack();
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

// ----------------------------------------------------------------------------
void TestKwiverTrackModel::updateTrack()
{
  KwiverTrackModel model;
  auto expected = data::track2;

  model.addTracks(
    std::make_shared<kv::object_track_set>(
      std::vector<kv::track_sptr>{createTrack(2, expected, 2)}));

  QCOMPARE(model.rowCount(), 1);
  testTrackData(model, 2, expected);

  // Set up signal tested
  ModelSignalCounter counter{&model};

  // Get index of track
  auto const& index = model.index(0, 0);

  // Test insertion at front
  auto const frontFrame = 0;
  auto const frontTime = 100;
  auto const frontState = TrackState{{0, 0, 10, 10}, {{"Dab", 0.2}}};

  model.updateTrack(index, createState(frontFrame, frontTime, frontState));
  expected.insert(frontTime, frontState);
  testTrackData(model, 2, expected);

  QCOMPARE(counter.count(), 2);
  QCOMPARE(counter[0].type, RowsAboutToBeInserted);
  QCOMPARE(counter[0].first, 0);
  QCOMPARE(counter[0].last, 0);
  QCOMPARE(counter[1].type, RowsInserted);
  QCOMPARE(counter[1].first, 0);
  QCOMPARE(counter[1].last, 0);
  counter.reset();

  // Test insertion at back
  auto const backFrame = 7;
  auto const backTime = 700;
  auto const backState = TrackState{{160, 190, 50, 70}, {{"Cod", 0.7}}};

  model.updateTrack(index, createState(backFrame, backTime, backState));
  expected.insert(backTime, backState);
  testTrackData(model, 2, expected);

  QCOMPARE(counter.count(), 2);
  QCOMPARE(counter[0].type, RowsAboutToBeInserted);
  QCOMPARE(counter[0].first, 3);
  QCOMPARE(counter[0].last, 3);
  QCOMPARE(counter[1].type, RowsInserted);
  QCOMPARE(counter[1].first, 3);
  QCOMPARE(counter[1].last, 3);
  counter.reset();

  // Test insertion in middle
  auto const midFrame = 5;
  auto const midTime = 500;
  auto const midState = TrackState{{120, 170, 45, 75}, {{"Cod", 0.6}}};

  model.updateTrack(index, createState(midFrame, midTime, midState));
  expected.insert(midTime, midState);
  testTrackData(model, 2, expected);

  QCOMPARE(counter.count(), 2);
  QCOMPARE(counter[0].type, RowsAboutToBeInserted);
  QCOMPARE(counter[0].first, 3);
  QCOMPARE(counter[0].last, 3);
  QCOMPARE(counter[1].type, RowsInserted);
  QCOMPARE(counter[1].first, 3);
  QCOMPARE(counter[1].last, 3);
  counter.reset();

  // Test modification of an existing state
  auto const alterFrame = 5;
  auto const alterTime = 500;
  auto const alterState = TrackState{{410, 150, 25, 35}, {{"Gar", 0.7}}};

  model.updateTrack(index, createState(alterFrame, alterTime, alterState));
  expected.insert(alterTime, alterState);
  testTrackData(model, 2, expected);

  QCOMPARE(counter.count(), 1);
  QCOMPARE(counter[0].type, DataChanged);
  QCOMPARE(counter[0].first, 3);
  QCOMPARE(counter[0].last, 3);
  counter.reset();
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverTrackModel)
#include "KwiverTrackModel.moc"
