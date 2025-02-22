//------------------------------------------------------------------------------
/*
    This file is part of clio: https://github.com/XRPLF/clio
    Copyright (c) 2023, the clio developers.

    Permission to use, copy, modify, and distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL,  DIRECT,  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include "util/prometheus/MetricBuilder.h"

#include "util/Assert.h"
#include "util/prometheus/Counter.h"
#include "util/prometheus/Gauge.h"
#include "util/prometheus/Histogram.h"
#include "util/prometheus/MetricBase.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace util::prometheus {

std::unique_ptr<MetricBase>
MetricBuilder::operator()(
    std::string name,
    std::string labelsString,
    MetricType const type,
    std::vector<std::int64_t> const& buckets
)
{
    ASSERT(type != MetricType::HISTOGRAM_DOUBLE, "Wrong metric type. Probably wrong bucket type was used.");
    if (type == MetricType::HISTOGRAM_INT) {
        return makeHistogram(std::move(name), std::move(labelsString), type, buckets);
    }
    ASSERT(buckets.empty(), "Buckets must be empty for non-histogram types.");
    return makeMetric(std::move(name), std::move(labelsString), type);
}

std::unique_ptr<MetricBase>
MetricBuilder::operator()(
    std::string name,
    std::string labelsString,
    MetricType const type,
    std::vector<double> const& buckets
)
{
    ASSERT(type == MetricType::HISTOGRAM_DOUBLE, "This method is for HISTOGRAM_DOUBLE only.");
    return makeHistogram(std::move(name), std::move(labelsString), type, buckets);
}

std::unique_ptr<MetricBase>
MetricBuilder::makeMetric(std::string name, std::string labelsString, MetricType const type)
{
    switch (type) {
        case MetricType::COUNTER_INT:
            return std::make_unique<CounterInt>(name, labelsString);
        case MetricType::COUNTER_DOUBLE:
            return std::make_unique<CounterDouble>(name, labelsString);
        case MetricType::GAUGE_INT:
            return std::make_unique<GaugeInt>(name, labelsString);
        case MetricType::GAUGE_DOUBLE:
            return std::make_unique<GaugeDouble>(name, labelsString);
        case MetricType::HISTOGRAM_INT:
            [[fallthrough]];
        case MetricType::HISTOGRAM_DOUBLE:
            [[fallthrough]];
        case MetricType::SUMMARY:
            [[fallthrough]];
        default:
            ASSERT(false, "Unknown metric type: {}", static_cast<int>(type));
    }
    return nullptr;
}

template <typename ValueType>
    requires std::same_as<ValueType, std::int64_t> || std::same_as<ValueType, double>
std::unique_ptr<MetricBase>
MetricBuilder::makeHistogram(
    std::string name,
    std::string labelsString,
    MetricType type,
    std::vector<ValueType> const& buckets
)
{
    switch (type) {
        case MetricType::HISTOGRAM_INT: {
            if constexpr (std::same_as<ValueType, std::int64_t>) {
                return std::make_unique<HistogramInt>(std::move(name), std::move(labelsString), buckets);
            } else {
                ASSERT(false, "Wrong bucket type for HISTOGRAM_INT.)");
                break;
            }
        }
        case MetricType::HISTOGRAM_DOUBLE:
            if constexpr (std::same_as<ValueType, double>) {
                return std::make_unique<HistogramDouble>(std::move(name), std::move(labelsString), buckets);
            } else {
                ASSERT(false, "Wrong bucket type for HISTOGRAM_DOUBLE.");
                break;
            }
        case MetricType::COUNTER_INT:
            [[fallthrough]];
        case MetricType::COUNTER_DOUBLE:
            [[fallthrough]];
        case MetricType::GAUGE_INT:
            [[fallthrough]];
        case MetricType::GAUGE_DOUBLE:
            [[fallthrough]];
        case MetricType::SUMMARY:
            [[fallthrough]];
        default:
            ASSERT(false, "Unknown metric type: {}", static_cast<int>(type));
    }
    return nullptr;
}

}  // namespace util::prometheus
