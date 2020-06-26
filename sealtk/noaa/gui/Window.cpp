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

#include <sealtk/noaa/PluginConfig.hpp>

#include <sealtk/gui/AbstractItemRepresentation.hpp>
#include <sealtk/gui/ClassificationSummaryRepresentation.hpp>
#include <sealtk/gui/CreateDetectionPlayerTool.hpp>
#include <sealtk/gui/FilterWidget.hpp>
#include <sealtk/gui/FusionModel.hpp>
#include <sealtk/gui/GlobInputDialog.hpp>

#include <sealtk/core/ChainedTransform.hpp>
#include <sealtk/core/ClassificationFilterModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/DirectoryListing.hpp>
#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/IdentityTransform.hpp>
#include <sealtk/core/KwiverTrackModel.hpp>
#include <sealtk/core/KwiverTrackSource.hpp>
#include <sealtk/core/KwiverTracksSink.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/TrackUtils.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>
#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/util/unique.hpp>

#include <sealtk/gui/SplitterWindow.hpp>

#include <vital/range/iota.h>
#include <vital/types/object_track_set.h>

#include <qtGet.h>
#include <qtStlUtil.h>
#include <qtUiState.h>

#include <QCollator>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QProgressDialog>
#include <QShortcut>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>

#include <algorithm>
#include <memory>

