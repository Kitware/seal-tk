/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverPipelineWorker.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/KwiverPipelinePortSet.hpp>
#include <sealtk/core/TrackUtils.hpp>
#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <sealtk/util/unique.hpp>

#include <vital/types/detected_object_set.h>

#include <vital/range/iota.h>

#include <QAbstractItemModel>
#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QMessageBox>
#include <QPointer>

namespace ka = kwiver::adapter;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using ts_time_t = kv::timestamp::time_t;

using super = kwiver::arrows::qt::EmbeddedPipelineWorker;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

// ============================================================================
struct Detection
{
  QRectF box;
  QVariantHash classification;
};

using DetectionSet = QList<Detection>;

// ============================================================================
class PortSet : public KwiverPipelinePortSet
{
public:
  PortSet(kwiver::embedded_pipeline& pipeline, int index);

  void addInputs(ka::adapter_data_set_t const& dataSet,
                 VideoFrame const& frame,
                 DetectionSet const& detections) const;
  void ensureInputs(ka::adapter_data_set_t const& dataSet) const;

private:
  std::string imagePort;
  std::string namePort;
  std::string detectionsPort;
};

// ----------------------------------------------------------------------------
PortSet::PortSet(kwiver::embedded_pipeline& pipeline, int index)
{
  KwiverPipelinePortSet::bind(
    pipeline, index, PortType::Input,
    {
      {this->imagePort, this->portName("image", index)},
      {this->namePort, this->portName("file_name", index)},
      {this->detectionsPort, this->portName("detected_object_set", index)},
    });
}

// ----------------------------------------------------------------------------
void PortSet::addInputs(
  ka::adapter_data_set_t const& dataSet,
  VideoFrame const& frame, DetectionSet const& detections) const
{
  Q_ASSERT(dataSet);

  this->addInput(dataSet, this->imagePort, frame.image);
  this->addInput(dataSet, this->namePort, frame.metaData.imageName());
  this->addInput(dataSet, this->timePort, frame.metaData.timeStamp());

  // Convert detections to KWIVER data structure
  auto dv = std::vector<kv::detected_object_sptr>{};
  for (auto const& detection : detections)
  {
    auto const& qbox = detection.box;
    auto const kbox = kv::bounding_box_d{{qbox.left(), qbox.top()},
                                          qbox.width(), qbox.height()};
    auto const kdot =
      classificationToDetectedObjectType(detection.classification);

    dv.emplace_back(std::make_shared<kv::detected_object>(kbox, 1.0, kdot));
  }

  auto ds = std::make_shared<kv::detected_object_set>(dv);
  this->addInput(dataSet, this->detectionsPort, ds);
}

// ----------------------------------------------------------------------------
void PortSet::ensureInputs(ka::adapter_data_set_t const& dataSet) const
{
  this->ensureInput(dataSet, this->imagePort, kv::image_container_sptr{});
  this->ensureInput(dataSet, this->namePort, kv::path_t{});
  this->ensureInput(dataSet, this->timePort, kv::timestamp{});
  this->ensureInput(dataSet, this->detectionsPort, nullptr);
}

// ============================================================================
class PipelineVideoRequestor
  : public VideoRequestor,
    public std::enable_shared_from_this<PipelineVideoRequestor>
{
public:
  PipelineVideoRequestor(
    PortSet const* ports, TimeMap<DetectionSet> const* detectionMap,
    QEventLoop* eventLoop);

  void setDetections(DetectionSet const& detections);
  void requestFrame(VideoSource* source, ts_time_t time);
  void waitForFrame() const;
  void dispatchFrame(ka::adapter_data_set_t& dataSet);

protected:
  void update(VideoRequestInfo const& requestInfo,
              VideoFrame&& response) override;

  PortSet const* const ports;
  TimeMap<DetectionSet> const* const detectionMap;

  QPointer<QEventLoop> const eventLoop;
  std::unique_ptr<VideoFrame> receivedFrame;
};

// ----------------------------------------------------------------------------
PipelineVideoRequestor::PipelineVideoRequestor(
  PortSet const* ports, TimeMap<DetectionSet> const* detectionMap,
  QEventLoop* eventLoop)
  : ports{ports}, detectionMap{detectionMap}, eventLoop{eventLoop}
{
}

