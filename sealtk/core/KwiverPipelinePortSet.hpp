/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverPipelinePortSet_hpp
#define sealtk_core_KwiverPipelinePortSet_hpp

#include <sealtk/core/Export.h>

#include <sprokit/processes/adapters/adapter_data_set.h>
#include <sprokit/processes/adapters/embedded_pipeline.h>

#include <string>
#include <vector>

namespace sealtk
{

namespace core
{

// ============================================================================
class SEALTK_CORE_EXPORT KwiverPipelinePortSet
{
public:
  static std::string portName(std::string const& base, int index);

protected:
  using PortReference = std::pair<std::string&, std::string>;

  enum class PortType
  {
    Input,
    Output,
  };

  KwiverPipelinePortSet() = default;
  ~KwiverPipelinePortSet() = default;

  void bind(
    kwiver::embedded_pipeline& pipeline, int index, PortType type,
    std::vector<PortReference> additionalPorts);

  static sprokit::process::ports_t portNames(
    kwiver::embedded_pipeline& pipeline, PortType type);

  template <typename T>
  static void addInput(kwiver::adapter::adapter_data_set_t const& dataSet,
                       std::string const& portName, T const& data);

  template <typename T>
  static void ensureInput(kwiver::adapter::adapter_data_set_t const& dataSet,
                          std::string const& portName, T const& data);

  std::string timePort;
};

// ----------------------------------------------------------------------------
template <typename T>
void KwiverPipelinePortSet::addInput(
  kwiver::adapter::adapter_data_set_t const& dataSet,
  std::string const& portName, T const& data)
{
  if (!portName.empty())
  {
    dataSet->add_value(portName, data);
  }
}

// ----------------------------------------------------------------------------
template <typename T>
void KwiverPipelinePortSet::ensureInput(
  kwiver::adapter::adapter_data_set_t const& dataSet,
  std::string const& portName, T const& data)
{
  if (!portName.empty())
  {
    auto const& iter = dataSet->find(portName);
    if (iter == dataSet->end())
    {
      dataSet->add_value(portName, data);
    }
  }
}

} // namespace core

} // namespace sealtk

#endif
