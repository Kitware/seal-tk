/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Window_hpp
#define sealtk_gui_Window_hpp

#include <QMainWindow>
#include <QMetaObject>
#include <QString>
#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

class WindowPrivate;

class Panel;

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window(QWidget* parent = nullptr);
  ~Window() override;

  template<typename T>
  void registerPanelType(QString const& name)
  {
    this->registerPanelType(name, T::staticMetaObject);
  }

  void registerPanelType(QString const& name, QMetaObject const& type);

  int panelCounter() const;
  void setPanelCounter(int counter);

signals:
  void panelTypeRegistered(QString const& name, QMetaObject const& type);

protected:
  QTE_DECLARE_PRIVATE(Window)

  Panel* createPanel(const QMetaObject& type);

private:
  QTE_DECLARE_PRIVATE_RPTR(Window)
};

}

}

#endif
