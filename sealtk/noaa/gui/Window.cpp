/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>
#include <sealtk/noaa/gui/Player.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/core/DirectoryListing.hpp>
#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/KwiverPipelineWorker.hpp>
#include <sealtk/core/KwiverTrackSource.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>
#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/util/unique.hpp>

#include <sealtk/gui/SplitterWindow.hpp>

#include <qtStlUtil.h>

#include <QCollator>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>

#include <memory>

namespace sealtk
{

namespace noaa
{

namespace gui
{

namespace sc = sealtk::core;
namespace sg = sealtk::gui;

//=============================================================================
struct WindowData
{
  sc::VideoSource* videoSource = nullptr;
  sg::SplitterWindow* window = nullptr;
  sealtk::noaa::gui::Player* player = nullptr;
};

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* q) : q_ptr{q} {}

  void registerVideoSourceFactory(
    QString const& name, sc::VideoSourceFactory* factory);

  void createWindow(WindowData* data, QString const& title,
                    sealtk::noaa::gui::Player::Role role);

  void loadDetections();
  void executePipeline(QString const& pipelineFile);

  Ui::Window ui;

  std::unique_ptr<sc::VideoController> videoController;
  std::shared_ptr<sc::AbstractDataSource> trackSource;
  std::shared_ptr<QAbstractItemModel> trackModel;

  WindowData eoWindow;
  WindowData irWindow;

  float zoom = 1.0f;
  QPointF center{0.0f, 0.0f};

private:
  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

//-----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : QMainWindow{parent},
    d_ptr{new WindowPrivate{this}}
{
  QTE_D();
  d->ui.setupUi(this);

  constexpr auto Master = sealtk::noaa::gui::Player::Role::Master;
  constexpr auto Slave = sealtk::noaa::gui::Player::Role::Slave;

  d->createWindow(&d->eoWindow, QStringLiteral("EO Imagery"), Master);
  d->createWindow(&d->irWindow, QStringLiteral("IR Imagery"), Slave);

  d->eoWindow.player->setContrastMode(sg::ContrastMode::Manual);
  d->irWindow.player->setContrastMode(sg::ContrastMode::Percentile);
  d->irWindow.player->setPercentiles(0.0, 1.0);

  connect(d->eoWindow.player, &sg::Player::imageSizeChanged,
          d->irWindow.player, &sg::Player::setHomographyImageSize);

  d->videoController = make_unique<sc::VideoController>(this);
  d->ui.control->setVideoController(d->videoController.get());

  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
  connect(d->ui.control, &sg::PlayerControl::previousFrameTriggered,
          this, [d]{
            d->videoController->previousFrame(0);
          });
  connect(d->ui.control, &sg::PlayerControl::nextFrameTriggered,
          this, [d]{
            d->videoController->nextFrame(0);
          });

  d->registerVideoSourceFactory(
    "Image List File...",
    new core::ImageListVideoSourceFactory{false, d->videoController.get()});
  d->registerVideoSourceFactory(
    "Image Directory...",
    new core::ImageListVideoSourceFactory{true, d->videoController.get()});
}

//-----------------------------------------------------------------------------
Window::~Window()
{
}

//-----------------------------------------------------------------------------
void Window::setPipelineDirectory(QString const& directory)
{
  QTE_D();

  qDeleteAll(d->ui.menuPipeline->actions());

  sc::DirectoryListing pdir{{"pipe"}, directory};
  auto const& pipelines = pdir.files();
  auto keys = pipelines.keys();

  QCollator collator;
  collator.setNumericMode(true);
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(keys.begin(), keys.end(), collator);

  for (auto const& key : keys)
  {
    auto* const action = d->ui.menuPipeline->addAction(key);
    auto const& filename = pipelines[key];
    connect(action, &QAction::triggered, this, [filename, d]{
      d->executePipeline(filename);
    });
  }
}

//-----------------------------------------------------------------------------
float Window::zoom() const
{
  QTE_D();
  return d->zoom;
}

//-----------------------------------------------------------------------------
QPointF Window::center() const
{
  QTE_D();
  return d->center;
}

//-----------------------------------------------------------------------------
void Window::setZoom(float zoom)
{
  QTE_D();
  if (!qFuzzyCompare(zoom, d->zoom))
  {
    d->zoom = zoom;
    emit this->zoomChanged(zoom);
  }
}

//-----------------------------------------------------------------------------
void Window::setCenter(QPointF center)
{
  QTE_D();
  if (!(qFuzzyCompare(center.x(), d->center.x()) &&
        qFuzzyCompare(center.y(), d->center.y())))
  {
    d->center = center;
    emit this->centerChanged(center);
  }
}

//-----------------------------------------------------------------------------
void Window::showAbout()
{
  About about{this};
  about.exec();
}

//-----------------------------------------------------------------------------
void WindowPrivate::registerVideoSourceFactory(
  QString const& name, sc::VideoSourceFactory* factory)
{
  QTE_Q();

  this->eoWindow.player->registerVideoSourceFactory(name, factory,
                                                    &this->eoWindow);
  this->irWindow.player->registerVideoSourceFactory(name, factory,
                                                    &this->irWindow);

  QObject::connect(
    factory, &sc::VideoSourceFactory::videoSourceLoaded,
    [this, q](void* handle, sc::VideoSource* videoSource)
  {
    auto* data = static_cast<WindowData*>(handle);

    // If this view had a video source previously, delete it
    delete data->videoSource;

    // Add the new video source
    data->videoSource = videoSource;
    auto* const videoDistributor =
      this->videoController->addVideoSource(videoSource);
    data->player->setVideoSource(videoDistributor);

    // Enable pipelines
    this->ui.menuPipeline->setEnabled(true);
  });

  auto* fileFactory =
    dynamic_cast<sc::FileVideoSourceFactory*>(factory);
  if (fileFactory)
  {
    QObject::connect(
      fileFactory, &sc::FileVideoSourceFactory::fileRequested,
      [q, fileFactory](void* handle){
        QString filename;
        if (fileFactory->expectsDirectory())
        {
          filename = QFileDialog::getExistingDirectory(q);
        }
        else
        {
          filename = QFileDialog::getOpenFileName(q);
        }

        if (!filename.isNull())
        {
          fileFactory->loadFile(handle, filename);
        }
      });
  }
}

//-----------------------------------------------------------------------------
void WindowPrivate::createWindow(WindowData* data, QString const& title,
                                 sealtk::noaa::gui::Player::Role role)
{
  QTE_Q();

  data->window = new sg::SplitterWindow{q};
  data->player = new sealtk::noaa::gui::Player{role, data->window};
  data->window->setCentralWidget(data->player);
  data->window->setClosable(false);
  data->window->setWindowTitle(title);

  QObject::connect(q, &Window::zoomChanged,
                   data->player, &sg::Player::setZoom);
  QObject::connect(data->player, &sg::Player::zoomChanged,
                   q, &Window::setZoom);
  data->player->setZoom(q->zoom());

  QObject::connect(q, &Window::centerChanged,
                   data->player, &sg::Player::setCenter);
  QObject::connect(data->player, &sg::Player::centerChanged,
                   q, &Window::setCenter);
  data->player->setCenter(q->center());

  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::loadDetectionsTriggered,
    q, [this]{ this->loadDetections(); });

