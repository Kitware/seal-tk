/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/DetectionRepresentation.hpp>

#include <vital/range/iota.h>

#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

// ============================================================================
class DetectionRepresentationPrivate
{
public:
  void initializeShader();

  std::function<QColor (qint64)> colorFunction;

  bool initialized = false;

  QOpenGLShaderProgram shaderProgram;

  int transformLocation;
  int colorLocation;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(DetectionRepresentation)

// ----------------------------------------------------------------------------
DetectionRepresentation::DetectionRepresentation()
  : d_ptr{new DetectionRepresentationPrivate}
{
}

// ----------------------------------------------------------------------------
DetectionRepresentation::~DetectionRepresentation()
{
}

// ----------------------------------------------------------------------------
void DetectionRepresentation::setColorFunction(
  std::function<QColor (qint64)> const& function)
{
  QTE_D();
  d->colorFunction = function;
}

// ----------------------------------------------------------------------------
void DetectionRepresentation::drawDetections(
  QOpenGLFunctions* functions, QMatrix4x4 const& transform,
  QOpenGLBuffer& vertexBuffer, std::vector<DetectionInfo> const& indices)
{
  QTE_D();

  if (!d->colorFunction)
  {
    qWarning() << this << "color function not specified";
    return;
  }

  d->initializeShader();
  d->shaderProgram.bind();
  vertexBuffer.bind();

  d->shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);
  d->shaderProgram.enableAttributeArray(0);

  d->shaderProgram.setUniformValue(d->transformLocation, transform);

  for (auto const& vertexInfo : indices)
  {
    if (auto const k = vertexInfo.count / 5)
    {
      auto const& color = d->colorFunction(vertexInfo.id);
      d->shaderProgram.setUniformValue(
        d->colorLocation,
        static_cast<float>(color.redF()),
        static_cast<float>(color.greenF()),
        static_cast<float>(color.blueF()),
        static_cast<float>(color.alphaF()));

      for (auto const n : kvr::iota(k))
      {
        auto const offset = vertexInfo.first + (n * 5);
        functions->glDrawArrays(GL_LINE_STRIP, offset, 5);
      }
    }
  }

  vertexBuffer.release();
  d->shaderProgram.release();
}

// ----------------------------------------------------------------------------
void DetectionRepresentationPrivate::initializeShader()
{
  if (this->initialized)
  {
    return;
  }

  this->shaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Vertex, ":/DetectionVertex.glsl");
  // TODO Get the geometry shader working
  // this->shaderProgram.addShaderFromSourceFile(
  //   QOpenGLShader::Geometry, ":/DetectionGeometry.glsl");
  this->shaderProgram.addShaderFromSourceFile(
    QOpenGLShader::Fragment, ":/DetectionFragment.glsl");
  this->shaderProgram.bindAttributeLocation("a_vertexCoords", 0);
  this->shaderProgram.link();

  this->transformLocation = this->shaderProgram.uniformLocation("transform");
  this->colorLocation = this->shaderProgram.uniformLocation("color");

  this->initialized = true;
}

} // namespace gui

} // namespace sealtk
