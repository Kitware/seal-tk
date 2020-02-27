/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>
#include <sealtk/core/test/TestTrackModel.hpp>
#include <sealtk/core/test/TestTracks.hpp>
#include <sealtk/core/test/TestVideoSource.hpp>

#include <sealtk/core/IdentityTransform.hpp>
#include <sealtk/core/KwiverTracksSink.hpp>
#include <sealtk/core/VideoRequest.hpp>

#include <vital/types/homography.h>

#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUrlQuery>

#include <QtTest>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
void compareFiles(QIODevice& actual, QString const& expected,
                  QRegularExpression const& ignoreLines)
{
  QFile ef{expected};
  QVERIFY(ef.open(QIODevice::ReadOnly));

  actual.seek(0);
  while (!actual.atEnd())
  {
    auto const& al = actual.readLine();
    if (ignoreLines.match(al).hasMatch())
    {
      continue;
    }

    QVERIFY(!ef.atEnd());
    QCOMPARE(al, ef.readLine());
  }

  QVERIFY(ef.atEnd());
}

} // namespace <anonymous>

// ============================================================================
class TestKwiverTracksSink : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void cleanup();

  void kw18();

private:
  std::unique_ptr<SimpleVideoSource> source;
  std::unique_ptr<SimpleTrackModel> primaryModel;
  std::unique_ptr<SimpleTrackModel> shadowModel;
  kv::transform_2d_sptr primaryTransform;
  kv::transform_2d_sptr shadowTransform;
};

// ----------------------------------------------------------------------------
void TestKwiverTracksSink::initTestCase()
{
  loadKwiverPlugins();
}

// ----------------------------------------------------------------------------
void TestKwiverTracksSink::init()
{
  using vmd = core::VideoMetaData;

  this->source.reset(new SimpleVideoSource{{
    {100, vmd{kv::timestamp{100, 1}, "frame0001.png"}},
    {300, vmd{kv::timestamp{300, 3}, "frame0003.png"}},
    {400, vmd{kv::timestamp{400, 4}, "frame0004.png"}},
    {700, vmd{kv::timestamp{700, 7}, "frame0007.png"}},
    {1100, vmd{kv::timestamp{1100, 11}, "frame0011.png"}},
    {1300, vmd{kv::timestamp{1300, 13}, "frame0013.png"}},
    {1800, vmd{kv::timestamp{1800, 18}, "frame0018.png"}},
    {1900, vmd{kv::timestamp{1900, 19}, "frame0019.png"}},
    {2000, vmd{kv::timestamp{2000, 20}, "frame0020.png"}},
    {2200, vmd{kv::timestamp{2200, 22}, "frame0022.png"}},
  }});

  this->primaryModel.reset(new SimpleTrackModel{{
    data::track1,
    data::track2,
    data::track3,
  }});
  this->primaryModel->setFirstId(1);

  this->shadowModel.reset(new SimpleTrackModel{{
    data::track4,
    data::track5,
  }});
  this->shadowModel->setFirstId(4);

  auto primaryMatrix = Eigen::Matrix3d{};
  primaryMatrix << 2.0, 0.0, 0.0,
                   0.0, 2.0, 0.0,
                   0.0, 0.0, 1.0;
  this->primaryTransform =
    std::make_shared<kv::homography_<double>>(primaryMatrix);

  auto shadowMatrix = Eigen::Matrix3d{};
  shadowMatrix << +4.0, -2.0, 800.0,
                  +2.0, +4.0, 200.0,
                  0.0, 0.0, 1.0;
  this->shadowTransform =
    std::make_shared<kv::homography_<double>>(shadowMatrix);
}

// ----------------------------------------------------------------------------
void TestKwiverTracksSink::cleanup()
{
  this->source.reset();
  this->primaryModel.reset();
  this->shadowModel.reset();
  this->shadowTransform.reset();
}

// ----------------------------------------------------------------------------
void TestKwiverTracksSink::kw18()
{
  KwiverTracksSink sink;

  connect(&sink, &AbstractDataSink::failed,
          this, [](QString const& message){
            QFAIL(qPrintable(message));
          });

  QVERIFY(sink.setData(this->source.get(), this->primaryModel.get()));
  QVERIFY(sink.setTransform(this->primaryTransform));
  QVERIFY(sink.addData(this->shadowModel.get(), this->shadowTransform));

  QTemporaryFile out;
  QVERIFY(out.open());

  auto uri = QUrl::fromLocalFile(out.fileName());
  auto params = QUrlQuery{};

  params.addQueryItem("output:type", "kw18");
  uri.setQuery(params);

  sink.writeData(uri);

  auto const& expected =
    SEALTK_TEST_DATA_PATH("KwiverTracksSink/expected.kw18");
  compareFiles(out, expected, QRegularExpression{QStringLiteral("^#")});
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverTracksSink)
#include "KwiverTracksSink.moc"
