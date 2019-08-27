/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoRequest_hpp
#define sealtk_core_VideoRequest_hpp

#include <sealtk/core/Export.h>
#include <sealtk/core/TimeMap.hpp>

#include <qtTransferablePointer.h>

#include <memory>

namespace sealtk
{

namespace core
{

class VideoRequestor;

struct VideoFrame;

// ============================================================================
/// Common information for a video request.
///
/// This structure provides a subset of the information that comprises a
/// complete video request. This subset consists of data which is trivial to
/// copy (in particular, it excludes the shared pointer to the requestor), and
/// is used when sending a response to a request.
struct SEALTK_CORE_EXPORT VideoRequestInfo
{
  /// Unique identifier of this request.
  ///
  /// This field provides a unique (to the requestor) identifier for the
  /// request which requestors can use to correlate requests to replies. This
  /// is useful in some situations since sources normally reply only to the
  /// most recent request. A negative value is "invalid"; if the identifier is
  /// negative, the source will reuse the most recent non-negative identifier
  /// when replying. Additionally, if a request cannot be satisfied, the source
  /// will only issue a response if the outstanding request identifier is
  /// non-negative.
  qint64 requestId = -1;

  /// Desired time of video to retrieve.
  ///
  /// This field specifies the desired time of the video frame to being
  /// requested. The actual time that will be retrieved also depends on the
  /// seek mode.
  kwiver::vital::timestamp::time_t time;

  /// Temporal seek mode.
  ///
  /// This field specifies how the requested time point should be interpreted.
  SeekMode mode = SeekNearest;
};

// ============================================================================
/// Video request.
///
/// This structure fully describes a video request. Most of the information is
/// carried by the base #VideoRequestInfo structure.
struct SEALTK_CORE_EXPORT VideoRequest : VideoRequestInfo
{
  /// Send reply to the request.
  ///
  /// This method invokes VideoRequestor::update on the requestor with the
  /// supplied \p response and a copy of the request information. If the
  /// requestor's event loop is running in a thread other than the thread which
  /// calls this method, the response will be delivered asynchronously.
  ///
  /// \sa requestId
  void sendReply(VideoFrame&& response) const;

  /// Pointer to requestor.
  ///
  /// This field provides a shared reference to the requestor that issued this
  /// request. The use of a shared pointer ensures that the video source is
  /// able to safely issue a response without being subject to race conditions
  /// if the video source and the final consumer of the video live in separate
  /// threads.
  std::shared_ptr<VideoRequestor> requestor;
};

} // namespace core

} // namespace sealtk

#endif
