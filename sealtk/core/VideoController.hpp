/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoController_hpp
#define sealtk_core_VideoController_hpp

#include <sealtk/core/Export.h>

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

class SEALTK_CORE_EXPORT VideoController : public QObject
{
  Q_OBJECT

public:
  explicit VideoController(QObject* parent = nullptr);
  ~VideoController() override;

  QSet<VideoSource*> videoSources() const;
  void addVideoSource(VideoSource* videoSource);
  void removeVideoSource(VideoSource* videoSource);

  QSet<kwiver::vital::timestamp::time_t> times() const;

  kwiver::vital::timestamp::time_t time() const;

signals:
  void videoSourcesChanged();
  void timeSelected(kwiver::vital::timestamp::time_t time);

public slots:
  void seek(kwiver::vital::timestamp::time_t time);
  void seekNearest(kwiver::vital::timestamp::time_t time);
  void previousFrame();
  void nextFrame();

protected:
  QTE_DECLARE_PRIVATE(VideoController)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoController)
};

}

}

#endif
