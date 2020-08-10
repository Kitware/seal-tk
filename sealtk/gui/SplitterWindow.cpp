/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/SplitterWindow.hpp>
#include "ui_SplitterWindow.h"

namespace sealtk
{

namespace gui
{

// ============================================================================
class SplitterWindowPrivate
{
public:
  SplitterWindowPrivate(SplitterWindow* parent);

  SplitterWindow* parent;
  Ui::SplitterWindow ui;

  bool closable = true;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(SplitterWindow)

// ----------------------------------------------------------------------------
SplitterWindow::SplitterWindow(QWidget* parent)
  : QWidget(parent),
    d_ptr{new SplitterWindowPrivate(this)}
{
  QTE_D();
  d->ui.setupUi(this);

  connect(this, &QWidget::windowTitleChanged,
          d->ui.titleLabel, &QLabel::setText);
  connect(d->ui.closeButton, &QToolButton::clicked,
          this, &QWidget::hide);
}

// ----------------------------------------------------------------------------
SplitterWindow::~SplitterWindow()
{
}

// ----------------------------------------------------------------------------
QWidget* SplitterWindow::centralWidget() const
{
  QTE_D();

  return d->ui.centralWidget;
}

// ----------------------------------------------------------------------------
void SplitterWindow::setCentralWidget(QWidget* widget)
{
  QTE_D();

  d->ui.verticalLayout->replaceWidget(d->ui.centralWidget, widget);
  d->ui.centralWidget = widget;
}

// ----------------------------------------------------------------------------
void SplitterWindow::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  emit this->visibilityChanged(this->isVisible());
}

// ----------------------------------------------------------------------------
void SplitterWindow::hideEvent(QHideEvent* event)
{
  QWidget::hideEvent(event);
  emit this->visibilityChanged(this->isVisible());
}

// ----------------------------------------------------------------------------
bool SplitterWindow::closable() const
{
  QTE_D();

  return d->closable;
}

// ----------------------------------------------------------------------------
void SplitterWindow::setClosable(bool closable)
{
  QTE_D();

  d->closable = closable;
  d->ui.closeButton->setEnabled(closable);
}

// ----------------------------------------------------------------------------
void SplitterWindow::setFilenameVisible(bool visible)
{
  QTE_D();

  d->ui.filenameLabel->setVisible(visible);
}

// ----------------------------------------------------------------------------
void SplitterWindow::setFilename(QString const& filename)
{
  QTE_D();

  d->ui.filenameLabel->setText(filename);
}

// ----------------------------------------------------------------------------
SplitterWindowPrivate::SplitterWindowPrivate(SplitterWindow* parent)
  : parent(parent)
{
}

} // namespace gui

} // namespace sealtk
