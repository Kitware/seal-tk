/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_PlayerViewer_hpp
#define SEALTK_PlayerViewer_hpp

#include <QOpenGLWidget>
#include <qtGlobal.h>

class QImage;

namespace sealtk
{

class PlayerViewerPrivate;

class PlayerViewer : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit PlayerViewer(QWidget* parent = nullptr);
  ~PlayerViewer() override;

public slots:
  void displayImage(const QImage& image);

protected:
  QTE_DECLARE_PRIVATE_RPTR(PlayerViewer)

  void initializeGL() override;
  void paintGL() override;

private:
  QTE_DECLARE_PRIVATE(PlayerViewer)
};

}

#endif
