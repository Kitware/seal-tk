/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/AbstractDataSink.hpp>

namespace kv = kwiver::vital;

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

// ----------------------------------------------------------------------------
bool AbstractDataSink::setTransform(kv::transform_2d_sptr const& transform)
{
  Q_UNUSED(transform)
  return false;
}

// ----------------------------------------------------------------------------
bool AbstractDataSink::addData(
  QAbstractItemModel* model, kwiver::vital::transform_2d_sptr const& transform,
  bool includeHidden)
{
  Q_UNUSED(model)
  Q_UNUSED(transform)
  Q_UNUSED(includeHidden)
  return false;
}

} // namespace core

} // namespace sealtk
