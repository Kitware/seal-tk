/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/PlayerControl.hpp>
#include "ui_PlayerControl.h"

#include <sealtk/core/VideoController.hpp>

#include <QAction>

namespace sealtk
{

namespace gui
{

// ============================================================================
class PlayerControlPrivate
{
public:
  Ui::PlayerControl ui;

  core::VideoController* videoController = nullptr;
  kwiver::vital::timestamp::time_t min = 0;
  kwiver::vital::timestamp::time_t max = 0;
  kwiver::vital::timestamp::time_t time = 0;
  PlayerControl::State state = PlayerControl::Paused;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(PlayerControl)

// ----------------------------------------------------------------------------
PlayerControl::PlayerControl(QWidget* parent)
  : QWidget{parent},
    d_ptr{new PlayerControlPrivate}
{
  QTE_D();

  QIcon::setThemeName("krest");

  d->ui.setupUi(this);

  connect(this, &PlayerControl::rangeSet,
          d->ui.scrubber, &qtDoubleSlider::setRange);
  connect(this, &PlayerControl::timeSet,
          d->ui.scrubber, &qtDoubleSlider::setValue);
  connect(d->ui.scrubber, &qtDoubleSlider::valueChanged,
          this, &PlayerControl::setTime);

  connect(d->ui.previousFrameButton, &QToolButton::pressed,
          this, &PlayerControl::previousFrameTriggered);
  connect(d->ui.nextFrameButton, &QToolButton::pressed,
          this, &PlayerControl::nextFrameTriggered);
}

// ----------------------------------------------------------------------------
PlayerControl::~PlayerControl()
{
}

// ----------------------------------------------------------------------------
core::VideoController* PlayerControl::videoController() const
{
  QTE_D();
  return d->videoController;
}

// ----------------------------------------------------------------------------
void PlayerControl::setVideoController(core::VideoController* videoController)
{
  QTE_D();

  if (d->videoController)
  {
    disconnect(d->videoController, nullptr, this, nullptr);
    disconnect(this, nullptr, d->videoController, nullptr);

    d->videoController = nullptr;
  }

  if (videoController)
  {
    d->videoController = videoController;

    connect(d->videoController, &core::VideoController::timesChanged,
            this, &PlayerControl::setParamsFromVideoController);
    connect(d->videoController, &core::VideoController::timeSelected,
            this, &PlayerControl::setTime);
    connect(this, &PlayerControl::timeSet, d->videoController,
            [d](kwiver::vital::timestamp::time_t time){
              d->videoController->seekNearest(time, 0);
            });
  }

  this->setParamsFromVideoController();
}

// ----------------------------------------------------------------------------
kwiver::vital::timestamp::time_t PlayerControl::min() const
{
  QTE_D();
  return d->min;
}

// ----------------------------------------------------------------------------
kwiver::vital::timestamp::time_t PlayerControl::max() const
{
  QTE_D();
  return d->max;
}

// ----------------------------------------------------------------------------
kwiver::vital::timestamp::time_t PlayerControl::time() const
{
  QTE_D();
  return d->time;
}

// ----------------------------------------------------------------------------
PlayerControl::State PlayerControl::state() const
{
  QTE_D();
  return d->state;
}

// ----------------------------------------------------------------------------
void PlayerControl::setRange(kwiver::vital::timestamp::time_t min,
                             kwiver::vital::timestamp::time_t max)
{
  QTE_D();
  if (min != d->min || max != d->max)
  {
    d->min = min;
    d->max = max;
    emit this->rangeSet(min, max);
  }
}

// ----------------------------------------------------------------------------
void PlayerControl::setMin(kwiver::vital::timestamp::time_t min)
{
  QTE_D();
  this->setRange(min, d->max);
}

// ----------------------------------------------------------------------------
void PlayerControl::setMax(kwiver::vital::timestamp::time_t max)
{
  QTE_D();
  this->setRange(d->min, max);
}

// ----------------------------------------------------------------------------
void PlayerControl::setTime(kwiver::vital::timestamp::time_t time)
{
  QTE_D();
  if (time != d->time)
  {
    d->time = time;
    emit this->timeSet(time);
  }
}

// ----------------------------------------------------------------------------
void PlayerControl::setState(State state)
{
  QTE_D();
  if (state != d->state)
  {
    d->state = state;
    emit this->stateSet(state);
  }
}

// ----------------------------------------------------------------------------
void PlayerControl::setParamsFromVideoController()
{
  QTE_D();

  if (d->videoController)
  {
    auto const& times = d->videoController->times();
    if (!times.isEmpty())
    {
      auto const min = times.begin().key();
      auto const max = (--times.end()).key();

      this->setMin(min);
      this->setMax(max);
    }

    this->setTime(d->videoController->time());
    d->ui.scrubber->setEnabled(true);
  }
  else
  {
    this->setMin(0);
    this->setMax(0);
    this->setTime(0);
    d->ui.scrubber->setEnabled(false);
  }
}

} // namespace gui

} // namespace sealtk
