/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverPipelineWorker.hpp>

#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <sealtk/util/unique.hpp>

#include <sprokit/processes/adapters/adapter_data_set.h>
#include <sprokit/processes/adapters/embedded_pipeline.h>

#include <vital/range/iota.h>

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
class PortSet
{
public:
  PortSet(kwiver::embedded_pipeline& pipeline, int index);

  void addInputs(ka::adapter_data_set_t const& dataSet,
                 VideoFrame const& frame);
  void ensureInputs(ka::adapter_data_set_t const& dataSet);

private:
  static std::string portName(std::string const& base, int index);

  static void bind(std::string& out, std::string const& expected,
                   std::string const& in);

  template <typename T>
  static void addInput(ka::adapter_data_set_t const& dataSet,
                       std::string const& portName, T const& data);

  template <typename T>
  static void ensureInput(ka::adapter_data_set_t const& dataSet,
                          std::string const& portName, T const& data);

  std::string imagePort;
  std::string namePort;
  std::string timePort;
};

// ----------------------------------------------------------------------------
PortSet::PortSet(kwiver::embedded_pipeline& pipeline, int index)
{
  using std::string;

  auto const imagePortName = this->portName("image", index);
  auto const namePortName = this->portName("file_name", index);
  auto const timePortName = this->portName("timestamp", index);

  for (auto const& p : pipeline.input_port_names())
  {
    this->bind(this->imagePort, imagePortName, p);
    this->bind(this->namePort, namePortName, p);
    this->bind(this->timePort, timePortName, p);
  }
}

// ----------------------------------------------------------------------------
void PortSet::addInputs(ka::adapter_data_set_t const& dataSet,
                        VideoFrame const& frame)
{
  Q_ASSERT(dataSet);

  this->addInput(dataSet, this->imagePort, frame.image);
  this->addInput(dataSet, this->namePort, frame.metaData.imageName());
  this->addInput(dataSet, this->timePort, frame.metaData.timeStamp());
}

// ----------------------------------------------------------------------------
void PortSet::ensureInputs(ka::adapter_data_set_t const& dataSet)
{
  this->ensureInput(dataSet, this->imagePort, kv::image_container_sptr{});
  this->ensureInput(dataSet, this->namePort, kv::path_t{});
  this->ensureInput(dataSet, this->timePort, kv::timestamp{});
}

// ----------------------------------------------------------------------------
template <typename T>
void PortSet::addInput(ka::adapter_data_set_t const& dataSet,
                       std::string const& portName, T const& data)
{
  if (!portName.empty())
  {
    dataSet->add_value(portName, data);
  }
}

// ----------------------------------------------------------------------------
template <typename T>
void PortSet::ensureInput(ka::adapter_data_set_t const& dataSet,
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

// ----------------------------------------------------------------------------
std::string PortSet::portName(std::string const& base, int index)
{
  if (index)
  {
    return base + std::to_string(index + 1);
  }

  return base;
}

// ----------------------------------------------------------------------------
void PortSet::bind(std::string& out, std::string const& expected,
                   std::string const& in)
{
  if (in == expected)
  {
    out = expected;
  }
}

// ============================================================================
class PipelineVideoRequestor
  : public VideoRequestor,
    public std::enable_shared_from_this<PipelineVideoRequestor>
{
public:
  PipelineVideoRequestor(PortSet* ports, QEventLoop* eventLoop);

  void requestFrame(VideoSource* source, ts_time_t time);
  void waitForFrame() const;
  void dispatchFrame(ka::adapter_data_set_t& dataSet);

protected:
  void update(VideoRequestInfo const& requestInfo,
              VideoFrame&& response) override;

  PortSet* const ports;
  QPointer<QEventLoop> const eventLoop;
  std::unique_ptr<VideoFrame> receivedFrame;
};

// ----------------------------------------------------------------------------
PipelineVideoRequestor::PipelineVideoRequestor(
  PortSet* ports, QEventLoop* eventLoop)
  : ports{ports}, eventLoop{eventLoop}
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
  this->ports->addInputs(dataSet, *this->receivedFrame);
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
void KwiverPipelineWorker::initializeInput(kwiver::embedded_pipeline& pipeline)
{
  QTE_D();

  Q_UNUSED(pipeline)

  int totalFrames = 0;
  for (auto const& f : d->frames)
  {
    totalFrames += f.count();
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
        std::make_shared<PipelineVideoRequestor>(&ports[i], &eventLoop);
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

} // namespace tools

} // namespace kwiver
