/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_Panel_hpp
#define SEALTK_Panel_hpp

#include <QMainWindow>
#include <qtGlobal.h>

namespace sealtk
{

class PanelPrivate;

class Window;

class Panel : public QMainWindow
{
  Q_OBJECT

public:
  explicit Panel(QWidget* parent = nullptr);
  ~Panel() override;

  virtual void init(Window* window);

protected:
  QTE_DECLARE_PRIVATE_RPTR(Panel)

private:
  QTE_DECLARE_PRIVATE(Panel)
};

}

#endif