// ----------------------------------------------------------------------------
void PipelineVideoRequestor::requestFrame(VideoSource* source, ts_time_t time)
{
  auto request = VideoRequest{};
  request.requestor = this->shared_from_this();
  request.requestId = 0;
  request.time = time;
  request.mode = SeekExact;

  source->requestFrame(std::move(request));
}

// ----------------------------------------------------------------------------
void PipelineVideoRequestor::waitForFrame() const
{
  while (!this->receivedFrame)
  {
    this->eventLoop->exec();
  }
}

// ----------------------------------------------------------------------------
void PipelineVideoRequestor::dispatchFrame(ka::adapter_data_set_t& dataSet)
{
  auto const time = this->receivedFrame->metaData.timeStamp().get_time_usec();
  auto const& detections = this->detectionMap->value(time);

  this->ports->addInputs(dataSet, *this->receivedFrame, detections);
  this->receivedFrame.reset();
}

// ----------------------------------------------------------------------------
void PipelineVideoRequestor::update(
  VideoRequestInfo const& requestInfo, VideoFrame&& response)
{
  Q_UNUSED(requestInfo);

  this->receivedFrame = make_unique<VideoFrame>(std::move(response));

  if (this->eventLoop)
  {
    this->eventLoop->quit();
  }
}

} // namespace <anonymous>

// ============================================================================
class KwiverPipelineWorkerPrivate
{
public:
  QVector<VideoSource*> sources;
  QList<TimeMap<kv::timestamp::frame_t>> frames;
  QList<TimeMap<DetectionSet>> detections;
};

QTE_IMPLEMENT_D_FUNC(KwiverPipelineWorker)

// ----------------------------------------------------------------------------
KwiverPipelineWorker::KwiverPipelineWorker(QWidget* parent)
  : KwiverPipelineWorker{RequiresInput, parent}
{
}

// ----------------------------------------------------------------------------
KwiverPipelineWorker::KwiverPipelineWorker(
  RequiredEndcaps endcaps, QWidget* parent)
  : super{endcaps, parent}, d_ptr{new KwiverPipelineWorkerPrivate}
{
}

// ----------------------------------------------------------------------------
KwiverPipelineWorker::~KwiverPipelineWorker()
{
}

// ----------------------------------------------------------------------------
void KwiverPipelineWorker::addVideoSource(sealtk::core::VideoSource* source)
{
  QTE_D();

  if (source)
  {
    // Don't add the same source more than once
    if (!d->sources.contains(source))
    {
      // Wait until source is ready to report its frames
      QEventLoop eventLoop;

      connect(source, &VideoSource::framesChanged,
              &eventLoop, &QEventLoop::quit);

      while (!source->isReady())
      {
        source->start();
        eventLoop.exec();
      }

      // Get source's frames and append to frame set
      d->sources.append(source);
      d->frames.append(source->frames());
    }
    else
    {
      qWarning() << __func__ << "refusing to add video source" << source
                 << "which has already been added";
    }
  }
  else
  {
    d->sources.append(nullptr);
    d->frames.append(TimeMap<kv::timestamp::frame_t>{});
  }
}

// ----------------------------------------------------------------------------
void KwiverPipelineWorker::addTrackSource(
  QAbstractItemModel* model, bool includeHidden)
{
  QTE_D();

  TimeMap<DetectionSet> detections;

  if (model)
  {
    // Iterate over all items in data model
    for (auto const i : kvr::iota(model->rowCount()))
    {
      auto const iIndex = model->index(i, 0);
      for (auto const j : kvr::iota(model->rowCount(iIndex)))
      {
        // Get detection (track state) information
        auto const jIndex = model->index(j, 0, iIndex);
        auto const& td = model->data(jIndex, StartTimeRole);
        auto const t = td.value<kv::timestamp::time_t>();

        if (!includeHidden)
        {
          auto const& vd = model->data(jIndex, VisibilityRole);
          if (!vd.toBool())
          {
            // Skip detections which are not visible
            continue;
          }
        }

        // Add detection to detection set for the appropriate time
        detections[t].append({
          model->data(jIndex, AreaLocationRole).toRectF(),
          model->data(jIndex, ClassificationRole).toHash()});
      }
    }
  }

  d->detections.append(detections);
}

