/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef SEALTK_Window_hpp
#define SEALTK_Window_hpp

#include <QMainWindow>
#include <QMetaObject>
#include <QString>
#include <qtGlobal.h>

namespace sealtk
{

class WindowPrivate;

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window(QWidget* parent = nullptr);
  ~Window() override;

  template<typename T>
  void registerPanelType(const QString& name)
  {
    this->registerPanelType(name, T::staticMetaObject);
  }

  void registerPanelType(const QString& name, const QMetaObject& type);

  int panelCounter() const;
  void setPanelCounter(int counter);

public slots:
  void showAbout();
  void newDockablePanel(const QMetaObject& type);
  void newLeftPanel(const QMetaObject& type);
  void newRightPanel(const QMetaObject& type);

protected:
  QTE_DECLARE_PRIVATE(Window)

private:
  QTE_DECLARE_PRIVATE_RPTR(Window)
};

}

#endif
