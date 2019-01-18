/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/PlayerViewer.hpp>

#include <QApplication>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QVector>
#include <QVector2D>

#include <memory>

namespace sealtk
{

namespace gui
{

struct VertexData
{
  QVector2D vertexCoords;
  QVector2D textureCoords;
};

//=============================================================================
class PlayerViewerPrivate
{
public:
  PlayerViewerPrivate(PlayerViewer* q);

  void destroyResources();
  void createTexture();

  PlayerViewer* q;
  QImage image;
  QMatrix3x3 homography;
  QMatrix4x4 homographyGl;
  int homographyLocation;
  std::unique_ptr<QOpenGLTexture> imageTexture;
  std::unique_ptr<QOpenGLBuffer> vertexBuffer;
  std::unique_ptr<QOpenGLShaderProgram> shaderProgram;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(PlayerViewer)

//-----------------------------------------------------------------------------
PlayerViewer::PlayerViewer(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerViewerPrivate{this}}
{
}

//-----------------------------------------------------------------------------
PlayerViewer::~PlayerViewer()
{
}

//-----------------------------------------------------------------------------
void PlayerViewer::setImage(QImage const& image)
{
  QTE_D();

  d->image = image;
  this->makeCurrent();
  d->createTexture();
  this->doneCurrent();
  this->update();
}

//-----------------------------------------------------------------------------
void PlayerViewer::setHomography(QMatrix3x3 const& homography)
{
  QTE_D();

  d->homography = homography;
  for (int row = 0; row < 3; row++)
  {
    for (int column = 0; column < 3; column++)
    {
      int glRow = row;
      if (glRow >= 2)
      {
        glRow++;
      }
      int glColumn = column;
      if (glColumn >= 2)
      {
        glColumn++;
      }
      d->homographyGl(glRow, glColumn) = d->homography(row, column);
    }
  }
  this->update();
}

//-----------------------------------------------------------------------------
void PlayerViewer::initializeGL()
{
  QTE_D();

  static QVector<VertexData> const vertexData{
    {{1.0f, 1.0f}, {1.0f, 0.0f}},
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},
    {{-1.0f, -1.0f}, {0.0f, 1.0f}},
    {{1.0f, -1.0f}, {1.0f, 1.0f}},
  };

  connect(this->context(), &QOpenGLContext::aboutToBeDestroyed,
          [d]() {d->destroyResources();});

  d->createTexture();

  d->vertexBuffer = std::make_unique<QOpenGLBuffer>(
    QOpenGLBuffer::VertexBuffer);
  d->vertexBuffer->create();
  d->vertexBuffer->bind();
  d->vertexBuffer->allocate(vertexData.data(),
                            vertexData.size() * sizeof(VertexData));

  d->shaderProgram = std::make_unique<QOpenGLShaderProgram>(this);
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/PlayerViewerVertex.glsl");
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/PlayerViewerFragment.glsl");
  d->shaderProgram->bindAttributeLocation("a_vertexCoords", 0);
  d->shaderProgram->bindAttributeLocation("a_textureCoords", 1);
  d->shaderProgram->link();

  d->homographyLocation = d->shaderProgram->uniformLocation("homography");
}

//-----------------------------------------------------------------------------
void PlayerViewer::paintGL()
{
  QTE_D();
  auto* functions = this->context()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (d->imageTexture)
  {
    d->shaderProgram->bind();
    d->imageTexture->bind();

    d->vertexBuffer->bind();
    d->shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2,
                                         sizeof(VertexData));
    d->shaderProgram->enableAttributeArray(0);
    d->shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 2,
                                        sizeof(VertexData));
    d->shaderProgram->enableAttributeArray(1);

    d->shaderProgram->setUniformValueArray(d->homographyLocation,
                                           &d->homographyGl, 1);

    functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    d->vertexBuffer->release();
    d->imageTexture->release();
    d->shaderProgram->release();
  }
}

//-----------------------------------------------------------------------------
PlayerViewerPrivate::PlayerViewerPrivate(PlayerViewer* q)
  : q{q}
{
  this->homography.setToIdentity();
  this->homographyGl.setToIdentity();
}

//-----------------------------------------------------------------------------
void PlayerViewerPrivate::createTexture()
{
  if (!this->image.isNull())
  {
    this->imageTexture = std::make_unique<QOpenGLTexture>(this->image);
  }
}

//-----------------------------------------------------------------------------
void PlayerViewerPrivate::destroyResources()
{
  this->q->makeCurrent();
  this->imageTexture = nullptr;
  this->vertexBuffer = nullptr;
  this->shaderProgram = nullptr;
  this->q->doneCurrent();
}

}

}
