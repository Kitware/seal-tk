/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestCommon.hpp>

#include <sealtk/core/ImageUtils.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>

#include <vital/algo/video_input.h>
#include <vital/config/config_block.h>
#include <vital/types/timestamp.h>

#include <arrows/qt/image_container.h>

#include <qtStlUtil.h>

#include <QImage>
#include <QSet>
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
class TestKwiverVideoSource : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void seek();
  void frames();
  void cleanup();

private:
  kv::config_block_sptr config;
  std::unique_ptr<core::KwiverVideoSource> videoSource;
};

// ----------------------------------------------------------------------------
void TestKwiverVideoSource::initTestCase()
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
void TestKwiverVideoSource::init()
{
  kv::algo::video_input_sptr videoReader;
  kv::algo::video_input::set_nested_algo_configuration(
    "video_reader", this->config, videoReader);
  videoReader->open(
    stdString(SEALTK_TEST_DATA_PATH("KwiverVideoSource/list.txt")));

  this->videoSource =
    std::unique_ptr<core::KwiverVideoSource>(new core::KwiverVideoSource());

  this->videoSource->setVideoInput(videoReader);
}

// ----------------------------------------------------------------------------
void TestKwiverVideoSource::cleanup()
{
  this->videoSource.reset();
}

// ----------------------------------------------------------------------------
void TestKwiverVideoSource::seek()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    1000, 2000, 3000, 4000, 5000, 3000, 1500,
  };
  static QVector<QString> const seekFiles{
    "1000.png", "2000.png", "3000.png", "4000.png", "5000.png", "3000.png",
    QString{},
  };

  QVector<QImage> seekImages;
  connect(this->videoSource.get(), &VideoSource::imageReady,
          [&seekImages](kv::image_container_sptr const& image)
  {
    if (image)
    {
      seekImages.append(sealtk::core::imageContainerToQImage(image));
    }
    else
    {
      seekImages.append(QImage{});
    }
  });

  for (auto t : seekTimes)
  {
    this->videoSource->seekTime(t);
  }

  QCOMPARE(seekImages.size(), seekFiles.size());

  for (int i = 0; i < seekFiles.size(); i++)
  {
    if (!seekFiles[i].isEmpty())
    {
      QImage expected{sealtk::test::testDataPath(
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
void TestKwiverVideoSource::frames()
{
  static TimeMap<kv::timestamp::frame_t> const frames{
    {1000, 1},
    {2000, 2},
    {3000, 3},
    {4000, 4},
    {5000, 5}
  };

  QCOMPARE(this->videoSource->frames(), frames);
}

}

}

}

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestKwiverVideoSource)
#include "KwiverVideoSource.moc"
