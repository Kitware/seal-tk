/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/GlobInputDialog.hpp>
#include "ui_GlobInputDialog.h"

#include <vital/range/iota.h>

#include <qtScopedSettingsGroup.h>

#include <QDebug>
#include <QSettings>

namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

namespace // anonymous
{

constexpr int MAX_RECENT_GLOBS = 10;

} // namespace <anonymous>

// ============================================================================
class GlobInputDialogPrivate
{
public:
  Ui::GlobInputDialog ui;

  QSettings settings;
  QString settingsKey;

  QStringList recentGlobs;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(GlobInputDialog)

// ----------------------------------------------------------------------------
GlobInputDialog::GlobInputDialog(
  QString const& settingsKey, QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr{new GlobInputDialogPrivate}
{
  QTE_D();

  d->ui.setupUi(this);
  this->setMaximumHeight(this->minimumSizeHint().height());

  d->settingsKey = settingsKey;
  QTE_WITH_EXPR(qtScopedSettingsGroup{d->settings, d->settingsKey})
  {
    for (auto const i : d->settings.childKeys())
    {
      auto const& item = d->settings.value(i).toString();
      d->recentGlobs.append(item);
      d->ui.glob->addItem(item);
    }
  }
}

// ----------------------------------------------------------------------------
GlobInputDialog::~GlobInputDialog()
{
}

// ----------------------------------------------------------------------------
void GlobInputDialog::addDefaultGlobString(QStringList const& defaultGlobs)
{
  QTE_D();

  if (!defaultGlobs.isEmpty())
  {
    auto const& item = defaultGlobs.join(';');
    if (d->ui.glob->findText(item) == -1)
    {
      d->ui.glob->addItem(item);
    }
  }
}

// ----------------------------------------------------------------------------
void GlobInputDialog::accept()
{
  QTE_D();

  auto const& current = this->globString();
  d->recentGlobs.prepend(current);
  d->recentGlobs.removeDuplicates();
  while (d->recentGlobs.count() > MAX_RECENT_GLOBS)
  {
    d->recentGlobs.removeLast();
  }

  d->settings.remove(d->settingsKey);
  QTE_WITH_EXPR(qtScopedSettingsGroup{d->settings, d->settingsKey})
  {
    for (auto const i : kvr::iota(d->recentGlobs.count()))
    {
      d->settings.setValue(QString::number(i), d->recentGlobs[i]);
    }
  }
  d->settings.sync();

  this->QDialog::accept();
}

// ----------------------------------------------------------------------------
QString GlobInputDialog::globString() const
{
  QTE_D();
  return d->ui.glob->currentText();
}

// ----------------------------------------------------------------------------
void GlobInputDialog::setGlobString(QString const& s)
{
  QTE_D();
  d->ui.glob->setCurrentText(s);
}

} // namespace gui

} // namespace sealtk
