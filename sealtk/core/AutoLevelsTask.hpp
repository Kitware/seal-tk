/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_AutoLevelsTask_hpp
#define sealtk_core_AutoLevelsTask_hpp

#include <sealtk/core/Export.h>

#include <vital/types/image_container.h>

#include <qtGlobal.h>

#include <QObject>

namespace sealtk
{

namespace core
{

class AutoLevelsTaskPrivate;

class SEALTK_CORE_EXPORT AutoLevelsTask : public QObject
{
  Q_OBJECT

public:
  explicit AutoLevelsTask(
    kwiver::vital::image_container_sptr const& image,
    double outlierDeviance, double outlierTolerance,
    QObject* parent = nullptr);
  ~AutoLevelsTask() override;

signals:
  void levelsUpdated(float min, float max);

public slots:
  virtual void execute();

protected:
  QTE_DECLARE_PRIVATE(AutoLevelsTask)

private:
  QTE_DECLARE_PRIVATE_RPTR(AutoLevelsTask)
};

} // namespace core

} // namespace sealtk

#endif
