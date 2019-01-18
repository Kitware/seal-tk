/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_lib_VideoSource_hpp
#define SEALTK_lib_VideoSource_hpp

#include <QImage>
#include <QObject>
#include <qtGlobal.h>

#include <vital/types/timestamp.h>
#include <vital/algo/video_input.h>

#include <set>

namespace sealtk
{

class VideoSourcePrivate;

class VideoSource : public QObject
{
  Q_OBJECT

public:
  explicit VideoSource(QObject* parent = nullptr);
  ~VideoSource() override;

  kwiver::vital::algo::video_input_sptr videoInput() const;
  void setVideoInput(kwiver::vital::algo::video_input_sptr const& videoInput);

  std::set<kwiver::vital::timestamp::time_t> times() const;

signals:
  void imageDisplayed(QImage const& image);
  void videoInputChanged();

public slots:
  void seek(kwiver::vital::timestamp::time_t time);

protected:
  QTE_DECLARE_PRIVATE(VideoSource)

private:
  QTE_DECLARE_PRIVATE_RPTR(VideoSource)
};

}

#endif
