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

}

namespace gui
{

class PlayerControlPrivate;

class SEALTK_GUI_EXPORT PlayerControl : public QWidget
{
  Q_OBJECT

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

  Q_PROPERTY(kwiver::vital::timestamp::time_t min READ min WRITE setMin
             NOTIFY minSet);
  Q_PROPERTY(kwiver::vital::timestamp::time_t max READ max WRITE setMax
             NOTIFY maxSet);
  Q_PROPERTY(kwiver::vital::timestamp::time_t time READ time WRITE setTime
             NOTIFY timeSet);
  Q_PROPERTY(State state READ state WRITE setState NOTIFY stateSet);

  kwiver::vital::timestamp::time_t min() const;
  kwiver::vital::timestamp::time_t max() const;
  kwiver::vital::timestamp::time_t time() const;
  State state() const;

signals:
  void previousFrameTriggered();
  void nextFrameTriggered();
  void minSet(kwiver::vital::timestamp::time_t time);
  void maxSet(kwiver::vital::timestamp::time_t time);
  void timeSet(kwiver::vital::timestamp::time_t time);
  void stateSet(State state);

public slots:
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

}

}

#endif