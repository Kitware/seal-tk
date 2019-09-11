/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/core/FilenameUtils.hpp>

#include <QObject>
#include <QtTest>

namespace sealtk
{

namespace noaa
{

namespace test
{

// ============================================================================
class TestFilenameUtils : public QObject
{
  Q_OBJECT

private slots:
  void imageFilenameToQDateTime();
  void imageFilenameToQDateTime_data();
};

// ----------------------------------------------------------------------------
void TestFilenameUtils::imageFilenameToQDateTime()
{
  QFETCH(QString, input);
  QFETCH(QDateTime, expected);

  QCOMPARE(core::imageFilenameToQDateTime(input), expected);
}

// ----------------------------------------------------------------------------
void TestFilenameUtils::imageFilenameToQDateTime_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QDateTime>("expected");

  QTest::newRow("rel_unix")
    << "data/CHESS_FL13_C_160423_003233.327_COLOR-8-BIT.JPG"
    << QDateTime{{2016, 4, 23}, {0, 32, 33, 327}, Qt::UTC};
  QTest::newRow("abs_unix")
    << "/path/to/data/CHESS_FL22_P_160517_024614.880_THERM-16BIT.JPG"
    << QDateTime{{2016, 5, 17}, {2, 46, 14, 880}, Qt::UTC};
  QTest::newRow("trick_unix")
    << "CHESS_FL23_P_160517_235521.737_COLOR-8-BIT.JPG/"
       "CHESS_FL22_P_160517_024614.880_THERM-16BIT.JPG"
    << QDateTime{{2016, 5, 17}, {2, 46, 14, 880}, Qt::UTC};
  QTest::newRow("null_unix")
    << "/path/to/file.txt"
    << QDateTime{};
  QTest::newRow("rel_windows")
    << "data\\CHESS_FL22_S_160517_012607.322_THERM-16BIT.JPG"
    << QDateTime{{2016, 5, 17}, {1, 26, 7, 322}, Qt::UTC};
  QTest::newRow("abs_windows")
    << "C:\\path\\to\\data\\CHESS_FL23_C_160517_235725.346_COLOR-8-BIT.JPG"
    << QDateTime{{2016, 5, 17}, {23, 57, 25, 346}, Qt::UTC};
  QTest::newRow("trick_windows")
    << "CHESS_FL2_C_160409_002737.735_THERM-16BIT.JPG\\"
       "CHESS_FL23_C_160517_235725.346_COLOR-8-BIT.JPG"
    << QDateTime{{2016, 5, 17}, {23, 57, 25, 346}, Qt::UTC};
  QTest::newRow("null_windows")
    << "C:\\path\\to\\file.txt"
    << QDateTime{};
}

} // namespace test

} // namespace noaa

} // namespace sealtk

QTEST_MAIN(sealtk::noaa::test::TestFilenameUtils)
#include "FilenameUtils.moc"
