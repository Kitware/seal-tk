/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_PlayerTool_hpp
#define sealtk_gui_PlayerTool_hpp

#include <sealtk/gui/Export.h>

#include <QObject>

#include <qtGlobal.h>

class QMouseEvent;

namespace sealtk
{

namespace gui
{

class Player;

class PlayerToolPrivate;

class SEALTK_GUI_EXPORT PlayerTool : public QObject
{
  Q_OBJECT

public:
  explicit PlayerTool(Player* parent = nullptr);
  ~PlayerTool();

  Player* player() const;

public slots:
  virtual void activate();
  virtual void deactivate();
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void paintGL();

protected:
  QTE_DECLARE_PRIVATE_RPTR(PlayerTool)

  void pushProperty(char const* name, QVariant const& value);

private:
  QTE_DECLARE_PRIVATE(PlayerTool)
};

} // namespace gui

} // namespace sealtk

#endif
