/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/DirectoryListing.hpp>

#include <QDir>

namespace sealtk
{

namespace core
{

// ============================================================================
class DirectoryListingData : public QSharedData
{
public:
  void update();

  QStringList types;
  QString directory;

  QHash<QString, QString> files;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC_SHARED(DirectoryListing)

// ----------------------------------------------------------------------------
DirectoryListing::DirectoryListing(
  QStringList const& types, QString const& directory)
  : d_ptr{new DirectoryListingData}
{
  QTE_D_DETACH();

  d->types = types;
  d->directory = directory;
  d->update();
}

// ----------------------------------------------------------------------------
DirectoryListing::~DirectoryListing() = default;
DirectoryListing::DirectoryListing(DirectoryListing const&) = default;
DirectoryListing::DirectoryListing(DirectoryListing&&) = default;
DirectoryListing& DirectoryListing::operator=(
  DirectoryListing const&) = default;
DirectoryListing& DirectoryListing::operator=(
  DirectoryListing&&) = default;

// ----------------------------------------------------------------------------
QString DirectoryListing::directory() const
{
  QTE_D();
  return d->directory;
}

// ----------------------------------------------------------------------------
void DirectoryListing::setDirectory(QString const& directory)
{
  QTE_D_DETACH();
  d->directory = QDir{directory}.absolutePath();
  d->update();
}

// ----------------------------------------------------------------------------
QStringList DirectoryListing::types() const
{
  QTE_D();
  return d->types;
}

// ----------------------------------------------------------------------------
void DirectoryListing::setTypes(QStringList const& types)
{
  QTE_D_DETACH();
  d->types = types;
  d->update();
}

// ----------------------------------------------------------------------------
QHash<QString, QString> DirectoryListing::files() const
{
  QTE_D();
  return d->files;
}

// ----------------------------------------------------------------------------
void DirectoryListing::refresh()
{
  QTE_D_DETACH();
  d->update();
}

// ----------------------------------------------------------------------------
void DirectoryListingData::update()
{
  auto ltypes = QStringList{};
  auto filters = QStringList{};
  for (auto const& t : this->types)
  {
    static auto const ff = QStringLiteral("*.%1");
    ltypes.append(t.toLower());
    filters.append(ff.arg(t));
  }

  auto dir = QDir{this->directory};
  dir.setNameFilters(filters);
  dir.setFilter(QDir::Files);

  this->files.clear();
  for (auto const entry : dir.entryList())
  {
    for (auto const lt : ltypes)
    {
      if (entry.toLower().endsWith(lt))
      {
        auto const& shortName = entry.left(entry.length() - (lt.length() + 1));
        this->files.insert(shortName, dir.absoluteFilePath(entry));
      }
    }
  }
}

} // namespace core

} // namespace sealtk
