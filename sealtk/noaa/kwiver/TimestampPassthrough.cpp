/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/kwiver/TimestampPassthrough.hpp>

#include <sealtk/noaa/core/FilenameUtils.hpp>

#include <sealtk/core/DateUtils.hpp>

#include <vital/algo/algorithm_factory.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <QRegularExpression>

#include <sstream>

namespace sealtk
{

namespace noaa
{

namespace kwiver
{

// ----------------------------------------------------------------------------
TimestampPassthrough::TimestampPassthrough()
{
  this->set_capability(::kwiver::vital::algo::image_io::HAS_TIME, true);
}

// ----------------------------------------------------------------------------
::kwiver::vital::config_block_sptr
  TimestampPassthrough::get_configuration() const
{
  auto config = ::kwiver::vital::algo::image_io::get_configuration();

  ::kwiver::vital::algo::image_io::get_nested_algo_configuration(
    "image_reader", config, this->imageReader);

  return config;
}

// ----------------------------------------------------------------------------
void TimestampPassthrough::set_configuration(
  ::kwiver::vital::config_block_sptr config)
{
  auto newConfig = this->get_configuration();
  newConfig->merge_config(config);

  ::kwiver::vital::algo::image_io::set_nested_algo_configuration(
    "image_reader", newConfig, this->imageReader);
}

// ----------------------------------------------------------------------------
bool TimestampPassthrough::check_configuration(
  ::kwiver::vital::config_block_sptr config) const
{
  return ::kwiver::vital::algo::image_io::check_nested_algo_configuration(
    "image_reader", config);
}

// ----------------------------------------------------------------------------
::kwiver::vital::image_container_sptr TimestampPassthrough::load_(
  std::string const& filename) const
{
  if (this->imageReader)
  {
    auto im = this->imageReader->load(filename);
    im->set_metadata(this->fixupMetadata(filename, im->get_metadata()));
    return im;
  }

  return nullptr;
}

// ----------------------------------------------------------------------------
void TimestampPassthrough::save_(
  std::string const& filename,
  ::kwiver::vital::image_container_sptr data) const
{
  if (this->imageReader)
  {
    this->imageReader->save(filename, data);
  }
}

// ----------------------------------------------------------------------------
::kwiver::vital::metadata_sptr TimestampPassthrough::load_metadata_(
  std::string const& filename) const
{
  if (this->imageReader)
  {
    return this->fixupMetadata(filename,
                               this->imageReader->load_metadata(filename));
  }

  return this->fixupMetadata(filename, nullptr);
}

// ----------------------------------------------------------------------------
::kwiver::vital::metadata_sptr TimestampPassthrough::fixupMetadata(
  std::string const& filename, ::kwiver::vital::metadata_sptr md) const
{
  if (!md)
  {
    md = std::make_shared<::kwiver::vital::metadata>();
  }

  auto dateTime =
    core::imageFilenameToQDateTime(QString::fromStdString(filename));
  if (!dateTime.isNull())
  {
    ::kwiver::vital::timestamp ts;
    ts.set_time_usec(sealtk::core::qDateTimeToVitalTime(dateTime));
    md->set_timestamp(ts);
  }

  return md;
}

}

}

}
