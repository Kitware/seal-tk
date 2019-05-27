/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>
#include <sealtk/core/test/TestVideo.hpp>

#include <sealtk/core/ImageUtils.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/VideoController.hpp>

#include <sealtk/util/unique.hpp>

#include <arrows/qt/image_container.h>

#include <vital/algo/video_input.h>
#include <vital/config/config_block.h>

#include <vital/range/iota.h>

#include <qtStlUtil.h>

#include <QImage>

#include <QtTest>

#include <array>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestVideoControllerRequestor : public TestVideoRequestor
{
public:
  void waitForFrame();

protected:
  void update(VideoRequestInfo const& requestInfo,
              VideoFrame&& response) override;

  bool frameReceived = false;
};

// ----------------------------------------------------------------------------
void TestVideoControllerRequestor::waitForFrame()
{
  // If we haven't already received a frame since the last call...
  if (!this->frameReceived)
  {
    // ...then wait for a response now
    this->eventLoop.exec();
  }

  // Reset flag
  this->frameReceived = false;
}

// ----------------------------------------------------------------------------
void TestVideoControllerRequestor::update(
  VideoRequestInfo const& requestInfo, VideoFrame&& response)
{
  // Note that a frame has been received (and next call to waitForFrame should
  // do nothing) and pass along to parent
  this->frameReceived = true;
  TestVideoRequestor::update(requestInfo, std::move(response));
}

// ============================================================================
class TestVideoController : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void seek();
  void removeVideoSource();
  void times();
  void cleanup();

private:
  using VideoRequestorPtr = std::shared_ptr<TestVideoControllerRequestor>;

  kv::config_block_sptr config;
  std::unique_ptr<VideoController> videoController;
  std::shared_ptr<VideoRequestor> videoRequestor;
  std::vector<std::pair<VideoSource*, VideoRequestorPtr>> videoSources;

  void createVideoSource(QString const& path);
  void waitForFrames(VideoSource* excludedSource = nullptr) const;
  void compareFrames(std::array<QVector<QString>, 3> const& seekFiles) const;
};

// ----------------------------------------------------------------------------
void TestVideoController::createVideoSource(QString const& path)
{
  kv::algo::video_input_sptr vi;
  kv::algo::video_input::set_nested_algo_configuration(
    "video_reader", this->config, vi);

  QVERIFY(vi);

  vi->open(stdString(path));
  auto* const vs = new KwiverVideoSource{vi, this->videoController.get()};
  auto requestor = std::make_shared<TestVideoControllerRequestor>();
  this->videoController->addVideoSource(vs, requestor);
  this->videoSources.emplace_back(vs, std::move(requestor));

  vs->start();
}

// ----------------------------------------------------------------------------
void TestVideoController::waitForFrames(VideoSource* excludedSource) const
{
  for (auto& source : this->videoSources)
  {
    if (source.first != excludedSource) // for removeVideoSource test
    {
      source.second->waitForFrame();
    }
  }
}

// ----------------------------------------------------------------------------
void TestVideoController::compareFrames(
  std::array<QVector<QString>, 3> const& seekFiles) const
{
  QCOMPARE(this->videoSources.size(), seekFiles.size());

  for (auto const i : kvr::iota(this->videoSources.size()))
  {
    auto const& requestor = this->videoSources[i].second;
    auto const& seekFrames = requestor->receivedFrames;
    QCOMPARE(seekFrames.size(), seekFiles[i].size());

    for (auto const j : kvr::iota(seekFiles[i].size()))
    {
      auto const& frame = seekFrames[j];

      if (!seekFiles[i][j].isEmpty())
      {
        auto const& expected = QImage{sealtk::test::testDataPath(
          "VideoController/" + seekFiles[i][j])};
        auto const& actual =
          sealtk::core::imageContainerToQImage(frame.image);
        QCOMPARE(actual, expected);

        QFileInfo fiActual{qtString(frame.metaData.imageName())};
        QFileInfo fiExpected{seekFiles[i][j]};
        QCOMPARE(fiActual.fileName(), fiExpected.fileName());
      }
      else
      {
        QCOMPARE(frame.image.get(), nullptr);
        QVERIFY(frame.metaData.imageName().empty());
      }
    }
  }
}

