/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoController.hpp>

#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <QSet>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoControllerPrivate
{
public:
  using time_t = kwiver::vital::timestamp::time_t;

  QHash<VideoSource*, std::shared_ptr<VideoRequestor>> videoSources;
  time_t time = std::numeric_limits<time_t>::min();
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoController)

// ----------------------------------------------------------------------------
VideoController::VideoController(QObject* parent)
  : QObject{parent},
    d_ptr{new VideoControllerPrivate}
{
}

// ----------------------------------------------------------------------------
VideoController::~VideoController()
{
}

// ----------------------------------------------------------------------------
QSet<VideoSource*> VideoController::videoSources() const
{
  QTE_D();

  auto out = QSet<VideoSource*>{};
  out.reserve(d->videoSources.size());

  for (auto const& item : qtEnumerate(d->videoSources))
  {
    out.insert(item.key());
  }

  return out;
}

// ----------------------------------------------------------------------------
void VideoController::addVideoSource(
  VideoSource* videoSource, std::shared_ptr<VideoRequestor> const& requestor)
{
  QTE_D();

  if (!d->videoSources.contains(videoSource))
  {
    connect(videoSource, &QObject::destroyed, this,
            [this, videoSource]{
              this->removeVideoSource(videoSource);
            });

    auto fetch = [d, videoSource](time_t t, qint64 i){
      auto requestor = d->videoSources.value(videoSource);
      Q_ASSERT(requestor);

      VideoRequest request;
      request.requestId = i;
      request.requestor = std::move(requestor);
      request.time = t;
      request.mode = SeekExact;

      videoSource->requestFrame(std::move(request));
    };

    auto const first = d->videoSources.isEmpty();
    d->videoSources.insert(videoSource, requestor);
    connect(this, &VideoController::timeSelected,
            videoSource, fetch);

    if (first)
    {
      auto const& frames = videoSource->frames();
      if (!frames.empty())
      {
        this->seek(frames.begin().key());
      }
    }
    else
    {
      fetch(d->time, -1);
    }

    emit this->videoSourcesChanged();
  }
}

// ----------------------------------------------------------------------------
void VideoController::removeVideoSource(VideoSource* videoSource)
{
  QTE_D();
  if (d->videoSources.remove(videoSource))
  {
    disconnect(this, nullptr, videoSource, nullptr);
    emit this->videoSourcesChanged();
  }
}

// ----------------------------------------------------------------------------
QSet<kwiver::vital::timestamp::time_t> VideoController::times() const
{
  QTE_D();
  QSet<time_t> result;

  for (auto const& vsItem : qtEnumerate(d->videoSources))
  {
    result.unite(vsItem.key()->frames().keySet());
  }

  return result;
}

// ----------------------------------------------------------------------------
kwiver::vital::timestamp::time_t VideoController::time() const
{
  QTE_D();
  return d->time;
}

// ----------------------------------------------------------------------------
void VideoController::seek(time_t time, qint64 requestId)
{
  QTE_D();
  if (time != d->time)
  {
    d->time = time;
    emit this->timeSelected(time, requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoController::seekNearest(time_t time, qint64 requestId)
{
  QTE_D();
  TimeMap<void*> times;
  for (auto t : this->times())
  {
    times[t] = nullptr;
  }
  auto it = times.find(time, SeekNearest);
  if (it != times.end())
  {
    time = it.key();
  }
  if (time != d->time)
  {
    d->time = time;
    emit this->timeSelected(time, requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoController::previousFrame(qint64 requestId)
{
  QTE_D();
  TimeMap<void*> times;
  for (auto t : this->times())
  {
    times[t] = nullptr;
  }
  auto it = times.find(d->time, SeekPrevious);
  if (it != times.end())
  {
    auto time = it.key();
    if (time != d->time)
    {
      d->time = time;
      emit this->timeSelected(time, requestId);
    }
  }
}

// ----------------------------------------------------------------------------
void VideoController::nextFrame(qint64 requestId)
{
  QTE_D();
  TimeMap<void*> times;
  for (auto t : this->times())
  {
    times[t] = nullptr;
  }
  auto it = times.find(d->time, SeekNext);
  if (it != times.end())
  {
    auto time = it.key();
    if (time != d->time)
    {
      d->time = time;
      emit this->timeSelected(time, requestId);
    }
  }
}

}

}
