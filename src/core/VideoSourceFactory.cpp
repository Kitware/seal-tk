/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/core/VideoController.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoSourceFactoryPrivate
{
public:
  VideoSourceFactoryPrivate(VideoController* videoController);

  VideoController* videoController;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoSourceFactory)

// ----------------------------------------------------------------------------
VideoSourceFactory::VideoSourceFactory(VideoController* parent)
  : QObject{parent},
    d_ptr{new VideoSourceFactoryPrivate{parent}}
{
}

// ----------------------------------------------------------------------------
VideoSourceFactory::~VideoSourceFactory()
{
}

// ----------------------------------------------------------------------------
VideoController* VideoSourceFactory::videoController() const
{
  QTE_D();
  return d->videoController;
}

// ----------------------------------------------------------------------------
VideoSourceFactoryPrivate::VideoSourceFactoryPrivate(
  VideoController* videoController)
  : videoController{videoController}
{
}

}

}
