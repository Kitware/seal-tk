/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoSource.hpp>

#include <sealtk/core/VideoFrame.hpp>
#include <sealtk/core/VideoProvider.hpp>
#include <sealtk/core/VideoRequest.hpp>
#include <sealtk/core/VideoRequestor.hpp>

#include <QDebug>
#include <QPointer>
#include <QThread>

#include <unordered_map>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoSourceThread : public QThread
{
public:
  VideoSourceThread(VideoSourcePrivate* q) : q_ptr{q} {}

protected:
  QTE_DECLARE_PUBLIC_PTR(VideoSourcePrivate);

  void run() override;

private:
  QTE_DECLARE_PUBLIC(VideoSourcePrivate);
};

// ============================================================================
class VideoSourcePrivate : public QObject
{
public:
  VideoSourcePrivate(VideoProvider* provider);

  void initializeProvider();
  void enqueueFrameRequest(VideoRequest&& request);
  void dispatchFrameRequests();

  std::unordered_map<VideoRequestor*, VideoRequest> requests;
  std::unordered_map<VideoRequestor*, kv::timestamp> lastFrameProvided;

  VideoSourceThread thread;
  QPointer<VideoProvider> const provider;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoSource)

// ----------------------------------------------------------------------------
VideoSource::VideoSource(VideoProvider* provider, QObject* parent)
  : QObject{parent}, d_ptr{new VideoSourcePrivate{provider}}
{
}

// ----------------------------------------------------------------------------
VideoSource::~VideoSource()
{
  this->cleanup();
}

// ----------------------------------------------------------------------------
void VideoSource::start()
{
  QTE_D();

  d->moveToThread(&d->thread);
  d->provider->moveToThread(&d->thread);
  d->thread.start();
}

// ----------------------------------------------------------------------------
void VideoSource::cleanup()
{
  QTE_D();

  if (d->thread.isRunning())
  {
    if (!d->provider)
    {
      qCritical()
        << this
        << "provider is destroyed but source's thread is still running";
      qDebug()
        << "(a derived type probably forgot to call cleanup())";
      qFatal("terminating execution as program is likely about to crash");
    }

    d->thread.exit();
    d->thread.wait();
  }
}

// ----------------------------------------------------------------------------
void VideoSource::requestFrame(VideoRequest&& request)
{
  QTE_D();

  if (!d->thread.isRunning())
  {
    this->start();
  }

  QMetaObject::invokeMethod(
    d, [d, request = std::move(request)]() mutable {
      d->enqueueFrameRequest(std::move(request));
    });
}

// ----------------------------------------------------------------------------
void VideoSource::requestFrame(VideoRequest const& request)
{
  auto requestCopy = request;
  return this->requestFrame(std::move(requestCopy));
}

// ----------------------------------------------------------------------------
void VideoSourceThread::run()
{
  QTE_Q();

  q->initializeProvider();
  this->exec();
}

// ----------------------------------------------------------------------------
VideoSourcePrivate::VideoSourcePrivate(VideoProvider* provider)
  : thread{this}, provider{provider}
{
}

// ----------------------------------------------------------------------------
void VideoSourcePrivate::initializeProvider()
{
  this->provider->initialize();
}

// ----------------------------------------------------------------------------
void VideoSourcePrivate::enqueueFrameRequest(VideoRequest&& request)
{
  // If this will be the first frame request of a batch (i.e. all previous
  // requests have been dispatched)...
  if (this->requests.empty())
  {
    // ...then schedule an event to dispatch this request (and any others that
    // are received between now and the next round of dispatching)
    QMetaObject::invokeMethod(
      this, &VideoSourcePrivate::dispatchFrameRequests,
      Qt::QueuedConnection);
  }

  // Enqueue request, potentially replacing existing request, and potentially
  // inheriting the request ID from the existing request
  auto* const requestor = request.requestor.get();
  auto& priorRequest = this->requests[requestor];
  if (request.requestId < 0)
  {
    request.requestId = priorRequest.requestId;
  }
  priorRequest = std::move(request);

  // Check if we have seen this request before
  if (!this->lastFrameProvided.count(requestor)) // TODO(C++20): use contains
  {
    // Set up a callback to erase our records of a requestor when it goes
    // away... note that this doesn't need to clean up the request queue, since
    // the request includes a smart pointer to the requestor, which means the
    // requestor cannot possibly go away while it has outstanding requests
    this->lastFrameProvided.insert({requestor, {}});
    connect(requestor, &QObject::destroyed, this,
            [this, requestor]{
              this->lastFrameProvided.erase(requestor);
            });
  }
}

// ----------------------------------------------------------------------------
void VideoSourcePrivate::dispatchFrameRequests()
{
  for (auto& request : this->requests)
  {
    Q_ASSERT(request.first == request.second.requestor.get());
    Q_ASSERT(this->lastFrameProvided.count(request.first)); // TODO(C++20)

    auto const& last = this->lastFrameProvided[request.first];
    auto const response =
      this->provider->processRequest(std::move(request.second), last);

    // Record the last time provided to the requestor, or send an empty frame
    // if a response is required
    if (response.is_valid())
    {
      this->lastFrameProvided[request.first] = response;
    }
    else if (request.second.requestId >= 0)
    {
      auto dummyFrame = VideoFrame{nullptr, VideoMetaData{}};
      request.second.sendReply(std::move(dummyFrame));
    }
  }
  this->requests.clear();
}

} // namespace core

} // namespace sealtk
