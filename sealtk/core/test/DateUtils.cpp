/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/DateUtils.hpp>

#include <QObject>
#include <QtTest>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestDateUtils : public QObject
{
  Q_OBJECT

private slots:
  void vitalTimeToQDateTime();
  void vitalTimeToQDateTime_data();
  void qDateTimeToVitalTime();
  void qDateTimeToVitalTime_data();
  void dateString();
  void dateString_data();
  void timeString();
  void timeString_data();
};

// ----------------------------------------------------------------------------
void TestDateUtils::vitalTimeToQDateTime()
{
  QFETCH(kv::timestamp::time_t, input);
  QFETCH(QDateTime, expected);

  QCOMPARE(core::vitalTimeToQDateTime(input), expected);
}

// ----------------------------------------------------------------------------
void TestDateUtils::vitalTimeToQDateTime_data()
{
  QTest::addColumn<kv::timestamp::time_t>("input");
  QTest::addColumn<QDateTime>("expected");

  QTest::newRow("zero") << 0l << QDateTime{{1970, 1, 1}, {0, 0, 0}, Qt::UTC};
  QTest::newRow("sample1") << 545415827289000l
    << QDateTime{{1987, 4, 14}, {16, 23, 47, 289}, Qt::UTC};
  QTest::newRow("sample2") << 1381481842745000l
    << QDateTime{{2013, 10, 11}, {8, 57, 22, 745}, Qt::UTC};
}

// ----------------------------------------------------------------------------
void TestDateUtils::qDateTimeToVitalTime()
{
  QFETCH(QDateTime, input);
  QFETCH(kv::timestamp::time_t, expected);

  QCOMPARE(core::qDateTimeToVitalTime(input), expected);
}

// ----------------------------------------------------------------------------
void TestDateUtils::qDateTimeToVitalTime_data()
{
  QTest::addColumn<QDateTime>("input");
  QTest::addColumn<kv::timestamp::time_t>("expected");

  QTest::newRow("zero") << QDateTime{{1970, 1, 1}, {0, 0, 0}, Qt::UTC} << 0l;
  QTest::newRow("sample1")
    << QDateTime{{1987, 4, 14}, {16, 23, 47, 289}, Qt::UTC}
    << 545415827289000l;
  QTest::newRow("sample2")
    << QDateTime{{2013, 10, 11}, {8, 57, 22, 745}, Qt::UTC}
    << 1381481842745000l;
}

// ----------------------------------------------------------------------------
void TestDateUtils::dateString()
{
  QFETCH(QDateTime, input);
  QFETCH(QString, expected);

  QCOMPARE(core::dateString(input), expected);
}

// ----------------------------------------------------------------------------
void TestDateUtils::dateString_data()
{
  QTest::addColumn<QDateTime>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("zero")
    << QDateTime{{1970, 1, 1}, {0, 0, 0}, Qt::UTC}
    << QStringLiteral("1970-01-01");
  QTest::newRow("sample1")
    << QDateTime{{1987, 4, 14}, {16, 23, 47, 289}, Qt::UTC}
    << QStringLiteral("1987-04-14");
  QTest::newRow("sample2")
    << QDateTime{{2013, 10, 11}, {8, 57, 22, 745}, Qt::UTC}
    << QStringLiteral("2013-10-11");
}

// ----------------------------------------------------------------------------
void TestDateUtils::timeString()
{
  QFETCH(QDateTime, input);
  QFETCH(QString, expected);

  QCOMPARE(core::timeString(input), expected);
}

// ----------------------------------------------------------------------------
void TestDateUtils::timeString_data()
{
  QTest::addColumn<QDateTime>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("zero")
    << QDateTime{{1970, 1, 1}, {0, 0, 0}, Qt::UTC}
    << QStringLiteral("00:00:00.000");
  QTest::newRow("sample1")
    << QDateTime{{1987, 4, 14}, {16, 23, 47, 289}, Qt::UTC}
    << QStringLiteral("16:23:47.289");
  QTest::newRow("sample2")
    << QDateTime{{2013, 10, 11}, {8, 57, 22, 745}, Qt::UTC}
    << QStringLiteral("08:57:22.745");
}

} // namespace test

} // namespace core

} // namespace sealtk

QTEST_MAIN(sealtk::core::test::TestDateUtils)
#include "DateUtils.moc"
