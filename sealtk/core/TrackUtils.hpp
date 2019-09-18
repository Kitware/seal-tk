/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_TrackUtils_hpp
#define sealtk_core_TrackUtils_hpp

#include <sealtk/core/Export.h>

#include <vital/types/detected_object_type.h>

class QString;
class QVariant;

template <typename K, typename V> class QHash;

using QVariantHash = QHash<QString, QVariant>;

namespace sealtk
{

namespace core
{

kwiver::vital::detected_object_type_sptr
SEALTK_CORE_EXPORT classificationToDetectedObjectType(
  QVariantHash const& classification);

} // namespace core

} // namespace sealtk

#endif
