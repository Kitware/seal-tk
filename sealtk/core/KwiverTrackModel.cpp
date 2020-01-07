/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverTrackModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/TrackUtils.hpp>
#include <sealtk/core/UnsharedPointer.hpp>

#include <vital/types/object_track_set.h>
#include <vital/types/track.h>

#include <vital/range/indirect.h>
#include <vital/range/transform.h>
#include <vital/range/valid.h>

#include <qtGet.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include <QRectF>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
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

// ============================================================================
struct Classifier
{
  QVariant type;  // nominally QString
  QVariant score; // nominally double
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
    out->append(s->clone(kv::clone_type::SHALLOW));
  }

  return out;
}

// ----------------------------------------------------------------------------
Classifier bestClassifier(std::shared_ptr<kv::object_track_state> const& state)
{
  if (auto const& detection = state->detection())
  {
    if (auto const& c = detection->type())
    {
      try
      {
        std::string type;
        double confidence;

        c->get_most_likely(type, confidence);
        return {qtString(type), confidence};
      }
      catch (...)
      {
      }
    }
  }

  return {};
}

// ----------------------------------------------------------------------------
QVariant fullClassifier(std::shared_ptr<kv::object_track_state> const& state)
{
  if (auto const& detection = state->detection())
  {
    if (auto const& c = detection->type())
    {
      QVariantHash classifier;
      for (auto const& s : *c)
      {
        classifier.insert(qtString(*s.first), s.second);
      }
      return classifier;
    }
  }

  return {};
}

} // namespace <anonymous>

// ============================================================================
class KwiverTrackModelData : public QSharedData
{
public:
  QHash<kv::track_id_t, size_t> trackMap;
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
    QTE_D();

    auto const pr = parent.row();
    if (pr < 0 || pr >= static_cast<int>(d->tracks.size()))
    {
      return 0;
    }

    auto const pru = static_cast<uint>(pr);
    return static_cast<int>(d->tracks[pru].track->size());
  }

  return static_cast<int>(d->tracks.size());
}

// ----------------------------------------------------------------------------
QModelIndex KwiverTrackModel::index(
  int row, int column, QModelIndex const& parent) const
{
  if (parent.isValid())
  {
    auto const data = static_cast<uint>(parent.row() + 1);
    return this->createIndex(row, column, data);
  }

  return this->createIndex(row, column);
}

// ----------------------------------------------------------------------------
QModelIndex KwiverTrackModel::parent(QModelIndex const& child) const
{
  if (auto const r = child.internalId())
  {
    return this->createIndex(static_cast<int>(r - 1), 0);
  }
  return {};
}

// ----------------------------------------------------------------------------
QVariant KwiverTrackModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid))
  {
    QTE_D_SHARED();

    if (auto const pr = static_cast<int>(index.internalId()))
    {
      auto const& track = d->tracks[static_cast<uint>(pr - 1)];
      auto const& state =
        std::static_pointer_cast<kv::object_track_state>(
          *(track.track->begin() + index.row()));

      switch (role)
      {
        case core::NameRole:
        case core::LogicalIdentityRole:
          return static_cast<qint64>(track.track->id());

        case core::StartTimeRole:
        case core::EndTimeRole:
          return QVariant::fromValue(state->time());

        case AreaLocationRole:
          if (auto const& detection = state->detection())
          {
            auto const& bb = detection->bounding_box();
            return QRectF{bb.min_x(), bb.min_y(), bb.width(), bb.height()};
          }
          return {};

        case ClassificationTypeRole:
          return bestClassifier(state).type;

        case ClassificationScoreRole:
          return bestClassifier(state).score;

        case ClassificationRole:
          return fullClassifier(state);

        case core::UserVisibilityRole:
          return track.visible;

        default:
          break;
      }
    }
    else
    {
      auto const& track = d->tracks[static_cast<uint>(index.row())];

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

        case ClassificationTypeRole:
        case ClassificationScoreRole:
        case ClassificationRole:
          if (!track.track->empty())
          {
            auto const& s =
              std::static_pointer_cast<kv::object_track_state>(
                track.track->back());
            switch (role)
            {
              case ClassificationTypeRole:  return bestClassifier(s).type;
              case ClassificationScoreRole: return bestClassifier(s).score;
              case ClassificationRole:      return fullClassifier(s);
              default: Q_UNREACHABLE();
            }
          }
          return {};

        case core::UserVisibilityRole:
          return track.visible;

        default:
          break;
      }
    }
  }

  return this->AbstractItemModel::data(index, role);
}

