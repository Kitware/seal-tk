/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/Player.hpp>

#include <sealtk/core/ImageUtils.hpp>

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
class PlayerPrivate
{
public:
  PlayerPrivate(Player* parent);

  void destroyResources();
  void createTexture();
  void calculateViewHomography();

  QTE_DECLARE_PUBLIC(Player)
  QTE_DECLARE_PUBLIC_PTR(Player)

  kwiver::vital::image_container_sptr image;
  QMatrix3x3 homography;
  QMatrix4x4 homographyGl;
  int homographyLocation;
  QMatrix4x4 viewHomography;
  int viewHomographyLocation;
  std::unique_ptr<QOpenGLTexture> imageTexture;
  std::unique_ptr<QOpenGLTexture> noImageTexture;
  std::unique_ptr<QOpenGLBuffer> vertexBuffer;
  std::unique_ptr<QOpenGLShaderProgram> shaderProgram;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Player)

//-----------------------------------------------------------------------------
Player::Player(QWidget* parent)
  : QOpenGLWidget(parent),
    d_ptr{new PlayerPrivate{this}}
{
}

//-----------------------------------------------------------------------------
Player::~Player()
{
}

//-----------------------------------------------------------------------------
void Player::setImage(kwiver::vital::image_container_sptr const& image)
{
  QTE_D();

  d->image = image;
  d->calculateViewHomography();
  this->makeCurrent();
  d->createTexture();
  this->doneCurrent();
  this->update();
}

//-----------------------------------------------------------------------------
void Player::setHomography(QMatrix3x3 const& homography)
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
void Player::initializeGL()
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

  d->noImageTexture = std::make_unique<QOpenGLTexture>(QImage{
    ":/PlayerX.png"});

  d->createTexture();

  d->vertexBuffer = std::make_unique<QOpenGLBuffer>(
    QOpenGLBuffer::VertexBuffer);
  d->vertexBuffer->create();
  d->vertexBuffer->bind();
  d->vertexBuffer->allocate(vertexData.data(),
                            vertexData.size() * sizeof(VertexData));

  d->shaderProgram = std::make_unique<QOpenGLShaderProgram>(this);
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/PlayerVertex.glsl");
  d->shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/PlayerFragment.glsl");
  d->shaderProgram->bindAttributeLocation("a_vertexCoords", 0);
  d->shaderProgram->bindAttributeLocation("a_textureCoords", 1);
  d->shaderProgram->link();

  d->homographyLocation = d->shaderProgram->uniformLocation("homography");
  d->viewHomographyLocation = d->shaderProgram->uniformLocation(
    "viewHomography");
}

//-----------------------------------------------------------------------------
void Player::paintGL()
{
  QTE_D();
  auto* functions = this->context()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  d->shaderProgram->bind();
  if (d->imageTexture)
  {
    d->imageTexture->bind();
  }
  else
  {
    d->noImageTexture->bind();
  }

  d->vertexBuffer->bind();
  d->shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2,
                                       sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(0);
  d->shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 2,
                                      sizeof(VertexData));
  d->shaderProgram->enableAttributeArray(1);

  d->shaderProgram->setUniformValueArray(d->homographyLocation,
                                         &d->homographyGl, 1);
  d->shaderProgram->setUniformValueArray(d->viewHomographyLocation,
                                         &d->viewHomography, 1);

  functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  d->vertexBuffer->release();
  if (d->imageTexture)
  {
    d->imageTexture->release();
  }
  else
  {
    d->noImageTexture->release();
  }
  d->shaderProgram->release();
}

//-----------------------------------------------------------------------------
void Player::resizeGL(int w, int h)
{
  QTE_D();
  d->calculateViewHomography();
}

//-----------------------------------------------------------------------------
PlayerPrivate::PlayerPrivate(Player* parent)
  : q_ptr{parent}
{
  this->homography.setToIdentity();
  this->homographyGl.setToIdentity();
  this->viewHomography.setToIdentity();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::createTexture()
{
  if (this->image)
  {
    this->imageTexture = std::make_unique<QOpenGLTexture>(
      sealtk::core::imageContainerToQImage(this->image));
  }
  else
  {
    this->imageTexture = nullptr;
  }
}

//-----------------------------------------------------------------------------
void PlayerPrivate::destroyResources()
{
  QTE_Q();

  q->makeCurrent();
  this->imageTexture = nullptr;
  this->noImageTexture = nullptr;
  this->vertexBuffer = nullptr;
  this->shaderProgram = nullptr;
  q->doneCurrent();
}

//-----------------------------------------------------------------------------
void PlayerPrivate::calculateViewHomography()
{
  QTE_Q();

  double width, height;

  if (this->image)
  {
    width = this->image->width();
    height = this->image->height();
  }
  else if (this->noImageTexture)
  {
    width = this->noImageTexture->width();
    height = this->noImageTexture->height();
  }

  float quotient = (width / height) /
    (static_cast<double>(q->width()) / static_cast<double>(q->height()));
  if (quotient > 1.0f)
  {
    this->viewHomography = QMatrix4x4{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f / quotient,
                  0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };
  }
  else
  {
    this->viewHomography = QMatrix4x4{
      quotient,
            0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };
  }
}

}

}
