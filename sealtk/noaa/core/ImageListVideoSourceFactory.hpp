/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_core_ImageListVideoSourceFactory_hpp
#define sealtk_noaa_core_ImageListVideoSourceFactory_hpp

#include <QObject>
#include <qtGlobal.h>

#include <sealtk/noaa/core/Export.h>

#include <sealtk/core/KwiverFileVideoSourceFactory.hpp>

namespace sealtk
{

namespace core
{

class VideoController;

}

namespace noaa
{

namespace core
{

class ImageListVideoSourceFactoryPrivate;

class SEALTK_NOAA_CORE_EXPORT ImageListVideoSourceFactory :
  public sealtk::core::KwiverFileVideoSourceFactory
{
  Q_OBJECT

public:
  explicit ImageListVideoSourceFactory(
    bool directory, sealtk::core::VideoController* parent = nullptr);
  ~ImageListVideoSourceFactory() override;

  bool expectsDirectory() const override;

protected:
  QTE_DECLARE_PRIVATE(ImageListVideoSourceFactory)

  kwiver::vital::config_block_sptr config(QString const& path) const override;

private:
  QTE_DECLARE_PRIVATE_RPTR(ImageListVideoSourceFactory)
};

}

}

}

#endif
