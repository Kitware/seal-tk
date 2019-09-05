/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_CreateTrackPlayerTool_hpp
#define sealtk_gui_CreateTrackPlayerTool_hpp

#include <sealtk/gui/Export.h>
#include <sealtk/gui/PlayerTool.hpp>

#include <qtGlobal.h>

#include <QRectF>

namespace sealtk
{

namespace gui
{

class CreateTrackPlayerToolPrivate;

class SEALTK_GUI_EXPORT CreateTrackPlayerTool : public PlayerTool
{
  Q_OBJECT

public:
  CreateTrackPlayerTool(Player* parent = nullptr);
  ~CreateTrackPlayerTool();

  void activate() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintGL() override;

signals:
  void detectionCreated(QRectF const& detection) const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(CreateTrackPlayerTool)

protected:
  QTE_DECLARE_PRIVATE(CreateTrackPlayerTool)
};

}

}

#endif
