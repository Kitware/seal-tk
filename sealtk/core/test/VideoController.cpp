/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>

#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/VideoController.hpp>

#include <vital/algo/video_input.h>
#include <vital/config/config_block.h>
#include <vital/types/timestamp.h>

#include <QImage>
#include <QVector>

#include <QtTest>

#include <array>
#include <memory>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

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
  kv::config_block_sptr config;
  std::unique_ptr<VideoController> videoController;
  QVector<KwiverVideoSource*> videoSources;

  kv::algo::video_input_sptr generateVideoInput(QString const& path);
};

// ----------------------------------------------------------------------------
kv::algo::video_input_sptr TestVideoController::generateVideoInput(
  QString const& path)
{
  kv::algo::video_input_sptr vi;
  kv::algo::video_input::set_nested_algo_configuration(
    "video_reader", this->config, vi);
  vi->open(path.toStdString());
  return vi;
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
  this->videoController = std::make_unique<VideoController>();

  KwiverVideoSource* vs;

  vs = new KwiverVideoSource{this->videoController.get()};
  vs->setVideoInput(this->generateVideoInput(SEALTK_TEST_DATA_PATH(
    "VideoController/1/list.txt")));
  this->videoController->addVideoSource(vs);
  this->videoSources.append(vs);

  vs = new KwiverVideoSource{this->videoController.get()};
  vs->setVideoInput(this->generateVideoInput(SEALTK_TEST_DATA_PATH(
    "VideoController/2/list.txt")));
  this->videoController->addVideoSource(vs);
  this->videoSources.append(vs);

  vs = new KwiverVideoSource{this->videoController.get()};
  vs->setVideoInput(this->generateVideoInput(SEALTK_TEST_DATA_PATH(
    "VideoController/3/list.txt")));
  this->videoController->addVideoSource(vs);
  this->videoSources.append(vs);
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
    100, 200, 400, 300, 600, 500, 700, 200, 150,
  };

  static std::array<QVector<QString>, 3> const seekFiles{{
    {
      "1/100.png", "1/200.png", "1/400.png", QString{}, "1/600.png", QString{},
      QString{}, "1/200.png", QString{},
    },
    {
      QString{}, "2/200.png", "2/400.png", "2/300.png", QString{}, "2/500.png",
      QString{}, "2/200.png", QString{},
    },
    {
      "3/100.png", QString{}, QString{}, "3/300.png", "3/600.png", "3/500.png",
      QString{}, QString{}, QString{},
    },
  }};

  std::array<QVector<QImage>, 3> seekImages;

  for (int i = 0; i < 3; i++)
  {
    connect(this->videoSources[i], &VideoSource::imageDisplayed,
            [&seekImages, i](QImage const& image)
    {
      seekImages[i].append(image);
    });
  }

  for (auto t : seekTimes)
  {
    this->videoController->seek(t);
  }

  for (int i = 0; i < 3; i++)
  {
    QCOMPARE(seekImages[i].size(), seekFiles[i].size());

    for (int j = 0; j < seekFiles[i].size(); j++)
    {
      if (!seekFiles[i][j].isNull())
      {
        QImage expected = QImage{sealtk::test::testDataPath(
          "VideoController/" + seekFiles[i][j])};
        QCOMPARE(seekImages[i][j], expected);
      }
      else
      {
        QCOMPARE(seekImages[i][j], QImage{});
      }
    }
  }
}

// ----------------------------------------------------------------------------
void TestVideoController::removeVideoSource()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    100, 200, 400, 300, 600, 500, 700, 200, 150,
  };

  static std::array<QVector<QString>, 3> const seekFiles{{
    {
      "1/100.png", "1/200.png", "1/400.png", QString{}, "1/600.png", QString{},
      QString{}, "1/200.png", QString{},
    },
    {
      QString{}, "2/200.png", "2/400.png", "2/300.png", QString{}, "2/500.png",
      QString{}, "2/200.png", QString{},
    },
    {
      "3/100.png", QString{}, QString{}, "3/300.png",
    },
  }};

  std::array<QVector<QImage>, 3> seekImages;

  for (int i = 0; i < 3; i++)
  {
    connect(this->videoSources[i], &VideoSource::imageDisplayed,
            [&seekImages, i](QImage const& image)
    {
      seekImages[i].append(image);
    });
  }

  for (int i = 0; i < 4; i++)
  {
    this->videoController->seek(seekTimes[i]);
  }

  this->videoController->removeVideoSource(this->videoSources[2]);

  for (int i = 4; i < seekTimes.size(); i++)
  {
    this->videoController->seek(seekTimes[i]);
  }

  for (int i = 0; i < 3; i++)
  {
    QCOMPARE(seekImages[i].size(), seekFiles[i].size());

    for (int j = 0; j < seekFiles[i].size(); j++)
    {
      if (!seekFiles[i][j].isNull())
      {
        QImage expected = QImage{sealtk::test::testDataPath(
          "VideoController/" + seekFiles[i][j])};
        QCOMPARE(seekImages[i][j], expected);
      }
      else
      {
        QCOMPARE(seekImages[i][j], QImage{});
      }
    }
  }
}

// ----------------------------------------------------------------------------
void TestVideoController::times()
{
  static QSet<kwiver::vital::timestamp::time_t> const times{
    100, 200, 300, 400, 500, 600,
  };

  QCOMPARE(this->videoController->times(), times);
}

}

}

}

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestVideoController)
#include "VideoController.moc"
