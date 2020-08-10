/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_AbstractDataSink_hpp
#define sealtk_core_AbstractDataSink_hpp

#include <sealtk/core/Export.h>

#include <vital/types/transform_2d.h>

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

  /// Sets the (primary) data to be written by the sink.
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
  ///
  /// \note Setting the primary data may erase any supplemental data that has
  ///       been added using #addData and may reset the primary transformation
  ///       supplied by #setTransform. Users should call this method \em first.
  virtual bool setData(VideoSource* video, QAbstractItemModel* model,
                       bool includeHidden = false) = 0;

  /// Sets the transform to be used for supplemental data.
  ///
  /// This method specifies a transform that maps from the primary data's
  /// coordinate space into a common coordinate space. This is required in
  /// order to use supplemental data. If the sink does not support supplemental
  /// data, or cannot compute the required inverse transform, this method will
  /// return \c false.
  ///
  /// \note Changing the transform may erase any supplemental data that has
  ///       been added using #addData.
  virtual bool setTransform(kwiver::vital::transform_2d_sptr const& transform);

  /// Adds supplemental data to be written by the sink.
  ///
  /// This method is used to add supplemental data for the sink to write.
  /// Typically, the sink will perform some collation of the input data.
  ///
  /// Supplemental data exists in a different coordinate space than the primary
  /// data. The provided transform is used to map supplemental data into a
  /// common coordinate space, which is \em not the same as the primary data's
  /// coordinate space.
  ///
  /// This method returns \c false if the sink does not find any data to write,
  /// or does not support supplemental data. Typically, the caller will use
  /// this to determine if there is anything to be written before prompting the
  /// user for the destination to which the data will be written.
  ///
  /// \note This method may fail if no primary data has been provided.
  ///
  /// \sa #setTransform
  virtual bool addData(QAbstractItemModel* model,
                       kwiver::vital::transform_2d_sptr const& transform,
                       bool includeHidden = false);

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