// ----------------------------------------------------------------------------
void KwiverPipelineWorker::initializeInput(kwiver::embedded_pipeline& pipeline)
{
  QTE_D();

  Q_UNUSED(pipeline)

  // Compute total number of frames
  int totalFrames = 0;
  for (auto const& f : d->frames)
  {
    totalFrames += f.count();
  }

  // Ensure that the detection data is non-empty
  auto const sourceCount = d->sources.count();
  while (d->detections.count() < sourceCount)
  {
    d->detections.append(TimeMap<DetectionSet>{});
  }

  emit this->progressRangeChanged(0, totalFrames);
}

// ----------------------------------------------------------------------------
void KwiverPipelineWorker::sendInput(kwiver::embedded_pipeline& pipeline)
{
  QTE_D();

  auto const sourcesCount = d->sources.count();

  QEventLoop eventLoop;
  auto ports = QList<PortSet>{};
  auto requestors =
    QHash<VideoSource*, std::shared_ptr<PipelineVideoRequestor>>{};

  auto sourcesToUse = QVector<VideoSource*>{};
  auto requestorsInUse = QVector<PipelineVideoRequestor*>{};
  auto lastTime = std::numeric_limits<ts_time_t>::min();
  auto framesProcessed = int{0};

  // For each source...
  for (auto const i : kvr::iota(sourcesCount))
  {
    // ...get ports...
    ports.append({pipeline, i});

    // ...and create a requestor to receive frames from that source
    if (auto* const source = d->sources[i])
    {
      requestors[source] =
        std::make_shared<PipelineVideoRequestor>(&ports[i], &d->detections[i],
                                                 &eventLoop);
    }
  }

  // Dispatch frames in a loop
  for (;;)
  {
    auto nextTime = std::numeric_limits<ts_time_t>::max();

    // Determine which sources will supply the next frame(s)
    sourcesToUse.clear();
    for (auto const i : kvr::iota(sourcesCount))
    {
      auto const& frames = d->frames[i];
      auto const ti = frames.find(lastTime, SeekNext);
      if (ti != frames.end() && ti.key() <= nextTime)
      {
        Q_ASSERT(ti.key() > lastTime);

        if (ti.key() < nextTime)
        {
          sourcesToUse.clear();
          nextTime = ti.key();
        }

        Q_ASSERT(d->sources[i]);
        sourcesToUse.append(d->sources[i]);
      }
    }
    lastTime = nextTime;

    auto const expectedFrames = sourcesToUse.count();
    if (!expectedFrames)
    {
      // No sources are providing frames; this should only happen when the last
      // time is greater than or equal to the maximum time for all sources,
      // which means we are done sending frames and can exit
      pipeline.send_end_of_input();
      return;
    }

    // Request frames from sources that will participate this iteration
    requestorsInUse.clear();
    for (auto* const vs : sourcesToUse)
    {
      auto* const requestor = requestors[vs].get();
      Q_ASSERT(requestor);

      requestor->requestFrame(vs, nextTime);
      requestorsInUse.append(requestor);
    }

    // Wait until all frames are ready
    for (auto* const requestor : requestorsInUse)
    {
      requestor->waitForFrame();
    }

    // Set up pipeline input...
    auto inputDataSet = ka::adapter_data_set::create();
    for (auto* const requestor : requestorsInUse)
    {
      requestor->dispatchFrame(inputDataSet);
    }

    // ...and send it along
    if (!inputDataSet->empty())
    {
      for (auto& p : ports)
      {
        p.ensureInputs(inputDataSet);
      }

      pipeline.send(inputDataSet);

      framesProcessed += sourcesToUse.count();
      emit this->progressValueChanged(framesProcessed);
    }
  }
}

// ----------------------------------------------------------------------------
void KwiverPipelineWorker::reportError(
  QString const& message, QString const& subject)
{
  auto* const p = qobject_cast<QWidget*>(this->parent());
  auto* const w = (p ? p : qApp->activeWindow());
  QMessageBox::warning(w, subject, message);
}

} // namespace core

} // namespace sealtk
