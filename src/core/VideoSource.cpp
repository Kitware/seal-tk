/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/VideoSource.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class VideoSourcePrivate
{
public:
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(VideoSource)

// ----------------------------------------------------------------------------
VideoSource::VideoSource(QObject* parent)
  : QObject{parent},
    d_ptr{new VideoSourcePrivate}
{
}

// ----------------------------------------------------------------------------
VideoSource::~VideoSource()
{
}

}

}
