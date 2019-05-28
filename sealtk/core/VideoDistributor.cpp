/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoDistributor.hpp>

#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>
#include <sealtk/core/VideoSource.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoDistributorRequestor : public VideoRequestor
{
public:
  VideoDistributorRequestor(VideoDistributorPrivate* q) : q_ptr{q} {}

protected:
  void update(VideoRequestInfo const& requestInfo,
              VideoFrame&& response) override;

private:
  QTE_DECLARE_PUBLIC_PTR(VideoDistributorPrivate);
  QTE_DECLARE_PUBLIC(VideoDistributorPrivate);
};

// ============================================================================
class VideoDistributorPrivate
{
public:
  VideoDistributorPrivate(VideoDistributor* q);

  std::shared_ptr<VideoDistributorRequestor> const requestor;

  void update(qint64 requestId, VideoFrame const& response);

private:
  QTE_DECLARE_PUBLIC_PTR(VideoDistributor);
  QTE_DECLARE_PUBLIC(VideoDistributor);
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoDistributor)

// ----------------------------------------------------------------------------
VideoDistributor::VideoDistributor(QObject* parent)
  : QObject{parent}, d_ptr{new VideoDistributorPrivate{this}}
{
}

// ----------------------------------------------------------------------------
VideoDistributor::~VideoDistributor()
{
}

// ----------------------------------------------------------------------------
void VideoDistributor::requestFrame(
  VideoSource* videoSource, kwiver::vital::timestamp::time_t time,
  SeekMode mode, qint64 requestId)
{
  QTE_D();

  VideoRequest request;
  request.requestor = d->requestor;
  request.requestId = requestId;
  request.time = time;
  request.mode = mode;

  videoSource->requestFrame(std::move(request));
}

// ----------------------------------------------------------------------------
VideoDistributorPrivate::VideoDistributorPrivate(VideoDistributor* q)
  : requestor{std::make_shared<VideoDistributorRequestor>(this)}, q_ptr{q}
{
}

// ----------------------------------------------------------------------------
void VideoDistributorPrivate::update(
  qint64 requestId, VideoFrame const& response)
{
  QTE_Q();

  if (response.image)
  {
    emit q->frameReady(response, requestId);
  }
  else
  {
    emit q->requestDeclined(requestId);
  }
}

// ----------------------------------------------------------------------------
void VideoDistributorRequestor::update(
  VideoRequestInfo const& requestInfo, VideoFrame&& response)
{
  QTE_Q();
  q->update(requestInfo.requestId, response);
}

} // namespace core

} // namespace sealtk
