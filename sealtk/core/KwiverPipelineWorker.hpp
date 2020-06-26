/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverPipelineWorker_hpp
#define sealtk_core_KwiverPipelineWorker_hpp

#include <sealtk/core/Export.h>

#include <arrows/qt/EmbeddedPipelineWorker.h>

#include <vital/types/transform_2d.h>

#include <qtGlobal.h>

class QAbstractItemModel;

namespace sealtk
{

namespace core
{

class VideoSource;

class KwiverPipelineWorkerPrivate;

// ----------------------------------------------------------------------------
class SEALTK_CORE_EXPORT KwiverPipelineWorker
  : public kwiver::arrows::qt::EmbeddedPipelineWorker
{
  Q_OBJECT

public:
  explicit KwiverPipelineWorker(QWidget* parent = nullptr);
  explicit KwiverPipelineWorker(RequiredEndcaps, QWidget* parent = nullptr);

  ~KwiverPipelineWorker() override;

  /// Add video source.
  ///
  /// This method adds a video source to the pipeline. Video sources are
  /// assigned ports according to the order in which they are added.
  void addVideoSource(VideoSource* source);

  /// Add (primary) track source for a video source.
  ///
  /// This method adds track data for a video source. This \em must be called
  /// once per video source, or the video and track data will get out of sync.
  /// If a video source does not have any tracks, call this method with a null
  /// model pointer.
  ///
  /// Users that do not provide tracks do not need to call this method.
  void addTrackSource(QAbstractItemModel* model, bool includeHidden = false);

  /// Add a supplemental track source for a video source.
  ///
  /// This method adds supplemental track data for a video source. The data is
  /// combined with the most recently added or supplemented track data. In case
  /// of overlap, the existing data is retained.
  ///
  /// Supplemental data exists in a different coordinate space than the primary
  /// data. The provided transform is used to map supplemental data into the
  /// coordinate space of the primary data.
  void addTrackSource(QAbstractItemModel* model,
                      kwiver::vital::transform_2d const& transform,
                      bool includeHidden = false);

signals:
  void progressRangeChanged(int minimum, int maximum);
  void progressValueChanged(int value);

protected:
  QTE_DECLARE_PRIVATE_RPTR(KwiverPipelineWorker);

  void initializeInput(kwiver::embedded_pipeline& pipeline) override;

  void sendInput(kwiver::embedded_pipeline& pipeline) override;

  void reportError(QString const& message, QString const& subject) override;

private:
  QTE_DECLARE_PRIVATE(KwiverPipelineWorker);
};

} // namespace core

} // namespace sealtk

#endif
