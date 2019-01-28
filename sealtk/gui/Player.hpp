/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Player_hpp
#define sealtk_gui_Player_hpp

#include <sealtk/gui/Export.h>

#include <sealtk/gui/Panel.hpp>

#include <qtGlobal.h>

class QImage;

namespace sealtk
{

namespace gui
{

class PlayerPrivate;

class Window;

class SEALTK_GUI_EXPORT Player : public Panel
{
  Q_OBJECT

public:
  Q_INVOKABLE explicit Player(QWidget* parent = nullptr);
  ~Player() override;

  void init(Window* window) override;

signals:
  void imageOpened(const QImage& image);

public slots:
  void openFile();

protected:
  QTE_DECLARE_PRIVATE(Player)

private:
  QTE_DECLARE_PRIVATE_RPTR(Player)
};

}

}

#endif
