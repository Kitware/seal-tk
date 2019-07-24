/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverPipelinePortSet.hpp>

namespace sealtk
{

namespace core
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
void bind(std::string& out, std::string const& expected, std::string const& in)
{
  if (in == expected)
  {
    out = expected;
  }
}

}

// ----------------------------------------------------------------------------
void KwiverPipelinePortSet::bind(
  kwiver::embedded_pipeline& pipeline, int index, PortType type,
  std::vector<PortReference> additionalPorts)
{
  auto const timePortName = this->portName("timestamp", index);

  for (auto const& p : this->portNames(pipeline, type))
  {
    sealtk::core::bind(this->timePort, timePortName, p);
    for (auto const& ap : additionalPorts)
    {
      sealtk::core::bind(ap.first, ap.second, p);
    }
  }
}

// ----------------------------------------------------------------------------
sprokit::process::ports_t KwiverPipelinePortSet::portNames(
  kwiver::embedded_pipeline& pipeline, PortType type)
{
  switch (type)
  {
    case PortType::Input: return pipeline.input_port_names();
    case PortType::Output: return pipeline.output_port_names();
    default: return {};
  }
}

// ----------------------------------------------------------------------------
std::string KwiverPipelinePortSet::portName(std::string const& base, int index)
{
  if (index)
  {
    return base + std::to_string(index + 1);
  }

  return base;
}

} // namespace core

} // namespace sealtk
