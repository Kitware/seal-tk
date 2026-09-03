/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <vital/algo/algorithm.txx>
#include <vital/algo/image_io.h>
#include <vital/plugin_management/pluggable_macro_magic.h>
#include <vital/plugin_management/plugin_manager.h>

#include <qtStlUtil.h>

#include <QRegularExpression>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TimestampPassthrough
  : public kv::algo::image_io
{
public:
  PLUGGABLE_IMPL_NAMED(
    TimestampPassthrough, "timestamp_passthrough",
    "Timestamp parser for test images",
    PARAM(
      image_reader, kv::algo::image_io_sptr,
      "Nested reader used to load the image data itself." )
  )

  ~TimestampPassthrough() override = default;

  bool check_configuration(kv::config_block_sptr config) const override;

private:
  void initialize() override;

  kv::image_container_sptr load_(std::string const& filename) const override;

  void save_(std::string const& filename,
             kv::image_container_sptr data) const override;

  kv::metadata_sptr load_metadata_(std::string const& filename) const override;

  kv::metadata_sptr fixupMetadata(std::string const& filename,
                                  kv::metadata_sptr md) const;
};

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
void TimestampPassthrough::save_(std::string const& filename,
                                 kv::image_container_sptr data) const
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
    return this->fixupMetadata(
      filename, this->get_image_reader()->load_metadata(filename));
  }

  return this->fixupMetadata(filename, nullptr);
}

// ----------------------------------------------------------------------------
kv::metadata_sptr TimestampPassthrough::fixupMetadata(
  std::string const& filename, kv::metadata_sptr md) const
{
  static QRegularExpression const regex{".*[^0-9]([0-9]+)\\.png"};

  if (!md)
  {
    md = std::make_shared<kv::metadata>();
  }

  auto match = regex.match(qtString(filename));
  if (match.hasMatch())
  {
    kv::timestamp ts;
    ts.set_time_usec(match.captured(1).toLongLong());
    md->set_timestamp(ts);
  }

  return md;
}

// ----------------------------------------------------------------------------
void loadKwiverPlugins()
{
  kv::plugin_manager::instance().load_all_plugins();
  kv::plugin_manager::instance().add_factory(
    new kv::concrete_plugin_factory<kv::algo::image_io, TimestampPassthrough>(
      TimestampPassthrough::plugin_name()));
}

} // namespace test

} // namespace core

} // namespace sealtk
