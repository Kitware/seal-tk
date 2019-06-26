/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_AbstractDataSource_hpp
#define sealtk_core_AbstractDataSource_hpp

#include <sealtk/core/Export.h>

#include <QObject>

#include <memory>

class QAbstractItemModel;
class QUrl;

namespace sealtk
{

namespace core
{

// ============================================================================
class SEALTK_CORE_EXPORT AbstractDataSource : public QObject
{
  Q_OBJECT

public:
  AbstractDataSource(QObject* parent = nullptr);
  ~AbstractDataSource() override;

  /// Test if the source is active.
  ///
  /// This method is used to test if the source is active (i.e. currently
  /// supplying data). If the source is active, it is an error to call
  /// #readData, and calls to #readData will return \c false.
  ///
  /// Although sources are not intended to be reused (i.e. #readData called
  /// more than once), some sources can be reused after they become inactive.
  virtual bool active() const = 0;

  /// Read data from the specified URI.
  ///
  /// This instructs the source to try to read data from the specified URI. The
  /// type of data that will be read, and the manner in which the URI is
  /// interpreted, depends on the actual source being used.
  ///
  /// Data reading is normally asynchronous. The source will emit either
  /// #modelReady once the data model is ready, or #failed if it determines
  /// that it cannot provide data. Neither signal will be emitted before
  /// #readData has been called at least once.
  ///
  /// If the source is able to determine quickly that it cannot read data,
  /// this method will return \c false instead, and no signals will be emitted.
  /// (This may happen if this method is called on an active source.)
  virtual bool readData(QUrl const& uri) = 0;

signals:
  void modelReady(std::shared_ptr<QAbstractItemModel> const& model);

  void failed(QString const& message);
};

} // namespace core

} // namespace sealtk

Q_DECLARE_METATYPE(std::shared_ptr<QAbstractItemModel>)

#endif
