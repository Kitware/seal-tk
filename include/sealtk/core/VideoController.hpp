/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoController_hpp
#define sealtk_core_VideoController_hpp

#include <QObject>
#include <QSet>
#include <qtGlobal.h>

#include <vital/types/timestamp.h>

namespace sealtk
{

namespace core
{

class VideoSource;

class VideoControllerPrivate;

class VideoController : public QObject
{
  Q_OBJECT

public:
  explicit VideoController(QObject* parent = nullptr);
  ~VideoController() override;

  QSet<VideoSource*> const& videoSources() const;
  void addVideoSource(VideoSource* videoSource);
  void removeVideoSource(VideoSource* videoSource);

signals:
  void videoSourcesChanged();
  void timestampSelected(kwiver::vital::timestamp::time_t time);

public slots:
  void seek(kwiver::vital::timestamp::time_t time);

protected:
  QTE_DECLARE_PRIVATE(VideoController)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoController)
};

}

}

#endif
