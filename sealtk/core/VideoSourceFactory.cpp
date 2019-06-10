/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoSourceFactory.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoSourceFactoryPrivate
{
public:
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoSourceFactory)

// ----------------------------------------------------------------------------
VideoSourceFactory::VideoSourceFactory(QObject* parent)
  : QObject{parent}, d_ptr{new VideoSourceFactoryPrivate}
{
}

// ----------------------------------------------------------------------------
VideoSourceFactory::~VideoSourceFactory()
{
}

}

}
