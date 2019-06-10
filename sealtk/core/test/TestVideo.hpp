/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_test_TestVideo_hpp
#define sealtk_core_test_TestVideo_hpp

#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

#include <vital/types/timestamp.h>

#include <QEventLoop>
#include <QVector>

#include <memory>

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
class TestVideoRequestor
  : public VideoRequestor,
    public std::enable_shared_from_this<TestVideoRequestor>
{
  Q_OBJECT

public:
  void request(VideoSource* source, kwiver::vital::timestamp::time_t time);

  QVector<VideoFrame> receivedFrames;

protected:
  QEventLoop eventLoop;

  void update(VideoRequestInfo const& requestInfo,
              VideoFrame&& response) override;
};

} // namespace test

} // namespace core

} // namespace sealtk

#endif
