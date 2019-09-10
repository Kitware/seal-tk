/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_Window_hpp
#define sealtk_noaa_gui_Window_hpp

#include <sealtk/noaa/gui/Export.h>

#include <QMainWindow>
#include <QPointF>
#include <QString>

#include <qtGlobal.h>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class WindowPrivate;

class SEALTK_NOAA_GUI_EXPORT Window : public QMainWindow
{
  Q_OBJECT

  Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
  Q_PROPERTY(QPointF center READ center WRITE setCenter NOTIFY centerChanged)

public:
  explicit Window(QWidget* parent = nullptr);
  ~Window() override;

  float zoom() const;
  QPointF center() const;

signals:
  void zoomChanged(float zoom) const;
  void centerChanged(QPointF center) const;

public slots:
  void setPipelineDirectory(QString const& directory);

  void showAbout();

  void setZoom(float zoom);
  void setCenter(QPointF center);

protected:
  QTE_DECLARE_PRIVATE(Window)

  void closeEvent(QCloseEvent* event) override;

private:
  QTE_DECLARE_PRIVATE_RPTR(Window)
};

} // namespace gui

} // namespace noaa

} // namespace sealtk

#endif
