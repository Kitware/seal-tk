/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/KwiverTrackSource.hpp>

#include <sealtk/core/KwiverTrackModel.hpp>

#include <vital/algo/read_object_track_set.h>

#include <qtStlUtil.h>

#include <QThread>
#include <QUrl>
#include <QUrlQuery>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;

namespace sealtk
{

namespace core
{

// ============================================================================
class KwiverTrackSourcePrivate : public QThread
{
public:
  KwiverTrackSourcePrivate(KwiverTrackSource* q) : q_ptr{q} {}

  void run() override;

  QUrl tracksUri;

private:
  QTE_DECLARE_PUBLIC(KwiverTrackSource)
  QTE_DECLARE_PUBLIC_PTR(KwiverTrackSource)
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(KwiverTrackSource)

// ----------------------------------------------------------------------------
KwiverTrackSource::KwiverTrackSource(QObject* parent)
  : AbstractDataSource{parent}, d_ptr{new KwiverTrackSourcePrivate{this}}
{
}

// ----------------------------------------------------------------------------
KwiverTrackSource::~KwiverTrackSource()
{
  QTE_D();
  d->wait();
}

// ----------------------------------------------------------------------------
bool KwiverTrackSource::active() const
{
  QTE_D();
  return d->isRunning();
}

// ----------------------------------------------------------------------------
bool KwiverTrackSource::readData(QUrl const& uri)
{
  QTE_D();

  if (d->isRunning())
  {
    return false;
  }

  d->tracksUri = uri;
  d->start();

  return true;
}

// ----------------------------------------------------------------------------
void KwiverTrackSourcePrivate::run()
{
  QTE_Q();

  kv::object_track_set_sptr intermediateTracks;
  kv::object_track_set_sptr finalTracks;

  try
  {
    // Set config options for reading tracks
    auto config = kwiver::vital::config_block::empty_config();
    auto params = QUrlQuery{this->tracksUri};
    for (auto const& p : params.queryItems())
    {
      config->set_value(stdString(p.first), stdString(p.second));
    }

    // Create algorithm to read tracks
    kva::read_object_track_set_sptr input;
    kva::read_object_track_set::set_nested_algo_configuration(
      "input", config, input);

    if (!input)
    {
      emit q->failed(QStringLiteral("Failed to initialize reader"));
      return;
    }

    input->open(stdString(this->tracksUri.toLocalFile()));

    // Read tracks
    while (input->read_set(intermediateTracks))
    {
      if (intermediateTracks)
      {
        if (finalTracks)
        {
          finalTracks->merge_in_other_track_set(intermediateTracks);
        }
        else
        {
          finalTracks = intermediateTracks;
        }
      }
    }
  }
  catch (std::exception const& e)
  {
    emit q->failed(QString::fromLocal8Bit(e.what()));
    return;
  }

  // Create the data model
  auto model = std::make_shared<KwiverTrackModel>();
  model->addTracks(finalTracks);

  emit q->modelReady(model);
}

} // namespace core

} // namespace sealtk
