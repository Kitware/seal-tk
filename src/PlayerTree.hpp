/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_PlayerTree_hpp
#define SEALTK_PlayerTree_hpp

#include "Panel.hpp"

#include <qtGlobal.h>

namespace sealtk
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
  QTE_DECLARE_PRIVATE_RPTR(PlayerTree)

private:
  QTE_DECLARE_PRIVATE(PlayerTree)
};

}

#endif
