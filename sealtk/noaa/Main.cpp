/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>

#include <sealtk/noaa/core/Config.h>
#include <sealtk/noaa/gui/Window.hpp>

#include <sealtk/core/Version.h>

#include <sealtk/gui/Resources.hpp>

#include <vital/plugin_loader/plugin_manager.h>

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  sealtk::gui::Resources r;

  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication app{argc, argv};
  QApplication::setApplicationName("SEAL-TK");
  QApplication::setApplicationVersion(SEALTK_Q_VERSION);

  QCommandLineParser parser;
  parser.setApplicationDescription(
    "SEAL-TK, the Stereoscopic Examination of Aquatic Life Tookit");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption pipelineDirectoryOption(
    "pipeline-directory",
    QApplication::translate(
      "noaa_app",
      "Alternative directory in which to search for KWIVER pipeline files."),
    QApplication::translate("noaa_app", "directory"));
  parser.addOption(pipelineDirectoryOption);

  parser.process(app);

  QString pipelineDirectory;
  if (parser.isSet(pipelineDirectoryOption))
  {
    pipelineDirectory = parser.value(pipelineDirectoryOption);
  }
  else
  {
    pipelineDirectory = QApplication::applicationDirPath() +
      QStringLiteral("/" SEALTK_NOAA_RELATIVE_SHARE_DIR "/seal-tk/pipelines");
  }

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  sealtk::noaa::gui::Window window{pipelineDirectory};
  window.show();

  return app.exec();
}
