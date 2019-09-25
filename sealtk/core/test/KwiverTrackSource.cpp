/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>
#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/KwiverTrackSource.hpp>

#include <vital/range/indirect.h>

#include <QEventLoop>
#include <QUrlQuery>

#include <QtTest>

namespace kvr = kwiver::vital::range;

using ModelPointer = std::shared_ptr<QAbstractItemModel>;

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
TimeMap<TrackState> stripClassification(TimeMap<TrackState> const& in)
{
  auto out = TimeMap<TrackState>{};

  for (auto const& i : in | kvr::indirect)
  {
    auto s = i.value();
    s.classification.clear();
    out.insert(i.key(), std::move(s));
  }

  return out;
}

} // namespace <anonymous>

// ============================================================================
class TestKwiverTrackSource : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void loadTracks();
};

// ----------------------------------------------------------------------------
void TestKwiverTrackSource::initTestCase()
{
  qRegisterMetaType<ModelPointer>();

  loadKwiverPlugins();
}

// ----------------------------------------------------------------------------
void TestKwiverTrackSource::loadTracks()
{
  KwiverTrackSource source;
  ModelPointer model;
  QEventLoop loop;

  connect(&source, &AbstractDataSource::modelReady, this,
          [&](ModelPointer const& sourceModel){
            model = sourceModel;
            loop.quit();
          });
  connect(&source, &AbstractDataSource::failed, this,
          [&](){ loop.quit(); });

  auto const filename =
    SEALTK_TEST_DATA_PATH("KwiverTrackSource/test.kw18");
  auto uri = QUrl::fromLocalFile(filename);
  auto params = QUrlQuery{};

  params.addQueryItem("input:type", "kw18");
  uri.setQuery(params);

  source.readData(uri);
  loop.exec();

  QVERIFY(model);
  QCOMPARE(model->rowCount(), 5);

  testTrackData(*model, 1, stripClassification(data::track1));
  testTrackData(*model, 2, stripClassification(data::track2));
  testTrackData(*model, 3, stripClassification(data::track3));
  testTrackData(*model, 4, stripClassification(data::track4));
  testTrackData(*model, 5, stripClassification(data::track5));
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverTrackSource)
#include "KwiverTrackSource.moc"
