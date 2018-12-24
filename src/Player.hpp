/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_Player_hpp
#define SEALTK_Player_hpp

#include <QMainWindow>
#include <qtGlobal.h>

namespace sealtk
{

class PlayerPrivate;

class Player : public QMainWindow
{
  Q_OBJECT

public:
  explicit Player(QWidget* parent = nullptr);
  ~Player() override;

protected:
  QTE_DECLARE_PRIVATE_RPTR(Player)

private:
  QTE_DECLARE_PRIVATE(Player)
};

}

#endif
