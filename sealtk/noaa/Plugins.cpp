/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/Plugins.hpp>

#include <sealtk/noaa/FilenameUtils.hpp>

#include <sealtk/core/DateUtils.hpp>

#include <vital/algo/algorithm_factory.h>
#include <vital/algo/image_io.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <QRegularExpression>

#include <sstream>

namespace sealtk
{

namespace noaa
{

// ============================================================================
class TimestampPassthrough : public kwiver::vital::algorithm_impl<
  TimestampPassthrough, kwiver::vital::algo::image_io>
{
public:
  TimestampPassthrough();

  ~TimestampPassthrough() override = default;

  kwiver::vital::config_block_sptr get_configuration() const
    override;

  void set_configuration(kwiver::vital::config_block_sptr config) override;

  bool check_configuration(
    kwiver::vital::config_block_sptr config) const override;

private:
  kwiver::vital::algo::image_io_sptr imageReader;

  kwiver::vital::image_container_sptr load_(
    std::string const& filename) const override;

  void save_(std::string const& filename,
             kwiver::vital::image_container_sptr data) const override;

  kwiver::vital::metadata_sptr load_metadata_(
    std::string const& filename) const override;

  kwiver::vital::metadata_sptr fixupMetadata(
    std::string const& filename, kwiver::vital::metadata_sptr md) const;
};

// ----------------------------------------------------------------------------
TimestampPassthrough::TimestampPassthrough()
{
  this->set_capability(kwiver::vital::algo::image_io::HAS_TIME, true);
}

// ----------------------------------------------------------------------------
kwiver::vital::config_block_sptr
  TimestampPassthrough::get_configuration() const
{
  auto config = kwiver::vital::algo::image_io::get_configuration();

  kwiver::vital::algo::image_io::get_nested_algo_configuration(
    "image_reader", config, this->imageReader);

  return config;
}

// ----------------------------------------------------------------------------
void TimestampPassthrough::set_configuration(
  kwiver::vital::config_block_sptr config)
{
  auto newConfig = this->get_configuration();
  newConfig->merge_config(config);

  kwiver::vital::algo::image_io::set_nested_algo_configuration(
    "image_reader", newConfig, this->imageReader);
}

// ----------------------------------------------------------------------------
bool TimestampPassthrough::check_configuration(
  kwiver::vital::config_block_sptr config) const
{
  return kwiver::vital::algo::image_io::check_nested_algo_configuration(
    "image_reader", config);
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr TimestampPassthrough::load_(
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
  std::string const& filename, kwiver::vital::image_container_sptr data) const
{
  if (this->imageReader)
  {
    this->imageReader->save(filename, data);
  }
}

// ----------------------------------------------------------------------------
kwiver::vital::metadata_sptr TimestampPassthrough::load_metadata_(
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
kwiver::vital::metadata_sptr TimestampPassthrough::fixupMetadata(
  std::string const& filename, kwiver::vital::metadata_sptr md) const
{
  if (!md)
  {
    md = std::make_shared<kwiver::vital::metadata>();
  }

  auto dateTime = imageFilenameToQDateTime(QString::fromStdString(filename));
  if (!dateTime.isNull())
  {
    kwiver::vital::timestamp ts;
    ts.set_time_usec(core::qDateTimeToVitalTime(dateTime));
    md->set_timestamp(ts);
  }

  return md;
}

// ----------------------------------------------------------------------------
void registerKwiverPlugins()
{
  kwiver::vital::plugin_manager::instance().ADD_ALGORITHM(
    "noaa_timestamp_passthrough", TimestampPassthrough);
}

}

}
