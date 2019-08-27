/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_core_KwiverPipelineWorker_hpp
#define sealtk_noaa_core_KwiverPipelineWorker_hpp

#include <sealtk/noaa/core/Export.h>

#include <sealtk/core/KwiverPipelineWorker.hpp>

#include <memory>

class QAbstractItemModel;

namespace sealtk
{

namespace noaa
{

namespace core
{

class NoaaPipelineWorkerPrivate;

// ----------------------------------------------------------------------------
class SEALTK_NOAA_CORE_EXPORT NoaaPipelineWorker
  : public sealtk::core::KwiverPipelineWorker
{
  Q_OBJECT

public:
  explicit NoaaPipelineWorker(QWidget* parent = nullptr);
  explicit NoaaPipelineWorker(RequiredEndcaps, QWidget* parent = nullptr);

  ~NoaaPipelineWorker() override;

signals:
  void trackModelReady(int index, std::shared_ptr<QAbstractItemModel> model);

protected:
  QTE_DECLARE_PRIVATE_RPTR(NoaaPipelineWorker);

  void initializeInput(kwiver::embedded_pipeline& pipeline) override;

  void processOutput(
    kwiver::adapter::adapter_data_set_t const& output) override;

private:
  QTE_DECLARE_PRIVATE(NoaaPipelineWorker);
};

} // namespace core

} // namespace noaa

} // namespace sealtk

#endif
