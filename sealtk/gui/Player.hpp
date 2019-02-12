/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Player_hpp
#define sealtk_gui_Player_hpp

#include <sealtk/gui/Export.h>

#include <QMatrix3x3>
#include <QOpenGLWidget>
#include <qtGlobal.h>

#include <vital/types/image_container.h>

class QImage;

namespace sealtk
{

namespace gui
{

class PlayerPrivate;

class SEALTK_GUI_EXPORT Player : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit Player(QWidget* parent = nullptr);
  ~Player() override;

public slots:
  void setImage(kwiver::vital::image_container_sptr const& image);
  void setHomography(QMatrix3x3 const& homography);

protected:
  QTE_DECLARE_PRIVATE(Player)

  void initializeGL() override;
  void paintGL() override;

private:
  QTE_DECLARE_PRIVATE_RPTR(Player)
};

}

}

#endif
