/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_FusionModel_hpp
#define sealtk_gui_FusionModel_hpp

#include <sealtk/gui/Export.h>

#include <sealtk/core/AbstractItemModel.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

class FusionModelPrivate;

class SEALTK_GUI_EXPORT FusionModel : public sealtk::core::AbstractItemModel
{
  Q_OBJECT

public:
  FusionModel(QObject* parent = nullptr);
  ~FusionModel();

  // Reimplemented from QAbstractItemModel
  int rowCount(QModelIndex const& parent) const override;
  QVariant data(QModelIndex const& index, int role) const override;

public slots:
 void addModel(QAbstractItemModel*);
 void removeModel(QAbstractItemModel*);

protected:
  QTE_DECLARE_PRIVATE_PTR(FusionModel);

private:
  QTE_DECLARE_PRIVATE(FusionModel);
};

} // namespace gui

} // namespace sealtk

#endif
