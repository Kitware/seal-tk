/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "SplitterWindow.hpp"
#include "ui_SplitterWindow.h"

namespace sealtk
{

class SplitterWindowPrivate
{
public:
  SplitterWindowPrivate(SplitterWindow* parent);

  SplitterWindow* parent;
  Ui::SplitterWindow ui;
};

///////////////////////////////////////////////////////////////////////////////

QTE_IMPLEMENT_D_FUNC(SplitterWindow)

///////////////////////////////////////////////////////////////////////////////

SplitterWindow::SplitterWindow(QWidget* parent)
  : QWidget(parent),
    d_ptr{new SplitterWindowPrivate(this)}
{
  QTE_D();
  d->ui.setupUi(this);

  connect(this, &QWidget::windowTitleChanged,
          d->ui.titleLabel, &QLabel::setText);
}

///////////////////////////////////////////////////////////////////////////////

SplitterWindow::~SplitterWindow()
{
}

///////////////////////////////////////////////////////////////////////////////

QWidget* SplitterWindow::centralWidget() const
{
  QTE_D();

  return d->ui.centralWidget;
}

///////////////////////////////////////////////////////////////////////////////

void SplitterWindow::setCentralWidget(QWidget* widget)
{
  QTE_D();

  d->ui.verticalLayout->replaceWidget(d->ui.centralWidget, widget);
  d->ui.centralWidget = widget;
}

///////////////////////////////////////////////////////////////////////////////

SplitterWindowPrivate::SplitterWindowPrivate(SplitterWindow* parent)
  : parent(parent)
{
}

}
