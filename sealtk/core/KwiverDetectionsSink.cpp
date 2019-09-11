/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverDetectionsSink.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoMetaData.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <vital/algo/detected_object_set_output.h>

#include <vital/range/indirect.h>
#include <vital/range/iota.h>

#include <qtStlUtil.h>

#include <QAbstractItemModel>
#include <QUrl>
#include <QUrlQuery>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

// ============================================================================
class KwiverDetectionsSinkPrivate
{
public:
  struct Frame
  {
    kv::path_t name;
    QModelIndexList detections;
  };

  QAbstractItemModel* model = nullptr;
  TimeMap<Frame> frames;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverDetectionsSink)

// ----------------------------------------------------------------------------
KwiverDetectionsSink::KwiverDetectionsSink(QObject* parent)
  : AbstractDataSink{parent}, d_ptr{new KwiverDetectionsSinkPrivate}
{
}

// ----------------------------------------------------------------------------
KwiverDetectionsSink::~KwiverDetectionsSink()
{
}

// ----------------------------------------------------------------------------
bool KwiverDetectionsSink::setData(
  VideoSource* video, QAbstractItemModel* model, bool includeHidden)
{
  QTE_D();

  Q_ASSERT(video);

  d->model = model;
  d->frames.clear();

  // Extract frame names from video
  auto haveData = false;
  for (auto const& md : video->metaData() | kvr::indirect)
  {
    d->frames.insert(md.key(), {md.value().imageName(), {}});
  }

  // Extract detections (if any)
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

        // Look up entry in frame map
        auto const fi = d->frames.find(t, SeekExact);
        if (fi != d->frames.end())
        {
          // Add detection to map
          fi->detections.append(jIndex);
          haveData = true;
        }
      }
    }
  }

  return haveData;
}

// ----------------------------------------------------------------------------
void KwiverDetectionsSink::writeData(QUrl const& uri) const
{
  QTE_D();

  auto config = kv::config_block::empty_config();
  auto params = QUrlQuery{uri};
  for (auto const& p : params.queryItems())
  {
    config->set_value(stdString(p.first), stdString(p.second));
  }

  try
  {
    // Create algorithm to write detections
    kva::detected_object_set_output_sptr writer;
    kva::detected_object_set_output::set_nested_algo_configuration(
      "output", config, writer);

    if (!writer)
    {
      emit this->failed(
        QStringLiteral("KwiverDetectionsSink::writeData: "
                       "Writer could not be configured"));
      return;
    }

    writer->open(stdString(uri.toLocalFile()));

    // Iterate over frames
    for (auto const& f : d->frames)
    {
      // Generate detection set
      auto ds = std::make_shared<kv::detected_object_set>();
      for (auto const& i : f.detections)
      {
        auto const& qbox = d->model->data(i, AreaLocationRole).toRectF();
        auto kbox = kv::bounding_box_d{{qbox.left(), qbox.top()},
                                       qbox.width(), qbox.height()};

        ds->add(std::make_shared<kv::detected_object>(kbox));
      }

      writer->write_set(ds, f.name);
    }
  }
  catch (std::exception const& e)
  {
    emit this->failed(QString::fromLocal8Bit(e.what()));
  }
}

} // namespace core

} // namespace sealtk
