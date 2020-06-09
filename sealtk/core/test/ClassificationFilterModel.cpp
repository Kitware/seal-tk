/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ClassificationFilterModel.hpp>

#include <sealtk/core/AbstractItemModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/indirect.h>

#include <QtTest>

namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

// ============================================================================
class TestModel : public AbstractItemModel
{
public:
  int rowCount(QModelIndex const& parent = {}) const override
  { return (parent.isValid() ? 0 : 1); }

  QVariant data(QModelIndex const& index, int role) const override;
};

// ----------------------------------------------------------------------------
QVariant TestModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    if (role == ClassificationRole)
    {
      return QVariantHash{
        {"Cod", 0.3},
        {"Eel", 0.8},
        {"Gar", 0.2},
      };
    }
  }

  return this->AbstractItemModel::data(index, role);
}

} // namespace <anonymous>

// ============================================================================
class TestClassificationFilterModel : public QObject
{
  Q_OBJECT

private slots:
  void filtering();
  void filtering_data();
};

// ----------------------------------------------------------------------------
void TestClassificationFilterModel::filtering()
{
  QFETCH(QVariantHash, lowerBounds);
  QFETCH(QVariantHash, upperBounds);
  QFETCH(bool, expectedVisibility);
  QFETCH(QVariant, expectedType);
  QFETCH(QVariant, expectedScore);
  QFETCH(QVariantHash, expectedClassification);

  TestModel model;
  ClassificationFilterModel filter;

  // Set up filter
  filter.setSourceModel(&model);
  for (auto const& iter : lowerBounds | kvr::indirect)
  {
    filter.setLowerBound(iter.key(), iter.value());
  }
  for (auto const& iter : upperBounds | kvr::indirect)
  {
    filter.setUpperBound(iter.key(), iter.value());
  }

  // Extract and validate data
  auto const& index = filter.index(0, 0);
  auto const& visibilityData = filter.data(index, VisibilityRole);
  auto const& typeData = filter.data(index, ClassificationTypeRole);
  auto const& scoreData = filter.data(index, ClassificationScoreRole);
  auto const& classificationData = filter.data(index, ClassificationRole);

  QCOMPARE(visibilityData.isValid(), true);
  QCOMPARE(visibilityData.canConvert<bool>(), true);
  QCOMPARE(visibilityData.toBool(), expectedVisibility);

  QCOMPARE(typeData, expectedType);
  QCOMPARE(scoreData, expectedScore);
  QCOMPARE(classificationData.toHash(), expectedClassification);
}

// ----------------------------------------------------------------------------
void TestClassificationFilterModel::filtering_data()
{
  QTest::addColumn<QVariantHash>("lowerBounds");
  QTest::addColumn<QVariantHash>("upperBounds");
  QTest::addColumn<bool>("expectedVisibility");
  QTest::addColumn<QVariant>("expectedType");
  QTest::addColumn<QVariant>("expectedScore");
  QTest::addColumn<QVariantHash>("expectedClassification");

  QTest::newRow("pass-through")
    << QVariantHash{}
    << QVariantHash{}
    << true
    << QVariant::fromValue(QStringLiteral("Eel"))
    << QVariant::fromValue(0.8)
    << QVariantHash{{"Cod", 0.3}, {"Eel", 0.8}, {"Gar", 0.2}};
  QTest::newRow("single lower")
    << QVariantHash{{"Cod", 0.5}}
    << QVariantHash{}
    << true
    << QVariant::fromValue(QStringLiteral("Eel"))
    << QVariant::fromValue(0.8)
    << QVariantHash{{"Eel", 0.8}, {"Gar", 0.2}};
  QTest::newRow("single upper")
    << QVariantHash{}
    << QVariantHash{{"Eel", 0.5}}
    << true
    << QVariant::fromValue(QStringLiteral("Cod"))
    << QVariant::fromValue(0.3)
    << QVariantHash{{"Cod", 0.3}, {"Gar", 0.2}};
  QTest::newRow("multiple lower")
    << QVariantHash{{"Cod", 0.5}, {"Eel", 0.5}}
    << QVariantHash{}
    << true
    << QVariant::fromValue(QStringLiteral("Eel"))
    << QVariant::fromValue(0.8)
    << QVariantHash{{"Eel", 0.8}, {"Gar", 0.2}};
  QTest::newRow("multiple upper")
    << QVariantHash{}
    << QVariantHash{{"Cod", 0.5}, {"Eel", 0.5}}
    << true
    << QVariant::fromValue(QStringLiteral("Cod"))
    << QVariant::fromValue(0.3)
    << QVariantHash{{"Cod", 0.3}, {"Gar", 0.2}};
  QTest::newRow("(single lower) + (single upper)")
    << QVariantHash{{"Cod", 0.5}}
    << QVariantHash{{"Eel", 0.5}}
    << true
    << QVariant::fromValue(QStringLiteral("Gar"))
    << QVariant::fromValue(0.2)
    << QVariantHash{{"Gar", 0.2}};
  QTest::newRow("(single lower) + (single upper)")
    << QVariantHash{{"Cod", 0.5}}
    << QVariantHash{{"Eel", 0.5}}
    << true
    << QVariant::fromValue(QStringLiteral("Gar"))
    << QVariant::fromValue(0.2)
    << QVariantHash{{"Gar", 0.2}};
  QTest::newRow("(single lower) + (multiple upper)")
    << QVariantHash{{"Cod", 0.5}}
    << QVariantHash{{"Eel", 0.5}, {"Gar", 0.5}}
    << true
    << QVariant::fromValue(QStringLiteral("Gar"))
    << QVariant::fromValue(0.2)
    << QVariantHash{{"Gar", 0.2}};
  QTest::newRow("(multiple lower) + (single upper)")
    << QVariantHash{{"Cod", 0.5}, {"Gar", 0.5}}
    << QVariantHash{{"Eel", 0.5}}
    << false << QVariant{} << QVariant{}
    << QVariantHash{};
}

} // namespace test

} // namespace core

} // namespace sealtk

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestClassificationFilterModel)
#include "ClassificationFilterModel.moc"
