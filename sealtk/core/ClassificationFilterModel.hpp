/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_ClassificationFilterModel_hpp
#define sealtk_core_ClassificationFilterModel_hpp

#include <sealtk/core/AbstractProxyModel.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class ClassificationFilterModelPrivate;

/// High/low-pass filter for classification data.
///
/// This class provides filtering of a data model based on high- or low-pass
/// filters applied to individual classifiers of an item's classifier set.
/// Such filtering only affects an item's classification and (potentially) its
/// visibility.
///
/// Note that, unlike a "normal" filter, this does \em not actually reject
/// rows, but rather modifies the VisibilityRole data of the underlying model.
class SEALTK_CORE_EXPORT ClassificationFilterModel : public AbstractProxyModel
{
  Q_OBJECT

public:
  explicit ClassificationFilterModel(QObject* parent = nullptr);
  ~ClassificationFilterModel() override;

  QVariant data(QModelIndex const& index, int role) const override;

  /// Return set of types which have active filters.
  QList<QString> types() const;

  QVariant lowerBound(QString const& type) const;
  QVariant upperBound(QString const& type) const;
  QPair<QVariant, QVariant> bound(QString const& type) const;

public slots:
  void setLowerBound(QString const& type, QVariant const& bound);
  void setUpperBound(QString const& type, QVariant const& bound);
  void setBound(
    QString const& type, QVariant const& lower, QVariant const& upper);

  void clearLowerBound(QString const& type);
  void clearUpperBound(QString const& type);
  void clearBound(QString const& type);
  void clearBounds();

protected:
  using AbstractProxyModel::isValidData;

  QTE_DECLARE_PRIVATE_PTR(ClassificationFilterModel)

private:
  bool isValidData(QVariant const& bound, QString const& type) const;

  QTE_DECLARE_PRIVATE(ClassificationFilterModel)
};

} // namespace core

} // namespace sealtk

#endif
