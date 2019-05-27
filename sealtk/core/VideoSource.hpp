/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoSource_hpp
#define sealtk_core_VideoSource_hpp

#include <sealtk/core/Export.h>
#include <sealtk/core/TimeMap.hpp>

#include <qtGlobal.h>

#include <QObject>

namespace sealtk
{

namespace core
{

class VideoProvider;

struct VideoRequest;

class VideoSourcePrivate;

// ============================================================================
class SEALTK_CORE_EXPORT VideoSource : public QObject
{
  Q_OBJECT

public:
  ~VideoSource() override;

  /// Query if the video source is "ready".
  ///
  /// This method is used to determine if the video source is "ready". In
  /// particular, this method can be used to determine if #frames will return
  /// meaningful data. A source will not return \c true until #start has been
  /// called, and until the initial frame set has been passed from the source's
  /// internal thread to the thread which owns the #VideoSource object.
  virtual bool isReady() const = 0;

  /// Get the set of frames for which this source has video.
  ///
  /// This method returns a map of times for which the video source has video,
  /// mapped to their corresponding frames. Note that calling this method
  /// before #framesChanged() is emitted may return an empty map.
  virtual TimeMap<kwiver::vital::timestamp::frame_t> frames() const = 0;

signals:
  /// Emitted when the set of available frames changes.
  ///
  /// This signal is emitted when the set of available frames changes. While
  /// this has obvious use for "streaming" sources (that is, the set of frames
  /// is properly time variable), even "static" sources, because video is
  /// normally loaded asynchronously, should emit this signal at least once to
  /// indicate when they are ready for users to call #frames.
  void framesChanged();

public slots:
  /// "Start" the video source.
  ///
  /// This method finalizes the construction of a video source. It must be
  /// called prior to using the video source, preferably by whatever code
  /// constructed the video source. If necessary, it will be called by
  /// #requestFrame, however this may impose a non-trivial latency on initial
  /// requests if the source must perform work in the source thread to prepare
  /// the source for use.
  ///
  /// The video source will not emit signals until this method has been called.
  /// If necessary, this gives users a chance to connect to #framesChanged
  ///
  /// \note
  ///   It is safe to call this method more than once, as long as it is not
  ///   called during destruction of the #VideoSource. Once this method is
  ///   called, the underlying #VideoProvider is moved to the video source's
  ///   service thread.
  void start();

  //@{
  /// Request video.
  ///
  /// This method is used to request a video frame from the source. If the
  /// requestor has previously requested a frame, and the request would
  /// otherwise result in the same frame being provided, then no frame is
  /// returned, and a response will only be sent if the requestor is waiting
  /// on a request with a non-negative request identifier.
  ///
  /// Note that it is slightly more efficient to provide the request as an
  /// r-value reference, as this will usually reduce the number of reference
  /// counting operations required to service the request.
  ///
  /// \sa ready
  void requestFrame(VideoRequest&& request);
  void requestFrame(VideoRequest const& request);
  //@}

protected:
  explicit VideoSource(VideoProvider* provider, QObject* parent = nullptr);

  /// Clean up the video source.
  ///
  /// This method cleans up the video source, in particular, by signaling the
  /// internal thread to terminate and waiting for it to do so. This method
  /// should be called from the destructor of classes inheriting #VideoSource.
  /// This is especially critical if the #VideoProvider is owned by the derived
  /// class (which is usually the case, and indeed, is recommended), as
  /// otherwise the provider may be destroyed while its thread is still
  /// executing.
  ///
  /// \note
  ///   It is safe to call this method more than once. Because failing to call
  ///   this method is very likely to cause the program to crash or otherwise
  ///   fall into the Dread Realm of Undefined Behavior, the base class
  ///   attempts to detect such failure and will issue a fatal program error in
  ///   such cases.
  void cleanup();

  QTE_DECLARE_PRIVATE(VideoSource)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoSource)
};

} // namespace core

} // namespace sealtk

#endif
