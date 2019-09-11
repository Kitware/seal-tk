/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ImageUtils.hpp>

#include <vital/types/image.h>

#include <vital/range/iota.h>

#include <QOffscreenSurface>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QtTest>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

using PixelTraits = kv::image_pixel_traits;

Q_DECLARE_METATYPE(PixelTraits::pixel_type)

namespace sealtk
{

namespace core
{

namespace test
{

namespace // anonymous
{

constexpr size_t IMAGE_SIZE = 64;
constexpr auto FBO_SIZE = static_cast<int>(IMAGE_SIZE);

#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
// GCC 4.8 chokes on raw string literals passed as arguments to a macro (i.e.
// the immediately following code). Work around this by instead using the
// QString-from-literal constructors. This is less efficient, but at least it
// compiles.
# undef QStringLiteral
# define QStringLiteral QString
#endif

static auto const VERTEX_SHADER_CODE = QStringLiteral(
  R"(
  #version 130

  in vec3 vert;
  out vec2 tc;

  void main()
  {
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position.x = +((2.0 * vert.x) - 1.0);
    gl_Position.y = -((2.0 * vert.y) - 1.0);
    tc = vert.xy;
  })");

static auto const PIXEL_PACKED_FRAGMENT_SHADER_CODE = QStringLiteral(
  R"(
  #version 130

  uniform sampler2DArray tex;
  in vec2 tc;
  out vec4 color;

  void main()
  {
    color = texture(tex, vec3(tc, 0));
  })");

static auto const PLANE_PACKED_FRAGMENT_SHADER_CODE = QStringLiteral(
  R"(
  #version 130

  uniform sampler2DArray tex;
  in vec2 tc;
  out vec4 color;

  void main()
  {
    color.r = texture(tex, vec3(tc, 0)).r;
    color.g = texture(tex, vec3(tc, 1)).g;
    color.b = texture(tex, vec3(tc, 2)).b;
    color.a = 1.0;
  })");

// ----------------------------------------------------------------------------
constexpr size_t operator"" _z(unsigned long long x)
{
  return static_cast<size_t>(x);
}

// ----------------------------------------------------------------------------
constexpr ptrdiff_t operator"" _t(unsigned long long x)
{
  return static_cast<ptrdiff_t>(x);
}

// ----------------------------------------------------------------------------
uchar pixelValue(int i, int j, int k)
{
  auto const u = static_cast<double>(i);
  auto const v = static_cast<double>(j);

  auto const tu = ((k + 1) & 1 ? +0.4 : 0.0);
  auto const tv = ((k + 1) & 2 ? -0.3 : 0.0);

  return static_cast<uchar>(127.5 * (sin((u * tu) + (v * tv)) + 1.0));
}

// ----------------------------------------------------------------------------
kv::image fillTestImage(kv::image image)
{
  for (auto const i : kvr::iota(image.width()))
  {
    for (auto const j : kvr::iota(image.height()))
    {
      for (auto const k : kvr::iota(image.depth()))
      {
        image.at<uchar>(i, j, k) = pixelValue(static_cast<int>(i),
                                              static_cast<int>(j),
                                              static_cast<int>(k));
      }
    }
  }

  return image;
}

// ----------------------------------------------------------------------------
kv::image makeTestImage(
  size_t channels, ptrdiff_t xs, ptrdiff_t ys, ptrdiff_t cs)
{
  auto const csu = static_cast<size_t>(std::abs(cs));
  auto const n =
    std::max(channels * csu, static_cast<size_t>(IMAGE_SIZE * ys));

  auto mp = std::make_shared<kv::image_memory>(n);
  auto const offset =
    std::max(0_t, -cs) * static_cast<ptrdiff_t>(channels - 1);
  auto* const pp = reinterpret_cast<char const*>(mp->data()) + offset;

  return fillTestImage({mp, pp, IMAGE_SIZE, IMAGE_SIZE, channels,
                        xs, ys, cs, PixelTraits{PixelTraits::UNSIGNED, 1}});
}

} // namespace <anonymous>

// ============================================================================
class TestImageUtils : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void imageToTexture();
  void imageToTexture_data();

private:
  QOpenGLContext m_context;
  QOffscreenSurface m_surface;
  QScopedPointer<QOpenGLFramebufferObject> m_fbo;

