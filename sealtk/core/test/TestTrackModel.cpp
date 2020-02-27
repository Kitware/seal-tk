/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestTrackModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>

namespace sealtk
{

namespace core
{

namespace test
{

// ----------------------------------------------------------------------------
SimpleTrackModel::SimpleTrackModel(QVector<TimeMap<TrackState>> data)
  : rowData{data}
{
}

// ----------------------------------------------------------------------------
void SimpleTrackModel::setFirstId(qint64 id)
{
  this->firstId = id;
}

// ----------------------------------------------------------------------------
int SimpleTrackModel::rowCount(QModelIndex const& parent) const
{
  if (parent.isValid())
  {
    auto const pr = parent.row();
    if (pr >= 0 && pr < this->rowData.count())
    {
      return this->rowData[pr].count();
    }
    return 0;
  }
  else
  {
    return this->rowData.count();
  }
}

// ----------------------------------------------------------------------------
QModelIndex SimpleTrackModel::index(
  int row, int column, QModelIndex const& parent) const
{
  auto const pr =
    static_cast<unsigned>(parent.isValid() ? parent.row() + 1 : 0);

  return this->createIndex(row, column, pr);
}

// ----------------------------------------------------------------------------
QModelIndex SimpleTrackModel::parent(QModelIndex const& index) const
{
  auto const parentRow = static_cast<int>(index.internalId()) - 1;
  return (parentRow >= 0 ? this->createIndex(parentRow, 0) : QModelIndex{});
}

// ----------------------------------------------------------------------------
QVariant SimpleTrackModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid))
  {
    auto const& p = index.parent();
    if (p.isValid())
    {
      auto const pr = p.row();
      auto const& track = this->rowData[pr];

      switch (role)
      {
        case LogicalIdentityRole:
          return this->firstId + pr;

        case StartTimeRole:
        case EndTimeRole:
          return QVariant::fromValue(track.keys()[index.row()]);

        case AreaLocationRole:
          return track.values()[index.row()].location;

        case ClassificationRole:
          return track.values()[index.row()].classification;

        default:
          break;
      }
    }
    else if (role == LogicalIdentityRole)
    {
      return this->firstId + index.row();
    }
  }

  return AbstractItemModel::data(index, role);
}

} // namespace test

} // namespace core

} // namespace sealtk
