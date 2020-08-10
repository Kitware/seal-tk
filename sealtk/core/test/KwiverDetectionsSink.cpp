/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>
#include <sealtk/core/test/TestTrackModel.hpp>
#include <sealtk/core/test/TestTracks.hpp>
#include <sealtk/core/test/TestVideoSource.hpp>

#include <sealtk/core/KwiverDetectionsSink.hpp>
#include <sealtk/core/VideoRequest.hpp>

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
class TestKwiverDetectionsSink : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void cleanup();

  void kw18();
  void csv();

private:
  std::unique_ptr<SimpleVideoSource> source;
  std::unique_ptr<SimpleTrackModel> model;
};

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::initTestCase()
{
  loadKwiverPlugins();
}

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::init()
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

  this->model.reset(new SimpleTrackModel{{
    data::track1,
    data::track2,
    data::track3,
    data::track4,
    data::track5,
  }});
}

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::cleanup()
{
  this->source.reset();
  this->model.reset();
}

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::kw18()
{
  KwiverDetectionsSink sink;

  connect(&sink, &AbstractDataSink::failed,
          this, [](QString const& message){
            QFAIL(qPrintable(message));
          });

  QVERIFY(sink.setData(this->source.get(), this->model.get()));

  QTemporaryFile out;
  QVERIFY(out.open());

  auto uri = QUrl::fromLocalFile(out.fileName());
  auto params = QUrlQuery{};

  params.addQueryItem("output:type", "kw18");
  uri.setQuery(params);

  sink.writeData(uri);

  auto const& expected =
    SEALTK_TEST_DATA_PATH("KwiverDetectionsSink/expected.kw18");
  compareFiles(out, expected, QRegularExpression{QStringLiteral("^#")});
}

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::csv()
{
  KwiverDetectionsSink sink;

  connect(&sink, &AbstractDataSink::failed,
          this, [](QString const& message){
            QFAIL(qPrintable(message));
          });

  QVERIFY(sink.setData(this->source.get(), this->model.get()));

  QTemporaryFile out;
  QVERIFY(out.open());

  auto uri = QUrl::fromLocalFile(out.fileName());
  auto params = QUrlQuery{};

  params.addQueryItem("output:type", "csv");
  uri.setQuery(params);

  sink.writeData(uri);

  auto const& expected =
    SEALTK_TEST_DATA_PATH("KwiverDetectionsSink/expected.csv");
  compareFiles(out, expected, QRegularExpression{QStringLiteral("^#")});
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverDetectionsSink)
#include "KwiverDetectionsSink.moc"
