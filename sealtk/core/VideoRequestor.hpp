/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoRequestor_hpp
#define sealtk_core_VideoRequestor_hpp

#include <sealtk/core/Export.h>

#include <QObject>

namespace sealtk
{

namespace core
{

struct VideoFrame;
struct VideoRequest;
struct VideoRequestInfo;

// ============================================================================
class SEALTK_CORE_EXPORT VideoRequestor : public QObject
{
  Q_OBJECT

public:
  virtual ~VideoRequestor() = default;

protected:
  friend struct VideoRequest;

  explicit VideoRequestor(QObject* parent = nullptr);

  virtual void update(VideoRequestInfo const& requestInfo,
                      VideoFrame&& response) = 0;
};

} // namespace core

} // namespace sealtk

#endif
