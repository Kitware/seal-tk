/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/core/NoaaPipelineWorker.hpp>

#include <sealtk/core/KwiverPipelinePortSet.hpp>
#include <sealtk/core/KwiverTrackModel.hpp>

#include <vital/types/detected_object_set.h>

#include <vital/range/valid.h>

#include <qtGet.h>
#include <qtStlUtil.h>

#include <QRegularExpression>

namespace ka = kwiver::adapter;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using ts_time_t = kv::timestamp::time_t;

#define DETECTIONS_PORT "detected_object_set"

using sealtk::core::KwiverTrackModel;

using super = sealtk::core::KwiverPipelineWorker;

namespace sealtk
{

namespace noaa
{

namespace core
{

namespace // anonymous
{

// ============================================================================
class PortSet : sealtk::core::KwiverPipelinePortSet
{
public:
  PortSet(kwiver::embedded_pipeline& pipeline, int index);

  static sprokit::process::ports_t portNames(
    kwiver::embedded_pipeline& pipeline)
  {
    return KwiverPipelinePortSet::portNames(pipeline, PortType::Output);
  }

  void extractOutput(ka::adapter_data_set_t const& dataSet);

  int index;
  std::shared_ptr<KwiverTrackModel> model;

private:
  std::string detectionsPort;

  kv::track_id_t nextTrack = 0;
};

// ----------------------------------------------------------------------------
PortSet::PortSet(kwiver::embedded_pipeline& pipeline, int index)
  : index{index}, model{std::make_shared<KwiverTrackModel>()}
{
  this->bind(
    pipeline, index, PortType::Output,
    {
      {this->detectionsPort, this->portName(DETECTIONS_PORT, index)},
    });
}

// ----------------------------------------------------------------------------
void PortSet::extractOutput(ka::adapter_data_set_t const& dataSet)
{
  // Get time stamp
  if (auto* const timeDatum = qtGet(*dataSet, this->timePort))
  {
    auto const ts = timeDatum->second->get_datum<kv::timestamp>();
    auto const t = ts.get_time_usec();

    // Get detections
    if (auto* const detectionsDatum = qtGet(*dataSet, this->detectionsPort))
    {
      auto const& detections =
        detectionsDatum->second->get_datum<kv::detected_object_set_sptr>();

      if (detections)
      {
        auto tracks = std::vector<kv::track_sptr>{};

        // Generate tracks from detections
        for (auto const& detection : *detections | kvr::valid)
        {
          // Create track
          auto track = kv::track::create();
          track->set_id(++this->nextTrack);

          // Create object state for track
          auto state =
            std::make_shared<kv::object_track_state>(0, t, detection);
          track->append(state);
        }

        // Add extracted tracks to model
        if (!tracks.empty())
        {
          auto trackSet = std::make_shared<kv::object_track_set>(tracks);

          QMetaObject::invokeMethod(
            model.get(),
            [trackSet = std::move(trackSet), model = model.get()]{
              model->addTracks(trackSet);
            });
        }
      }
    }
  }
}

} // namespace <anonymous>

// ============================================================================
class NoaaPipelineWorkerPrivate
{
public:
  std::vector<PortSet> outputs;
};

QTE_IMPLEMENT_D_FUNC(NoaaPipelineWorker)

// ----------------------------------------------------------------------------
NoaaPipelineWorker::NoaaPipelineWorker(QWidget* parent)
  : NoaaPipelineWorker{RequiresInput, parent}
{
}

// ----------------------------------------------------------------------------
NoaaPipelineWorker::NoaaPipelineWorker(
  RequiredEndcaps endcaps, QWidget* parent)
  : super{endcaps, parent}, d_ptr{new NoaaPipelineWorkerPrivate}
{
}

// ----------------------------------------------------------------------------
NoaaPipelineWorker::~NoaaPipelineWorker()
{
}

// ----------------------------------------------------------------------------
void NoaaPipelineWorker::initializeInput(kwiver::embedded_pipeline& pipeline)
{
  QTE_D();

  Q_UNUSED(pipeline)

  // Find output ports
  auto portNameTemplate =
    QRegularExpression{QStringLiteral(DETECTIONS_PORT "([0-9]+)?")};

  for (auto const& p : PortSet::portNames(pipeline))
  {
    auto const& m = portNameTemplate.match(qtString(p));
    if (m.isValid())
    {
      auto const& n = m.captured(1);
      auto const i = (n.isEmpty() ? 0 : n.toInt() - 1);

      d->outputs.emplace_back(pipeline, i);
    }
  }

  for (auto const& p : d->outputs)
  {
    emit this->trackModelReady(p.index, p.model);
  }
}

// ----------------------------------------------------------------------------
void NoaaPipelineWorker::processOutput(ka::adapter_data_set_t const& output)
{
  if (output)
  {
    QTE_D();

    for (auto& p : d->outputs)
    {
      p.extractOutput(output);
    }
  }
}

} // namespace core

} // namespace noaa

} // namespace kwiver
