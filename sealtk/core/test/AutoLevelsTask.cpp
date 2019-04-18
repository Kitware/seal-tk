/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/AutoLevelsTask.hpp>

#include <vital/algo/image_io.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <vital/range/iota.h>

#include <qtStlUtil.h>

#include <QObject>
#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace // anonymous
{

// ============================================================================
struct LevelPair
{
  float low;
  float high;
};

} // namespace <anonymous>

Q_DECLARE_METATYPE(LevelPair)

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestAutoLevelsTask : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void levels();
  void levels_data();

private:
  kv::image_container_sptr m_image;
};

// ----------------------------------------------------------------------------
void TestAutoLevelsTask::initTestCase()
{
  kv::plugin_manager::instance().load_all_plugins();
  if (auto io = kv::algo::image_io::create("vxl"))
  {
    m_image = io->load(
      stdString(SEALTK_TEST_DATA_PATH("AutoLevelsTask/test.tif")));
  }
}

// ----------------------------------------------------------------------------
void TestAutoLevelsTask::levels()
{
  // Check that test image was loaded (pointless to proceed otherwise!)
  QVERIFY(m_image);

  // Fetch test case parameters
  QFETCH(double, outlierDeviance);
  QFETCH(double, outlierTolerance);
  QFETCH(QVector<LevelPair>, expected);

  // Set up the task
  QVector<LevelPair> computed;
  AutoLevelsTask task{m_image, outlierDeviance, outlierTolerance};

  connect(&task, &AutoLevelsTask::levelsUpdated,
          [&](float l, float h){ computed.append({l, h}); });

  // Run the task
  task.execute();

  // Compare computed result to expected result
  QCOMPARE(computed.count(), expected.count());
  for (auto i : kvr::iota(expected.count()))
  {
    QCOMPARE(computed[i].low, expected[i].low);
    QCOMPARE(computed[i].high, expected[i].high);
  }
}

// ----------------------------------------------------------------------------
void TestAutoLevelsTask::levels_data()
{
  QTest::addColumn<double>("outlierDeviance");
  QTest::addColumn<double>("outlierTolerance");
  QTest::addColumn<QVector<LevelPair>>("expected");

  QTest::newRow("strict") << 0.0 << 0.0 << QVector<LevelPair>{
    {0.3642578125f, 0.630859375f},
    {0.3291015625f, 0.7900390625f},
    {0.3291015625f, 1.0f},
    {0.30859375f, 1.0f},
    {0.30859375f, 1.0f},
  };
  QTest::newRow("d=5%, t=0") << 0.05 << 0.00 << QVector<LevelPair>{
    {0.4248046875f, 0.5703125f},
    {0.427734375f, 0.568359375f},
    {0.427734375f, 0.568359375f},
    {0.427734375f, 0.568359375f},
    {0.427734375f, 0.568359375f},
  };
  QTest::newRow("d=2%, t=0") << 0.02 << 0.00 << QVector<LevelPair>{
    {0.4091796875f, 0.583984375f},
    {0.41015625f, 0.583984375f},
    {0.41015625f, 0.5849609375f},
    {0.41015625f, 0.5849609375f},
    {0.41015625f, 0.5849609375f},
  };
  QTest::newRow("d=2%, t=20%") << 0.02 << 0.20 << QVector<LevelPair>{
    {0.4091796875f, 0.583984375f},
    {0.41015625f, 0.583984375f},
    {0.41015625f, 0.5849609375f},
    {0.41015625f, 0.5849609375f},
    {0.41015625f, 0.5849609375f},
  };
  QTest::newRow("d=2%, t=60%") << 0.02 << 0.60 << QVector<LevelPair>{
    {0.3642578125f, 0.630859375f},
    {0.3291015625f, 0.583984375f},
    {0.3291015625f, 0.5849609375f},
    {0.30859375f, 0.5849609375f},
    {0.30859375f, 0.5849609375f},
  };
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestAutoLevelsTask)
#include "AutoLevelsTask.moc"
