/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoMetaData.hpp>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoMetaDataData : public QSharedData
{
public:
  VideoMetaDataData(kv::timestamp const& ts, kv::path_t const& in)
    : timeStamp{ts}, imageName{in}
  {}

  kv::timestamp timeStamp;
  kv::path_t imageName;
};

QTE_IMPLEMENT_D_FUNC_SHARED(VideoMetaData)

// ----------------------------------------------------------------------------
VideoMetaData::VideoMetaData(kv::timestamp const& ts, kv::path_t const& in)
  : d_ptr{new VideoMetaDataData{ts, in}}
{
}

// ----------------------------------------------------------------------------
VideoMetaData::~VideoMetaData()
{
}

// ----------------------------------------------------------------------------
kv::timestamp VideoMetaData::timeStamp() const
{
  QTE_D();
  return d->timeStamp;
}

// ----------------------------------------------------------------------------
kv::path_t VideoMetaData::imageName() const
{
  QTE_D();
  return d->imageName;
}

// ----------------------------------------------------------------------------
void VideoMetaData::setTimeStamp(kv::timestamp const& ts)
{
  QTE_D_DETACH();
  d->timeStamp = ts;
}


// ----------------------------------------------------------------------------
void VideoMetaData::setImageName(kv::path_t const& in)
{
  QTE_D_DETACH();
  d->imageName = in;
}
}

}
