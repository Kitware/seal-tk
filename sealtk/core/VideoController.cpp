/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoController.hpp>

#include <sealtk/core/TimeMap.hpp>
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
  QSet<VideoSource*> videoSources;
  kwiver::vital::timestamp::time_t time;

  void rebuildTimes();
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
  return d->videoSources;
}

// ----------------------------------------------------------------------------
void VideoController::addVideoSource(VideoSource* videoSource)
{
  QTE_D();

  using Time = kwiver::vital::timestamp::time_t;

  if (!d->videoSources.contains(videoSource))
  {
    d->videoSources.insert(videoSource);
    connect(this, &VideoController::timeSelected,
            videoSource, &VideoSource::seekTime);

    if (d->videoSources.size() == 1)
    {
      auto const& frames = videoSource->frames();
      if (!frames.empty())
      {
        seek(frames.begin().key());
      }
    }
    else
    {
      videoSource->seekTime(d->time);
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
  QSet<kwiver::vital::timestamp::time_t> result;

  for (auto* vs : d->videoSources)
  {
    result.unite(vs->frames().keySet());
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
void VideoController::seek(kwiver::vital::timestamp::time_t time)
{
  QTE_D();
  if (time != d->time)
  {
    d->time = time;
    emit this->timeSelected(time);
  }
}

// ----------------------------------------------------------------------------
void VideoController::seekNearest(kwiver::vital::timestamp::time_t time)
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
    emit this->timeSelected(time);
  }
}

// ----------------------------------------------------------------------------
void VideoController::previousFrame()
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
      emit this->timeSelected(time);
    }
  }
}

// ----------------------------------------------------------------------------
void VideoController::nextFrame()
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
      emit this->timeSelected(time);
    }
  }
}

// ----------------------------------------------------------------------------
void VideoControllerPrivate::rebuildTimes()
{
}

}

}
