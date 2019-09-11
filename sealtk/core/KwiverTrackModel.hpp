/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverTrackModel_hpp
#define sealtk_core_KwiverTrackModel_hpp

#include <sealtk/core/AbstractItemModel.hpp>

#include <vital/types/object_track_set.h>

#include <qtGlobal.h>

#include <QSharedDataPointer>

namespace sealtk
{

namespace core
{

class KwiverTrackModelData;

class SEALTK_CORE_EXPORT KwiverTrackModel : public AbstractItemModel
{
  Q_OBJECT

public:
  KwiverTrackModel(QObject* parent = nullptr);
  ~KwiverTrackModel();

  // Reimplemented from QAbstractItemModel
  int rowCount(QModelIndex const& parent = {}) const override;
  QModelIndex parent(QModelIndex const& child) const override;
  QModelIndex index(
    int row, int column, QModelIndex const& parent) const override;

  QVariant data(QModelIndex const& index, int role) const override;
  bool setData(
    QModelIndex const& index, QVariant const& value, int role) override;

public slots:
  void clear();

  void addTracks(kwiver::vital::object_track_set_sptr const& trackSet);
  void setTracks(kwiver::vital::object_track_set_sptr const& trackSet);

protected:
  KwiverTrackModel(
    QSharedDataPointer<KwiverTrackModelData> const& d,
    QObject* parent = nullptr);

  QTE_DECLARE_SHARED_PTR(KwiverTrackModel);

private:
  QTE_DECLARE_SHARED(KwiverTrackModel);
};

} // namespace core

} // namespace sealtk

#endif
