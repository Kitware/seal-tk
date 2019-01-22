/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/noaa/ImageListVideoSourceFactory.hpp>
#include <sealtk/noaa/Plugins.hpp>

#include <sealtk/core/DateUtils.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <vital/plugin_loader/plugin_manager.h>

#include <QDateTime>
#include <QObject>

#include <QtTest>

#include <memory>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace noaa
{

namespace test
{

// ============================================================================
class TestImageListVideoSourceFactory : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void loadVideoSource();
  void cleanup();

private:
  std::unique_ptr<core::VideoController> videoController;
  ImageListVideoSourceFactory* videoSourceFactory;
};

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::initTestCase()
{
  kwiver::vital::plugin_manager::instance().load_all_plugins();
  registerKwiverPlugins();
}

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::init()
{
  this->videoController = std::make_unique<core::VideoController>();
  this->videoSourceFactory =
    new ImageListVideoSourceFactory{this->videoController.get()};
}

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::cleanup()
{
  this->videoController.reset();
}

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::loadVideoSource()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    core::qDateTimeToVitalTime({{2016, 4, 9}, {0, 27, 37, 735}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 4, 14}, {23, 28, 3, 877}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 4, 23}, {0, 32, 33, 327}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 5, 17}, {1, 26, 7, 322}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 5, 17}, {2, 46, 14, 880}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 5, 17}, {23, 55, 21, 737}, Qt::UTC}),
    core::qDateTimeToVitalTime({{2016, 5, 17}, {23, 57, 25, 346}, Qt::UTC}),
    0,
  };

  static QVector<QString> const seekFiles{
    "CHESS_FL2_C_160409_002737.735_COLOR-8-BIT.JPG",
    "CHESS_FL7_C_160414_232803.877_COLOR-8-BIT.JPG",
    "CHESS_FL13_C_160423_003233.327_COLOR-8-BIT.JPG",
    "CHESS_FL22_S_160517_012607.322_COLOR-8-BIT.JPG",
    "CHESS_FL22_P_160517_024614.880_COLOR-8-BIT.JPG",
    "CHESS_FL23_P_160517_235521.737_COLOR-8-BIT.JPG",
    "CHESS_FL23_C_160517_235725.346_COLOR-8-BIT.JPG",
    QString{},
  };

  core::VideoSource* videoSource = nullptr;

  connect(this->videoSourceFactory,
          &core::FileVideoSourceFactory::fileRequested,
          [this](void* handle)
  {
    this->videoSourceFactory->loadFile(
      handle, SEALTK_TEST_DATA_PATH("ImageListVideoSourceFactory/list.txt"));
  });
  connect(this->videoSourceFactory,
          &core::VideoSourceFactory::videoSourceLoaded,
          [&videoSource](core::VideoSource* vs)
  {
    videoSource = vs;
  });
  this->videoSourceFactory->loadVideoSource();

  QVector<QImage> seekImages;

  connect(videoSource, &core::VideoSource::imageDisplayed,
          [&seekImages](QImage const& image)
  {
    seekImages.append(image);
  });

  for (auto t : seekTimes)
  {
    videoSource->seek(t);
  }

  QCOMPARE(seekImages.size(), seekFiles.size());

  for (int i = 0; i < seekImages.size(); i++)
  {
    if (!seekFiles[i].isNull())
    {
      QImage expected{sealtk::test::testDataPath(
        "ImageListVideoSourceFactory/" + seekFiles[i])};
      QCOMPARE(seekImages[i], expected);
    }
    else
    {
      QCOMPARE(seekImages[i], QImage{});
    }
  }
}

}

}

}

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::noaa::test::TestImageListVideoSourceFactory)
#include "ImageListVideoSourceFactory.moc"
