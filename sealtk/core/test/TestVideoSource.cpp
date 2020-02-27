/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestVideoSource.hpp>

#include <vital/range/indirect.h>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace test
{

// ----------------------------------------------------------------------------
SimpleVideoSource::SimpleVideoSource(TimeMap<VideoMetaData> const& data)
  : SimpleVideoSource{new Provider, data}
{
}

// ----------------------------------------------------------------------------
SimpleVideoSource::SimpleVideoSource(
  Provider* provider, TimeMap<VideoMetaData> const& data)
  : VideoSource{provider}, provider{provider}, data{data}
{
}

// ----------------------------------------------------------------------------
bool SimpleVideoSource::isReady() const
{
  return true;
}

// ----------------------------------------------------------------------------
TimeMap<VideoMetaData> SimpleVideoSource::metaData() const
{
  return this->data;
}

// ----------------------------------------------------------------------------
TimeMap<kv::timestamp::frame_t> SimpleVideoSource::frames() const
{
  TimeMap<kv::timestamp::frame_t> result;
  for (auto const& i : this->data | kvr::indirect)
  {
    auto const& ts = i.value().timeStamp();
    if (ts.has_valid_frame())
    {
      result.insert(i.key(), ts.get_frame());
    }
  }

  return result;
}

} // namespace test

} // namespace core

} // namespace sealtk