  this->ui.centralwidget->addWidget(data->window);
}

//-----------------------------------------------------------------------------
void WindowPrivate::loadDetections()
{
  QTE_Q();

  auto const& filename = QFileDialog::getOpenFileName(q);
  if (!filename.isNull())
  {
    auto uri = QUrl::fromLocalFile(filename);
    auto params = QUrlQuery{};

    params.addQueryItem("input:type", "kw18");
    uri.setQuery(params);

    this->trackSource =
      std::make_shared<sc::KwiverTrackSource>(q);

    QObject::connect(
      this->trackSource.get(), &sc::AbstractDataSource::modelReady, q,
      [this](std::shared_ptr<QAbstractItemModel> const& model){
        this->trackModel = model;
        // TODO
      });
    QObject::connect(
      this->trackSource.get(), &sc::AbstractDataSource::failed, q,
      [q](QString const& message){
        QMessageBox mb{q};
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(QStringLiteral("Failed to read detections"));
        mb.setText(
          QStringLiteral("An exception occurred while reading detections."));
        mb.setDetailedText(message);
        mb.exec();
      });

    this->trackSource->readData(uri);
  }
}

//-----------------------------------------------------------------------------
void WindowPrivate::executePipeline(QString const& pipelineFile)
{
  QTE_Q();

  if (!this->eoWindow.videoSource && !this->irWindow.videoSource)
  {
    // This should never happen
    qDebug() << "Eek! No video sources loaded?!";
    return;
  }

  QProgressDialog progressDialog{
    QStringLiteral("Executing Pipeline..."), QString{}, 0, 0, q};
  progressDialog.setAutoReset(false);
  progressDialog.show();

  sc::KwiverPipelineWorker worker{q};

  worker.addVideoSource(this->eoWindow.videoSource);
  worker.addVideoSource(this->irWindow.videoSource);

  QObject::connect(
    &worker, &sc::KwiverPipelineWorker::progressRangeChanged,
    &progressDialog, &QProgressDialog::setRange);

  QObject::connect(
    &worker, &sc::KwiverPipelineWorker::progressValueChanged,
    &progressDialog, &QProgressDialog::setValue);

  if (worker.initialize(pipelineFile))
  {
    worker.execute();
  }
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
