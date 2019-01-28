/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_About_hpp
#define sealtk_noaa_gui_About_hpp

#include <sealtk/noaa/gui/Export.h>

#include <QDialog>
#include <qtGlobal.h>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class AboutPrivate;

class SEALTK_NOAA_GUI_EXPORT About : public QDialog
{
  Q_OBJECT

public:
  explicit About(QWidget* parent = nullptr);
  ~About() override;

protected:
  QTE_DECLARE_PRIVATE(About)

private:
  QTE_DECLARE_PRIVATE_RPTR(About)
};

}

}

}

#endif
