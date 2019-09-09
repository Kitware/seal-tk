/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_AbstractDataSink_hpp
#define sealtk_core_AbstractDataSink_hpp

#include <sealtk/core/Export.h>

#include <QObject>

class QAbstractItemModel;
class QUrl;

namespace sealtk
{

namespace core
{

class VideoSource;

// ============================================================================
class SEALTK_CORE_EXPORT AbstractDataSink : public QObject
{
  Q_OBJECT

public:
  AbstractDataSink(QObject* parent = nullptr);
  ~AbstractDataSink() override;

  /// Sets the data to be written by the sink.
  ///
  /// This method is used to tell the sink what data to write. Typically, the
  /// sink will perform some collation of the input data.
  ///
  /// This method returns \c false if the sink does not find any data to write.
  /// Typically, the caller will use this to determine if there is anything to
  /// be written before prompting the user for the destination to which the
  /// data will be written.
  ///
  /// \note Changing the model data after calling #setData and before calling
  ///       #writeData may result in undefined behavior.
  virtual bool setData(VideoSource* video, QAbstractItemModel* model,
                       bool includeHidden = false) = 0;

  /// Write data to the specified URI.
  ///
  /// This instructs the sink to write the data that was previously provided by
  /// a call to #setData to the specified URI. Data writing is synchronous; the
  /// call will not return until the data has been written. (However, the sink
  /// may internally implement an event loop.)
  ///
  /// If an error occurs, #failed will be emitted, and the contents of the
  /// specified output location are unspecified. (For example, if the URI named
  /// an existing file, its contents may have been overwritten.)
  virtual void writeData(QUrl const& uri) const = 0;

signals:
  void failed(QString const& message) const;
};

} // namespace core

} // namespace sealtk

#endif
