/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_FileVideoSourceFactory_hpp
#define sealtk_core_FileVideoSourceFactory_hpp

#include <QObject>
#include <QString>
#include <qtGlobal.h>

#include <sealtk/core/VideoSourceFactory.hpp>

namespace sealtk
{

namespace core
{

class VideoController;
class VideoSource;

class FileVideoSourceFactoryPrivate;

class FileVideoSourceFactory : public VideoSourceFactory
{
  Q_OBJECT

public:
  explicit FileVideoSourceFactory(VideoController* parent = nullptr);
  ~FileVideoSourceFactory() override;

signals:
  void fileRequested(void* handle);

public slots:
  void loadVideoSource() override;
  virtual void loadFile(void* handle, QString const& path) = 0;

protected:
  QTE_DECLARE_PRIVATE(FileVideoSourceFactory)

  void freeHandle(void* handle);

private:
  QTE_DECLARE_PRIVATE_RPTR(FileVideoSourceFactory)
};

}

}

#endif
