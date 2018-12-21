/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_About_hpp
#define SEALTK_About_hpp

#include <QDialog>
#include <qtGlobal.h>

namespace sealtk
{

class AboutPrivate;

class About : public QDialog
{
  Q_OBJECT

public:
  explicit About(QWidget* parent = nullptr);
  ~About() override;

protected:
  QTE_DECLARE_PRIVATE_RPTR(About)

private:
  QTE_DECLARE_PRIVATE(About)
};

}

#endif
