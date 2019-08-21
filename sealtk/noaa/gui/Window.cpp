/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>
#include <sealtk/noaa/gui/Player.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/gui/AbstractItemRepresentation.hpp>
#include <sealtk/gui/FusionModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>
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
#include <qtUiState.h>

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

namespace sc = sealtk::core;
namespace sg = sealtk::gui;

namespace sealtk
{

namespace noaa
{

namespace gui
{

namespace // anonymous
{

//=============================================================================
struct WindowData
{
  sc::VideoSource* videoSource = nullptr;
  sg::SplitterWindow* window = nullptr;
  sealtk::noaa::gui::Player* player = nullptr;

  std::shared_ptr<sc::AbstractDataSource> trackSource;
  std::shared_ptr<QAbstractItemModel> trackModel;
};

//=============================================================================
class TrackRepresentation : public sg::AbstractItemRepresentation
{
public:
  TrackRepresentation()
  {
    this->setColumnRoles({sc::NameRole, sc::StartTimeRole});
  }

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
        section >= 0 && section < this->columnCount())
    {
      switch (this->roleForColumn(section))
      {
        case sc::StartTimeRole:
        case sc::EndTimeRole:
          return QStringLiteral("Time");
        default:
          break;
      }
    }

    return this->AbstractItemRepresentation::headerData(
      section, orientation, role);
  }
};

} // namespace <anonymous>

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* q) : q_ptr{q} {}

  void registerVideoSourceFactory(
    QString const& name, sc::VideoSourceFactory* factory);

  void createWindow(WindowData* data, QString const& title,
                    sealtk::noaa::gui::Player::Role role);

  void loadDetections(WindowData* data);
  void executePipeline(QString const& pipelineFile);

  Ui::Window ui;
  qtUiState uiState;

  sg::FusionModel trackModel;
  TrackRepresentation trackRepresentation;

  std::unique_ptr<sc::VideoController> videoController;

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

  d->trackRepresentation.setSourceModel(&d->trackModel);
  d->ui.tracks->setModel(&d->trackRepresentation);

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
  connect(d->ui.tracks, &QAbstractItemView::doubleClicked,
          this, [d](QModelIndex const& index){
            auto const& t =
              d->trackModel.data(d->trackRepresentation.mapToSource(index),
                                 sc::StartTimeRole);
            if (t.isValid())
            {
              d->ui.control->setTime(
                t.value<kwiver::vital::timestamp::time_t>());
            }
          });

  d->registerVideoSourceFactory(
    "Image List File...",
    new core::ImageListVideoSourceFactory{false, d->videoController.get()});
  d->registerVideoSourceFactory(
    "Image Directory...",
    new core::ImageListVideoSourceFactory{true, d->videoController.get()});

  d->uiState.mapState("Window/state", this);
  d->uiState.mapGeometry("Window/geometry", this);
  d->uiState.mapState("Window/splitter", d->ui.centralwidget);
  d->uiState.mapState("Tracks/state", d->ui.tracks->header());

  d->uiState.restore();
}

//-----------------------------------------------------------------------------
Window::~Window()
{
}

//-----------------------------------------------------------------------------
void Window::closeEvent(QCloseEvent* e)
{
  QTE_D();

  d->uiState.save();

  QMainWindow::closeEvent(e);
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
    q, [data, this]{ this->loadDetections(data); });

  this->ui.centralwidget->addWidget(data->window);
}

//-----------------------------------------------------------------------------
void WindowPrivate::loadDetections(WindowData* data)
{
  QTE_Q();

  auto const& filename = QFileDialog::getOpenFileName(q);
  if (!filename.isNull())
  {
    auto uri = QUrl::fromLocalFile(filename);
    auto params = QUrlQuery{};

    params.addQueryItem("input:type", "kw18");
    uri.setQuery(params);

    data->trackSource =
      std::make_shared<sc::KwiverTrackSource>(q);

    QObject::connect(
      data->trackSource.get(), &sc::AbstractDataSource::modelReady, q,
      [data, this](std::shared_ptr<QAbstractItemModel> const& model){
        data->trackModel = model;
        this->trackModel.addModel(model.get());
      });
    QObject::connect(
      data->trackSource.get(), &sc::AbstractDataSource::failed, q,
      [q](QString const& message){
        QMessageBox mb{q};
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(QStringLiteral("Failed to read detections"));
        mb.setText(
          QStringLiteral("An exception occurred while reading detections."));
        mb.setDetailedText(message);
        mb.exec();
      });

    data->trackSource->readData(uri);
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
