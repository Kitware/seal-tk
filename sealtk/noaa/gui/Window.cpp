/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>
#include <sealtk/noaa/gui/Player.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/KwiverVideoSource.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>
#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/util/unique.hpp>

#include <sealtk/gui/SplitterWindow.hpp>

#include <qtStlUtil.h>

#include <QDockWidget>
#include <QFileDialog>
#include <QVector>

#include <memory>

namespace sealtk
{

namespace noaa
{

namespace gui
{

//=============================================================================
class WindowData
{
public:
  sealtk::gui::SplitterWindow* window;
  sealtk::noaa::gui::Player* player;
};

//=============================================================================
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  void registerVideoSourceFactory(QString const& name,
                                  sealtk::core::VideoSourceFactory* factory);
  void createWindow(WindowData* data, QString const& title,
                    sealtk::noaa::gui::Player::Role role);

  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)

  Ui::Window ui;

  std::unique_ptr<sealtk::core::VideoController> videoController;

  WindowData eoWindow;
  WindowData irWindow;

  bool firstWindow = true;

  float zoom = 1.0f;
  QPointF center{0.0f, 0.0f};
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

  d->videoController = make_unique<sealtk::core::VideoController>(this);
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
WindowPrivate::WindowPrivate(Window* parent)
  : q_ptr{parent}
{
}

//-----------------------------------------------------------------------------
void WindowPrivate::registerVideoSourceFactory(
  QString const& name, sealtk::core::VideoSourceFactory* factory)
{
  QTE_Q();

  this->eoWindow.player->registerVideoSourceFactory(name, factory,
                                                    &this->eoWindow);
  this->irWindow.player->registerVideoSourceFactory(name, factory,
                                                    &this->irWindow);

  QObject::connect(
    factory, &sealtk::core::VideoSourceFactory::videoSourceLoaded,
    [this, q](void* handle, sealtk::core::VideoSource* videoSource)
  {
    auto* data = static_cast<WindowData*>(handle);
    auto* oldVideoSource = data->player->videoSource();
    data->player->setVideoSource(videoSource);
    if (oldVideoSource)
    {
      delete oldVideoSource;
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
    });
  }
}

//-----------------------------------------------------------------------------
void WindowPrivate::createWindow(WindowData* data, QString const& title,
                                 sealtk::noaa::gui::Player::Role role)
{
  QTE_Q();

  data->window = new sealtk::gui::SplitterWindow{q};
  data->player = new sealtk::noaa::gui::Player{role, data->window};
  data->window->setCentralWidget(data->player);
  data->window->setClosable(false);
  data->window->setWindowTitle(title);

  QObject::connect(q, &Window::zoomChanged,
                   data->player, &sealtk::gui::Player::setZoom);
  QObject::connect(data->player, &sealtk::gui::Player::zoomChanged,
                   q, &Window::setZoom);
  data->player->setZoom(q->zoom());

  QObject::connect(q, &Window::centerChanged,
                   data->player, &sealtk::gui::Player::setCenter);
  QObject::connect(data->player, &sealtk::gui::Player::centerChanged,
                   q, &Window::setCenter);
  data->player->setCenter(q->center());

  QObject::connect(
    data->player, &sealtk::noaa::gui::Player::loadDetectionsTriggered,
    [q, data]()
  {
    auto* kwiverVideoSource = qobject_cast<sealtk::core::KwiverVideoSource*>(
      data->player->videoSource());
    if (kwiverVideoSource)
    {
      QString filename = QFileDialog::getOpenFileName(q);
      if (!filename.isNull())
      {
        auto config = kwiver::vital::config_block::empty_config();
        config->set_value("input:type", "csv");
        kwiver::vital::algo::detected_object_set_input_sptr input;
        kwiver::vital::algo::detected_object_set_input
          ::set_nested_algo_configuration("input", config, input);
        input->open(stdString(filename));
        kwiverVideoSource->setDetectedObjectSetInput(input);
      }
    }
  });

  this->ui.centralwidget->addWidget(data->window);
}

}

}

}
