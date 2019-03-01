/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverVideoSource.hpp>

#include <arrows/qt/image_container.h>

#include <map>
#include <vector>

namespace sealtk
{

namespace core
{

// ============================================================================
class KwiverVideoSourcePrivate
{
public:
  kwiver::vital::algo::video_input_sptr videoInput;
  std::map<kwiver::vital::timestamp::time_t, kwiver::vital::timestamp::frame_t>
    timestampMap;
  kwiver::vital::image_container_sptr image;
  kwiver::vital::timestamp::frame_t frame;
  std::vector<kwiver::vital::detected_object_set_sptr> detectedObjectSets;

  void rebuildTimestampMap();
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverVideoSource)

// ----------------------------------------------------------------------------
KwiverVideoSource::KwiverVideoSource(QObject* parent)
  : VideoSource{parent},
    d_ptr{new KwiverVideoSourcePrivate}
{
  QTE_D();
  d->frame = 1;
}

// ----------------------------------------------------------------------------
KwiverVideoSource::~KwiverVideoSource()
{
}

// ----------------------------------------------------------------------------
kwiver::vital::algo::video_input_sptr KwiverVideoSource::videoInput() const
{
  QTE_D();
  return d->videoInput;
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::setVideoInput(
  kwiver::vital::algo::video_input_sptr const& videoInput)
{
  QTE_D();
  d->videoInput = videoInput;
  d->rebuildTimestampMap();
  emit this->videoInputChanged();
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::setDetectedObjectSetInput(
  kwiver::vital::algo::detected_object_set_input_sptr const&
    detectedObjectSetInput)
{
  QTE_D();
  d->detectedObjectSets.clear();

  if (detectedObjectSetInput)
  {
    kwiver::vital::detected_object_set_sptr set;
    std::string imageName;
    while (detectedObjectSetInput->read_set(set, imageName))
    {
      d->detectedObjectSets.push_back(set);
    }
  }

  this->invalidate();
}

// ----------------------------------------------------------------------------
QSet<kwiver::vital::timestamp::time_t> KwiverVideoSource::times() const
{
  QTE_D();
  QSet<kwiver::vital::timestamp::time_t> result;
  for (auto const& f : d->timestampMap)
  {
    result.insert(f.first);
  }
  return result;
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::seek(kwiver::vital::timestamp::time_t time)
{
  QTE_D();
  auto it = d->timestampMap.find(time);
  if (it != d->timestampMap.end())
  {
    d->frame = it->second;
    kwiver::vital::timestamp ts;
    if (d->videoInput->seek_frame(ts, d->frame))
    {
      d->image = d->videoInput->frame_image();
    }
    else
    {
      d->image = nullptr;
    }
  }
  else
  {
    d->image = nullptr;
  }

  this->invalidate();
}

// ----------------------------------------------------------------------------
void KwiverVideoSource::invalidate() const
{
  QTE_D();
  if (d->image)
  {
    emit this->kwiverImageDisplayed(d->image);
  }
  else
  {
    emit this->noImageDisplayed();
  }
  auto frame = d->frame - 1;
  if (frame >= 0 && frame < d->detectedObjectSets.size())
  {
    auto set = d->detectedObjectSets[frame];
    if (set)
    {
      emit this->detectedObjectSetDisplayed(set);
    }
    else
    {
      emit this->noDetectedObjectSetDisplayed();
    }
  }
  else
  {
    emit this->noDetectedObjectSetDisplayed();
  }
}

// ----------------------------------------------------------------------------
void KwiverVideoSourcePrivate::rebuildTimestampMap()
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

}
