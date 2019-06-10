/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoDistributor_hpp
#define sealtk_core_VideoDistributor_hpp

#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoFrame.hpp>

#include <qtGlobal.h>

#include <QObject>

namespace sealtk
{

namespace core
{

class VideoSource;

class VideoDistributorPrivate;

// ============================================================================
class SEALTK_CORE_EXPORT VideoDistributor : public QObject
{
  Q_OBJECT

public:
  explicit VideoDistributor(QObject* parent = nullptr);

  ~VideoDistributor() override;

signals:
  /// Emitted when a frame is obtained in response to a request made by this
  /// distributor.
  void frameReady(VideoFrame const& frame, qint64 requestId);

  /// Emitted when a request made by this distributor is declined.
  void requestDeclined(qint64 requestId);

public slots:
  /// Request video.
  ///
  /// This method is used to request a video frame from the specified
  /// \p videoSource.
  ///
  /// \sa VideoSource::requestFrame, VideoRequest
  void requestFrame(VideoSource* videoSource,
                    kwiver::vital::timestamp::time_t time,
                    SeekMode mode, qint64 requestId = -1);

protected:
  QTE_DECLARE_PRIVATE(VideoDistributor)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoDistributor)
};

} // namespace core

} // namespace sealtk

#endif
