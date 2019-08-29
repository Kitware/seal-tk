/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/AbstractDataSink.hpp>

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
AbstractDataSink::AbstractDataSink(QObject* parent) : QObject{parent}
{
}

// ----------------------------------------------------------------------------
AbstractDataSink::~AbstractDataSink()
{
}

} // namespace core

} // namespace sealtk
