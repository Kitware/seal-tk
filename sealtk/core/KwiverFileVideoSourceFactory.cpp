/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverFileVideoSourceFactory.hpp>

#include <sealtk/core/KwiverVideoSource.hpp>

#include <sealtk/util/unique.hpp>

#include <vital/algo/video_input.h>

#include <qtStlUtil.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrlQuery>

namespace sealtk
{

namespace core
{

// ============================================================================
class KwiverFileVideoSourceFactoryPrivate
{
public:
  QTemporaryFile* imageList = nullptr;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverFileVideoSourceFactory)

// ----------------------------------------------------------------------------
KwiverFileVideoSourceFactory::KwiverFileVideoSourceFactory(QObject* parent)
  : FileVideoSourceFactory{parent},
    d_ptr{new KwiverFileVideoSourceFactoryPrivate}
{
}

// ----------------------------------------------------------------------------
KwiverFileVideoSourceFactory::~KwiverFileVideoSourceFactory()
{
}

// ----------------------------------------------------------------------------
void KwiverFileVideoSourceFactory::loadVideoSource(
  void* handle, QUrl const& uri)
{
  auto realUri = uri;

  if (uri.hasQuery())
  {
    static auto const filterKey = QStringLiteral("filter");

    auto params = QUrlQuery{uri};
    auto const& filter = params.queryItemValue(filterKey);
    if (!filter.isEmpty() && filter != "*")
    {
      realUri = this->applyFilters(uri, filter.split(";"));
      if (realUri != uri)
      {
        if (!realUri.isValid())
        {
          return;
        }

        params.removeQueryItem(filterKey);
        realUri.setQuery(params);
      }
    }
  }

  kwiver::vital::algo::video_input_sptr vi;
  kwiver::vital::algo::video_input::set_nested_algo_configuration(
    "video_reader", this->config(realUri), vi);
  if (vi)
  {
    QTE_D();

    vi->open(stdString(realUri.toLocalFile()));

    auto* vs = new KwiverVideoSource{vi, this->parent()};
    emit this->videoSourceLoaded(handle, vs);

    if (d->imageList)
    {
      d->imageList->setParent(vs);
    }
  }
}

// ----------------------------------------------------------------------------
QUrl KwiverFileVideoSourceFactory::applyFilters(
  QUrl const& uri, QStringList const& filters)
{
  static constexpr auto types = QDir::Files | QDir::NoDotAndDotDot;
  static constexpr auto sorting = QDir::Name | QDir::LocaleAware;

  auto const& path = uri.toLocalFile();
  if (!QFileInfo{path}.isDir())
  {
    qWarning() << "Video Source:"
               << "don't know how to apply filters to non-directory"
               << "for URI" << uri;
    return uri;
  }

  // Get list of images that match the specified filters
  QDir dir{path};
  auto const& entries = dir.entryList(filters, types, sorting);
  if (entries.isEmpty())
  {
    auto details = QStringLiteral("Image path:\n  %1\nFilter(s):\n").arg(path);
    for (auto const& f : filters)
    {
      details += QStringLiteral("  ") + f;
    }

    QMessageBox mb{qApp->activeWindow()};

    mb.setIcon(QMessageBox::Information);
    mb.setWindowTitle(QStringLiteral("No images found"));
    mb.setText(
      QStringLiteral("No images matching the specified filters were found."));
    mb.setDetailedText(details);

    mb.exec();
    return {};
  }

  // Create a temporary image list file
  auto t = make_unique<QTemporaryFile>(this);
  if (!t->open())
  {
    QMessageBox mb{qApp->activeWindow()};

    mb.setIcon(QMessageBox::Warning);
    mb.setWindowTitle(QStringLiteral("Could not create image list"));
    mb.setText(QStringLiteral("Failed to create temporary image list file."));
    mb.setDetailedText(t->errorString());

    mb.exec();
    return {};
  }

  // Write matching image paths
  QTextStream s{t.get()};
  for (auto const& i : entries)
  {
    s << dir.absoluteFilePath(i) << '\n';
  }

  // Update URI with the path to the temporary image list file
  auto newUri = uri;
  newUri.setPath(t->fileName());

  // Take ownership of the temporary file and return the updated URI
  QTE_D();

  d->imageList = t.release();

  return newUri;
}

} // namespace core

} // namespace sealtk
