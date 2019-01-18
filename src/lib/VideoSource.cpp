/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "VideoSource.hpp"

#include <arrows/qt/image_container.h>

#include <map>

namespace sealtk
{

// ============================================================================
class VideoSourcePrivate
{
public:
  kwiver::vital::algo::video_input_sptr videoInput;
  std::map<kwiver::vital::timestamp::time_t, kwiver::vital::timestamp::frame_t>
    timestampMap;

  void rebuildTimestampMap();
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoSource)

// ----------------------------------------------------------------------------
VideoSource::VideoSource(QObject* parent)
  : QObject{parent},
    d_ptr{new VideoSourcePrivate}
{
  QTE_D();
}

// ----------------------------------------------------------------------------
VideoSource::~VideoSource()
{
}

// ----------------------------------------------------------------------------
kwiver::vital::algo::video_input_sptr VideoSource::videoInput() const
{
  QTE_D();
  return d->videoInput;
}

// ----------------------------------------------------------------------------
void VideoSource::setVideoInput(
  kwiver::vital::algo::video_input_sptr const& videoInput)
{
  QTE_D();
  d->videoInput = videoInput;
  d->rebuildTimestampMap();
  emit this->videoInputChanged();
}

// ----------------------------------------------------------------------------
std::set<kwiver::vital::timestamp::time_t> VideoSource::times() const
{
  QTE_D();
  std::set<kwiver::vital::timestamp::time_t> result;
  for (auto const& f : d->timestampMap)
  {
    result.insert(f.first);
  }
  return result;
}

// ----------------------------------------------------------------------------
void VideoSource::seek(kwiver::vital::timestamp::time_t time)
{
  QTE_D();
  auto it = d->timestampMap.find(time);
  if (it != d->timestampMap.end())
  {
    kwiver::vital::timestamp ts;
    if (d->videoInput->seek_frame(ts, it->second))
    {
      emit this->imageDisplayed(
        kwiver::arrows::qt::image_container::vital_to_qt(
          d->videoInput->frame_image()->get_image()));
    }
    else
    {
      emit this->imageDisplayed(QImage{});
    }
  }
  else
  {
    emit this->imageDisplayed(QImage{});
  }
}

// ----------------------------------------------------------------------------
void VideoSourcePrivate::rebuildTimestampMap()
{
  this->timestampMap.clear();
  if (this->videoInput)
  {
    kwiver::vital::timestamp ts;
    for (kwiver::vital::timestamp::frame_t i = 1;
        i <= this->videoInput->num_frames(); i++)
    {
      if (this->videoInput->seek_frame(ts, i) && ts.has_valid_time() &&
          ts.has_valid_frame())
      {
        this->timestampMap[ts.get_time_usec()] = ts.get_frame();
      }
    }
  }
}

}
