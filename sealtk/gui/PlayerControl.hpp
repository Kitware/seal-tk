/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_PlayerControl_hpp
#define sealtk_gui_PlayerControl_hpp

#include <sealtk/gui/Export.h>

#include <qtGlobal.h>

#include <QWidget>

#include <vital/types/timestamp.h>

namespace sealtk
{

namespace core
{

class VideoController;

} // namespace core

namespace gui
{

class PlayerControlPrivate;

class SEALTK_GUI_EXPORT PlayerControl : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(kwiver::vital::timestamp::time_t min READ min WRITE setMin)
  Q_PROPERTY(kwiver::vital::timestamp::time_t max READ max WRITE setMax)
  Q_PROPERTY(kwiver::vital::timestamp::time_t time READ time WRITE setTime
             NOTIFY timeSet)
  Q_PROPERTY(State state READ state WRITE setState NOTIFY stateSet)

public:
  enum State
  {
    Paused,
    Playing,
  };

  Q_INVOKABLE explicit PlayerControl(QWidget* parent = nullptr);
  ~PlayerControl() override;

  core::VideoController* videoController() const;
  void setVideoController(core::VideoController* videoController);

  kwiver::vital::timestamp::time_t min() const;
  kwiver::vital::timestamp::time_t max() const;
  kwiver::vital::timestamp::time_t time() const;
  State state() const;

signals:
  void previousFrameTriggered();
  void nextFrameTriggered();
  void rangeSet(kwiver::vital::timestamp::time_t min,
                kwiver::vital::timestamp::time_t max);
  void timeSet(kwiver::vital::timestamp::time_t time);
  void stateSet(State state);

public slots:
  void setRange(kwiver::vital::timestamp::time_t min,
                kwiver::vital::timestamp::time_t max);
  void setMin(kwiver::vital::timestamp::time_t min);
  void setMax(kwiver::vital::timestamp::time_t max);
  void setTime(kwiver::vital::timestamp::time_t time);
  void setState(State state);
  void setParamsFromVideoController();

protected:
  QTE_DECLARE_PRIVATE(PlayerControl)

private:
  QTE_DECLARE_PRIVATE_RPTR(PlayerControl)
};

} // namespace gui

} // namespace sealtk

#endif
