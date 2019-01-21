/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoController.hpp>

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
QSet<VideoSource*> const& VideoController::videoSources() const
{
  QTE_D();
  return d->videoSources;
}

// ----------------------------------------------------------------------------
void VideoController::addVideoSource(VideoSource* videoSource)
{
  QTE_D();
  d->videoSources.insert(videoSource);

  connect(this, &VideoController::timestampSelected,
          videoSource, &VideoSource::seek);

  emit this->videoSourcesChanged();
}

// ----------------------------------------------------------------------------
void VideoController::removeVideoSource(VideoSource* videoSource)
{
  QTE_D();
  d->videoSources.remove(videoSource);

  disconnect(this, &VideoController::timestampSelected,
             videoSource, &VideoSource::seek);

  emit this->videoSourcesChanged();
}

// ----------------------------------------------------------------------------
void VideoController::seek(kwiver::vital::timestamp::time_t time)
{
  emit this->timestampSelected(time);
}

}

}
