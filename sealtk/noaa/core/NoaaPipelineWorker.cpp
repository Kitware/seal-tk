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
#define TRACKS_PORT "object_track_set"

#define RE_ANY_OUTPUT_PORT "(" DETECTIONS_PORT "|" TRACKS_PORT ")"

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

// ----------------------------------------------------------------------------
template <typename T> std::weak_ptr<T> weakRef(std::shared_ptr<T> const& p)
{
  return p;
}

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
  std::string tracksPort;

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
      {this->tracksPort, this->portName(TRACKS_PORT, index)},
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

          // Add track to set
          tracks.emplace_back(std::move(track));
        }

        // Add extracted tracks to model
        if (!tracks.empty())
        {
          auto trackSet = std::make_shared<kv::object_track_set>(tracks);
          auto&& mwp = weakRef(model);

          QMetaObject::invokeMethod(
            model.get(),
            [trackSet = std::move(trackSet), mwp = std::move(mwp)]{
              if (auto const& model = mwp.lock())
              {
                model->addTracks(trackSet);
              }
            });
        }
      }
    }

    if (auto* const tracksDatum = qtGet(*dataSet, this->tracksPort))
    {
      auto&& trackSet =
        tracksDatum->second->get_datum<kv::object_track_set_sptr>();

      if (trackSet && !trackSet->empty())
      {
        auto&& mwp = weakRef(model);

        QMetaObject::invokeMethod(
          model.get(),
          [trackSet = std::move(trackSet), mwp = std::move(mwp)]{
            if (auto const& model = mwp.lock())
            {
              model->mergeTracks(trackSet);
            }
          });
      }
    }
  }
}

} // namespace <anonymous>

// ============================================================================
class NoaaPipelineWorkerPrivate
{
public:
  QSet<int> outputSets;
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

  // Find output ports
  auto portNameTemplate =
    QRegularExpression{QStringLiteral(RE_ANY_OUTPUT_PORT "([0-9]+)?")};

  for (auto const& p : PortSet::portNames(pipeline))
  {
    auto const& m = portNameTemplate.match(qtString(p));
    if (m.hasMatch())
    {
      auto const& n = m.captured(2);
      auto const i = (n.isEmpty() ? 0 : n.toInt() - 1);

      if (!d->outputSets.contains(i))
      {
        d->outputs.emplace_back(pipeline, i);
        d->outputSets.insert(i);
      }
    }
  }

  for (auto const& p : d->outputs)
  {
    p.model->moveToThread(this->thread());
    emit this->trackModelReady(p.index, p.model);
  }

  this->KwiverPipelineWorker::initializeInput(pipeline);
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