  QScopedPointer<QOpenGLShaderProgram> m_pixelPackedShader;
  QScopedPointer<QOpenGLShaderProgram> m_planePackedShader;
  QOpenGLBuffer m_vertices{QOpenGLBuffer::VertexBuffer};
};

// ----------------------------------------------------------------------------
void TestImageUtils::initTestCase()
{
  m_surface.create();
  QVERIFY(m_context.create());
  QVERIFY(m_context.makeCurrent(&m_surface));

  m_fbo.reset(new QOpenGLFramebufferObject{FBO_SIZE, FBO_SIZE});
  QVERIFY(m_fbo->isValid());

  m_pixelPackedShader.reset(new QOpenGLShaderProgram);
  QVERIFY(m_pixelPackedShader->create());
  QVERIFY(m_pixelPackedShader->addShaderFromSourceCode(
            QOpenGLShader::Vertex, VERTEX_SHADER_CODE));
  QVERIFY(m_pixelPackedShader->addShaderFromSourceCode(
            QOpenGLShader::Fragment, PIXEL_PACKED_FRAGMENT_SHADER_CODE));
  QVERIFY(m_pixelPackedShader->link());

  m_planePackedShader.reset(new QOpenGLShaderProgram);
  QVERIFY(m_planePackedShader->create());
  QVERIFY(m_planePackedShader->addShaderFromSourceCode(
            QOpenGLShader::Vertex, VERTEX_SHADER_CODE));
  QVERIFY(m_planePackedShader->addShaderFromSourceCode(
            QOpenGLShader::Fragment, PLANE_PACKED_FRAGMENT_SHADER_CODE));
  QVERIFY(m_planePackedShader->link());

  float vertexData[] = {
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f
  };

  m_vertices.create();
  m_vertices.bind();
  m_vertices.allocate(vertexData, static_cast<int>(sizeof(vertexData)));
  m_vertices.release();

  m_context.doneCurrent();
}

// ----------------------------------------------------------------------------
void TestImageUtils::cleanupTestCase()
{
  QVERIFY(m_context.makeCurrent(&m_surface));
  m_fbo.reset();
  m_pixelPackedShader.reset();
  m_planePackedShader.reset();
  m_context.doneCurrent();
  m_surface.destroy();
}

// ----------------------------------------------------------------------------
void TestImageUtils::imageToTexture()
{
  // Get test parameters
  QFETCH(size_t, channels);
  QFETCH(ptrdiff_t, xStride);
  QFETCH(ptrdiff_t, yStride);
  QFETCH(ptrdiff_t, cStride);

  // Create input image
  auto const& image = makeTestImage(channels, xStride, yStride, cStride);
  auto imageContainer =
    std::make_shared<kv::simple_image_container>(image);

  // Bind FBO
  QVERIFY(m_context.makeCurrent(&m_surface));
  QVERIFY(m_fbo->bind());

  // Create and fill texture
  QOpenGLTexture texture{QOpenGLTexture::Target2DArray};
  ::sealtk::core::imageToTexture(texture, imageContainer);

  // Select shader
  auto const& shader =
    (texture.layers() > 1 ? m_planePackedShader : m_pixelPackedShader);

  // Render to FBO
  m_context.functions()->glViewport(0, 0, FBO_SIZE, FBO_SIZE);
  m_context.functions()->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  m_context.functions()->glClear(GL_COLOR_BUFFER_BIT);

  shader->bind();
  m_vertices.bind();
  texture.bind();

  shader->setAttributeBuffer(0, GL_FLOAT, 0, 3);
  shader->enableAttributeArray(0);

  m_context.functions()->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  m_vertices.release();
  texture.release();
  shader->release();

  // Get FBO contents
  m_fbo->release();
  auto const& out = m_fbo->toImage();

#if 0 // TESTING
  // This code will write the test and reference images to files, which should
  // be useful for debugging if the test is failing.

  // Write the test image to a sequentially-numbered file
  static int x = 0;
  out.save(QStringLiteral("/tmp/test%1.png").arg(++x));

  // Generate the reference image and write it to a file
  QImage test{FBO_SIZE, FBO_SIZE, QImage::Format_RGB888};
  for (auto const i : kvr::iota(test.width()))
  {
    for (auto const j : kvr::iota(test.height()))
    {
      auto const r = pixelValue(i, j, 0);
      auto const g = pixelValue(i, j, 1);
      auto const b = pixelValue(i, j, 2);
      test.setPixel(i, j, qRgb(r, g, b));
    }
  }
  test.save("/tmp/test-ref.png");
#endif

  // Validate result
  for (auto const i : kvr::iota(out.width()))
  {
    for (auto const j : kvr::iota(out.height()))
    {
      auto const c = out.pixel(i, j);
      QCOMPARE(qRed(c), pixelValue(i, j, 0));

      if (channels > 1)
      {
        QCOMPARE(qGreen(c), pixelValue(i, j, 1));
        QCOMPARE(qBlue(c),  pixelValue(i, j, 2));
      }
      else
      {
        QCOMPARE(qGreen(c), pixelValue(i, j, 0));
        QCOMPARE(qBlue(c),  pixelValue(i, j, 0));
      }
    }
  }

  // Clean up
  texture.destroy();
  m_context.doneCurrent();
}

// ----------------------------------------------------------------------------
void TestImageUtils::imageToTexture_data()
{
  constexpr auto s = static_cast<ptrdiff_t>(IMAGE_SIZE);

  QTest::addColumn<size_t>("channels");
  QTest::addColumn<ptrdiff_t>("xStride");
  QTest::addColumn<ptrdiff_t>("yStride");
  QTest::addColumn<ptrdiff_t>("cStride");

  QTest::newRow("Y no padding")
    << 1_z << 1_t << s << 1_t;
  QTest::newRow("RGB pixel-packed no padding")
    << 3_z << 3_t << 3 * s << +1_t;
  QTest::newRow("BGR pixel-packed no padding")
    << 3_z << 3_t << 3 * s << -1_t;
  QTest::newRow("RGB plane-packed no padding")
    << 3_z << 1_t << 1 * s << +(s * s);
  QTest::newRow("BGR plane-packed no padding")
    << 3_z << 1_t << 1 * s << -(s * s);
}

} // namespace test

} // namespace core

} // namespace sealtk

QTEST_MAIN(sealtk::core::test::TestImageUtils)
#include "ImageUtils.moc"
