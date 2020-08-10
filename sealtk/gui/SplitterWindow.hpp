/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_SplitterWindow_hpp
#define sealtk_gui_SplitterWindow_hpp

#include <sealtk/gui/Export.h>

#include <qtGlobal.h>

#include <QWidget>

namespace sealtk
{

namespace gui
{

class SplitterWindowPrivate;

class Window;

class SEALTK_GUI_EXPORT SplitterWindow : public QWidget
{
  Q_OBJECT

public:
  explicit SplitterWindow(QWidget* parent = nullptr);
  ~SplitterWindow() override;

  QWidget* centralWidget() const;
  void setCentralWidget(QWidget* widget);
  bool closable() const;
  void setClosable(bool closable);
  void setFilenameVisible(bool visible);
  void setFilename(QString const& filename);

signals:
  void visibilityChanged(bool visible);

protected:
  QTE_DECLARE_PRIVATE(SplitterWindow)

  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

private:
  QTE_DECLARE_PRIVATE_RPTR(SplitterWindow)
};

} // namespace gui

} // namespace sealtk

#endif
