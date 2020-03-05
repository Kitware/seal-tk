/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_CreateDetectionPlayerTool_hpp
#define sealtk_gui_CreateDetectionPlayerTool_hpp

#include <sealtk/gui/PlayerTool.hpp>

#include <sealtk/gui/Export.h>

#include <QRectF>

#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

class CreateDetectionPlayerToolPrivate;

class SEALTK_GUI_EXPORT CreateDetectionPlayerTool : public PlayerTool
{
  Q_OBJECT

public:
  CreateDetectionPlayerTool(Player* parent = nullptr);
  ~CreateDetectionPlayerTool();

signals:
  void detectionCreated(QRectF const& detection) const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(CreateDetectionPlayerTool)

  void activate() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintGL() override;

private:
  QTE_DECLARE_PRIVATE(CreateDetectionPlayerTool)
};

} // namespace gui

} // namespace sealtk

#endif
