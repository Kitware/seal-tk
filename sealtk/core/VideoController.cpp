/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoController.hpp>

#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoDistributor.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoControllerPrivate
{
public:
  void updateTimes();

  using time_t = kwiver::vital::timestamp::time_t;

  QHash<VideoSource*, VideoDistributor*> videoSources;
  time_t time = std::numeric_limits<time_t>::min();
  bool timeIsValid = false;

  TimeMap<std::nullptr_t> times;
  bool timesDirty = false;
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
VideoDistributor* VideoController::distributor(VideoSource* videoSource) const
{
  QTE_D();
  return d->videoSources.value(videoSource);
}

// ----------------------------------------------------------------------------
VideoDistributor* VideoController::addVideoSource(VideoSource* videoSource)
{
  QTE_D();

  if (auto* const existingDistributor = d->videoSources.value(videoSource))
  {
    return existingDistributor;
  }

  connect(videoSource, &VideoSource::framesChanged, this,
          [this, d]{
            d->timesDirty = true;

            if (!d->timeIsValid)
            {
              d->updateTimes();
              if (!d->times.isEmpty())
              {
                this->seek(d->times.begin().key());
              }
            }

            emit this->timesChanged();
          });

  connect(videoSource, &QObject::destroyed, this,
          [this, videoSource]{
            this->removeVideoSource(videoSource);
          });

  auto fetch = [d, videoSource](time_t t, qint64 i){
    auto distributor = d->videoSources.value(videoSource);
    Q_ASSERT(distributor);

    distributor->requestFrame(videoSource, t, SeekExact, i);
  };

  auto* const newDistributor = new VideoDistributor{this};

  auto const first = d->videoSources.isEmpty();
  d->videoSources.insert(videoSource, newDistributor);
  connect(this, &VideoController::timeSelected,
          videoSource, fetch);

  d->timesDirty = true;
  if (!first && d->timeIsValid)
  {
    fetch(d->time, -1);
  }

  videoSource->start();

  emit this->videoSourcesChanged();

  return newDistributor;
}

// ----------------------------------------------------------------------------
void VideoController::removeVideoSource(VideoSource* videoSource)
{
  QTE_D();

  if (auto* const distributor = d->videoSources.take(videoSource))
  {
    disconnect(videoSource, nullptr, this, nullptr);
    disconnect(this, nullptr, videoSource, nullptr);
    delete distributor;

    d->timesDirty = true;

    emit this->videoSourcesChanged();
    emit this->timesChanged();

    if (d->videoSources.isEmpty())
    {
      d->timeIsValid = false;
    }
  }
}

// ----------------------------------------------------------------------------
TimeMap<std::nullptr_t> VideoController::times()
{
  QTE_D();

  d->updateTimes();
  return d->times;
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

  if (!d->timeIsValid || time != d->time)
  {
    d->time = time;
    d->timeIsValid = true;
    emit this->timeSelected(time, requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoController::seekNearest(time_t time, qint64 requestId)
{
  QTE_D();

  d->updateTimes();

  auto it = d->times.find(time, SeekNearest);
  if (it != d->times.end())
  {
    this->seek(it.key(), requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoController::previousFrame(qint64 requestId)
{
  QTE_D();

  d->updateTimes();

  auto const& it = d->times.find(d->time, SeekPrevious);
  if (it != d->times.end())
  {
    this->seek(it.key(), requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoController::nextFrame(qint64 requestId)
{
  QTE_D();

  d->updateTimes();

  auto const& it = d->times.find(d->time, SeekNext);
  if (it != d->times.end())
  {
    this->seek(it.key(), requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoControllerPrivate::updateTimes()
{
  if (this->timesDirty)
  {
    this->times.clear();
    for (auto const& vsItem : qtEnumerate(this->videoSources))
    {
      this->times.unite(vsItem.key()->frames().keyMap());
    }
    this->timesDirty = false;
  }
}

} // namespace core

} // namespace sealtk
