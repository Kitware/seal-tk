/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>

#include <sealtk/core/KwiverVideoSource.hpp>

#include <vital/algo/video_input.h>
#include <vital/config/config_block.h>
#include <vital/types/timestamp.h>

#include <QImage>
#include <QVector>

#include <QtTest>

#include <memory>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestVideoSource : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void seek();
  void times();
  void cleanup();

private:
  kv::config_block_sptr config;
  std::unique_ptr<core::KwiverVideoSource> videoSource;
};

// ----------------------------------------------------------------------------
void TestVideoSource::initTestCase()
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
void TestVideoSource::init()
{
  kv::algo::video_input_sptr videoReader;
  kv::algo::video_input::set_nested_algo_configuration(
    "video_reader", this->config, videoReader);
  videoReader->open(
    SEALTK_TEST_DATA_PATH("KwiverVideoSource/list1.txt").toStdString());

  this->videoSource = std::make_unique<core::KwiverVideoSource>();
  this->videoSource->setVideoInput(videoReader);
}

// ----------------------------------------------------------------------------
void TestVideoSource::cleanup()
{
  this->videoSource.reset();
}

// ----------------------------------------------------------------------------
void TestVideoSource::seek()
{
  static QVector<kwiver::vital::timestamp::time_t> const seekTimes{
    1000, 2000, 3000, 4000, 5000, 3000, 1500,
  };
  static QVector<QString> const seekFiles{
    "1000.png", "2000.png", "3000.png", "4000.png", "5000.png", "3000.png",
    QString{},
  };

  QVector<QImage> seekImages;
  connect(this->videoSource.get(), &core::VideoSource::imageDisplayed,
    [&seekImages](QImage const& image)
  {
    seekImages.append(image);
  });

  for (auto t : seekTimes)
  {
    this->videoSource->seek(t);
  }

  QCOMPARE(seekImages.size(), seekFiles.size());

  for (int i = 0; i < seekFiles.size(); i++)
  {
    QImage expected;
    if (!seekFiles[i].isNull())
    {
      expected = QImage{sealtk::test::testDataPath(
        "KwiverVideoSource/" + seekFiles[i])};
      QCOMPARE(seekImages[i], expected);
    }
    else
    {
      QCOMPARE(seekImages[i], QImage{});
    }
  }
}

// ----------------------------------------------------------------------------
void TestVideoSource::times()
{
  static std::set<kv::timestamp::time_t> const times{
    1000, 2000, 3000, 4000, 5000,
  };

  QCOMPARE(this->videoSource->times(), times);
}

}

}

}

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestVideoSource)
#include "KwiverVideoSource.moc"
