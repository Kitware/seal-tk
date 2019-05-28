/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/test/TestVideo.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/core/DateUtils.hpp>
#include <sealtk/core/ImageUtils.hpp>

#include <arrows/qt/image_container.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <vital/range/iota.h>

#include <qtStlUtil.h>

#include <QDateTime>
#include <QObject>

#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

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

private:
  sealtk::noaa::core::ImageListVideoSourceFactory* videoSourceFactory;
};

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::initTestCase()
{
  kwiver::vital::plugin_manager::instance().load_all_plugins();
}

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::init()
{
  this->videoSourceFactory =
    new core::ImageListVideoSourceFactory{false, this};
}

// ----------------------------------------------------------------------------
void TestImageListVideoSourceFactory::loadVideoSource()
{
  static QVector<kv::timestamp::time_t> const seekTimes{
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 4, 9}, {0, 27, 37, 735}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 4, 14}, {23, 28, 3, 877}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 4, 23}, {0, 32, 33, 327}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 5, 17}, {1, 26, 7, 322}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 5, 17}, {2, 46, 14, 880}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 5, 17}, {23, 55, 21, 737}, Qt::UTC}),
    sealtk::core::qDateTimeToVitalTime(
      {{2016, 5, 17}, {23, 57, 25, 346}, Qt::UTC}),
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

  sealtk::core::VideoSource* videoSource = nullptr;

  connect(this->videoSourceFactory,
          &sealtk::core::FileVideoSourceFactory::fileRequested,
          [this](void* handle)
  {
    this->videoSourceFactory->loadFile(
      handle, SEALTK_TEST_DATA_PATH("ImageListVideoSourceFactory/list.txt"));
  });
  connect(this->videoSourceFactory,
          &sealtk::core::VideoSourceFactory::videoSourceLoaded,
          [&videoSource](void* handle, sealtk::core::VideoSource* vs)
  {
    Q_UNUSED(handle);
    videoSource = vs;
  });
  this->videoSourceFactory->loadVideoSource(nullptr);

  auto requestor = std::make_shared<sealtk::core::test::TestVideoRequestor>();

  for (auto t : seekTimes)
  {
    requestor->request(videoSource, t);
  }

  auto const& seekFrames = requestor->receivedFrames;
  QCOMPARE(seekFrames.size(), seekFiles.size());

  for (auto const i : kvr::iota(seekFrames.size()))
  {
    auto const& frame = seekFrames[i];

    if (!seekFiles[i].isNull())
    {
      auto const& expected = QImage{sealtk::test::testDataPath(
        "ImageListVideoSourceFactory/" + seekFiles[i])};
      auto const& actual =
        sealtk::core::imageContainerToQImage(frame.image);

      QCOMPARE(actual, expected);

      QFileInfo fi{qtString(frame.metaData.imageName())};
      QCOMPARE(fi.fileName(), seekFiles[i]);

      QCOMPARE(frame.metaData.timeStamp().get_time_usec(),
               seekTimes[i]);
    }
    else
    {
      QCOMPARE(frame.image.get(), nullptr);
      QVERIFY(frame.metaData.imageName().empty());
      QVERIFY(!frame.metaData.timeStamp().is_valid());
    }
  }
}

}

}

}

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::noaa::test::TestImageListVideoSourceFactory)
#include "ImageListVideoSourceFactory.moc"
