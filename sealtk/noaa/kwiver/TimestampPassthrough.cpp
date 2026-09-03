/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/kwiver/TimestampPassthrough.hpp>

#include <sealtk/noaa/core/FilenameUtils.hpp>

#include <sealtk/core/DateUtils.hpp>

#include <vital/algo/algorithm.txx>

#include <sstream>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace noaa
{

namespace kwiver
{

// ----------------------------------------------------------------------------
void TimestampPassthrough::initialize()
{
  this->set_capability(kv::algo::image_io::HAS_TIME, true);
}

// ----------------------------------------------------------------------------
bool TimestampPassthrough::check_configuration(
  kv::config_block_sptr config) const
{
  return kv::check_nested_algo_configuration<kv::algo::image_io>(
    "image_reader", config);
}

// ----------------------------------------------------------------------------
kv::image_container_sptr TimestampPassthrough::load_(
  std::string const& filename) const
{
  if (this->get_image_reader())
  {
    auto im = this->get_image_reader()->load(filename);
    im->set_metadata(this->fixupMetadata(filename, im->get_metadata()));
    return im;
  }

  return nullptr;
}

// ----------------------------------------------------------------------------
void TimestampPassthrough::save_(
  std::string const& filename, kv::image_container_sptr data) const
{
  if (this->get_image_reader())
  {
    this->get_image_reader()->save(filename, data);
  }
}

// ----------------------------------------------------------------------------
kv::metadata_sptr TimestampPassthrough::load_metadata_(
  std::string const& filename) const
{
  if (this->get_image_reader())
  {
    return this->fixupMetadata(filename,
                               this->get_image_reader()->load_metadata(filename));
  }

  return this->fixupMetadata(filename, nullptr);
}

// ----------------------------------------------------------------------------
kv::metadata_sptr TimestampPassthrough::fixupMetadata(
  std::string const& filename, kv::metadata_sptr md) const
{
  if (!md)
  {
    md = std::make_shared<kv::metadata>();
  }

  auto dateTime =
    core::imageFilenameToQDateTime(QString::fromStdString(filename));
  if (!dateTime.isNull())
  {
    kv::timestamp ts;
    ts.set_time_usec(sealtk::core::qDateTimeToVitalTime(dateTime));
    md->set_timestamp(ts);
  }

  return md;
}

} // namespace kwiver

} // namespace noaa

} // namespace sealtk
