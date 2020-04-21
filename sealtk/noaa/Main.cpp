/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Resources.hpp>
#include <sealtk/noaa/gui/Window.hpp>

#include <sealtk/noaa/core/Config.h>

#include <sealtk/gui/Resources.hpp>

#include <sealtk/core/AbstractDataSource.hpp>
#include <sealtk/core/Version.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <qtColorScheme.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QSettings>

#include <memory>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  sealtk::gui::Resources commonResources;
  sealtk::gui::Resources noaaResources;

  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  QApplication app{argc, argv};
  QApplication::setApplicationName(QStringLiteral("SEAL-TK"));
  QApplication::setApplicationVersion(QStringLiteral(SEALTK_VERSION));
  QApplication::setOrganizationName(QStringLiteral("Kitware"));

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QIcon::setThemeName("krest");

  QCommandLineParser parser;
  parser.setApplicationDescription(
    QStringLiteral(
      "SEAL-TK, the Stereoscopic Examination of Aquatic Life Tookit"));
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption pipelineDirectoryOption{
    QStringLiteral("pipeline-directory"),
    QStringLiteral(
      "Alternative directory in which to search for KWIVER pipeline files."),
    QStringLiteral("directory")};
  parser.addOption(pipelineDirectoryOption);

  QCommandLineOption applicationThemeOption{
    QStringLiteral("theme"),
    QStringLiteral(
      "Path to application theme to be used (instead of the system theme)."),
    QStringLiteral("file")};
  parser.addOption(applicationThemeOption);

  parser.process(app);

  if (parser.isSet(applicationThemeOption))
  {
    QSettings settings{parser.value(applicationThemeOption),
                       QSettings::IniFormat};

    QApplication::setStyle(settings.value("WidgetStyle").toString());
    QApplication::setPalette(qtColorScheme::fromSettings(settings));
  }

  QString pipelineDirectory;
  if (parser.isSet(pipelineDirectoryOption))
  {
    pipelineDirectory = parser.value(pipelineDirectoryOption);
  }
  else
  {
    pipelineDirectory =
      QApplication::applicationDirPath() +
      QStringLiteral("/" SEALTK_NOAA_RELATIVE_SHARE_DIR "/seal-tk/pipelines");
  }

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  qRegisterMetaType<std::shared_ptr<QAbstractItemModel>>();

  sealtk::noaa::gui::Window window;
  window.setPipelineDirectory(pipelineDirectory);
  window.show();

  return app.exec();
}
