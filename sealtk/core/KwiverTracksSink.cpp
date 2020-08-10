/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverTracksSink.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/IdentityTransform.hpp>
#include <sealtk/core/TrackUtils.hpp>
#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoMetaData.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <vital/algo/write_object_track_set.h>

#include <vital/range/indirect.h>
#include <vital/range/iota.h>

#include <qtGet.h>
#include <qtStlUtil.h>

#include <QAbstractItemModel>
#include <QPolygonF>
#include <QUrl>
#include <QUrlQuery>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

// ============================================================================
class KwiverTracksSinkPrivate
{
public:
  QPointF transformPoint(
    QPointF const& in, kv::transform_2d_sptr const& transform) const;
  kv::detected_object_sptr makeDetection(
    QAbstractItemModel* model, QModelIndex const& index,
    kv::transform_2d_sptr const& transform) const;

  struct Frame
  {
    kv::path_t name;
    kv::frame_id_t frameNumber;
    std::unordered_map<qint64, kv::track_state_sptr> trackStates;
  };

  QMap<kv::timestamp::time_t, Frame> frames;
  kv::transform_2d_sptr transform;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverTracksSink)

// ----------------------------------------------------------------------------
KwiverTracksSink::KwiverTracksSink(QObject* parent)
  : AbstractDataSink{parent}, d_ptr{new KwiverTracksSinkPrivate}
{
}

// ----------------------------------------------------------------------------
KwiverTracksSink::~KwiverTracksSink()
{
}

// ----------------------------------------------------------------------------
bool KwiverTracksSink::setData(
  VideoSource* video, QAbstractItemModel* model, bool includeHidden)
{
  QTE_D();

  Q_ASSERT(video);

  d->frames.clear();
  d->transform = std::make_shared<IdentityTransform>();

  // Extract frame names from video
  for (auto const& md : video->metaData() | kvr::indirect)
  {
    auto const& mdv = md.value();
    auto const frameNumber = mdv.timeStamp().get_frame();
    d->frames.insert(md.key(), {mdv.imageName(), frameNumber, {}});
  }

  // Extract tracks (if any) as "supplemental data", exploiting that we know
  // that d->transform is an identity transform at time of call
  return this->addData(model, d->transform, includeHidden);
}

// ----------------------------------------------------------------------------
bool KwiverTracksSink::setTransform(kv::transform_2d_sptr const& transform)
{
  QTE_D();

  try
  {
    d->transform = (transform ? transform->inverse() : nullptr);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

// ----------------------------------------------------------------------------
bool KwiverTracksSink::addData(
  QAbstractItemModel* model, kv::transform_2d_sptr const& transform,
  bool includeHidden)
{
  QTE_D();

  auto haveData = false;

  if (model && transform && d->transform)
  {

    // Iterate over all items in data model
    for (auto const i : kvr::iota(model->rowCount()))
    {
      auto const iIndex = model->index(i, 0);

      if (!includeHidden)
      {
        // Skip tracks which are not visible
        if (!model->data(iIndex, VisibilityRole).toBool())
        {
          continue;
        }
      }

      auto const& idData = model->data(iIndex, LogicalIdentityRole);
      auto const id = idData.value<qint64>();

      for (auto const j : kvr::iota(model->rowCount(iIndex)))
      {
        // Get detection (track state) information
        auto const jIndex = model->index(j, 0, iIndex);
        auto const& timeData = model->data(jIndex, StartTimeRole);
        auto const t = timeData.value<kv::timestamp::time_t>();

        if (!includeHidden)
        {
          // Skip detections which are not visible
          if (!model->data(jIndex, VisibilityRole).toBool())
          {
            continue;
          }
        }

        // Look up entry in frame map
        if (auto* const frame = qtGet(d->frames, t))
        {
          // Create track state
          auto const f = frame->frameNumber;
          auto detection = d->makeDetection(model, jIndex, transform);
          auto state = createTrackState(f, t, std::move(detection));

          // Add state to map
          frame->trackStates.emplace(id, std::move(state));
          haveData = true;
        }
      }
    }
  }

  return haveData;
}

// ----------------------------------------------------------------------------
void KwiverTracksSink::writeData(QUrl const& uri) const
{
  QTE_D();

  auto config = kv::config_block::empty_config();
  auto params = QUrlQuery{uri};
  for (auto const& p : params.queryItems())
  {
    config->set_value(stdString(p.first), stdString(p.second));
  }

  try
  {
    // Create algorithm to write detections
    kva::write_object_track_set_sptr writer;
    kva::write_object_track_set::set_nested_algo_configuration(
      "output", config, writer);

    if (!writer)
    {
      emit this->failed(
        QStringLiteral("KwiverTracksSink::writeData: "
                       "Writer could not be configured"));
      return;
    }

    writer->open(stdString(uri.toLocalFile()));

    auto tracks = std::unordered_map<qint64, kv::track_sptr>{};
    auto trackSet = std::make_shared<kv::object_track_set>();

    // Iterate over frames
    for (auto const& fi : d->frames | kvr::indirect)
    {
      auto const& frame = fi.value();
      auto const timeStamp = kv::timestamp{fi.key(), frame.frameNumber};

      // Update tracks
      for (auto const& si : frame.trackStates)
      {
        // Get track for state, creating a new one if necessary
        auto& track = tracks[si.first];
        if (!track)
        {
          track = kv::track::create();
          track->set_id(si.first);
          trackSet->insert(track);
        }

        // Set frame on track state
        si.second->set_frame(frame.frameNumber);

        // Update track
        track->append(si.second);
        trackSet->notify_new_state(si.second);
      }

      // Write tracks at current frame
      writer->write_set(trackSet, timeStamp, frame.name);
    }

    writer->close();
  }
  catch (std::exception const& e)
  {
    emit this->failed(QString::fromLocal8Bit(e.what()));
  }
}

// ----------------------------------------------------------------------------
QPointF KwiverTracksSinkPrivate::transformPoint(
  QPointF const& in, kv::transform_2d_sptr const& transform) const
{
  auto const& worldPoint = transform->map({in.x(), in.y()});
  auto const& out = this->transform->map(worldPoint);
  return {out.x(), out.y()};
}

// ----------------------------------------------------------------------------
kv::detected_object_sptr KwiverTracksSinkPrivate::makeDetection(
  QAbstractItemModel* model, QModelIndex const& index,
  kv::transform_2d_sptr const& transform) const
{
  auto const& originalBox = model->data(index, AreaLocationRole).toRectF();
  auto const& type = model->data(index, ClassificationRole).toHash();
  auto const& notes = model->data(index, NotesRole).toStringList();

  auto poly = QPolygonF{};
  poly.append(this->transformPoint(originalBox.topLeft(), transform));
  poly.append(this->transformPoint(originalBox.topRight(), transform));
  poly.append(this->transformPoint(originalBox.bottomLeft(), transform));
  poly.append(this->transformPoint(originalBox.bottomRight(), transform));
  auto const& detection = createDetection(poly.boundingRect(), type, notes);

  return detection;
}

} // namespace core

} // namespace sealtk
