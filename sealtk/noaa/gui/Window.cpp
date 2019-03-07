/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>
#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/gui/Player.hpp>
#include <sealtk/gui/SplitterWindow.hpp>

#include <QDockWidget>
#include <QFileDialog>
#include <QVector>

#include <QtGlobal>

#include <memory>

namespace sealtk
{

namespace noaa
{

namespace gui
{

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)

  Ui::Window ui;

  std::unique_ptr<sealtk::core::VideoController> videoController;

  bool firstWindow = true;

  float zoom = 1.0f;
  QPointF center{0.0f, 0.0f};

  void registerVideoSourceFactory(QString const& name,
                                  sealtk::core::VideoSourceFactory* factory);

  enum WindowType
  {
    Left,
    Right,
    Dockable,
  };
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

  d->videoController = std::make_unique<sealtk::core::VideoController>(this);
  d->ui.control->setVideoController(d->videoController.get());

  connect(d->ui.actionAbout, &QAction::triggered,
          this, &Window::showAbout);
  connect(
    d->ui.control, &sealtk::gui::PlayerControl::previousFrameTriggered,
    d->videoController.get(), &sealtk::core::VideoController::previousFrame);
  connect(d->ui.control, &sealtk::gui::PlayerControl::nextFrameTriggered,
          d->videoController.get(), &sealtk::core::VideoController::nextFrame);

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
    emit this->zoomSet(zoom);
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
    emit this->centerSet(center);
  }
}

//-----------------------------------------------------------------------------
void Window::showAbout()
{
  About about{this};
  about.exec();
}

//-----------------------------------------------------------------------------
WindowPrivate::WindowPrivate(Window* parent)
  : q_ptr{parent}
{
}

//-----------------------------------------------------------------------------
void WindowPrivate::registerVideoSourceFactory(
  QString const& name, sealtk::core::VideoSourceFactory* factory)
{
  QTE_Q();

  auto* leftAction = new QAction{name, q};
  this->ui.menuNewLeftWindow->addAction(leftAction);
  QObject::connect(leftAction, &QAction::triggered, [factory]()
  {
    factory->loadVideoSource(new WindowType{Left});
  });

  auto* rightAction = new QAction{name, q};
  this->ui.menuNewRightWindow->addAction(rightAction);
  QObject::connect(rightAction, &QAction::triggered, [factory]()
  {
    factory->loadVideoSource(new WindowType{Right});
  });

  auto* dockableAction = new QAction{name, q};
  this->ui.menuNewDockableWindow->addAction(dockableAction);
  QObject::connect(dockableAction, &QAction::triggered, [factory]()
  {
    factory->loadVideoSource(new WindowType{Dockable});
  });

  QObject::connect(
    factory, &sealtk::core::VideoSourceFactory::videoSourceLoaded,
    [this, q](void* handle, sealtk::core::VideoSource* videoSource)
  {
    auto* player = new sealtk::gui::Player{q};
    player->setVideoSource(videoSource);

    player->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* kwiverVideoSource = dynamic_cast<sealtk::core::KwiverVideoSource*>(
      videoSource);
    if (kwiverVideoSource)
    {
      auto* loadDetectionsAction = new QAction{"Load Detections...", player};
      player->addAction(loadDetectionsAction);

      QObject::connect(loadDetectionsAction, &QAction::triggered,
                       [q, kwiverVideoSource]()
      {
        QString filename = QFileDialog::getOpenFileName(q);
        if (!filename.isNull())
        {
          auto config = kwiver::vital::config_block::empty_config();
          config->set_value("input:type", "csv");
          kwiver::vital::algo::detected_object_set_input_sptr input;
          kwiver::vital::algo::detected_object_set_input
            ::set_nested_algo_configuration("input", config, input);
          input->open(filename.toStdString());
          kwiverVideoSource->setDetectedObjectSetInput(input);
        }
      });
    }

    QObject::connect(
      videoSource, &sealtk::core::VideoSource::detectedObjectSetDisplayed,
      player, &sealtk::gui::Player::setDetectedObjectSet);

    QObject::connect(q, &Window::zoomSet,
                     player, &sealtk::gui::Player::setZoom);
    QObject::connect(player, &sealtk::gui::Player::zoomSet,
                     q, &Window::setZoom);
    player->setZoom(q->zoom());

    QObject::connect(q, &Window::centerSet,
                     player, &sealtk::gui::Player::setCenter);
    QObject::connect(player, &sealtk::gui::Player::centerSet,
                     q, &Window::setCenter);
    player->setCenter(q->center());

    WindowType* type = static_cast<WindowType*>(handle);
    switch (*type)
    {
      case Left:
      case Right:
      {
        auto* splitterWindow = new sealtk::gui::SplitterWindow{q};
        splitterWindow->setCentralWidget(player);
        if (*type == Left)
        {
          this->ui.centralwidget->insertWidget(0, splitterWindow);
        }
        else
        {
          this->ui.centralwidget->addWidget(splitterWindow);
        }
        break;
      }

      case Dockable:
      {
        auto* dockableWindow = new QDockWidget{q};
        dockableWindow->setWidget(player);
        q->addDockWidget(Qt::LeftDockWidgetArea, dockableWindow);
        break;
      }
    }

    if (this->firstWindow)
    {
      this->firstWindow = false;
      auto times = this->videoController->times();
      auto it = times.begin();
      if (it != times.end())
      {
        auto min = *it;
        while (++it != times.end())
        {
          if (*it < min)
          {
            min = *it;
          }
        }

        this->videoController->seek(min);
      }
    }

    delete type;

    videoSource->invalidate();
  });

  auto* fileFactory =
    dynamic_cast<sealtk::core::FileVideoSourceFactory*>(factory);
  if (fileFactory)
  {
    QObject::connect(
      fileFactory, &sealtk::core::FileVideoSourceFactory::fileRequested,
      [q, fileFactory](void* handle)
    {
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
      else
      {
        delete static_cast<WindowType*>(handle);
      }
    });
  }
}

}

}

}
