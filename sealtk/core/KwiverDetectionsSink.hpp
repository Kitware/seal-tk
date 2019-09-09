/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverDetectionsSink_hpp
#define sealtk_core_KwiverDetectionsSink_hpp

#include <sealtk/core/AbstractDataSink.hpp>

#include <qtGlobal.h>

class QAbstractItemModel;
class QUrl;

namespace sealtk
{

namespace core
{

class VideoSource;

class KwiverDetectionsSinkPrivate;

// ============================================================================
class SEALTK_CORE_EXPORT KwiverDetectionsSink : public AbstractDataSink
{
  Q_OBJECT

public:
  KwiverDetectionsSink(QObject* parent = nullptr);
  ~KwiverDetectionsSink() override;

  bool setData(VideoSource* video, QAbstractItemModel* model,
               bool includeHidden = false) override;
  void writeData(QUrl const& uri) const override;

protected:
  QTE_DECLARE_PRIVATE_RPTR(KwiverDetectionsSink)

private:
  QTE_DECLARE_PRIVATE(KwiverDetectionsSink)
};

} // namespace core

} // namespace sealtk

#endif