namespace sc = sealtk::core;
namespace sg = sealtk::gui;

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace noaa
{

namespace gui
{

namespace // anonymous
{

using WindowRole = sealtk::noaa::gui::Player::Role;

// ============================================================================
struct WindowData
{
  sc::VideoSource* videoSource = nullptr;
  sg::SplitterWindow* window = nullptr;
  sealtk::noaa::gui::Player* player = nullptr;

  sg::CreateDetectionPlayerTool* createDetectionTool = nullptr;

  std::shared_ptr<sc::AbstractDataSource> trackSource;
  std::shared_ptr<QAbstractItemModel> trackModel;

  kwiver::vital::transform_2d_sptr transform;
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
      sc::EndTimeRole,
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

    if (index.column() == 3 || index.column() == 5)
    {
      return defaultFlags | Qt::ItemIsEditable;
    }

    return defaultFlags;
  }
};

// ----------------------------------------------------------------------------
void addShortcut(QAction* action, QKeySequence const& shortcut)
{
  auto shortcuts = action->shortcuts();
  shortcuts.append(shortcut);
  action->setShortcuts(shortcuts);
}

} // namespace <anonymous>

// ============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* q) : q_ptr{q} {}

  void registerVideoSourceFactory(
    QString const& name, sc::VideoSourceFactory* factory);

  void createWindow(WindowData* data, QString const& title, WindowRole role);

  void loadDetections(WindowData* data);
  void saveDetections(WindowData* data);
  void executePipeline(QString const& pipelineFile);
  void createDetection(WindowData* data, QRectF const& detection);
  void createDetection(WindowData* data, qint64 trackId,
                       kv::track_state_sptr&& trackState);

  template <typename Tool>
  void setActiveTool(Tool* WindowData::* tool, QString const& text);
  void resetActiveTool();

  WindowData* dataForView(int viewIndex);
  void setTrackModel(
    WindowData* data, std::shared_ptr<QAbstractItemModel> const& model);

  void setSelectedTrack(qint64 id);
  void updateTrackSelection(QModelIndexList const &selectedIndexes);

  QModelIndex modelIndex(QModelIndex const& representationIndex) const;

  Ui::Window ui;
  qtUiState uiState;

  QLabel* statusText;

  sg::FusionModel trackModel;
  sc::ClassificationFilterModel trackModelFilter;
  TrackRepresentation trackRepresentation;
  TrackTypeDelegate typeDelegate;
  NotesDelegate notesDelegate;

  sg::ClassificationSummaryRepresentation statisticsRepresentation;

  sc::VideoController* videoController;

  WindowData eoWindow;
  WindowData irWindow;
  WindowData uvWindow;

  WindowData* const allWindows[3] = {
    &this->eoWindow,
    &this->irWindow,
    &this->uvWindow,
  };

  QPointer<QShortcut> cancelToolShortcut;

  float zoom = 1.0f;
  QPointF center{0.0f, 0.0f};

  qint64 trackToEdit = -1;

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
  d->ui.actionAbout->setIcon(this->windowIcon());

  d->statusText = new QLabel{this};
  d->ui.statusBar->addWidget(d->statusText);

  addShortcut(d->ui.actionCreateTrack, Qt::Key_Insert);
  addShortcut(d->ui.actionDeleteTrack, Qt::Key_D);
  addShortcut(d->ui.actionAmendTrack, Qt::Key_Plus);
  addShortcut(d->ui.actionPreviousFrame, Qt::Key_BracketLeft);
  addShortcut(d->ui.actionNextFrame, Qt::Key_BraceRight);

  d->ui.menuView->addAction(d->ui.trackDock->toggleViewAction());
  d->ui.menuView->addAction(d->ui.statisticsDock->toggleViewAction());
  d->ui.menuView->addAction(d->ui.filtersDock->toggleViewAction());
  d->ui.menuView->addAction(d->ui.toolBar->toggleViewAction());

  // Set up track model and list
  d->trackModelFilter.setSourceModel(&d->trackModel);
  d->trackRepresentation.setSourceModel(&d->trackModelFilter);
  d->ui.tracks->setModel(&d->trackRepresentation);
  d->ui.tracks->setItemDelegateForColumn(3, &d->typeDelegate);
  d->ui.tracks->setItemDelegateForColumn(5, &d->notesDelegate);

  connect(d->ui.filters, &ClassificationFilterWidget::valueChanged, this,
          [d](QString const& type, double value){
            d->trackModelFilter.setLowerBound(type, value);
          });

  // Set up statistics panel
  d->statisticsRepresentation.setSourceModel(&d->trackModelFilter);
  d->ui.statistics->setModel(&d->statisticsRepresentation);

  // Set up view panes
  constexpr auto Master = WindowRole::Master;
  constexpr auto Slave = WindowRole::Slave;

  d->createWindow(&d->eoWindow, QStringLiteral("EO Imagery"), Master);
  d->createWindow(&d->irWindow, QStringLiteral("IR Imagery"), Slave);
  d->createWindow(&d->uvWindow, QStringLiteral("UV Imagery"), Slave);

  d->eoWindow.player->setContrastMode(sg::ContrastMode::Manual);
  d->irWindow.player->setContrastMode(sg::ContrastMode::Percentile);
  d->irWindow.player->setPercentiles(0.0, 1.0);
  d->uvWindow.player->setContrastMode(sg::ContrastMode::Percentile);
  d->uvWindow.player->setPercentiles(0.0, 1.0);

  connect(d->eoWindow.player, &sg::Player::imageSizeChanged,
          d->irWindow.player, &sg::Player::setHomographyImageSize);
  connect(d->eoWindow.player, &sg::Player::imageSizeChanged,
          d->uvWindow.player, &sg::Player::setHomographyImageSize);

  connect(d->ui.actionShowIrPane, &QAction::toggled,
          d->irWindow.window, &QWidget::setVisible);
  connect(d->irWindow.window, &sg::SplitterWindow::visibilityChanged,
          d->ui.actionShowIrPane, &QAction::setChecked);
  connect(d->ui.actionShowUvPane, &QAction::toggled,
          d->uvWindow.window, &QWidget::setVisible);
  connect(d->uvWindow.window, &sg::SplitterWindow::visibilityChanged,
          d->ui.actionShowUvPane, &QAction::setChecked);

  // Connect general actions
  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
  connect(d->ui.actionShowImageFilename, &QAction::toggled,
          this, [d](bool show){
            for (auto* const w : d->allWindows)
            {
              w->window->setFilenameVisible(show);
            }
          });

  // Set up video controller
  d->videoController = new sc::VideoController{this};
  d->ui.control->setVideoController(d->videoController);

  connect(d->ui.control, &sg::PlayerControl::previousFrameTriggered,
          d->ui.actionPreviousFrame, &QAction::trigger);
  connect(d->ui.control, &sg::PlayerControl::nextFrameTriggered,
          d->ui.actionNextFrame, &QAction::trigger);
  connect(d->ui.actionPreviousFrame, &QAction::triggered,
          this, [d]{ d->videoController->previousFrame(0); });
  connect(d->ui.actionNextFrame, &QAction::triggered,
          this, [d]{ d->videoController->nextFrame(0); });

  // Handle track selection changes
  connect(d->ui.tracks->selectionModel(), &QItemSelectionModel::currentChanged,
          this, [d](QModelIndex const& repIndex){
            auto const& modelIndex = d->modelIndex(repIndex);
            auto const role =
              (repIndex.column() == 2 ? sc::EndTimeRole : sc::StartTimeRole);

            auto const& idData =
              d->trackModel.data(modelIndex, sc::LogicalIdentityRole);
            auto const& timeData =
              d->trackModel.data(modelIndex, role);
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
          this, [d]{
            auto const& selection =
              d->ui.tracks->selectionModel()->selectedRows();

            d->updateTrackSelection(selection);
            d->ui.actionDeleteTrack->setEnabled(!selection.isEmpty());
            d->ui.actionAmendTrack->setEnabled(selection.size() == 1);
          });

  // Set up actions to create and delete detections
  connect(d->ui.actionCreateTrack, &QAction::triggered,
          this, [d]{
            static auto const text =
              QStringLiteral("Creating new track");
            d->setActiveTool(&WindowData::createDetectionTool, text);
          });

  connect(d->ui.actionAmendTrack, &QAction::triggered,
          this, [d]{

            auto const& selection =
              d->ui.tracks->selectionModel()->selectedRows();
            if (selection.size() == 1)
            {
              auto const& index = d->modelIndex(selection.first());
              auto const& idData =
                d->trackModel.data(index, sc::LogicalIdentityRole);
              if (idData.canConvert<qint64>())
              {
                d->trackToEdit = idData.value<qint64>();

                static auto const text =
                  QStringLiteral("Adding/editing detection for track %1");
                d->setActiveTool(&WindowData::createDetectionTool,
                                 text.arg(d->trackToEdit));
              }
            }
          });

  connect(d->ui.actionDeleteTrack, &QAction::triggered,
          this, [d]{
            // Get selected items
            auto items = QModelIndexList{};
            auto const& rows = d->ui.tracks->selectionModel()->selectedRows();
            for (auto const& ri : rows)
            {
              auto const& si = d->modelIndex(ri);
              items.append(si);
            }

            // Mark selected items as hidden
            for (auto const& si : items)
            {
              d->trackModel.setData(si, false, sc::UserVisibilityRole);
            }
          });

  // Set up video source factories
  d->registerVideoSourceFactory(
    QStringLiteral("Image List File..."),
    new core::ImageListVideoSourceFactory{false, d->videoController});
  d->registerVideoSourceFactory(
    QStringLiteral("Image Directory..."),
    new core::ImageListVideoSourceFactory{true, d->videoController});

  // Set up UI persistence
  d->uiState.mapState("Window/state", this);
  d->uiState.mapGeometry("Window/geometry", this);
  d->uiState.mapState("Window/splitter", d->ui.centralwidget);
  d->uiState.mapState("Tracks/state", d->ui.tracks->header());
  d->uiState.mapState("Statistics/state", d->ui.statistics->header());
  d->uiState.mapChecked("View/showIR", d->ui.actionShowIrPane);
  d->uiState.mapChecked("View/showUV", d->ui.actionShowUvPane);
  d->uiState.mapChecked("View/showFileName", d->ui.actionShowImageFilename);

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

        if (!filename.isEmpty())
        {
          if (fileFactory->expectsDirectory())
          {
            static auto const defaultGlobs = QStringList{
              QStringLiteral("*.bmp"),
              QStringLiteral("*.jpg"),
              QStringLiteral("*.jpeg"),
              QStringLiteral("*.pgm"),
              QStringLiteral("*.png"),
              QStringLiteral("*.sgi"),
              QStringLiteral("*.tif"),
              QStringLiteral("*.tiff")
            };
            static auto const key =
              QStringLiteral("FileVideoSource/NameFilter");

            sg::GlobInputDialog gid{key, q};
            gid.addDefaultGlobString(defaultGlobs);
            if (gid.exec() == QDialog::Accepted)
            {
              auto uri = QUrl::fromLocalFile(filename);
              auto params = QUrlQuery{};
              params.addQueryItem("filter", gid.globString());
              uri.setQuery(params);

              fileFactory->loadVideoSource(handle, uri);
            }
          }
          else
          {
            auto const& uri = QUrl::fromLocalFile(filename);
            fileFactory->loadVideoSource(handle, uri);
          }
        }
      });
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::createWindow(
  WindowData* data, QString const& title, WindowRole role)
{
  QTE_Q();

  data->window = new sg::SplitterWindow{q};
  data->player = new sealtk::noaa::gui::Player{role, data->window};
  data->window->setCentralWidget(data->player);
  data->window->setClosable(role != WindowRole::Master);
  data->window->setWindowTitle(title);
  data->player->setDefaultColor(qRgb(240, 176, 48));
  data->createDetectionTool = new sg::CreateDetectionPlayerTool{data->player};

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

  QObject::connect(data->player, &sg::Player::imageNameChanged,
                   data->window, &sg::SplitterWindow::setFilename);

  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::loadDetectionsTriggered,
    q, [data, this]{ this->loadDetections(data); });
  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::saveDetectionsTriggered,
    q, [data, this]{ this->saveDetections(data); });

  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::trackPicked,
    q, [this](qint64 id){ this->setSelectedTrack(id); });

  QObject::connect(
    data->createDetectionTool,
    &sg::CreateDetectionPlayerTool::detectionCreated,
    q, [this, data](QRectF const& detection){
      this->createDetection(data, detection);
      this->resetActiveTool();
    });

  QObject::connect(
    this->ui.filters, &ClassificationFilterWidget::valueChanged,
    data->player,
    [p = data->player](QString const& type, double value){
      p->setTrackFilter(type, value, {});
    });

  if (role == WindowRole::Master)
  {
    data->transform = std::make_shared<sealtk::core::IdentityTransform>();
    data->player->setTransform(data->transform);
  }
  else
  {
    using kwiver::vital::transform_2d_sptr;

    data->player->setShadowTransform(
      this->eoWindow.player, this->eoWindow.transform);

    QObject::connect(
      data->player, &sealtk::noaa::gui::Player::transformChanged,
      q, [s = data->player, this](transform_2d_sptr const& xf)
      {
        for (auto* const w : this->allWindows)
        {
          if (w->player == s)
          {
            w->transform = xf;
          }
          else
          {
            w->player->setShadowTransform(s, xf);
          }
        }
      });
  }

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

    params.addQueryItem("input:type", config::trackReader);
    params.addQueryItem("input:viame_csv:frame_id_adjustment", "1");
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

  auto setTypeFilters =
    [&, filters = this->ui.filters](sc::ClassificationFilterModel* model){
    for (auto const& type : filters->types())
    {
      model->setLowerBound(type, filters->value(type));
    }
  };

  // Set up writer
  sc::KwiverTracksSink writer;

  auto* const primaryFilter = new sc::ClassificationFilterModel{&writer};
  primaryFilter->setSourceModel(data->trackModel.get());
  setTypeFilters(primaryFilter);

  auto haveData = writer.setData(data->videoSource, primaryFilter);
  if (writer.setTransform(data->transform))
  {
    for (auto* const w : this->allWindows)
    {
      if (w != data)
      {
        auto* const shadowFilter = new sc::ClassificationFilterModel{&writer};
        shadowFilter->setSourceModel(w->trackModel.get());
        setTypeFilters(shadowFilter);

        haveData =
          writer.addData(shadowFilter, w->transform) || haveData;
      }
    }
  }

  if (haveData)
  {
    auto const& filename = QFileDialog::getSaveFileName(q);
    if (!filename.isNull())
    {
      auto uri = QUrl::fromLocalFile(filename);
      auto params = QUrlQuery{};

      params.addQueryItem("output:type", config::trackWriter);
      params.addQueryItem("output:viame_csv:frame_id_adjustment", "-1");
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

  auto setTypeFilters =
    [&, filters = this->ui.filters](sc::ClassificationFilterModel* model){
    for (auto const& type : filters->types())
    {
      model->setLowerBound(type, filters->value(type));
    }
  };

  core::NoaaPipelineWorker worker{q};

  for (auto* const w : this->allWindows)
  {
    // Add video source and tracks for current view
    worker.addVideoSource(w->videoSource);

    auto* const primaryFilter = new sc::ClassificationFilterModel{&worker};
    primaryFilter->setSourceModel(w->trackModel.get());
    setTypeFilters(primaryFilter);

    worker.addTrackSource(primaryFilter);

    // Get inverse transform
    if (w->transform)
    {
      try
      {
        auto const& ixf = w->transform->inverse();

        // Add tracks from other views
        for (auto* const sw : this->allWindows)
        {
          if (sw != w && sw->transform)
          {
            auto* const shadowFilter =
              new sc::ClassificationFilterModel{&worker};
            shadowFilter->setSourceModel(sw->trackModel.get());
            setTypeFilters(shadowFilter);

            auto cxf = sc::ChainedTransform{sw->transform, ixf};
            worker.addTrackSource(shadowFilter, cxf);
          }
        }
      }
      catch (...) {}
    }
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
  if (data->trackModel)
  {
    this->trackModel.removeModel(data->trackModel.get());
  }

  data->trackModel = model;
  this->trackModel.addModel(model.get());
  data->player->setTrackModel(model.get());

  for (auto* const w : this->allWindows)
  {
    if (w != data)
    {
      w->player->setShadowTrackModel(data->player, model.get());
    }
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::setSelectedTrack(qint64 id)
{
  QItemSelection selection;

  for (auto const row : kvr::iota(this->trackModelFilter.rowCount()))
  {
    auto const& index = this->trackModelFilter.index(row, 0);
    auto const& data =
      this->trackModelFilter.data(index, sc::LogicalIdentityRole);
    if (data.value<qint64>() == id)
    {
      auto const r = this->trackRepresentation.mapFromSource(index).row();
      auto const c = this->trackRepresentation.columnCount();
      auto const& left = this->trackRepresentation.index(r, 0);
      auto const& right = this->trackRepresentation.index(r, c - 1);
      selection.merge({left, right}, QItemSelectionModel::Select);
    }
  }

  if (!selection.isEmpty())
  {
    this->ui.tracks->selectionModel()->select(
      selection, QItemSelectionModel::ClearAndSelect);
    this->ui.tracks->scrollTo(selection.indexes().first());
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::updateTrackSelection(
  QModelIndexList const &selectedIndexes)
{
  auto selectedTracks = QSet<qint64>{};

  for (auto const& ri : selectedIndexes)
  {
    auto const& mi = this->modelIndex(ri);
    auto const& data = this->trackModel.data(mi, sc::LogicalIdentityRole);
    selectedTracks.insert(data.value<qint64>());
  }

  for (auto* const w : this->allWindows)
  {
    w->player->setSelectedTrackIds(selectedTracks);
  }
}

// ----------------------------------------------------------------------------
template <typename Tool>
void WindowPrivate::setActiveTool(
  Tool* WindowData::* tool, QString const& text)
{
  if (!this->cancelToolShortcut)
  {
    QTE_Q();

    this->cancelToolShortcut = new QShortcut{Qt::Key_Escape, q};
    QObject::connect(this->cancelToolShortcut.data(), &QShortcut::activated,
                     q, [this]{ this->resetActiveTool(); });
  }

  for (auto* const w : this->allWindows)
  {
    w->player->setActiveTool(w->*tool);
  }

  this->statusText->setText(text);
}

// ----------------------------------------------------------------------------
void WindowPrivate::resetActiveTool()
{
  this->trackToEdit = -1;
  this->statusText->clear();
  for (auto* const w : this->allWindows)
  {
    w->player->setActiveTool(nullptr);
    w->player->update();
  }
  delete this->cancelToolShortcut.data();
}

// ----------------------------------------------------------------------------
void WindowPrivate::createDetection(WindowData* data, QRectF const& detection)
{
  // Determine the time stamp for the detection
  auto const& allFrames = data->videoSource->frames();
  auto const time = this->videoController->time();
  auto const framePtr = qtGet(allFrames, time);
  auto const frame = (framePtr ? *framePtr : 0);

  // Create the detection
  auto detectedObject =
    sc::createDetection(detection, {{"unspecified", 1.0}});
  auto trackState =
    sc::createTrackState(frame, time, std::move(detectedObject));

  if (this->trackToEdit < 0)
  {
    // Determine what index to use for the new detection (using the fused
    // model, not the per-view model!)
    qint64 maxId = 0;
    for (auto const row : kvr::iota(this->trackModel.rowCount()))
    {
      auto const& index = this->trackModel.index(row, 0);
      auto const& idData =
        this->trackModel.data(index, sc::LogicalIdentityRole);
      maxId = std::max(maxId, idData.value<qint64>());
    }

    // Add the detection to the window's track model as a new track
    this->createDetection(data, maxId + 1, std::move(trackState));
  }
  else
  {
    // Try to find the track in the track model (it may not exist)
    if (auto* const kwiverTrackModel =
          qobject_cast<sc::KwiverTrackModel*>(data->trackModel.get()))
    {
      for (auto const row : kvr::iota(kwiverTrackModel->rowCount()))
      {
        auto const& index = kwiverTrackModel->index(row, 0);
        auto const& idData =
          kwiverTrackModel->data(index, sc::LogicalIdentityRole);
        if (idData.value<qint64>() == this->trackToEdit)
        {
          // Get and copy the existing track type
          auto const& typeData =
            kwiverTrackModel->data(index, sc::ClassificationRole);

          sc::objectTrackState(trackState)->detection()->set_type(
            sc::classificationToDetectedObjectType(typeData.toHash()));

          // Amend the track with the new state
          kwiverTrackModel->updateTrack(index, std::move(trackState));
          return;
        }
      }
    }

    // Looks like we are creating a new track for this view; try to find the
    // existing track (in the fusion model) so we can copy the type
    for (auto const row : kvr::iota(this->trackModel.rowCount()))
    {
      auto const& index = this->trackModel.index(row, 0);
      auto const& idData =
        this->trackModel.data(index, sc::LogicalIdentityRole);

      if (idData.value<qint64>() == this->trackToEdit)
      {
        // Get and copy the existing track type
        auto const& typeData =
          this->trackModel.data(index, sc::ClassificationRole);

        sc::objectTrackState(trackState)->detection()->set_type(
          sc::classificationToDetectedObjectType(typeData.toHash()));

        break;
      }
    }

    // Create the new track
    this->createDetection(data, this->trackToEdit, std::move(trackState));
  }
}

// ----------------------------------------------------------------------------
void WindowPrivate::createDetection(
  WindowData* data, qint64 trackId, kv::track_state_sptr&& trackState)
{
  // Wrap the detection in a track
  auto const& track = kv::track::create();
  track->append(std::move(trackState));
  track->set_id(trackId);

  // Ensure that the view has a track model; create one if necessary
  if (!data->trackModel)
  {
    this->setTrackModel(data, std::make_shared<sc::KwiverTrackModel>());
  }

  // Add the detection to the model
  if (auto* const kwiverTrackModel =
        qobject_cast<sc::KwiverTrackModel*>(data->trackModel.get()))
  {
    auto const& newTracks = std::make_shared<kv::object_track_set>();
    newTracks->insert(track);
    kwiverTrackModel->addTracks(newTracks);
  }
}

// ----------------------------------------------------------------------------
QModelIndex WindowPrivate::modelIndex(
  QModelIndex const& representationIndex) const
{
  auto const& filterIndex =
    this->trackRepresentation.mapToSource(representationIndex);
  return this->trackModelFilter.mapToSource(filterIndex);
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
