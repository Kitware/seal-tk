/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>

#include <sealtk/core/ImageUtils.hpp>
#include <sealtk/core/KwiverPipelineWorker.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>

#include <sealtk/util/unique.hpp>

#include <sprokit/processes/adapters/adapter_data_set.h>

#include <vital/algo/video_input.h>
#include <vital/config/config_block.h>

#include <vital/range/iota.h>

#include <qtGet.h>
#include <qtStlUtil.h>

#include <QImage>

#include <QtTest>

namespace ka = kwiver::adapter;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace test
{

using SourceVector = std::vector<std::unique_ptr<core::KwiverVideoSource>>;
using SourceIndex = SourceVector::size_type;

// ============================================================================
class TestPipelineWorker : public KwiverPipelineWorker
{
public:
  TestPipelineWorker() : KwiverPipelineWorker{RequiresInputAndOutput} {}

  QList<QHash<int, QString>> outputNames;
  QList<QHash<int, QImage>> outputImages;

protected:
  void processOutput(ka::adapter_data_set_t const& output) override;
};

// ----------------------------------------------------------------------------
void TestPipelineWorker::processOutput(ka::adapter_data_set_t const& output)
{
  static auto const portRegExp =
    QRegularExpression{QStringLiteral("^(image|name)_(\\d+)$")};

  if (output)
  {
    auto newNames = QHash<int, QString>{};
    auto newImages = QHash<int, QImage>{};

    for (auto const& datum : *output)
    {
      auto const& portName = qtString(datum.first);
      if (!datum.second)
      {
        qWarning() << "Unexpectedly empty datum" << portName;
        continue;
      }

      auto const& p = portRegExp.match(portName);
      if (p.isValid())
      {
        auto const n = p.captured(2).toInt();
        if (p.captured(1) == QStringLiteral("image"))
        {
          auto const& image =
            datum.second->get_datum<kv::image_container_sptr>();
          if (image)
          {
            newImages.insert(n, imageContainerToQImage(image));
          }
        }
        else if (p.captured(1) == QStringLiteral("name"))
        {
          newNames.insert(n, qtString(datum.second->get_datum<std::string>()));
        }
      }
    }

    this->outputNames.append(newNames);
    this->outputImages.append(newImages);
  }
}

// ============================================================================
class TestKwiverPipelineWorker : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void pipeline();
  void pipeline_data();

private:
  SourceVector videoSources;
};

// ----------------------------------------------------------------------------
void TestKwiverPipelineWorker::initTestCase()
{
  kv::config_block_sptr config;

  loadKwiverPlugins();
  config = kv::config_block::empty_config();
  config->set_value("video_reader:type", "image_list");
  config->set_value("video_reader:image_list:image_reader:type",
                    "timestamp_passthrough");
  config->set_value("video_reader:image_list:image_reader:"
                    "timestamp_passthrough:image_reader:type", "qt");

  for (auto const n : { 1, 2, 3 })
  {
    auto const& input =
      SEALTK_TEST_DATA_PATH("KwiverPipelineWorker/list%1.txt").arg(n);

    kv::algo::video_input_sptr videoReader;
    kv::algo::video_input::set_nested_algo_configuration(
      "video_reader", config, videoReader);
    videoReader->open(stdString(input));

    auto source = make_unique<core::KwiverVideoSource>(videoReader);
    this->videoSources.emplace_back(std::move(source));
  }
}

// ----------------------------------------------------------------------------
void TestKwiverPipelineWorker::cleanupTestCase()
{
  this->videoSources.clear();
}

// ----------------------------------------------------------------------------
void TestKwiverPipelineWorker::pipeline()
{
  QFETCH(QString, pipeline);
  QFETCH(QVector<int>, sourceIndices);
  QFETCH(QVector<QStringList>, expectedFrames);
  QFETCH(bool, testProgress);

  TestPipelineWorker worker;
  for (auto const si : sourceIndices)
  {
    if (si >= 0)
    {
      auto const& source =
        this->videoSources[static_cast<SourceIndex>(si)];
      worker.addVideoSource(source.get());
    }
    else
    {
      worker.addVideoSource(nullptr);
    }
  }

  auto progressMax = int{-1};
  auto progressReports = QVector<int>{};
  auto totalExpectedFrames = int{0};

  connect(&worker, &KwiverPipelineWorker::progressRangeChanged,
          this, [&progressMax](int min, int max){
            QVERIFY(min == 0);
            progressMax = max;
          });
  connect(&worker, &KwiverPipelineWorker::progressValueChanged,
          this, [&progressReports](int value){
            progressReports.append(value);
          });

  QVERIFY(worker.initialize(pipeline));
  worker.execute();

  QCOMPARE(progressReports.count(), expectedFrames.count());
  QCOMPARE(worker.outputNames.count(), expectedFrames.count());
  QCOMPARE(worker.outputImages.count(), expectedFrames.count());
  for (auto const i : kvr::iota(expectedFrames.count()))
  {
    auto const& an = worker.outputNames[i];
    auto const& af = worker.outputImages[i];
    auto const& en = expectedFrames[i];

    for (auto const j : kvr::iota(en.count()))
    {
      auto const& expectedName = en[j];
      auto const& actualName = an.value(j);
      auto const& actualImage = af.value(j);

      if (expectedName.isEmpty())
      {
        QVERIFY(actualName.isEmpty());
        QVERIFY(actualImage.isNull());
      }
      else
      {
        auto const& expectedImage = QImage{
          sealtk::test::testDataPath("KwiverPipelineWorker/" + expectedName)};

        QCOMPARE(QFileInfo{actualName}.fileName(), expectedName);
        QCOMPARE(actualImage, expectedImage);

        ++totalExpectedFrames;
      }
    }

    if (testProgress)
    {
      QCOMPARE(progressReports[i], totalExpectedFrames);
    }
  }

  if (testProgress)
  {
    QCOMPARE(progressMax, totalExpectedFrames);
  }
}

// ----------------------------------------------------------------------------
void TestKwiverPipelineWorker::pipeline_data()
{
  QTest::addColumn<QString>("pipeline");
  QTest::addColumn<bool>("testProgress");
  QTest::addColumn<QVector<int>>("sourceIndices");
  QTest::addColumn<QVector<QStringList>>("expectedFrames");

  QTest::newRow("matching sources")
    << SEALTK_TEST_DATA_PATH("KwiverPipelineWorker/matching.pipe")
    << true << QVector<int>{0, 1, 2}
    << QVector<QStringList>{
         {"1000.png", "1000.png", QString{}},
         {QString{},  "2000.png", "2000.png"},
         {"3000.png", QString{},  "3000.png"},
         {"4000.png", "4000.png", "4000.png"},
         {QString{},  QString{},  "5000.png"},
       };

  QTest::newRow("missing source")
    << SEALTK_TEST_DATA_PATH("KwiverPipelineWorker/missing.pipe")
    << true << QVector<int>{0, -1, 2}
    << QVector<QStringList>{
         {"1000.png", QString{}, QString{}},
         {QString{},  QString{}, "2000.png"},
         {"3000.png", QString{}, "3000.png"},
         {"4000.png", QString{}, "4000.png"},
         {QString{},  QString{}, "5000.png"},
       };

  QTest::newRow("excess sources")
    << SEALTK_TEST_DATA_PATH("KwiverPipelineWorker/excess.pipe")
    << false << QVector<int>{0, 1, 2}
    << QVector<QStringList>{
         {"1000.png"},
         {"2000.png"},
         {"4000.png"},
       };
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverPipelineWorker)
#include "KwiverPipelineWorker.moc"
