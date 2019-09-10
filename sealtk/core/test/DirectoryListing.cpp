/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/test/TestCore.hpp>

#include <sealtk/core/DirectoryListing.hpp>

#include <qtEnumerate.h>

#include <QDir>
#include <QObject>

#include <QtTest>

namespace sealtk
{

namespace core
{

namespace test
{

using FileMap = QHash<QString, QString>;

auto const TEST_DATA_ROOT =
  QDir{SEALTK_TEST_DATA_PATH("DirectoryListing")}.absolutePath();

auto const TYPE_EX1 = QStringLiteral("ex1");
auto const TYPE_EX2 = QStringLiteral("ex2");
auto const TYPE_TXT = QStringLiteral("txt");
auto const TYPE_TXT_NEW = QStringLiteral("txt.new");

// ============================================================================
class TestDirectoryListing : public QObject
{
  Q_OBJECT

private slots:
  void interface();
  void listing();
  void listing_data();
};

// ----------------------------------------------------------------------------
void TestDirectoryListing::interface()
{
  auto dl = DirectoryListing{{TYPE_TXT}, TEST_DATA_ROOT};

  QCOMPARE(dl.directory(), TEST_DATA_ROOT);
  QCOMPARE(dl.types(), (QStringList{{TYPE_TXT}}));

  dl.setTypes({TYPE_EX1, TYPE_EX2});
  QCOMPARE(dl.types(), (QStringList{{TYPE_EX1, TYPE_EX2}}));

  dl.setDirectory(".");
  QCOMPARE(dl.directory(), QDir{}.absolutePath());
}

// ----------------------------------------------------------------------------
void TestDirectoryListing::listing()
{
  QFETCH(QStringList, types);
  QFETCH(FileMap, expectedFiles);

  auto dl = DirectoryListing{types, TEST_DATA_ROOT};
  auto actualFiles = dl.files();

  QCOMPARE(actualFiles.size(), expectedFiles.size());
  for (auto const& iter : qtEnumerate(expectedFiles))
  {
    auto fi = QFileInfo{actualFiles.value(iter.key())};
    QCOMPARE(fi.fileName(), iter.value());
    QCOMPARE(fi.path(), TEST_DATA_ROOT);
  }
}

// ----------------------------------------------------------------------------
void TestDirectoryListing::listing_data()
{
  QTest::addColumn<QStringList>("types");
  QTest::addColumn<FileMap>("expectedFiles");

  QTest::newRow("single type")
    << QStringList{TYPE_TXT}
    << FileMap{
         {"foo", "foo.txt"},
         {"bar", "bar.txt"},
       };

  QTest::newRow("multiple types")
    << QStringList{TYPE_EX1, TYPE_EX2}
    << FileMap{
         {"file1", "file1.ex1"},
         {"file2", "file2.ex2"},
       };

  QTest::newRow("'complex' type")
    << QStringList{TYPE_TXT_NEW}
    << FileMap{
         {"foo", "foo.txt.new"},
         {"bar.none", "bar.none.txt.new"},
       };
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestDirectoryListing)
#include "DirectoryListing.moc"
