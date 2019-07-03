/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_KwiverTrackSource_hpp
#define sealtk_core_KwiverTrackSource_hpp

#include <sealtk/core/Export.h>

#include <sealtk/core/AbstractDataSource.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class KwiverTrackSourcePrivate;

// ============================================================================
class SEALTK_CORE_EXPORT KwiverTrackSource : public AbstractDataSource
{
  Q_OBJECT

public:
  explicit KwiverTrackSource(QObject* parent = nullptr);
  ~KwiverTrackSource() override;

  bool active() const override;
  bool readData(QUrl const& uri) override;

protected:
  QTE_DECLARE_PRIVATE_RPTR(KwiverTrackSource)

private:
  QTE_DECLARE_PRIVATE(KwiverTrackSource)
};

} // namespace core

} // namespace sealtk

#endif
