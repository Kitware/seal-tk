/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_GlobInputDialog_hpp
#define sealtk_gui_GlobInputDialog_hpp

#include <sealtk/gui/Export.h>

#include <qtGlobal.h>

#include <QDialog>

namespace sealtk
{

namespace gui
{

class GlobInputDialogPrivate;

class SEALTK_GUI_EXPORT GlobInputDialog : public QDialog
{
  Q_OBJECT

  Q_PROPERTY(QString globString READ globString WRITE setGlobString)

public:
  explicit GlobInputDialog(QString const& settingsKey,
                           QWidget* parent = nullptr,
                           Qt::WindowFlags flags = {});
  ~GlobInputDialog() override;

  QString globString() const;

public slots:
  void setGlobString(QString const&);

  void accept() override;

protected:
  QTE_DECLARE_PRIVATE(GlobInputDialog)

private:
  QTE_DECLARE_PRIVATE_RPTR(GlobInputDialog)
};

} // namespace gui

} // namespace sealtk

#endif
