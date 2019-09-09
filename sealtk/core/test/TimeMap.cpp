/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/TimeMap.hpp>

#include <QObject>

#include <QtTest>

namespace kv = kwiver::vital;

Q_DECLARE_METATYPE(sealtk::core::TimeMap<int>)

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestTimeMap : public QObject
{
  Q_OBJECT

private slots:
  void find();
  void find_data();
};

// ----------------------------------------------------------------------------
void TestTimeMap::find()
{
  // For the sake of simplicity in testing, expected == 0 means that we expect
  // the seek operations to "fail" (return an invalid iterator).

  QFETCH(TimeMap<int>, data);
  QFETCH(kv::timestamp::time_t, searchValue);
  QFETCH(SeekMode, seekMode);
  QFETCH(int, expected);

  // find()
  auto const result1 = data.find(searchValue, seekMode);
  if (expected)
  {
    QCOMPARE(*result1, expected);
  }
  else
  {
    QCOMPARE(result1, data.end());
  }

  // find() const
  auto const result2 = qAsConst(data).find(searchValue, seekMode);
  if (expected)
  {
    QCOMPARE(*result2, expected);
  }
  else
  {
    QCOMPARE(result2, qAsConst(data).end());
  }

  // constFind() const
  auto const result3 = data.constFind(searchValue, seekMode);
  if (expected)
  {
    QCOMPARE(*result3, expected);
  }
  else
  {
    QCOMPARE(result3, data.constEnd());
  }
}

// ----------------------------------------------------------------------------
void TestTimeMap::find_data()
{
  static TimeMap<int> const data{
    {0, 1},
    {1000, 2},
    {1500, 3},
    {2000, 4},
    {3000, 5},
  };

  QTest::addColumn<TimeMap<int>>("data");
  QTest::addColumn<kv::timestamp::time_t>("searchValue");
  QTest::addColumn<SeekMode>("seekMode");
  QTest::addColumn<int>("expected");

  QTest::newRow("Nearest (exact match)")
    << data <<  1000l << SeekNearest << 2;
  QTest::newRow("Nearest (earlier match)")
    << data <<  1249l << SeekNearest << 2;
  QTest::newRow("Nearest (later match)")
    << data <<  1251l << SeekNearest << 3;
  QTest::newRow("Nearest (tied match)")
    << data <<  2500l << SeekNearest << 4;
  QTest::newRow("Nearest (before start)")
    << data << -1000l << SeekNearest << 1;
  QTest::newRow("Nearest (after end)")
    << data <<  4000l << SeekNearest << 5;
  QTest::newRow("Nearest (equals start)")
    << data << 0l << SeekNearest << 1;
  QTest::newRow("Nearest (equals end)")
    << data << 3000l << SeekNearest << 5;
  QTest::newRow("Nearest (on empty map)")
    << TimeMap<int>{} << 2500l << SeekNearest << 0;

  QTest::newRow("LowerBound (exact match)")
    << data << 1000l << SeekLowerBound << 2;
  QTest::newRow("LowerBound (near earlier match)")
    << data << 1001l << SeekLowerBound << 3;
  QTest::newRow("LowerBound (near later match)")
    << data << 1499l << SeekLowerBound << 3;
  QTest::newRow("LowerBound (tied match)")
    << data << 2500l << SeekLowerBound << 5;
  QTest::newRow("LowerBound (before start)")
    << data << -1000l << SeekLowerBound << 1;
  QTest::newRow("LowerBound (after end)")
    << data << 4000l << SeekLowerBound << 0;
  QTest::newRow("LowerBound (equals start)")
    << data << 0l << SeekLowerBound << 1;
  QTest::newRow("LowerBound (equals end)")
    << data << 3000l << SeekLowerBound << 5;
  QTest::newRow("LowerBound (on empty map)")
    << TimeMap<int>{} << 2500l << SeekLowerBound << 0;

  QTest::newRow("UpperBound (exact match)")
    << data << 1000l << SeekUpperBound << 2;
  QTest::newRow("UpperBound (near earlier match)")
    << data << 1001l << SeekUpperBound << 2;
  QTest::newRow("UpperBound (near later match)")
    << data << 1499l << SeekUpperBound << 2;
  QTest::newRow("UpperBound (tied match)")
    << data << 2500l << SeekUpperBound << 4;
  QTest::newRow("UpperBound (before start)")
    << data << -1000l << SeekUpperBound << 0;
  QTest::newRow("UpperBound (after end)")
    << data << 4000l << SeekUpperBound << 5;
  QTest::newRow("UpperBound (equals start)")
    << data << 0l << SeekUpperBound << 1;
  QTest::newRow("UpperBound (equals end)")
    << data << 3000l << SeekUpperBound << 5;
  QTest::newRow("UpperBound (on empty map)")
    << TimeMap<int>{} << 2500l << SeekUpperBound << 0;

  QTest::newRow("Exact (exact match)")
    << data << 1000l << SeekExact << 2;
  QTest::newRow("Exact (near earlier match)")
    << data << 1001l << SeekExact << 0;
  QTest::newRow("Exact (near later match)")
    << data << 1499l << SeekExact << 0;
  QTest::newRow("Exact (tied match)")
    << data << 2500l << SeekExact << 0;
  QTest::newRow("Exact (before start)")
    << data << -1000l << SeekExact << 0;
  QTest::newRow("Exact (after end)")
    << data << 4000l << SeekExact << 0;
  QTest::newRow("Exact (equals start)")
    << data << 0l << SeekExact << 1;
  QTest::newRow("Exact (equals end)")
    << data << 3000l << SeekExact << 5;
  QTest::newRow("Exact (on empty map)")
    << TimeMap<int>{} << 2500l << SeekExact << 0;

  QTest::newRow("Next (exact match)")
    << data << 1000l << SeekNext << 3;
  QTest::newRow("Next (near earlier match)")
    << data << 1001l << SeekNext << 3;
  QTest::newRow("Next (near later match)")
    << data << 1499l << SeekNext << 3;
  QTest::newRow("Next (tied match)")
    << data << 2500l << SeekNext << 5;
  QTest::newRow("Next (before start)")
    << data << -1000l << SeekNext << 1;
  QTest::newRow("Next (after end)")
    << data << 4000l << SeekNext << 0;
  QTest::newRow("Next (equals start)")
    << data << 0l << SeekNext << 2;
  QTest::newRow("Next (equals end)")
    << data << 3000l << SeekNext << 0;
  QTest::newRow("Next (on empty map)")
    << TimeMap<int>{} << 2500l << SeekNext << 0;

  QTest::newRow("Previous (exact match)")
    << data << 1000l << SeekPrevious << 1;
  QTest::newRow("Previous (near earlier match)")
    << data << 1001l << SeekPrevious << 2;
  QTest::newRow("Previous (near later match)")
    << data << 1499l << SeekPrevious << 2;
  QTest::newRow("Previous (tied match)")
    << data << 2500l << SeekPrevious << 4;
  QTest::newRow("Previous (before start)")
    << data << -1000l << SeekPrevious << 0;
  QTest::newRow("Previous (after end)")
    << data << 4000l << SeekPrevious << 5;
  QTest::newRow("Previous (equals start)")
    << data << 0l << SeekPrevious << 0;
  QTest::newRow("Previous (equals end)")
    << data << 3000l << SeekPrevious << 4;
  QTest::newRow("Previous (on empty map)")
    << TimeMap<int>{} << 2500l << SeekPrevious << 0;
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestTimeMap)
#include "TimeMap.moc"
