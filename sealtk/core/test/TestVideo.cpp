/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/test/TestVideo.hpp>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace test
{

// ----------------------------------------------------------------------------
void TestVideoRequestor::request(
  VideoSource* source, kv::timestamp::time_t time)
{
  // Build request
  VideoRequest request;
  request.requestId = 0; // Require a response
  request.requestor = this->shared_from_this();
  request.mode = SeekExact;
  request.time = time;

  // Issue request
  source->requestFrame(std::move(request));

  // Wait for response
  this->eventLoop.exec();
}

// ----------------------------------------------------------------------------
void TestVideoRequestor::update(
  VideoRequestInfo const& requestInfo, VideoFrame&& response)
{
  Q_UNUSED(requestInfo);

  this->receivedFrames.append(std::move(response));
  this->eventLoop.quit();
}

} // namespace test

} // namespace core

} // namespace sealtk
