/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>
#include <sealtk/core/test/TestTracks.hpp>

#include <sealtk/core/AbstractItemModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/KwiverDetectionsSink.hpp>
#include <sealtk/core/VideoMetaData.hpp>
#include <sealtk/core/VideoProvider.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <vital/range/indirect.h>

#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUrlQuery>

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

// ============================================================================
class TestSource : public core::VideoSource
{
public:
  TestSource(TimeMap<VideoMetaData> const& data)
    : TestSource{new Provider, data} {}

  bool isReady() const override { return true; }
  TimeMap<VideoMetaData> metaData() const override { return this->data; }
  TimeMap<kv::timestamp::frame_t> frames() const override
  {
    TimeMap<kv::timestamp::frame_t> result;
    for (auto const& i : this->data | kvr::indirect)
    {
      auto const& ts = i.value().timeStamp();
      if (ts.has_valid_frame())
      {
        result.insert(i.key(), ts.get_frame());
      }
    }

    return result;
  }

private:
  class Provider : public core::VideoProvider
  {
  public:
    void initialize() override {}

    kv::timestamp processRequest(
      core::VideoRequest&&, kv::timestamp const&) override
    { return {}; }
  };

  TestSource(Provider* provider, TimeMap<VideoMetaData> const& data)
    : VideoSource{provider}, provider{provider}, data{data} {}

  std::unique_ptr<Provider> provider;
  TimeMap<VideoMetaData> const data;
};

// ============================================================================
class TestModel : public core::AbstractItemModel
{
public:
  TestModel(QVector<TimeMap<QRectF>> data) : rowData{data} {}

  int rowCount(QModelIndex const& parent = {}) const override;

  QVariant data(QModelIndex const& index, int role) const override;
  QModelIndex parent(QModelIndex const& index) const override;
  QModelIndex index(int row, int column,
                    QModelIndex const& parent) const override;

private:
  QVector<TimeMap<QRectF>> const rowData;
};

// ----------------------------------------------------------------------------
int TestModel::rowCount(QModelIndex const& parent) const
{
  if (parent.isValid())
  {
    auto const pr = parent.row();
    if (pr >= 0 && pr < this->rowData.count())
    {
      return this->rowData[pr].count();
    }
    return 0;
  }
  else
  {
    return this->rowData.count();
  }
}

// ----------------------------------------------------------------------------
QModelIndex TestModel::index(
  int row, int column, QModelIndex const& parent) const
{
  auto const pr =
    static_cast<unsigned>(parent.isValid() ? parent.row() + 1 : 0);

  return this->createIndex(row, column, pr);
}

// ----------------------------------------------------------------------------
QModelIndex TestModel::parent(QModelIndex const& index) const
{
  auto const parentRow = static_cast<int>(index.internalId()) - 1;
  return (parentRow >= 0 ? this->createIndex(parentRow, 0) : QModelIndex{});
}

// ----------------------------------------------------------------------------
QVariant TestModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid))
  {
    auto const& p = index.parent();
    if (p.isValid())
    {
      auto const pr = p.row();
      auto const& track = this->rowData[pr];

      switch (role)
      {
        case StartTimeRole:
        case EndTimeRole:
          return QVariant::fromValue(track.keys()[index.row()]);

        case AreaLocationRole:
          return QVariant::fromValue(track.values()[index.row()]);

        default:
          break;
      }
    }
  }

  return AbstractItemModel::data(index, role);
}

} // namespace <anonymous>

// ============================================================================
class TestKwiverDetectionsSink : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void write();
};

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::initTestCase()
{
  loadKwiverPlugins();
}

// ----------------------------------------------------------------------------
void TestKwiverDetectionsSink::write()
{
  using vmd = core::VideoMetaData;

  TestSource source{{
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
  }};

  TestModel model{{
    data::track1,
    data::track2,
    data::track3,
    data::track4,
    data::track5,
  }};

  KwiverDetectionsSink sink;

  connect(&sink, &AbstractDataSink::failed,
          this, [](QString const& message){
            QFAIL(qPrintable(message));
          });

  QVERIFY(sink.setData(&source, &model));

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

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverDetectionsSink)
#include "KwiverDetectionsSink.moc"
