/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/kwiver/TimestampPassthrough.hpp>

#include <sealtk/core/Version.h>

#include <vital/algo/algorithm_factory.h>

#define MODULE_NAME "sealtk"
#define MODULE_VERSION SEALTK_VERSION
#define MODULE_ORGANIZATION "Kitware, Inc."

namespace sealtk
{

namespace noaa
{

namespace kwiver
{

extern "C" void register_factories(::kwiver::vital::plugin_loader& vpm)
{
  using kvpf = ::kwiver::vital::plugin_factory;

  if (vpm.is_module_loaded(MODULE_NAME))
  {
    return;
  }

  auto fact = vpm.ADD_ALGORITHM("noaa_timestamp_passthrough",
                                TimestampPassthrough);
  fact->add_attribute(kvpf::PLUGIN_DESCRIPTION,
                      "Timestamp parser for NOAA images")
       .add_attribute(kvpf::PLUGIN_MODULE_NAME, MODULE_NAME)
       .add_attribute(kvpf::PLUGIN_VERSION, MODULE_VERSION)
       .add_attribute(kvpf::PLUGIN_ORGANIZATION, MODULE_ORGANIZATION);
}

}

}

}
