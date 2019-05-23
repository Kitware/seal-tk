/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverVideoSource.hpp>

#include <sealtk/core/TimeMap.hpp>

#include <arrows/qt/image_container.h>

#include <vital/range/iota.h>
#include <vital/range/valid.h>

#include <QDebug>
#include <QHash>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using frame_t = kv::timestamp::frame_t;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
kv::path_t getImageName(kv::metadata_vector const& mdv)
{
  for (auto const& mdp : mdv | kvr::valid)
  {
    if ( auto const& mdi = mdp->find( kwiver::vital::VITAL_META_IMAGE_URI ) )
    {
      return mdi.as_string();
    }
  }

  return {};
}

} // namespace <anonymous>

// ============================================================================
class KwiverVideoSourcePrivate
{
public:
  kv::algo::video_input_sptr videoInput;
  TimeMap<frame_t> timestampMap;

  kv::image_container_sptr image;
  kv::timestamp timeStamp;
  kv::path_t imageName;

  QHash<frame_t, kv::detected_object_set_sptr> detectedObjectSets;

  void rebuildTimestampMap();
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverVideoSource)

// ----------------------------------------------------------------------------
KwiverVideoSource::KwiverVideoSource(QObject* parent)
  : VideoSource{parent},
    d_ptr{new KwiverVideoSourcePrivate}
{
}

// ----------------------------------------------------------------------------
KwiverVideoSource::~KwiverVideoSource()
{
}

// ----------------------------------------------------------------------------
kv::algo::video_input_sptr KwiverVideoSource::videoInput() const
{
  QTE_D();
  return d->videoInput;
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::setVideoInput(
  kv::algo::video_input_sptr const& videoInput)
{
  QTE_D();

  d->videoInput = videoInput;
  d->image = nullptr;
  d->timeStamp = {};
  d->imageName = {};

  d->rebuildTimestampMap();

  emit this->videoInputChanged();
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::setDetectedObjectSetInput(
  kv::algo::detected_object_set_input_sptr const& detectedObjectSetInput)
{
  QTE_D();
  d->detectedObjectSets.clear();

  if (detectedObjectSetInput)
  {
    auto set = kv::detected_object_set_sptr{};
    auto frame = frame_t{0};
    auto imageName = std::string{};

    while (detectedObjectSetInput->read_set(set, imageName))
    {
      if (set)
      {
        d->detectedObjectSets.insert(frame, set);
      }
      ++frame;
    }
  }

  this->invalidate();
}

// ----------------------------------------------------------------------------
TimeMap<frame_t> KwiverVideoSource::frames() const
{
  QTE_D();
  return d->timestampMap;
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::seek(kv::timestamp::time_t time, SeekMode mode)
{
  QTE_D();
  auto const iter = d->timestampMap.find(time, mode);
  if (iter != d->timestampMap.end())
  {
    if (d->videoInput->seek_frame(d->timeStamp, iter.value()))
    {
      Q_ASSERT(d->timeStamp.has_valid_time());
      Q_ASSERT(d->timeStamp.has_valid_frame());
      Q_ASSERT(d->timeStamp.get_time_usec() == iter.key());
      Q_ASSERT(d->timeStamp.get_frame() == iter.value());
      d->image = d->videoInput->frame_image();
      d->imageName = getImageName(d->videoInput->frame_metadata());
    }
    else
    {
      // This should never happen
      qWarning()
        << this << __func__
        << "underlying video source failed to seek to frame" << iter.value()
        << "with expected time" << iter.key();
      d->imageName = {};
      d->timeStamp = {};
      d->image = nullptr;
    }
  }
  else
  {
    d->imageName = {};
    d->timeStamp = {};
    d->image = nullptr;
  }

  this->invalidate();
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::seekFrame(frame_t frame)
{
  QTE_D();

  if (d->videoInput->seek_frame(d->timeStamp, frame))
  {
    Q_ASSERT(d->timeStamp.has_valid_frame());
    Q_ASSERT(d->timeStamp.get_frame() == frame);
    d->image = d->videoInput->frame_image();
    d->imageName = getImageName(d->videoInput->frame_metadata());
  }
  else
  {
    d->imageName = {};
    d->timeStamp = {};
    d->image = nullptr;
  }

  invalidate();
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::invalidate() const
{
  QTE_D();

  emit imageReady(d->image, VideoMetaData{d->timeStamp, d->imageName});

  if (d->timeStamp.has_valid_frame())
  {
    auto const& detections =
      d->detectedObjectSets.value(d->timeStamp.get_frame());
    emit detectionsReady(detections, d->timeStamp);
  }
  else
  {
    emit detectionsReady(nullptr, d->timeStamp);
  }
}

// ----------------------------------------------------------------------------
void KwiverVideoSourcePrivate::rebuildTimestampMap()
{
  this->timestampMap.clear();

  if (this->videoInput)
  {
    kv::timestamp ts;

    for (auto const i : kvr::iota(videoInput->num_frames()))
    {
      auto const frame = static_cast<kv::timestamp::frame_t>(i) + 1;
      if (videoInput->seek_frame(ts, frame) && ts.has_valid_time())
      {
        Q_ASSERT(ts.has_valid_frame());
        Q_ASSERT(ts.get_frame() == frame);
        timestampMap.insert(ts.get_time_usec(), ts.get_frame());
      }
    }
  }
}

}

}
