/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/AutoLevelsTask.hpp>

#include <vital/range/iota.h>

#include <QDebug>
#include <QtAlgorithms>

#include <array>

#include <climits>
#include <cmath>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

constexpr quint64 MIN_SAMPLES = 1024;
constexpr size_t NUM_BUCKETS = 1024;

using BucketArray = std::array<quint64, NUM_BUCKETS>;

using PixelType = kv::image_pixel_traits::pixel_type;

using PixelFunc = double (*)(kv::image const&, size_t, size_t,
                             std::vector<double> const&);

// ----------------------------------------------------------------------------
template <typename T>
unsigned int leadingBit(T dim)
{
  auto const udim = static_cast<typename std::make_unsigned<T>::type>(dim);
  auto const lzb = qCountLeadingZeroBits(udim);
  return static_cast<unsigned int>(sizeof(T) * CHAR_BIT) - lzb;
}

// ----------------------------------------------------------------------------
template <typename T>
double getPixel(kv::image const& image, size_t i, size_t j,
                std::vector<double> const& channelScales)
{
  auto result = 0.0;
  for (auto c : kvr::iota(channelScales.size()))
    result += static_cast<double>(image.at<T>(i, j, c)) * channelScales[c];

  return result;
}

// ----------------------------------------------------------------------------
PixelFunc pixelFunc(kv::image_pixel_traits const& traits)
{
#define PIXELFUNC_CASE(t) case sizeof(t): return &getPixel<t>

  switch (traits.type)
  {
    case PixelType::SIGNED:
      switch (traits.num_bytes)
      {
        PIXELFUNC_CASE(signed char);
        PIXELFUNC_CASE(signed short);
        PIXELFUNC_CASE(signed int);
        PIXELFUNC_CASE(signed long long);
        default: return nullptr;
      }
    case PixelType::UNSIGNED:
      switch (traits.num_bytes)
      {
        PIXELFUNC_CASE(unsigned char);
        PIXELFUNC_CASE(unsigned short);
        PIXELFUNC_CASE(unsigned int);
        PIXELFUNC_CASE(unsigned long long);
        default: return nullptr;
      }
    case PixelType::FLOAT:
      switch (traits.num_bytes)
      {
        PIXELFUNC_CASE(float);
        PIXELFUNC_CASE(double);
        default: return nullptr;
      }
    default:
      return nullptr;
  }

#undef PIXELFUNC_CASE
}

// ----------------------------------------------------------------------------
double imageChannelScale(kv::image_pixel_traits const& traits)
{
  if (traits.type == PixelType::FLOAT)
    return 1.0;

  return std::ldexp(1.0, static_cast<int>(-(traits.num_bytes * CHAR_BIT)));
}

// ----------------------------------------------------------------------------
template <typename Iterator>
std::pair<size_t, size_t> scanBuckets(
  Iterator const& begin, Iterator const& end, quint64 threshold)
{
  auto first = NUM_BUCKETS;
  auto accum = quint64{0};
  auto n = size_t{0};

  for (auto i = begin; i != end; ++i, ++n)
  {
    auto const s = *i;
    if (s > 0)
    {
      first = std::min(first, n);
      accum += s;
      if (accum >= threshold)
        return {first, n};
    }
  }

  qWarning() << __func__ << "did not attain threshold!";
  return {first, n};
}

// ----------------------------------------------------------------------------
float applyTolerance(std::pair<size_t, size_t> indices, size_t tolerance)
{
  static constexpr auto scale = 1.0 / static_cast<double>(NUM_BUCKETS);

  auto const delta = indices.second - indices.first;
  auto const i = (delta > tolerance ? indices.second : indices.first);
  return static_cast<float>(scale * static_cast<double>(i));
}

} // namespace <anonymous>

// ============================================================================
class AutoLevelsTaskPrivate
{
public:
  void update(AutoLevelsTask* q, quint64 samples, BucketArray const& buckets);

  kv::image_container_sptr const image;
  double const outlierDeviance;
  double const outlierTolerance;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(AutoLevelsTask)

// ----------------------------------------------------------------------------
void AutoLevelsTaskPrivate::update(
  AutoLevelsTask* q, quint64 samples,
  BucketArray const& buckets)
{
  // Compute sample count threshold for outliers
  auto const threshold =
    static_cast<quint64>(static_cast<double>(samples) * outlierDeviance);

  // Find outlier and non-outlier buckets
  auto const& lo = scanBuckets(buckets.begin(), buckets.end(), threshold);
  auto const& hi = scanBuckets(buckets.rbegin(), buckets.rend(), threshold);

  // Compute span and number of buckets for tolerance
  auto const span = (NUM_BUCKETS - hi.second) - lo.second;
  auto const tolf = static_cast<double>(span) * outlierTolerance;
  auto const tol = static_cast<size_t>(std::ceil(tolf));

  // Apply tolerance and emit update
  emit q->levelsUpdated(
    applyTolerance(lo, tol), 1.0f - applyTolerance(hi, tol));
}

// ----------------------------------------------------------------------------
AutoLevelsTask::AutoLevelsTask(
  kv::image_container_sptr const& image,
  double outlierDeviance, double outlierTolerance,
  QObject* parent)
  : QObject{parent},
    d_ptr{new AutoLevelsTaskPrivate{image, outlierDeviance, outlierTolerance}}
{
}

// ----------------------------------------------------------------------------
AutoLevelsTask::~AutoLevelsTask()
{
}

// ----------------------------------------------------------------------------
void AutoLevelsTask::execute()
{
  QTE_D();

  if (!d->image)
    return;

  // Get image and image dimensions
  auto const& image = d->image->get_image();
  auto const iCount = image.width();
  auto const jCount = image.height();
  auto const channels = image.depth();

  if (iCount < 1 || jCount < 1)
    return;

  // Get image pixel traits and function to read a pixel
  auto const pt = image.pixel_traits();
  auto const pf = pixelFunc(pt);

  if (!pf)
    return;

  // Compute initial stride
  auto stride = std::max(leadingBit(iCount), leadingBit(jCount));

  // Declare sample count and histogram buckets
  auto samples = quint64{0};
  auto buckets = BucketArray{};

  // Zero-initialize histogram buckets
  for (auto& b : buckets)
    b = 0;

  // Determine channel weights
  auto const channelOffset = (pt.type == PixelType::SIGNED ? 0.5 : 0.0);
  auto const channelScale =
    imageChannelScale(pt) / static_cast<double>(channels);
  auto chanelWeights = std::vector<double>(channels, channelScale);

  // Examine image
  while (stride--)
  {
    for (auto const i : kvr::iota(iCount >> stride))
    {
      for (auto const j : kvr::iota(jCount >> stride))
      {
        if ((i & 0x1) && (j & 0x1))
          continue; // Skip pixels we already looked at

        // Get pixel value
        auto const v = (*pf)(image, i << stride, j << stride, chanelWeights);
        auto const vn = std::min(std::max(0.0, v + channelOffset), 1.0);

        // Get bucket index and increment bucket count
        auto const b =
          std::min(static_cast<size_t>(vn * NUM_BUCKETS), NUM_BUCKETS - 1);
        ++buckets[b];
        ++samples;
      }
    }

    if (samples > MIN_SAMPLES)
      d->update(this, samples, buckets);
  }

  d->update(this, samples, buckets);
}

} // namespace core

} // namespace sealtk
