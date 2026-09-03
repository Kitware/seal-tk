/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_kwiver_TimestampPassthrough_hpp
#define sealtk_noaa_kwiver_TimestampPassthrough_hpp

#include <sealtk/noaa/kwiver/Export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/image_io.h>
#include <vital/plugin_management/pluggable_macro_magic.h>

namespace sealtk
{

namespace noaa
{

namespace kwiver
{

// The pluggable macros below emit unqualified kwiver::vital::..., which would
// otherwise resolve against this namespace rather than KWIVER's.
namespace vital = ::kwiver::vital;

class SEALTK_NOAA_KWIVER_EXPORT TimestampPassthrough
  : public ::kwiver::vital::algo::image_io
{
public:
  PLUGGABLE_IMPL_NAMED(
    TimestampPassthrough, "noaa_timestamp_passthrough",
    "Timestamp parser for NOAA images",
    PARAM(
      image_reader, ::kwiver::vital::algo::image_io_sptr,
      "Nested reader used to load the image data itself." )
  )

  ~TimestampPassthrough() override = default;

  bool check_configuration(
    ::kwiver::vital::config_block_sptr config) const override;

private:
  void initialize() override;

  ::kwiver::vital::image_container_sptr load_(
    std::string const& filename) const override;

  void save_(std::string const& filename,
             ::kwiver::vital::image_container_sptr data) const override;

  ::kwiver::vital::metadata_sptr load_metadata_(
    std::string const& filename) const override;

  ::kwiver::vital::metadata_sptr fixupMetadata(
    std::string const& filename, ::kwiver::vital::metadata_sptr md) const;
};

} // namespace kwiver

} // namespace noaa

} // namespace sealtk

#endif
