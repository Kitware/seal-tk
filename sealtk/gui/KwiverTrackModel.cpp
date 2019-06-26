/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/KwiverTrackModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/UnsharedPointer.hpp>

#include <vital/types/object_track_set.h>
#include <vital/types/track.h>

#include <vital/range/transform.h>
#include <vital/range/valid.h>

#include <qtScopedValueChange.h>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

namespace // anonymous
{

// ============================================================================
struct KwiverTrack
{
  KwiverTrack() = default;
  KwiverTrack(KwiverTrack&&) = default;
  KwiverTrack(KwiverTrack const& other) = default;
  KwiverTrack(kv::track_sptr&& object);

  core::UnsharedPointer<kv::track> track;
  bool visible = true;
};

// ----------------------------------------------------------------------------
kv::track_sptr cleanTrack(kv::track_sptr const& in)
{
  // Check that we have a track at all
  if (!in)
  {
    return nullptr;
  }

  // Check if the track has only object track states
  auto clean = true;
  for (auto const& s : *in | kv::as_object_track)
  {
    if (!s)
    {
      clean = false;
      break;
    }
  }

  if (clean)
  {
    return in;
  }

  // Create a cleaned track using only the object track states
  auto out = kv::track::create();
  out->set_id(in->id());

  for (auto const& s : *in | kv::as_object_track | kvr::valid)
  {
    out->append(s->clone());
  }

  return out;
}

} // namespace <anonymous>

// ============================================================================
class KwiverTrackModelData : public QSharedData
{
public:
  std::vector<KwiverTrack> tracks;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC_SHARED(KwiverTrackModel)

// ----------------------------------------------------------------------------
KwiverTrackModel::KwiverTrackModel(QObject* parent)
  : AbstractItemModel{parent}, d_ptr{new KwiverTrackModelData}
{
}

// ----------------------------------------------------------------------------
KwiverTrackModel::KwiverTrackModel(
  QSharedDataPointer<KwiverTrackModelData> const& d, QObject* parent)
  : AbstractItemModel{parent}, d_ptr{d}
{
}

// ----------------------------------------------------------------------------
KwiverTrackModel::~KwiverTrackModel()
{
}

// ----------------------------------------------------------------------------
int KwiverTrackModel::rowCount(QModelIndex const& parent) const
{
  QTE_D_SHARED();

  if (parent.isValid())
  {
    // TODO detections?
    return 0;
  }

  return static_cast<int>(d->tracks.size());
}

// ----------------------------------------------------------------------------
QVariant KwiverTrackModel::data(QModelIndex const& index, int role) const
{
  // TODO detections?
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    QTE_D_SHARED();

    auto const& track = d->tracks[static_cast<unsigned>(index.row())];
    switch (role)
    {
      case core::NameRole:
      case core::LogicalIdentityRole:
        return static_cast<qint64>(track.track->id());

      case core::StartTimeRole:
        if (!track.track->empty())
        {
          auto const& s =
            std::static_pointer_cast<kv::object_track_state>(
              track.track->front());
          return QVariant::fromValue(s->time());
        }
        return {};

      case core::EndTimeRole:
        if (!track.track->empty())
        {
          auto const& s =
            std::static_pointer_cast<kv::object_track_state>(
              track.track->back());
          return QVariant::fromValue(s->time());
        }
        return {};

      case core::UserVisibilityRole:
        return track.visible;

      default:
        break;
    }
  }

  return AbstractItemModel::data(index, role);
}

// ----------------------------------------------------------------------------
void KwiverTrackModel::addTracks(
  kv::object_track_set_sptr const& trackSet)
{
  QVector<kv::track_sptr> newTracks;

  if (trackSet)
  {
    auto const& tracks = trackSet->tracks();
    for (auto&& track : tracks | kvr::transform(cleanTrack) | kvr::valid)
    {
      newTracks.append(std::move(track));
    }
  }

  if (!newTracks.isEmpty())
  {
    QTE_D_DETACH();

    auto const oldRows = this->rowCount({});
    auto const newRows = newTracks.size();

    this->beginInsertRows({}, oldRows, newRows);

    for (auto& track : newTracks)
    {
      d->tracks.emplace_back(std::move(track));
    }

    this->endInsertRows();
  }
}

// ----------------------------------------------------------------------------
void KwiverTrackModel::setTracks(
  kv::object_track_set_sptr const& trackSet)
{
  this->beginResetModel();

  with_expr(qtScopedBlockSignals{this})
  {
    this->clear();
    this->addTracks(trackSet);
  }

  this->endResetModel();
}

// ----------------------------------------------------------------------------
void KwiverTrackModel::clear()
{
  if (this->rowCount({}))
  {
    this->beginResetModel();
    this->d_ptr = new KwiverTrackModelData;
    this->endResetModel();
  }
}

// ----------------------------------------------------------------------------
KwiverTrack::KwiverTrack(kv::track_sptr&& track)
  : track{std::move(track)}
{
}

} // namespace gui

} // namespace sealtk
