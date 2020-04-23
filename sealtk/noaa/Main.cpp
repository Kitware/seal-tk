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
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>

#include <memory>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  sealtk::gui::Resources commonResources;
  sealtk::gui::Resources noaaResources;

  // Set application attributes
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  // Create application and set identity information
  QApplication app{argc, argv};
  QApplication::setApplicationName(QStringLiteral("SEAL-TK"));
  QApplication::setApplicationVersion(QStringLiteral(SEALTK_VERSION));
  QApplication::setOrganizationName(QStringLiteral("Kitware"));

  // Set icon theme and fallback theme
  auto const& originalTheme = QIcon::themeName();
  QIcon::setThemeName("krest");

  if (!originalTheme.isEmpty())
  {
    QIcon::setFallbackThemeName(originalTheme);
  }
  else
  {
    // QIcon::themeName appears to have a bug that causes it to always return
    // an empty string; this is a grotesque work-around to get the theme name
    QProcess p{&app};
    p.start(
      QStringLiteral("gsettings"),
      {
        QStringLiteral("get"),
        QStringLiteral("org.gnome.desktop.interface"),
        QStringLiteral("icon-theme")
      });
    if (p.waitForFinished())
    {
      auto const& output =
        QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
      auto const re = QRegularExpression{QStringLiteral("'([-\\w]+)'")};
      auto const m = re.match(output);
      if (m.isValid())
      {
        QIcon::setFallbackThemeName(m.captured(1));
      }
    }
  }

  // Set up command line parser
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

  // Parse command line options
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

  // Load all KWIVER plugins
  kwiver::vital::plugin_manager::instance().load_all_plugins();

  // Register meta-types
  qRegisterMetaType<std::shared_ptr<QAbstractItemModel>>();

  // Set up the main window
  sealtk::noaa::gui::Window window;
  window.setPipelineDirectory(pipelineDirectory);
  window.show();

  // Hand off to main event loop
  return app.exec();
}
