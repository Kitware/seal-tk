/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_DirectoryListing_hpp
#define sealtk_core_DirectoryListing_hpp

#include <sealtk/core/Export.h>

#include <qtGlobal.h>

#include <QHash>
#include <QSharedDataPointer>
#include <QStringList>

namespace sealtk
{

namespace core
{

class DirectoryListingData;

/// List of files in a directory matching a specified set of file types.
///
/// This class encapsulates a listing of all files in a specified directory
/// which match a specified set of file types (extensions).
class SEALTK_CORE_EXPORT DirectoryListing
{
public:
  //@{
  /// Construct a directory listing for a list of file types.
  ///
  /// \param types
  ///   List of file extensions to be recognized by this instance. The
  ///   extension must \em not contain the extension delimiter ('.').
  ///   Behavior is unspecified if any type contains wild card characters.
  DirectoryListing(QStringList const& types, QString const& directory = {});
  //@}

  ~DirectoryListing();

  DirectoryListing(DirectoryListing const& other);
  DirectoryListing(DirectoryListing&& other);

  DirectoryListing& operator=(DirectoryListing const& other);
  DirectoryListing& operator=(DirectoryListing&& other);

  QString directory() const;
  void setDirectory(QString const& directory);

  QStringList types() const;
  void setTypes(QStringList const& types);

  /// Obtain the set of files.
  ///
  /// This method returns the set of matching files that is represented by the
  /// instance. The result is a mapping from the file name without the matching
  /// extension to the absolute path of the file, for each matching file.
  QHash<QString, QString> files() const;

  /// Update the directory listing.
  ///
  /// This updates the directory listing by re-scanning the file system. This
  /// is useful if a DirectoryListing instance is long lived in case the file
  /// system contents have changed.
  void refresh();

private:
  QTE_DECLARE_SHARED_PTR(DirectoryListing)
  QTE_DECLARE_SHARED(DirectoryListing)
};

} // namespace core

} // namespace sealtk

#endif