// ----------------------------------------------------------------------------
bool KwiverTrackModel::setData(
  QModelIndex const& index, QVariant const& value, int role)
{
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    QTE_D_DETACH();

    auto& track = d->tracks[static_cast<uint>(index.row())];
    switch (role)
    {
      case core::ClassificationRole:
        if (value.canConvert<QVariantHash>())
        {
          auto const dot = classificationToDetectedObjectType(value.toHash());
          for (auto const& s : *track.track | kv::as_object_track)
          {
            if (auto const& detection = s->detection())
            {
              detection->set_type(dot);
            }
          }

          auto const& canonicalIndex = this->createIndex(index.row(), 0);
          emit this->dataChanged(canonicalIndex, canonicalIndex, {role});
        }

      case core::UserVisibilityRole:
        if (value.canConvert<bool>())
        {
          track.visible = value.toBool();

          auto const& canonicalIndex = this->createIndex(index.row(), 0);
          emit this->dataChanged(canonicalIndex, canonicalIndex,
                                 {role, core::VisibilityRole});
        }

      default:
        break;
    }
  }

  return this->AbstractItemModel::setData(index, value, role);
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

  this->addTracks(std::move(newTracks));
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
void KwiverTrackModel::mergeTracks(
  kv::object_track_set_sptr const& trackSet)
{
  QTE_D_SHARED();

  QVector<kv::track_sptr> newTracks;

  if (trackSet)
  {
    auto const& tracks = trackSet->tracks();
    for (auto&& track : tracks | kvr::transform(cleanTrack) | kvr::valid)
    {
      auto const id = track->id();

      auto* const row = qtGet(d->trackMap, id);
      if (row)
      {
        this->mergeTracks(*row, *track);
      }
      else
      {
        newTracks.append(std::move(track));
      }
    }
  }

  this->addTracks(std::move(newTracks));
}

// ----------------------------------------------------------------------------
void KwiverTrackModel::addTracks(QVector<kv::track_sptr>&& tracks)
{
  if (!tracks.isEmpty())
  {
    QTE_D_DETACH();

    auto const oldRows = this->rowCount({});
    auto const newRows = tracks.size();

    this->beginInsertRows({}, oldRows, oldRows + newRows - 1);

    for (auto&& track : tracks)
    {
      d->trackMap.insert(track->id(), d->tracks.size());
      d->tracks.emplace_back(std::move(track));
    }

    this->endInsertRows();
  }
}

// ----------------------------------------------------------------------------
void KwiverTrackModel::mergeTracks(
  size_t existingTrackIndex, kv::track const& track)
{
  QTE_D_DETACH();

  auto const& parent = this->index(static_cast<int>(existingTrackIndex), 0);
  auto& existingTrack = *(d->tracks[existingTrackIndex].track);

  // TODO properly merge; right now this just appends states
  auto iter = track.begin();
  auto end = track.end();

  auto const lastFrame = existingTrack.back()->frame();
  while (iter != end && (*iter)->frame() <= lastFrame)
  {
    ++iter;
  }

  if (iter != end)
  {
    auto const k = static_cast<int>(existingTrack.size());
    this->beginInsertRows(parent, k, k + static_cast<int>(end - iter) - 1);

    while (iter != end)
    {
      existingTrack.append((*iter)->clone(kv::clone_type::SHALLOW));
      ++iter;
    }

    this->endInsertRows();

    emit this->dataChanged(parent, parent);
  }
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

} // namespace core

} // namespace sealtk
