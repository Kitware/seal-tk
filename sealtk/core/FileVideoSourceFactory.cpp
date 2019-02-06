/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/FileVideoSourceFactory.hpp>

namespace sealtk
{

namespace core
{

// ============================================================================
class FileVideoSourceFactoryPrivate
{
public:
};

// ----------------------------------------------------------------------------
FileVideoSourceFactory::FileVideoSourceFactory(VideoController* parent)
  : VideoSourceFactory{parent},
    d_ptr{new FileVideoSourceFactoryPrivate}
{
}

// ----------------------------------------------------------------------------
FileVideoSourceFactory::~FileVideoSourceFactory()
{
}

// ----------------------------------------------------------------------------
bool FileVideoSourceFactory::expectsDirectory() const
{
  return false;
}

// ----------------------------------------------------------------------------
void FileVideoSourceFactory::loadVideoSource(void* handle)
{
  emit this->fileRequested(handle);
}

}

}
