/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/PlayerControl.hpp>
#include "ui_PlayerControl.h"

#include <sealtk/core/VideoController.hpp>

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

  d->ui.setupUi(this);

  connect(this, &PlayerControl::minSet,
          d->ui.scrubber, &qtDoubleSlider::setMinimum);
  connect(this, &PlayerControl::maxSet,
          d->ui.scrubber, &qtDoubleSlider::setMaximum);
  connect(this, &PlayerControl::timeSet,
          d->ui.scrubber, &qtDoubleSlider::setValue);
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
    disconnect(d->videoController, &core::VideoController::videoSourcesChanged,
               this, &PlayerControl::setParamsFromVideoController);
    disconnect(d->videoController, &core::VideoController::timeSelected,
               this, &PlayerControl::setTime);
    disconnect(this, &PlayerControl::timeSet,
               d->videoController, &core::VideoController::seek);

    d->videoController = nullptr;
  }

  if (videoController)
  {
    d->videoController = videoController;

    connect(d->videoController, &core::VideoController::videoSourcesChanged,
            this, &PlayerControl::setParamsFromVideoController);
    connect(d->videoController, &core::VideoController::timeSelected,
            this, &PlayerControl::setTime);
    connect(this, &PlayerControl::timeSet,
            d->videoController, &core::VideoController::seek);
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
void PlayerControl::setMin(kwiver::vital::timestamp::time_t min)
{
  QTE_D();
  if (min != d->min)
  {
    d->min = min;
    emit this->minSet(min);
  }
}

// ----------------------------------------------------------------------------
void PlayerControl::setMax(kwiver::vital::timestamp::time_t max)
{
  QTE_D();
  if (max != d->max)
  {
    d->max = max;
    emit this->maxSet(max);
  }
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
    auto times = d->videoController->times();
    auto it = times.begin();
    if (it != times.end())
    {
      kwiver::vital::timestamp::time_t min = *it, max = *it;
      while (++it != times.end())
      {
        if (*it < min)
        {
          min = *it;
        }
        if (*it >max)
        {
          max = *it;
        }
      }

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

}

}
