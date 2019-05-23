/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoRequest.hpp>

#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoRequestor.hpp>

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
void VideoRequest::sendReply(VideoFrame&& frame) const
{
  auto* const requestor = this->requestor.get();
  auto const info = VideoRequestInfo{*this};

  QMetaObject::invokeMethod(
    requestor, [requestor, info, frame = std::move(frame)]() mutable {
      requestor->update(info, std::move(frame));
    });
}

} // namespace core

} // namespace sealtk
