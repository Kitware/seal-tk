/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_ScalarFilterModel_hpp
#define sealtk_core_ScalarFilterModel_hpp

#include <sealtk/core/AbstractProxyModel.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class ScalarFilterModelPrivate;

/// Generic high/low-pass filter for scalar data.
///
/// This class provides simple filtering of a data model based on high- or
/// low-pass filters applied to scalar data. Any number of such filters may be
/// configured.
///
/// Note that, unlike a "normal" filter, this does \em not actually reject
/// rows, but rather modifies the VisibilityRole data of the underlying model.
class SEALTK_CORE_EXPORT ScalarFilterModel : public AbstractProxyModel
{
  Q_OBJECT

public:
  explicit ScalarFilterModel(QObject* parent = nullptr);
  ~ScalarFilterModel() override;

  QVariant data(QModelIndex const& index, int role) const override;

public slots:
  void setLowerBound(int role, QVariant const& bound);
  void setUpperBound(int role, QVariant const& bound);
  void setBound(int role, QVariant const& lower, QVariant const& upper);

  void clearLowerBound(int role);
  void clearUpperBound(int role);
  void clearBound(int role);
  void clearBounds();

protected:
  QTE_DECLARE_PRIVATE_PTR(ScalarFilterModel)

private:
  QTE_DECLARE_PRIVATE(ScalarFilterModel)
};

} // namespace core

} // namespace sealtk

#endif
