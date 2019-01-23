/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_PlayerViewer_hpp
#define sealtk_gui_PlayerViewer_hpp

#include <QMatrix3x3>
#include <QOpenGLWidget>
#include <qtGlobal.h>

class QImage;

namespace sealtk
{

namespace gui
{

class PlayerViewerPrivate;

class PlayerViewer : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit PlayerViewer(QWidget* parent = nullptr);
  ~PlayerViewer() override;

public slots:
  void setImage(QImage const& image);
  void setHomography(QMatrix3x3 const& homography);

protected:
  QTE_DECLARE_PRIVATE(PlayerViewer)

  void initializeGL() override;
  void paintGL() override;

private:
  QTE_DECLARE_PRIVATE_RPTR(PlayerViewer)
};

}

}

#endif
