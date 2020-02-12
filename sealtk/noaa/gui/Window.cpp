/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>
#include <sealtk/noaa/gui/NotesDelegate.hpp>
#include <sealtk/noaa/gui/Player.hpp>
#include <sealtk/noaa/gui/TrackTypeDelegate.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>
#include <sealtk/noaa/core/NoaaPipelineWorker.hpp>

#include <sealtk/gui/AbstractItemRepresentation.hpp>
#include <sealtk/gui/FusionModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/DirectoryListing.hpp>
#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/KwiverDetectionsSink.hpp>
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

// ============================================================================
struct WindowData
{
  sc::VideoSource* videoSource = nullptr;
  sg::SplitterWindow* window = nullptr;
  sealtk::noaa::gui::Player* player = nullptr;

  std::shared_ptr<sc::AbstractDataSource> trackSource;
  std::shared_ptr<QAbstractItemModel> trackModel;
};

// ============================================================================
class TrackRepresentation : public sg::AbstractItemRepresentation
{
public:
  TrackRepresentation()
  {
    this->setColumnRoles({
      sc::NameRole,
      sc::StartTimeRole,
      sc::ClassificationTypeRole,
      sc::ClassificationScoreRole,
      sc::NotesRole,
    });
    this->setItemVisibilityMode(sg::OmitHidden);
  }

