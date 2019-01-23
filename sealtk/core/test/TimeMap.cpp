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
  // For sake of simplicity in testing, 0 means "not found".

  QFETCH(TimeMap<int>, data);
  QFETCH(kv::timestamp::time_t, searchValue);
  QFETCH(int, seekMode);
  QFETCH(int, expected);

  SeekMode realSeekMode = static_cast<SeekMode>(seekMode);

  // find()
  auto result = data.find(searchValue, realSeekMode);
  if (expected)
  {
    QCOMPARE(*result, expected);
  }
  else
  {
    QCOMPARE(result, data.end());
  }

  TimeMap<int> const constData{data};

  // find() const
  auto constResult = constData.find(searchValue, realSeekMode);
  if (expected)
  {
    QCOMPARE(*constResult, expected);
  }
  else
  {
    QCOMPARE(constResult, constData.end());
  }

  // constFind() const
  constResult = constData.constFind(searchValue, realSeekMode);
  if (expected)
  {
    QCOMPARE(*constResult, expected);
  }
  else
  {
    QCOMPARE(constResult, constData.end());
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
  QTest::addColumn<int>("seekMode");
  QTest::addColumn<int>("expected");

  QTest::newRow("Nearest_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekNearest) << 2;
  QTest::newRow("Nearest_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekNearest) << 2;
  QTest::newRow("Nearest_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekNearest) << 3;
  QTest::newRow("Nearest_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekNearest) << 4;
  QTest::newRow("Nearest_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekNearest) << 1;
  QTest::newRow("Nearest_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekNearest) << 5;
  QTest::newRow("Nearest_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekNearest) << 1;
  QTest::newRow("Nearest_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekNearest) << 5;
  QTest::newRow("Nearest_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekNearest) << 0;

  QTest::newRow("LowerBound_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 2;
  QTest::newRow("LowerBound_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 3;
  QTest::newRow("LowerBound_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 3;
  QTest::newRow("LowerBound_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 5;
  QTest::newRow("LowerBound_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 1;
  QTest::newRow("LowerBound_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 0;
  QTest::newRow("LowerBound_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 1;
  QTest::newRow("LowerBound_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 5;
  QTest::newRow("LowerBound_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekLowerBound) << 0;

  QTest::newRow("UpperBound_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 2;
  QTest::newRow("UpperBound_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 2;
  QTest::newRow("UpperBound_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 2;
  QTest::newRow("UpperBound_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 4;
  QTest::newRow("UpperBound_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 0;
  QTest::newRow("UpperBound_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 5;
  QTest::newRow("UpperBound_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 1;
  QTest::newRow("UpperBound_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 5;
  QTest::newRow("UpperBound_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekUpperBound) << 0;

  QTest::newRow("Exact_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekExact) << 2;
  QTest::newRow("Exact_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekExact) << 0;
  QTest::newRow("Exact_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekExact) << 0;
  QTest::newRow("Exact_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekExact) << 0;
  QTest::newRow("Exact_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekExact) << 0;
  QTest::newRow("Exact_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekExact) << 0;
  QTest::newRow("Exact_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekExact) << 1;
  QTest::newRow("Exact_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekExact) << 5;
  QTest::newRow("Exact_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekExact) << 0;

  QTest::newRow("Next_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekNext) << 3;
  QTest::newRow("Next_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekNext) << 3;
  QTest::newRow("Next_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekNext) << 3;
  QTest::newRow("Next_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekNext) << 5;
  QTest::newRow("Next_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekNext) << 1;
  QTest::newRow("Next_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekNext) << 0;
  QTest::newRow("Next_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekNext) << 2;
  QTest::newRow("Next_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekNext) << 0;
  QTest::newRow("Next_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekNext) << 0;

  QTest::newRow("Previous_exact") << data << 1000l
    << static_cast<int>(sealtk::core::SeekPrevious) << 1;
  QTest::newRow("Previous_down") << data << 1249l
    << static_cast<int>(sealtk::core::SeekPrevious) << 2;
  QTest::newRow("Previous_up") << data << 1251l
    << static_cast<int>(sealtk::core::SeekPrevious) << 2;
  QTest::newRow("Previous_split") << data << 2500l
    << static_cast<int>(sealtk::core::SeekPrevious) << 4;
  QTest::newRow("Previous_low") << data << -1000l
    << static_cast<int>(sealtk::core::SeekPrevious) << 0;
  QTest::newRow("Previous_high") << data << 4000l
    << static_cast<int>(sealtk::core::SeekPrevious) << 5;
  QTest::newRow("Previous_lowerbound") << data << 0l
    << static_cast<int>(sealtk::core::SeekPrevious) << 0;
  QTest::newRow("Previous_upperbound") << data << 3000l
    << static_cast<int>(sealtk::core::SeekPrevious) << 4;
  QTest::newRow("Previous_empty") << TimeMap<int>{} << 2500l
    << static_cast<int>(sealtk::core::SeekPrevious) << 0;
}

}

}

}

QTEST_MAIN(sealtk::core::test::TestTimeMap)
#include "TimeMap.moc"
