/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverVideoSource.hpp>

#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoProvider.hpp>
#include <sealtk/core/VideoRequest.hpp>

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
    if (auto const& mdi = mdp->find(kwiver::vital::VITAL_META_IMAGE_URI))
    {
      return mdi.as_string();
    }
  }

  return {};
}

} // namespace <anonymous>

// ============================================================================
class KwiverVideoSourcePrivate : public VideoProvider
{
public:
  KwiverVideoSourcePrivate(KwiverVideoSource* q) : q_ptr{q} {}

  void initialize() override;
  kv::timestamp processRequest(
    VideoRequest&& request, kv::timestamp const& lastTime) override;

  kv::algo::video_input_sptr videoInput;
  TimeMap<frame_t> timestampMap;
  TimeMap<VideoMetaData> metaDataMap;

  // CAUTION: Members below this line are owned by the UI thread!
  TimeMap<frame_t> externalTimestampMap;
  TimeMap<VideoMetaData> externalMetaDataMap;
  bool ready = false;

private:
  QTE_DECLARE_PUBLIC_PTR(VideoSource)
  QTE_DECLARE_PUBLIC(VideoSource)
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverVideoSource)

// ----------------------------------------------------------------------------
KwiverVideoSource::KwiverVideoSource(
  kv::algo::video_input_sptr const& videoInput, QObject* parent)
  : KwiverVideoSource{new KwiverVideoSourcePrivate{this}, parent}
{
  QTE_D();
  d->videoInput = videoInput;
}

// ----------------------------------------------------------------------------
KwiverVideoSource::KwiverVideoSource(
  KwiverVideoSourcePrivate* d, QObject* parent)
  : VideoSource{d, parent}, d_ptr{d}
{
}

// ----------------------------------------------------------------------------
KwiverVideoSource::~KwiverVideoSource()
{
  this->cleanup();
}

// ----------------------------------------------------------------------------
bool KwiverVideoSource::isReady() const
{
  QTE_D();
  return d->ready;
}

// ----------------------------------------------------------------------------
TimeMap<frame_t> KwiverVideoSource::frames() const
{
  QTE_D();
  return d->externalTimestampMap;
}

// ----------------------------------------------------------------------------
TimeMap<VideoMetaData> KwiverVideoSource::metaData() const
{
  QTE_D();
  return d->externalMetaDataMap;
}

// ----------------------------------------------------------------------------
void KwiverVideoSourcePrivate::initialize()
{
  if (this->videoInput)
  {
    QTE_Q();

    kv::timestamp ts;

    for (auto const i : kvr::iota(videoInput->num_frames()))
    {
      auto const frame = static_cast<kv::timestamp::frame_t>(i) + 1;
      if (videoInput->seek_frame(ts, frame) && ts.has_valid_time())
      {
        Q_ASSERT(ts.has_valid_frame());
        Q_ASSERT(ts.get_frame() == frame);

        auto const& frameName =
          getImageName(this->videoInput->frame_metadata());
        auto md = VideoMetaData{ts, frameName};

        this->timestampMap.insert(ts.get_time_usec(), ts.get_frame());
        this->metaDataMap.insert(ts.get_time_usec(), md);
      }
    }

    QMetaObject::invokeMethod(
      q, [this, q, tsMap = this->timestampMap, mdMap = this->metaDataMap]{
        this->externalTimestampMap = tsMap;
        this->externalMetaDataMap = mdMap;
        this->ready = true;
        emit q->framesChanged();
      });
  }
}

// ----------------------------------------------------------------------------
kv::timestamp KwiverVideoSourcePrivate::processRequest(
  VideoRequest&& request, kv::timestamp const& lastTime)
{
  auto const iter = this->timestampMap.find(request.time, request.mode);
  if (iter != this->timestampMap.end())
  {
    if (lastTime.has_valid_time() &&
        lastTime.get_time_usec() == iter.key())
    {
      return {};
    }

    kv::timestamp ts;
    if (this->videoInput->seek_frame(ts, iter.value()))
    {
      Q_ASSERT(ts.has_valid_time());
      Q_ASSERT(ts.has_valid_frame());
      Q_ASSERT(ts.get_time_usec() == iter.key());
      Q_ASSERT(ts.get_frame() == iter.value());

      VideoFrame response;
      response.image = this->videoInput->frame_image();
      response.metaData.setTimeStamp(ts);
      response.metaData.setImageName(
        getImageName(this->videoInput->frame_metadata()));

      request.sendReply(std::move(response));

      return ts;
    }
    else
    {
      // This should never happen
      qWarning()
        << this << __func__
        << "underlying video source failed to seek to frame" << iter.value()
        << "with expected time" << iter.key();
      return {};
    }
  }

  return {};
}

} // namespace core

} // namespace sealtk
