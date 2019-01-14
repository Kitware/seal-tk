/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_app_SplitterWindow_hpp
#define SEALTK_app_SplitterWindow_hpp

#include <QWidget>
#include <qtGlobal.h>

namespace sealtk
{

class SplitterWindowPrivate;

class Window;

class SplitterWindow : public QWidget
{
  Q_OBJECT

public:
  explicit SplitterWindow(QWidget* parent = nullptr);
  ~SplitterWindow() override;

  QWidget* centralWidget() const;
  void setCentralWidget(QWidget* widget);

protected:
  QTE_DECLARE_PRIVATE(SplitterWindow)

private:
  QTE_DECLARE_PRIVATE_RPTR(SplitterWindow)
};

}

#endif
