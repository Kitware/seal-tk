/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "KwiverPipelineWorker.hpp"

#include "VideoSource.hpp"

#include <sprokit/processes/adapters/adapter_data_set.h>
#include <sprokit/processes/adapters/embedded_pipeline.h>

#include <vital/range/iota.h>

#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using ts_time_t = kv::timestamp::time_t;

using super = kwiver::arrows::qt::EmbeddedPipelineWorker;

namespace sealtk {

namespace core {

namespace { // anonymous

// ============================================================================
struct ReadyFrame
{
  int sourceIndex;
  kv::image_container_sptr image;
  VideoMetaData metaData;
};

// ============================================================================
class PortSet
{
public:
  PortSet(kwiver::embedded_pipeline& pipeline, int index);

  void addInputs(kwiver::adapter::adapter_data_set_t& dataSet,
                 ReadyFrame const& frame);

private:
  static std::string portName(std::string const& base, int index);

  static void bind(std::string& out, std::string const& expected,
                   std::string const& in);

  template <typename T>
  static void addInput(kwiver::adapter::adapter_data_set_t& dataSet,
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
  }
}

// ----------------------------------------------------------------------------
void PortSet::addInputs(kwiver::adapter::adapter_data_set_t& dataSet,
                        ReadyFrame const& frame)
{
  Q_ASSERT(dataSet);

  this->addInput(dataSet, this->imagePort, frame.image);
  this->addInput(dataSet, this->namePort, frame.metaData.imageName());
  this->addInput(dataSet, this->timePort, frame.metaData.timeStamp());
}

// ----------------------------------------------------------------------------
template <typename T>
void PortSet::addInput(kwiver::adapter::adapter_data_set_t& dataSet,
                       std::string const& portName, T const& data)
{
  if (!portName.empty())
  {
    dataSet->add_value(portName, data);
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
  : super{RequiresInput, parent}
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

  if (source && !d->sources.contains(source))
  {
    d->sources.append(source);
    d->frames.append(source->frames());
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

  auto lastTime = std::numeric_limits<ts_time_t>::min();
  auto ports = QList<PortSet>{};
  auto sourcesToUse = QVector<VideoSource*>{};
  auto readyFrames = QVector<ReadyFrame>{};
  QEventLoop eventLoop;

  // For each source...
  for (auto const i : kvr::iota(sourcesCount))
  {
    // ...get ports...
    ports.append({pipeline, i});

    // ...and set up slots to add ready frames to the set
    connect(d->sources[i], &VideoSource::imageReady, &eventLoop,
            [i, &readyFrames](kv::image_container_sptr const& image,
                              VideoMetaData const& metaData)
            { readyFrames.append({i, image, metaData}); });
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

        sourcesToUse.append(d->sources[i]);
      }
    }

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
    readyFrames.clear();
    for (auto* const vs : sourcesToUse)
    {
      vs->seekTime(nextTime);
    }

    // Wait until all frames are ready
    while (readyFrames.count() < expectedFrames)
    {
      eventLoop.exec();
    }

    // Set up pipeline input...
    auto inputDataSet = kwiver::adapter::adapter_data_set::create();
    for (auto const& readyFrame : readyFrames)
    {
      Q_ASSERT(readyFrame.sourceIndex >= 0);
      Q_ASSERT(readyFrame.sourceIndex < ports.count());

      auto& p = ports[readyFrame.sourceIndex];
      p.addInputs(inputDataSet, readyFrame);
    }

    // ...and send it along
    if (!inputDataSet->empty())
    {
      pipeline.send(inputDataSet);
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
