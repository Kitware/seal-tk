/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoSource_hpp
#define sealtk_core_VideoSource_hpp

#include <sealtk/core/Export.h>
#include <sealtk/core/TimeMap.hpp>
#include <sealtk/core/VideoMetaData.hpp>

#include <vital/types/detected_object_set.h>
#include <vital/types/image_container.h>

#include <qtGlobal.h>

#include <QImage>
#include <QObject>

namespace sealtk
{

namespace core
{

class VideoSourcePrivate;

class SEALTK_CORE_EXPORT VideoSource : public QObject
{
  Q_OBJECT

public:
  explicit VideoSource(QObject* parent = nullptr);
  ~VideoSource() override;

  virtual TimeMap<kwiver::vital::timestamp::frame_t> frames() const = 0;

signals:
  void imageReady(
    kwiver::vital::image_container_sptr const& image,
    VideoMetaData const& metaData) const;
  void detectionsReady(
    kwiver::vital::detected_object_set_sptr const& detetedObjectSet,
    kwiver::vital::timestamp const& timeStamp) const;
  void videoInputChanged() const;

public slots:
  virtual void invalidate() const = 0;
  virtual void seek(kwiver::vital::timestamp::time_t time, SeekMode mode) = 0;
  virtual void seekFrame(kwiver::vital::timestamp::frame_t frame) = 0;

  virtual void seekTime(kwiver::vital::timestamp::time_t time)
  { seek(time, SeekExact); }

protected:
  QTE_DECLARE_PRIVATE(VideoSource)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoSource)
};

}

}

#endif
