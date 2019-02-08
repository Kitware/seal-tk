/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/Window.hpp>
#include "ui_Window.h"

#include <sealtk/noaa/gui/About.hpp>

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

#include <sealtk/core/FileVideoSourceFactory.hpp>
#include <sealtk/core/VideoController.hpp>
#include <sealtk/core/VideoSource.hpp>
#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/gui/Player.hpp>
#include <sealtk/gui/SplitterWindow.hpp>

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
class WindowPrivate
{
public:
  WindowPrivate(Window* parent);

  QTE_DECLARE_PUBLIC(Window)
  QTE_DECLARE_PUBLIC_PTR(Window)

  Ui::Window ui;

  std::unique_ptr<sealtk::core::VideoController> videoController;

  bool firstWindow = true;

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
    QObject::connect(
      videoSource, &sealtk::core::VideoSource::kwiverImageDisplayed,
      player, &sealtk::gui::Player::setImage);
    QObject::connect(
      videoSource, &sealtk::core::VideoSource::noImageDisplayed,
      [player]()
    {
      player->setImage(kwiver::vital::image{});
    });

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