// ----------------------------------------------------------------------------
void TestVideoController::initTestCase()
{
  loadKwiverPlugins();
  this->config = kv::config_block::empty_config();
  this->config->set_value("video_reader:type", "image_list");
  this->config->set_value("video_reader:image_list:image_reader:type",
    "timestamp_passthrough");
  this->config->set_value("video_reader:image_list:image_reader:"
    "timestamp_passthrough:image_reader:type", "qt");
}

// ----------------------------------------------------------------------------
void TestVideoController::init()
{
  this->videoController = make_unique<VideoController>();

  this->createVideoSource(
    SEALTK_TEST_DATA_PATH("VideoController/1/list.txt"));
  this->createVideoSource(
    SEALTK_TEST_DATA_PATH("VideoController/2/list.txt"));
  this->createVideoSource(
    SEALTK_TEST_DATA_PATH("VideoController/3/list.txt"));
}

// ----------------------------------------------------------------------------
void TestVideoController::cleanup()
{
  this->videoSources.clear();
  this->videoController.reset();
}

// ----------------------------------------------------------------------------
void TestVideoController::seek()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    200, 100, 400, 300, 600, 500, 700, 200, 150,
  };

  static std::array<QVector<QString>, 3> const seekFiles{{
    {
      "1/200.png", "1/100.png", "1/400.png", QString{}, "1/600.png", QString{},
      QString{}, "1/200.png", QString{},
    },
    {
      "2/200.png", QString{}, "2/400.png", "2/300.png", QString{}, "2/500.png",
      QString{}, "2/200.png", QString{},
    },
    {
      QString{}, "3/100.png", QString{}, "3/300.png", "3/600.png", "3/500.png",
      QString{}, QString{}, QString{},
    },
  }};

  for (auto t : seekTimes)
  {
    this->videoController->seek(t, 0);
    this->waitForFrames();
  }

  this->compareFrames(seekFiles);
}

// ----------------------------------------------------------------------------
void TestVideoController::removeVideoSource()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    200, 100, 400, 300, 600, 500, 700, 200, 150,
  };

  static std::array<QVector<QString>, 3> const seekFiles{{
    {
      "1/200.png", "1/100.png", "1/400.png", QString{}, "1/600.png", QString{},
      QString{}, "1/200.png", QString{},
    },
    {
      "2/200.png", QString{}, "2/400.png", "2/300.png", QString{}, "2/500.png",
      QString{}, "2/200.png", QString{},
    },
    {
      QString{}, "3/100.png", QString{}, "3/300.png",
    },
  }};

  for (int i = 0; i < 4; i++)
  {
    this->videoController->seek(seekTimes[i], 0);
    this->waitForFrames();
  }

  auto* const testSource = this->videoSources[2].first;
  this->videoController->removeVideoSource(testSource);

  for (int i = 4; i < seekTimes.size(); i++)
  {
    this->videoController->seek(seekTimes[i], 0);
    this->waitForFrames(testSource);
  }

  this->compareFrames(seekFiles);
}

// ----------------------------------------------------------------------------
void TestVideoController::times()
{
  static QSet<kwiver::vital::timestamp::time_t> const times{
    100, 200, 300, 400, 500, 600,
  };

  // Busy-loop until all sources report readiness; this is hardly the most
  // efficient way, but it is the safest
  for (auto& source : this->videoSources)
  {
    while (!source.first->isReady())
    {
      QApplication::processEvents();
    }
  }

  QCOMPARE(this->videoController->times(), times);
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestVideoController)
#include "VideoController.moc"
