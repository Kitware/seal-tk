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
  if (!d->videoSources.contains(videoSource))
  {
    d->videoSources.insert(videoSource);
    connect(this, &VideoController::timeSelected,
            videoSource, &VideoSource::seek);
    if (d->videoSources.size() == 1)
    {
      auto times = videoSource->times();
      auto it = times.begin();
      if (it != times.end())
      {
        auto min = *it;
        while (++it != times.end())
        {
          if (*it < min)
          {
            min = *it;
          }
        }
        this->seek(min);
      }
    }
    else
    {
      videoSource->seek(d->time);
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
    disconnect(this, &VideoController::timeSelected,
               videoSource, &VideoSource::seek);
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
    result.unite(vs->times());
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
void VideoControllerPrivate::rebuildTimes()
{
}

}

}