  Qt::ItemFlags flags(QModelIndex const& index) const override
  {
    auto const defaultFlags =
      this->sg::AbstractItemRepresentation::flags(index);

    if (index.column() == 2 || index.column() == 4)
    {
      return defaultFlags | Qt::ItemIsEditable;
    }

    return defaultFlags;
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

// ============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* q) : q_ptr{q} {}

  void registerVideoSourceFactory(
    QString const& name, sc::VideoSourceFactory* factory);

  void createWindow(WindowData* data, QString const& title,
                    sealtk::noaa::gui::Player::Role role);

  void loadDetections(WindowData* data);
  void saveDetections(WindowData* data);
  void executePipeline(QString const& pipelineFile);

  WindowData* dataForView(int viewIndex);
  void setTrackModel(
    WindowData* data, std::shared_ptr<QAbstractItemModel> const& model);

  void updateTrackSelection(QItemSelection const& selection);

  Ui::Window ui;
  qtUiState uiState;

  sg::FusionModel trackModel;
  TrackRepresentation trackRepresentation;
  TrackTypeDelegate typeDelegate;
  NotesDelegate notesDelegate;

  std::unique_ptr<sc::VideoController> videoController;

  WindowData eoWindow;
  WindowData irWindow;
  WindowData uvWindow;

  WindowData* const allWindows[3] = {
    &this->eoWindow,
    &this->irWindow,
    &this->uvWindow,
  };

  float zoom = 1.0f;
  QPointF center{0.0f, 0.0f};

private:
  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Window)

// ----------------------------------------------------------------------------
Window::Window(QWidget* parent)
  : QMainWindow{parent},
    d_ptr{new WindowPrivate{this}}
{
  QTE_D();
  d->ui.setupUi(this);

  d->trackRepresentation.setSourceModel(&d->trackModel);
  d->ui.tracks->setModel(&d->trackRepresentation);
  d->ui.tracks->setItemDelegateForColumn(2, &d->typeDelegate);
  d->ui.tracks->setItemDelegateForColumn(4, &d->notesDelegate);

  constexpr auto Master = sealtk::noaa::gui::Player::Role::Master;
  constexpr auto Slave = sealtk::noaa::gui::Player::Role::Slave;

  d->createWindow(&d->eoWindow, QStringLiteral("EO Imagery"), Master);
  d->createWindow(&d->irWindow, QStringLiteral("IR Imagery"), Slave);
  d->createWindow(&d->uvWindow, QStringLiteral("UV Imagery"), Slave);

  d->eoWindow.player->setContrastMode(sg::ContrastMode::Manual);
  d->irWindow.player->setContrastMode(sg::ContrastMode::Percentile);
  d->irWindow.player->setPercentiles(0.0, 1.0);

  d->eoWindow.player->setContrastMode(sg::ContrastMode::Manual);
  d->uvWindow.player->setContrastMode(sg::ContrastMode::Percentile);
  d->uvWindow.player->setPercentiles(0.0, 1.0);

  connect(d->eoWindow.player, &sg::Player::imageSizeChanged,
          d->irWindow.player, &sg::Player::setHomographyImageSize);
  connect(d->eoWindow.player, &sg::Player::imageSizeChanged,
          d->uvWindow.player, &sg::Player::setHomographyImageSize);

  connect(d->ui.actionShowIrPane, &QAction::toggled,
          d->irWindow.window, &QWidget::setVisible);
  connect(d->ui.actionShowUvPane, &QAction::toggled,
          d->uvWindow.window, &QWidget::setVisible);

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
  connect(d->ui.tracks->selectionModel(), &QItemSelectionModel::currentChanged,
          this, [d](QModelIndex const& repIndex){
            auto const& modelIndex =
              d->trackRepresentation.mapToSource(repIndex);

            auto const& idData =
              d->trackModel.data(modelIndex, sc::LogicalIdentityRole);
            auto const& timeData =
              d->trackModel.data(modelIndex, sc::StartTimeRole);
            if (idData.isValid() && timeData.isValid())
            {
              auto const id = idData.value<qint64>();
              auto const time =
                timeData.value<kwiver::vital::timestamp::time_t>();

              d->ui.control->setTime(time);
              for (auto* const w : d->allWindows)
              {
                w->player->setCenterToTrack(id, time);
              }
            }
          });
  connect(d->ui.tracks->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this, [d](QItemSelection const& selection){
            d->updateTrackSelection(selection);
          });

  connect(d->ui.tracks->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this, [d](QItemSelection const& selection){
            d->ui.actionDeleteDetection->setEnabled(!selection.isEmpty());
          });
  connect(d->ui.actionDeleteDetection, &QAction::triggered,
          this, [d]{
            // Get selected items
            auto items = QModelIndexList{};
            auto const& rows = d->ui.tracks->selectionModel()->selectedRows();
            for (auto const& ri : rows)
            {
              auto const& si = d->trackRepresentation.mapToSource(ri);
              items.append(si);
            }

            // Mark selected items as hidden
            for (auto const& si : items)
            {
              d->trackModel.setData(si, false, sc::UserVisibilityRole);
            }
          });

  d->registerVideoSourceFactory(
    QStringLiteral("Image List File..."),
    new core::ImageListVideoSourceFactory{false, d->videoController.get()});
  d->registerVideoSourceFactory(
    QStringLiteral("Image Directory..."),
    new core::ImageListVideoSourceFactory{true, d->videoController.get()});

  d->uiState.mapState("Window/state", this);
  d->uiState.mapGeometry("Window/geometry", this);
  d->uiState.mapState("Window/splitter", d->ui.centralwidget);
  d->uiState.mapState("Tracks/state", d->ui.tracks->header());
  d->uiState.mapChecked("View/showIR", d->ui.actionShowIrPane);
  d->uiState.mapChecked("View/showUV", d->ui.actionShowUvPane);

  d->uiState.restore();
}

// ----------------------------------------------------------------------------
Window::~Window()
{
}

// ----------------------------------------------------------------------------
void Window::closeEvent(QCloseEvent* e)
{
  QTE_D();

  d->uiState.save();

  QMainWindow::closeEvent(e);
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
float Window::zoom() const
{
  QTE_D();
  return d->zoom;
}

// ----------------------------------------------------------------------------
QPointF Window::center() const
{
  QTE_D();
  return d->center;
}

// ----------------------------------------------------------------------------
void Window::setZoom(float zoom)
{
  QTE_D();
  if (!qFuzzyCompare(zoom, d->zoom))
  {
    d->zoom = zoom;
    emit this->zoomChanged(zoom);
  }
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
void Window::showAbout()
{
  About about{this};
  about.exec();
}

// ----------------------------------------------------------------------------
void WindowPrivate::registerVideoSourceFactory(
  QString const& name, sc::VideoSourceFactory* factory)
{
  QTE_Q();

  for (auto* const w : this->allWindows)
  {
    w->player->registerVideoSourceFactory(name, factory, w);
  }

  QObject::connect(
    factory, &sc::VideoSourceFactory::videoSourceLoaded,
    [this](void* handle, sc::VideoSource* videoSource){
      auto* const data = static_cast<WindowData*>(handle);

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

  auto* const fileFactory =
    dynamic_cast<sc::FileVideoSourceFactory*>(factory);
  if (fileFactory)
  {
    QObject::connect(
      fileFactory, &sc::FileVideoSourceFactory::fileRequested,
      [q, fileFactory](void* handle){
        auto const& filename =
          (fileFactory->expectsDirectory()
           ? QFileDialog::getExistingDirectory(q)
           : QFileDialog::getOpenFileName(q));

        if (!filename.isNull())
        {
          fileFactory->loadFile(handle, filename);
        }
      });
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::createWindow(WindowData* data, QString const& title,
                                 sealtk::noaa::gui::Player::Role role)
{
  QTE_Q();

  data->window = new sg::SplitterWindow{q};
  data->player = new sealtk::noaa::gui::Player{role, data->window};
  data->window->setCentralWidget(data->player);
  data->window->setClosable(false);
  data->window->setWindowTitle(title);
  data->player->setDefaultColor(qRgb(240, 176, 48));

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
  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::saveDetectionsTriggered,
    q, [data, this]{ this->saveDetections(data); });

  this->ui.centralwidget->addWidget(data->window);
}

// ----------------------------------------------------------------------------
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
        this->setTrackModel(data, model);
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

// ----------------------------------------------------------------------------
void WindowPrivate::saveDetections(WindowData* data)
{
  QTE_Q();

  // Check for video
  if (!data->videoSource)
  {
    return;
  }

  // Set up writer
  sc::KwiverDetectionsSink writer;
  if (writer.setData(data->videoSource, data->trackModel.get()))
  {
    auto const& filename = QFileDialog::getSaveFileName(q);
    if (!filename.isNull())
    {
      auto uri = QUrl::fromLocalFile(filename);
      auto params = QUrlQuery{};

      params.addQueryItem("output:type", "kw18");
      uri.setQuery(params);

      QObject::connect(
        &writer, &sc::AbstractDataSink::failed, q,
        [q](QString const& message){
          QMessageBox mb{q};
          mb.setIcon(QMessageBox::Critical);
          mb.setWindowTitle(QStringLiteral("Failed to write detections"));
          mb.setText(
            QStringLiteral("An exception occurred while reading detections. "
                           "The output file may be corrupt."));
          mb.setDetailedText(message);
          mb.exec();
        });

      writer.writeData(uri);
    }
  }
  else
  {
    QMessageBox::information(
      q, QStringLiteral("Nothing to do!"),
      QStringLiteral("There are no detections to be saved."));
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::executePipeline(QString const& pipelineFile)
{
  QTE_Q();

  auto const haveVideoSource = [&]{
    for (auto* const w : this->allWindows)
    {
      if (w->videoSource)
      {
        return true;
      }
    }
    return false;
  }();
  if (!haveVideoSource)
  {
    // This should never happen
    qDebug() << "Eek! No video sources loaded?!";
    return;
  }

  QProgressDialog progressDialog{
    QStringLiteral("Executing Pipeline..."), QString{}, 0, 0, q};
  progressDialog.setAutoReset(false);
  progressDialog.show();

  core::NoaaPipelineWorker worker{q};

  for (auto* const w : this->allWindows)
  {
    worker.addVideoSource(w->videoSource);
  }

  QObject::connect(
    &worker, &sc::KwiverPipelineWorker::progressRangeChanged,
    &progressDialog, &QProgressDialog::setRange);

  QObject::connect(
    &worker, &sc::KwiverPipelineWorker::progressValueChanged,
    &progressDialog, &QProgressDialog::setValue);

  QObject::connect(
    &worker, &core::NoaaPipelineWorker::trackModelReady, q,
    [this](int i, std::shared_ptr<QAbstractItemModel> const& model){
      this->setTrackModel(this->dataForView(i), model);
    });

  if (worker.initialize(pipelineFile))
  {
    worker.execute();
  }
}

// ----------------------------------------------------------------------------
WindowData* WindowPrivate::dataForView(int viewIndex)
{
  switch (viewIndex)
  {
    case 0: return &this->eoWindow;
    case 1: return &this->irWindow;
    case 2: return &this->uvWindow;
    default: return nullptr;
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::setTrackModel(
  WindowData* data, std::shared_ptr<QAbstractItemModel> const& model)
{
  if (data)
  {
    if (data->trackModel)
    {
      this->trackModel.removeModel(data->trackModel.get());
    }

    data->trackModel = model;
    this->trackModel.addModel(model.get());
    data->player->setTrackModel(model.get());
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::updateTrackSelection(QItemSelection const& selection)
{
  auto selectedTracks = QSet<qint64>{};

  for (auto const& ri : selection.indexes())
  {
    auto const& mi = this->trackRepresentation.mapToSource(ri);
    auto const& data = this->trackModel.data(mi, sc::LogicalIdentityRole);
    selectedTracks.insert(data.value<qint64>());
  }

  for (auto* const w : this->allWindows)
  {
    w->player->setSelectedTrackIds(selectedTracks);
  }
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
