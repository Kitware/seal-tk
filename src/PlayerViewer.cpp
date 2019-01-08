/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "PlayerViewer.hpp"

#include <QApplication>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QVector>
#include <QVector2D>

#include <memory>

namespace sealtk
{

//=============================================================================
class PlayerViewerPrivate
{
public:
  PlayerViewerPrivate(PlayerViewer* q);

  void destroyResources();
  void createTexture();

  PlayerViewer* q;
  QImage image;
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
void PlayerViewer::displayImage(const QImage& image)
{
  QTE_D();

  d->image = image;
  this->makeCurrent();
  d->createTexture();
  this->doneCurrent();
  this->update();
}

//-----------------------------------------------------------------------------
void PlayerViewer::initializeGL()
{
  QTE_D();

  struct VertexData
  {
    QVector2D vertexCoords;
    QVector2D textureCoords;
  };

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

  d->shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(0);
  d->shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 2,
                                      sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(1);
}

//-----------------------------------------------------------------------------
void PlayerViewer::paintGL()
{
  QTE_D();
  auto* functions = this->context()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  d->shaderProgram->bind();
  if (d->imageTexture)
  {
    d->imageTexture->bind();
  }
  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  d->shaderProgram->release();
}

//-----------------------------------------------------------------------------
PlayerViewerPrivate::PlayerViewerPrivate(PlayerViewer* q)
  : q{q}
{
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
