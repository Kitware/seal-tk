/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoMetaData_hpp
#define sealtk_core_VideoMetaData_hpp

#include <sealtk/core/Export.h>

#include <vital/types/timestamp.h>
#include <vital/vital_types.h>

#include <qtGlobal.h>

#include <QSharedDataPointer>

namespace sealtk
{

namespace core
{

class VideoMetaDataData;

class SEALTK_CORE_EXPORT VideoMetaData
{
public:
  explicit VideoMetaData(kwiver::vital::timestamp const& timeStamp = {},
                         kwiver::vital::path_t const& imageName = {});
  ~VideoMetaData();

  kwiver::vital::timestamp timeStamp() const;
  kwiver::vital::path_t imageName() const;

  void setTimeStamp(kwiver::vital::timestamp const&);
  void setImageName(kwiver::vital::path_t const&);

private:
  QTE_DECLARE_SHARED_PTR(VideoMetaData)
  QTE_DECLARE_SHARED(VideoMetaData)
};

}

}

#endif
