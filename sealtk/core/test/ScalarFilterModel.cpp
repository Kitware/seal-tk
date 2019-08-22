/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ScalarFilterModel.hpp>

#include <sealtk/core/AbstractItemModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/timestamp.h>

#include <vital/range/iota.h>

#include <QSet>

#include <QtTest>

namespace kvr = kwiver::vital::range;

using time_us_t = kwiver::vital::timestamp::time_t;

namespace QTest
{

// ----------------------------------------------------------------------------
template<> char* toString<QSet<int>>(QSet<int> const& set)
{
  QString s;

  QDebug d{&s};
  d << set;

  return qstrdup(qPrintable(s));
}

} // namespace QTest

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

// ============================================================================
class TestModel : public sealtk::core::AbstractItemModel
{
public:
  TestModel(int rows) : rows{rows} {}

  int rowCount(QModelIndex const& parent = {}) const override
  { return (parent.isValid() ? 0 : this->rows); }

  QVariant data(QModelIndex const& index, int role) const override;

private:
  int const rows;
};

// ----------------------------------------------------------------------------
QVariant TestModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    auto const row = index.row();
    switch (role)
    {
      case sealtk::core::NameRole:
      case sealtk::core::LogicalIdentityRole:
        return row;

      case sealtk::core::StartTimeRole:
      case sealtk::core::EndTimeRole:
        return QVariant::fromValue(static_cast<time_us_t>(row * 100));

      default:
        break;
    }
  }

  return this->sealtk::core::AbstractItemModel::data(index, role);
}

// ----------------------------------------------------------------------------
QSet<int> visibleRows(QAbstractItemModel const& model)
{
  using sealtk::core::VisibilityRole;

  auto result = QSet<int>{};
  for (auto const row : kvr::iota(model.rowCount()))
  {
    auto const& index = model.index(row, 0);

    [&]{ QVERIFY(model.data(index, VisibilityRole).canConvert<bool>()); }();
    if (model.data(model.index(row, 0), VisibilityRole).toBool())
    {
      result.insert(row);
    }
  }

  return result;
}

} // namespace <anonymous>

// ============================================================================
class TestScalarFilterModel : public QObject
{
  Q_OBJECT

private slots:
  void filtering();
  void filtering_data();
};

// ----------------------------------------------------------------------------
void TestScalarFilterModel::filtering()
{
  using sealtk::core::StartTimeRole;
  using sealtk::core::EndTimeRole;

  QFETCH(int, rows);
  QFETCH(QVariant, startLowerBound);
  QFETCH(QVariant, startUpperBound);
  QFETCH(QVariant, endLowerBound);
  QFETCH(QVariant, endUpperBound);
  QFETCH(QSet<int>, expected);

  TestModel model{rows};
  ScalarFilterModel filter;

  filter.setSourceModel(&model);
  if (startLowerBound.isValid())
  {
    filter.setLowerBound(StartTimeRole, startLowerBound);
  }
  if (startUpperBound.isValid())
  {
    filter.setUpperBound(StartTimeRole, startUpperBound);
  }
  if (endLowerBound.isValid())
  {
    filter.setLowerBound(EndTimeRole, endLowerBound);
  }
  if (endUpperBound.isValid())
  {
    filter.setUpperBound(EndTimeRole, endUpperBound);
  }

  QCOMPARE(visibleRows(filter), expected);
}

// ----------------------------------------------------------------------------
void TestScalarFilterModel::filtering_data()
{
  QTest::addColumn<int>("rows");
  QTest::addColumn<QVariant>("startLowerBound");
  QTest::addColumn<QVariant>("startUpperBound");
  QTest::addColumn<QVariant>("endLowerBound");
  QTest::addColumn<QVariant>("endUpperBound");
  QTest::addColumn<QSet<int>>("expected");

  QTest::newRow("single lower") << 10
    << QVariant::fromValue(300) << QVariant{}
    << QVariant{} << QVariant{}
    << QSet<int>{3, 4, 5, 6, 7, 8, 9};
  QTest::newRow("single upper") << 10
    << QVariant{} << QVariant::fromValue(700)
    << QVariant{} << QVariant{}
    << QSet<int>{0, 1, 2, 3, 4, 5, 6, 7};
  QTest::newRow("single (lower + upper)") << 10
    << QVariant::fromValue(300) << QVariant::fromValue(700)
    << QVariant{} << QVariant{}
    << QSet<int>{3, 4, 5, 6, 7};
  QTest::newRow("multiple lower") << 10
    << QVariant::fromValue(200) << QVariant{}
    << QVariant::fromValue(400) << QVariant{}
    << QSet<int>{4, 5, 6, 7, 8, 9};
  QTest::newRow("multiple upper") << 10
    << QVariant{} << QVariant::fromValue(600)
    << QVariant{} << QVariant::fromValue(800)
    << QSet<int>{0, 1, 2, 3, 4, 5, 6};
  QTest::newRow("(single lower) + (single upper)") << 10
    << QVariant::fromValue(300) << QVariant{}
    << QVariant{} << QVariant::fromValue(700)
    << QSet<int>{3, 4, 5, 6, 7};
  QTest::newRow("(multiple lower) + (multiple upper)") << 10
    << QVariant::fromValue(400) << QVariant::fromValue(800)
    << QVariant::fromValue(200) << QVariant::fromValue(600)
    << QSet<int>{4, 5, 6};
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestScalarFilterModel)
#include "ScalarFilterModel.moc"
