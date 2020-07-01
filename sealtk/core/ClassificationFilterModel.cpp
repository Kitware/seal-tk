/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ClassificationFilterModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/indirect.h>

#include <qtGet.h>

#include <QHash>

namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

// ============================================================================
struct Classifier
{
  QVariant type;  // nominally QString
  QVariant score; // nominally double
};

// ----------------------------------------------------------------------------
Classifier bestClassifier(QVariantHash const& classifier)
{
  auto best = -qInf();
  auto out = Classifier{};

  for (auto const& iter : classifier | kvr::indirect())
  {
    auto const& scoreData = iter.value();
    if (scoreData.canConvert<double>())
    {
      auto const score = scoreData.toDouble();
      if (score > best)
      {
        out = {iter.key(), score};
        best = score;
      }
    }
  }

  return out;
}

} // namespace <anonymous>

// ============================================================================
class ClassificationFilterModelPrivate
{
public:
  using Bound = QPair<QVariant, QVariant>;

  QVariantHash filter(QVariantHash const& in) const;

  static bool filter(QVariant const& in, Bound const& bound);

  QHash<QString, Bound> bounds;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(ClassificationFilterModel)

// ----------------------------------------------------------------------------
ClassificationFilterModel::ClassificationFilterModel(QObject* parent)
  : AbstractProxyModel{parent}, d_ptr{new ClassificationFilterModelPrivate}
{
  // Our filtering is dependent on the logical data model's data; therefore, we
  // need to re-filter and/or re-sort when the underlying data changes, and so
  // we enable doing so by default
  this->setDynamicSortFilter(true);
}

// ----------------------------------------------------------------------------
ClassificationFilterModel::~ClassificationFilterModel()
{
}

// ----------------------------------------------------------------------------
QList<QString> ClassificationFilterModel::types() const
{
  QTE_D();
  return d->bounds.keys();
}

// ----------------------------------------------------------------------------
QVariant ClassificationFilterModel::lowerBound(
  QString const& type) const
{
  QTE_D();
  return d->bounds.value(type).first;
}

// ----------------------------------------------------------------------------
QVariant ClassificationFilterModel::upperBound(
  QString const& type) const
{
  QTE_D();
  return d->bounds.value(type).second;
}

// ----------------------------------------------------------------------------
QPair<QVariant, QVariant> ClassificationFilterModel::bound(
  QString const& type) const
{
  QTE_D();
  return d->bounds.value(type);
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::setLowerBound(
  QString const& type, QVariant const& bound)
{
  if (this->isValidData(bound, type))
  {
    QTE_D();

    auto& old = d->bounds[type];
    if (bound != old.first)
    {
      old.first = bound;
      this->invalidateVisibility();
    }
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::setUpperBound(
  QString const& type, QVariant const& bound)
{
  if (this->isValidData(bound, type))
  {
    QTE_D();

    auto& old = d->bounds[type];
    if (bound != old.second)
    {
      old.second = bound;
      this->invalidateVisibility();
    }
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::setBound(
  QString const& type, QVariant const& lower, QVariant const& upper)
{
  if (this->isValidData(lower, type) && this->isValidData(upper, type))
  {
    QTE_D();

    auto& old = d->bounds[type];
    if (lower != old.first || upper != old.second)
    {
      old.first = lower;
      old.second = upper;
      this->invalidateVisibility();
    }
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::clearLowerBound(QString const& type)
{
  QTE_D();

  auto const& i = d->bounds.find(type);
  if (i != d->bounds.end() && i->first.isValid())
  {
    if (i->second.isValid())
    {
      i->first.clear();
    }
    else
    {
      d->bounds.erase(i);
    }

    this->invalidateVisibility();
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::clearUpperBound(QString const& type)
{
  QTE_D();

  auto const& i = d->bounds.find(type);
  if (i != d->bounds.end() && i->second.isValid())
  {
    if (i->first.isValid())
    {
      i->second.clear();
    }
    else
    {
      d->bounds.erase(i);
    }

    this->invalidateVisibility();
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::clearBound(QString const& type)
{
  QTE_D();

  if (d->bounds.remove(type))
  {
    this->invalidateVisibility();
  }
}

// ----------------------------------------------------------------------------
void ClassificationFilterModel::clearBounds()
{
  QTE_D();

  if (!d->bounds.isEmpty())
  {
    d->bounds.clear();
    this->invalidateVisibility();
  }
}

// ----------------------------------------------------------------------------
QVariant ClassificationFilterModel::data(
  QModelIndex const& index, int role) const
{
  switch (role)
  {
    case ClassificationRole:
    case ClassificationTypeRole:
    case ClassificationScoreRole:
    case VisibilityRole:
    {
      QTE_D();

      auto* const sm = this->sourceModel();
      auto const& si = this->mapToSource(index);
      auto const& scd = sm->data(si, ClassificationRole).toHash();
      auto const cd = d->filter(scd);

      switch (role)
      {
        case ClassificationRole:
          return cd;

        case ClassificationTypeRole:
          return bestClassifier(cd).type;

        case ClassificationScoreRole:
          return bestClassifier(cd).score;

        default: // must be VisibilityRole
          if (scd.isEmpty() || !cd.isEmpty())
          {
            break; // fall through to return data from parent
          }
          return false;
      }
    };
    default:
      break;
  }

  return this->AbstractProxyModel::data(index, role);
}

// ----------------------------------------------------------------------------
bool ClassificationFilterModel::isValidData(
  QVariant const& bound, QString const& type) const
{
  return !type.isEmpty() && this->isValidData(bound, ClassificationScoreRole);
}

// ----------------------------------------------------------------------------
QVariantHash ClassificationFilterModelPrivate::filter(
  QVariantHash const& in) const
{
  auto out = QVariantHash{};

  for (auto const& iter : in | kvr::indirect())
  {
    auto const& key = iter.key();
    auto const& value = iter.value();

    if (auto const* f = qtGet(this->bounds, key))
    {
      if (!this->filter(value, *f))
      {
        continue;
      }
    }
    out.insert(key, value);
  }

  return out;
}

// ----------------------------------------------------------------------------
bool ClassificationFilterModelPrivate::filter(
  QVariant const& in, Bound const& bound)
{
  auto const score = in.toDouble();

  if (bound.first.isValid() && score < bound.first.toDouble())
  {
    return false;
  }

  if (bound.second.isValid() && bound.second.toDouble() < score)
  {
    return false;
  }

  return true;
}

} // namespace core

} // namespace sealtk
