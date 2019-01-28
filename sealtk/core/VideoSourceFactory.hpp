/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoSourceFactory_hpp
#define sealtk_core_VideoSourceFactory_hpp

#include <sealtk/core/Export.h>

#include <QObject>
#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class VideoController;
class VideoSource;

class VideoSourceFactoryPrivate;

class SEALTK_CORE_EXPORT VideoSourceFactory : public QObject
{
  Q_OBJECT

public:
  explicit VideoSourceFactory(VideoController* parent = nullptr);
  ~VideoSourceFactory() override;

  VideoController* videoController() const;

signals:
  void videoSourceLoaded(VideoSource* videoSource);

public slots:
  virtual void loadVideoSource() = 0;

protected:
  QTE_DECLARE_PRIVATE(VideoSourceFactory)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoSourceFactory)
};

}

}

#endif
