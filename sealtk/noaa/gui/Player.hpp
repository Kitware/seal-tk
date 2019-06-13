/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_Player_hpp
#define sealtk_noaa_gui_Player_hpp

#include <sealtk/noaa/gui/Export.h>

#include <sealtk/core/VideoSourceFactory.hpp>

#include <sealtk/gui/Player.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class PlayerPrivate;

class SEALTK_NOAA_GUI_EXPORT Player : public sealtk::gui::Player
{
  Q_OBJECT

public:
  enum class Role
  {
    Master,
    Slave,
  };

  explicit Player(Role role, QWidget* parent = nullptr);
  ~Player() override;

  void registerVideoSourceFactory(QString const& name,
                                  sealtk::core::VideoSourceFactory* factory,
                                  void* handle);

  void setImage(kwiver::vital::image_container_sptr const& image) override;

signals:
  void loadDetectionsTriggered() const;

protected:
  QTE_DECLARE_PRIVATE(Player)

  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  QTE_DECLARE_PRIVATE_RPTR(Player)
};

}

}

}

#endif
