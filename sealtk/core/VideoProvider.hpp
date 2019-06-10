/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoProvider_hpp
#define sealtk_core_VideoProvider_hpp

#include <sealtk/core/Export.h>

#include <QObject>

namespace sealtk
{

namespace core
{

struct VideoRequest;

class VideoSourcePrivate;

// ============================================================================
/// Abstract video provider.
///
/// This class provides an interface for the threaded implementation of
/// #VideoSource implementations. Its use allows the base #VideoSource
/// implementation to interface with the concrete implementation in order to
/// manage request queuing and other common tasks related to the multi-threaded
/// nature of video sources.
class SEALTK_CORE_EXPORT VideoProvider : public QObject
{
protected:
  friend class VideoSourcePrivate;

  VideoProvider(QObject* parent = nullptr) : QObject{parent} {}
  virtual ~VideoProvider() = default;

  /// Initialize the video source.
  ///
  /// This method is called in the video source thread to prepare the video
  /// source for use. It executed from the video source's private thread.
  /// Implementations should strive to delay any long-running code until this
  /// method is called, in order to avoid blocking the UI.
  virtual void initialize() = 0;

  /// Process a video request.
  ///
  /// This method must be overridden by implementations. The #VideoSource
  /// implementation should handle the request by finding and replying with
  /// the requested frame and returning the timestamp of the same, or returning
  /// an invalid timestam if a) the request cannot be satisfied, or b) the
  /// request resolves to the same frame as \p lastTime.
  ///
  /// If an invalid timestamp is returned, the base VideoSource will take care
  /// of issuing an empty response if required.
  virtual kwiver::vital::timestamp processRequest(
    VideoRequest&& request, kwiver::vital::timestamp const& lastTime) = 0;
};

} // namespace core

} // namespace sealtk

#endif
