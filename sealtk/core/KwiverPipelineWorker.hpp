/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverPipelineWorker_hpp
#define sealtk_core_KwiverPipelineWorker_hpp

#include <arrows/qt/EmbeddedPipelineWorker.h>

#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class VideoSource;

class KwiverPipelineWorkerPrivate;

// ----------------------------------------------------------------------------
class KwiverPipelineWorker : public kwiver::arrows::qt::EmbeddedPipelineWorker
{
  Q_OBJECT

public:
  explicit KwiverPipelineWorker(QWidget* parent = nullptr);
  explicit KwiverPipelineWorker(RequiredEndcaps, QWidget* parent = nullptr);

  ~KwiverPipelineWorker() override;

  void addVideoSource(VideoSource* source);

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
