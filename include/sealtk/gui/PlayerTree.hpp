/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_PlayerTree_hpp
#define sealtk_gui_PlayerTree_hpp

#include <sealtk/gui/Panel.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

class PlayerTreePrivate;

class Window;

class PlayerTree : public Panel
{
  Q_OBJECT

public:
  Q_INVOKABLE explicit PlayerTree(QWidget* parent = nullptr);
  ~PlayerTree() override;

  void init(Window* window) override;

protected:
  QTE_DECLARE_PRIVATE(PlayerTree)

private:
  QTE_DECLARE_PRIVATE_RPTR(PlayerTree)
};

}

}

#endif
