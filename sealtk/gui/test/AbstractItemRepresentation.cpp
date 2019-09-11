/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/AbstractItemRepresentation.hpp>

#include <sealtk/core/AbstractItemModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/timestamp.h>

#include <vital/range/iota.h>

#include <QUuid>
#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

namespace test
{

namespace // anonymous
{

constexpr int ROW_COUNT = 3;
constexpr int COLUMN_COUNT = 3;

QString const nameData[ROW_COUNT] = {
  QStringLiteral("Pig"),
  QStringLiteral("Cow"),
  QStringLiteral("Emu"),
};

QUuid const uniqueIdentityData[ROW_COUNT] = {
  {0xe98b70d3, 0x3daf, 0x48de, 0x91, 0x2b, 0x1b, 0xe1, 0x11, 0xf6, 0x61, 0x89},
  {0xede5f114, 0x0333, 0x4c5f, 0x96, 0x59, 0x52, 0x50, 0xef, 0x24, 0xfe, 0x0d},
  {0x478d0854, 0x9d2e, 0x478a, 0xb8, 0x82, 0x47, 0x56, 0x13, 0xfa, 0xe4, 0x53},
};

kv::timestamp::time_t const timeData[ROW_COUNT] = {
  545415827289077l,
  1381481842745356l,
  1559840855588146l,
};

bool const visibilityData[ROW_COUNT] = {
  true,
  true,
  false,
};

// ============================================================================
class TestModel : public core::AbstractItemModel
{
public:
  TestModel(QObject* parent) : AbstractItemModel{parent} {}

  int rowCount(QModelIndex const& parent) const override;
  QVariant data(QModelIndex const& index, int role) const override;
};

// ----------------------------------------------------------------------------
int TestModel::rowCount(QModelIndex const& parent) const
{
  return (parent.isValid() ? 0 : ROW_COUNT);
}

// ----------------------------------------------------------------------------
QVariant TestModel::data(QModelIndex const& index, int role) const
{
  auto const r = index.row();
  if (!index.isValid() || r < 0 || r >= ROW_COUNT)
  {
    return {};
  }

  switch (role)
  {
    case core::NameRole:
      return nameData[r];

    case core::UniqueIdentityRole:
      return uniqueIdentityData[r];

    case core::StartTimeRole:
    case core::EndTimeRole:
      return static_cast<qint64>(timeData[r]);

    case core::UserVisibilityRole:
      return visibilityData[r];

    default:
      return this->AbstractItemModel::data(index, role);
  }
}

// ============================================================================
class TestRepresentation : public AbstractItemRepresentation
{
public:
  TestRepresentation(QObject* parent);
};

// ----------------------------------------------------------------------------
TestRepresentation::TestRepresentation(QObject* parent)
  : AbstractItemRepresentation{parent}
{
  this->setColumnRoles({
    core::NameRole,
    core::UniqueIdentityRole,
    core::StartTimeRole,
  });
}

} // namespace <anonymous>

// ============================================================================
class TestAbstractItemRepresentation : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void modelAttributes();
  void displayData();
  void displayData_data();
  void tooltipData();
  void tooltipData_data();

private:
  QAbstractItemModel* model;
  AbstractItemRepresentation* representation;

  void testData(int role);
};

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::initTestCase()
{
  this->model = new TestModel{this};
  this->representation = new TestRepresentation{this};
  this->representation->setSourceModel(this->model);
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::modelAttributes()
{
  // Test with visibility::shadow
  this->representation->setItemVisibilityMode(ShadowHidden);
  QCOMPARE(this->representation->rowCount(), ROW_COUNT);
  QCOMPARE(this->representation->columnCount(), COLUMN_COUNT);

  // Test with visibility::omit
  this->representation->setItemVisibilityMode(OmitHidden);
  QCOMPARE(this->representation->rowCount(), ROW_COUNT - 1);
  QCOMPARE(this->representation->columnCount(), COLUMN_COUNT);
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::displayData()
{
  testData(Qt::DisplayRole);
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::displayData_data()
{
  QTest::addColumn<int>("row");
  QTest::addColumn<ItemVisibilityMode>("mode");
  QTest::addColumn<QStringList>("expected");

  QTest::newRow("row 0")
    << 0 << ShadowHidden
    << QStringList{
         QStringLiteral("Pig"),
         QStringLiteral("{e98b70d3-3daf-48de-912b-1be111f66189}"),
         QStringLiteral("16:23:47.289"),
       };

  QTest::newRow("row 1")
    << 1 << ShadowHidden
    << QStringList{
         QStringLiteral("Cow"),
         QStringLiteral("{ede5f114-0333-4c5f-9659-5250ef24fe0d}"),
         QStringLiteral("08:57:22.745"),
       };

  QTest::newRow("row 2 (shadow)")
    << 2 << ShadowHidden
    << QStringList{
         QStringLiteral("Emu"),
         QStringLiteral("{478d0854-9d2e-478a-b882-475613fae453}"),
         QStringLiteral("17:07:35.588"),
       };

  QTest::newRow("row 2 (omit)")
    << 2 << OmitHidden << QStringList{};
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::tooltipData()
{
  testData(Qt::ToolTipRole);
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::tooltipData_data()
{
  QTest::addColumn<int>("row");
  QTest::addColumn<ItemVisibilityMode>("mode");
  QTest::addColumn<QStringList>("expected");

  QTest::newRow("row 0")
    << 0 << ShadowHidden
    << QStringList{
         QStringLiteral("Pig"),
         QStringLiteral("{e98b70d3-3daf-48de-912b-1be111f66189}"),
         QStringLiteral("1987-04-14 16:23:47.289"),
       };

  QTest::newRow("row 1")
    << 1 << ShadowHidden
    << QStringList{
         QStringLiteral("Cow"),
         QStringLiteral("{ede5f114-0333-4c5f-9659-5250ef24fe0d}"),
         QStringLiteral("2013-10-11 08:57:22.745"),
       };

  QTest::newRow("row 2 (shadow)")
    << 2 << ShadowHidden
    << QStringList{
         QStringLiteral("Emu"),
         QStringLiteral("{478d0854-9d2e-478a-b882-475613fae453}"),
         QStringLiteral("2019-06-06 17:07:35.588"),
       };

  QTest::newRow("row 2 (omit)")
    << 2 << OmitHidden << QStringList{};
}

// ----------------------------------------------------------------------------
void TestAbstractItemRepresentation::testData(int role)
{
  QFETCH(int, row);
  QFETCH(ItemVisibilityMode, mode);
  QFETCH(QStringList, expected);

  this->representation->setItemVisibilityMode(mode);

  if (expected.isEmpty())
  {
    QVERIFY(row >= this->representation->rowCount());
  }
  else
  {
    for (auto const i : kvr::iota(expected.size()))
    {
      auto const& index = this->representation->index(row, i);
      auto const& data = this->representation->data(index, role);
      QCOMPARE(data.toString(), expected[i]);
    }
  }
}

} // namespace test

} // namespace gui

} // namespace sealtk

QTEST_MAIN(sealtk::gui::test::TestAbstractItemRepresentation)
#include "AbstractItemRepresentation.moc"
