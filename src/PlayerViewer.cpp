/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "PlayerViewer.hpp"

#include <QDebug>

namespace sealtk
{

//=============================================================================
class PlayerViewerPrivate
{
public:
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(PlayerViewer)

//-----------------------------------------------------------------------------
PlayerViewer::PlayerViewer(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerViewerPrivate}
{
}

//-----------------------------------------------------------------------------
PlayerViewer::~PlayerViewer()
{
}

//-----------------------------------------------------------------------------
void PlayerViewer::displayImage(const QImage& image)
{
  qDebug() << "displayImage() called";
}

}
