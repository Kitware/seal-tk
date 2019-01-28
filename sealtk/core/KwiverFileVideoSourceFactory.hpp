/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverFileVideoSourceFactory_hpp
#define sealtk_core_KwiverFileVideoSourceFactory_hpp

#include <sealtk/core/Export.h>

#include <QObject>
#include <QString>
#include <qtGlobal.h>

#include <sealtk/core/FileVideoSourceFactory.hpp>

#include <vital/config/config_block.h>

namespace sealtk
{

namespace core
{

class VideoController;
class VideoSource;

class KwiverFileVideoSourceFactoryPrivate;

class SEALTK_CORE_EXPORT KwiverFileVideoSourceFactory :
  public FileVideoSourceFactory
{
  Q_OBJECT

public:
  explicit KwiverFileVideoSourceFactory(VideoController* parent = nullptr);
  ~KwiverFileVideoSourceFactory() override;

public slots:
  void loadFile(void* handle, QString const& path) override;

protected:
  QTE_DECLARE_PRIVATE(KwiverFileVideoSourceFactory)

  virtual kwiver::vital::config_block_sptr config(
    QString const& path) const = 0;

private:
  QTE_DECLARE_PRIVATE_RPTR(KwiverFileVideoSourceFactory)
};

}

}

#endif
