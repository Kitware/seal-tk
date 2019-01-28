/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_kwiver_TimestampPassthrough_hpp
#define sealtk_noaa_kwiver_TimestampPassthrough_hpp

#include <sealtk/noaa/kwiver/Export.h>

#include <vital/algo/image_io.h>

namespace sealtk
{

namespace noaa
{

namespace kwiver
{

class SEALTK_NOAA_KWIVER_EXPORT
  TimestampPassthrough : public ::kwiver::vital::algorithm_impl<
    TimestampPassthrough, ::kwiver::vital::algo::image_io>
{
public:
  TimestampPassthrough();

  ~TimestampPassthrough() override = default;

  ::kwiver::vital::config_block_sptr get_configuration() const
    override;

  void set_configuration(::kwiver::vital::config_block_sptr config) override;

  bool check_configuration(
    ::kwiver::vital::config_block_sptr config) const override;

private:
  ::kwiver::vital::algo::image_io_sptr imageReader;

  ::kwiver::vital::image_container_sptr load_(
    std::string const& filename) const override;

  void save_(std::string const& filename,
             ::kwiver::vital::image_container_sptr data) const override;

  ::kwiver::vital::metadata_sptr load_metadata_(
    std::string const& filename) const override;

  ::kwiver::vital::metadata_sptr fixupMetadata(
    std::string const& filename, ::kwiver::vital::metadata_sptr md) const;
};

}

}

}

#endif
