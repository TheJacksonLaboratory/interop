/** Plot the flowcell map
 *
 *  @file
 *  @date 5/5/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */
#pragma once

#include "interop/util/exception.h"
#include "interop/util/statistics.h"
#include "interop/constants/enums.h"
#include "interop/model/model_exceptions.h"
#include "interop/model/run_metrics.h"
#include "interop/model/plot/filter_options.h"
#include "interop/model/plot/flowcell_data.h"
#include "interop/logic/utils/metric_type_ext.h"
#include "interop/logic/utils/channel.h"
#include "interop/logic/metric/metric_value.h"

namespace illumina { namespace interop { namespace logic { namespace plot {


    /** Populate the flowcell map based on the filter options
     *
     * @param beg iterator to start of q-metric collection
     * @param end iterator to end of q-metric collection
     * @param proxy functor that takes a metric record and returns a metric value
     * @param layout layout of the flowcell
     * @param options filter for metric records
     * @param data flowcell map
     */
    template<typename I, typename MetricProxy>
    void populate_flowcell_map(I beg,
                               I end,
                               MetricProxy& proxy,
                               const constants::metric_type type,
                               const model::run::flowcell_layout& layout,
                               const model::plot::filter_options &options,
                               model::plot::flowcell_data& data,
                               std::vector<float>& values_for_scaling)
    {
        if(beg == end) return;
        const bool all_surfaces = !options.is_specific_surface();
        for (;beg != end;++beg)
        {
            if( !options.valid_tile_cycle(*beg) ) continue;
            const float val = proxy(*beg, type);
            if(std::isnan(val)) continue;
            data.set_data(beg->lane()-1,
                          beg->physical_location_index(
                                  layout.naming_method(),
                                  layout.sections_per_lane(),
                                  layout.tile_count(),
                                  layout.swath_count(),
                                  all_surfaces),
                          beg->tile(),
                          val);
            values_for_scaling.push_back(val);
        }
    }

    /** Plot a flowcell map
     *
     * @ingroup plot_logic
     * @param metrics run metrics
     * @param type specific metric value to plot by cycle
     * @param options options to filter the data
     * @param data output flowcell map
     */
    inline void plot_flowcell_map(model::metrics::run_metrics& metrics,
                                  const constants::metric_type type,
                                  const model::plot::filter_options& options,
                                  model::plot::flowcell_data& data)
                                  throw(model::invalid_filter_option,
                                  model::invalid_metric_type,
                                  model::index_out_of_bounds_exception)
    {
        const model::run::flowcell_layout& layout = metrics.run_info().flowcell();
        data.clear();
        data.resize(layout.lane_count(), layout.total_swaths(!options.is_specific_surface()), layout.tiles_per_lane());
        std::vector<float> values_for_scaling;
        values_for_scaling.reserve(data.length());

        options.validate(type, metrics.run_info());

        if(utils::is_cycle_metric(type) && options.all_cycles())
            INTEROP_THROW( model::invalid_filter_option, "All cycles is unsupported");
        if(utils::is_read_metric(type) && options.all_reads() && metrics.run_info().reads().size() > 1)
            INTEROP_THROW( model::invalid_filter_option, "All reads is unsupported");
        switch(logic::utils::to_group(type))
        {
            case constants::Tile:
            {
                typedef model::metrics::tile_metric metric_t;
                typedef model::metric_base::metric_set<metric_t> metric_set_t;
                const metric_set_t& metric_set = metrics.get_set<metric_t>();
                metric::metric_value<metric_t> proxy(options.read());
                populate_flowcell_map(metric_set.begin(), metric_set.end(), proxy, type, layout, options, data,
                                      values_for_scaling);
                break;
            }
            case constants::Extraction:
            {
                typedef model::metrics::extraction_metric metric_t;
                typedef model::metric_base::metric_set<metric_t> metric_set_t;
                const metric_set_t& metric_set = metrics.get_set<metric_t>();
                const size_t channel = options.channel();
                if(options.all_channels(type))
                    INTEROP_THROW(model::invalid_filter_option, "All channels is unsupported");
                metric::metric_value<metric_t> proxy(channel);
                populate_flowcell_map(metric_set.begin(), metric_set.end(), proxy, type, layout, options, data,
                                      values_for_scaling);
                break;
            }
            case constants::CorrectedInt:
            {
                typedef model::metrics::corrected_intensity_metric metric_t;
                typedef model::metric_base::metric_set<metric_t> metric_set_t;
                const metric_set_t& metric_set = metrics.get_set<metric_t>();
                const constants::dna_bases base = options.dna_base();
                if(options.all_bases(type))
                    INTEROP_THROW( model::invalid_filter_option, "All bases is unsupported");
                metric::metric_value<metric_t> proxy(base);
                populate_flowcell_map(metric_set.begin(), metric_set.end(), proxy, type, layout, options, data,
                        values_for_scaling);
                break;
            }
            case constants::Q:
            {
                typedef model::metrics::q_collapsed_metric metric_t;
                typedef model::metric_base::metric_set<metric_t> metric_set_t;
                metric_set_t &metric_set = metrics.get_set<metric_t>();
                if(0 == metric_set.size())
                {
                    logic::metric::create_collapse_q_metrics(metrics.get_set<model::metrics::q_metric>(), metric_set);
                    if(type == constants::AccumPercentQ20 || type == constants::AccumPercentQ30)
                        logic::metric::populate_cumulative_distribution(metric_set);
                }
                metric::metric_value<metric_t> proxy;
                populate_flowcell_map(metric_set.begin(), metric_set.end(), proxy, type, layout, options, data,
                                      values_for_scaling);
                break;
            }
            case constants::Error:
            {
                typedef model::metrics::error_metric metric_t;
                typedef model::metric_base::metric_set<metric_t> metric_set_t;
                const metric_set_t& metric_set = metrics.get_set<metric_t>();
                metric::metric_value<metric_t> proxy;
                populate_flowcell_map(metric_set.begin(), metric_set.end(), proxy, type, layout, options, data,
                                      values_for_scaling);
                break;
            }
            default:
                INTEROP_THROW( model::invalid_metric_type, "Unsupported metric type: " << constants::to_string(type));
        };

        if(!values_for_scaling.empty())
        {
            std::sort(values_for_scaling.begin(), values_for_scaling.end());
            // TODO: Put this back
            /*
            const float lower = util::percentile_sorted<float>(values_for_scaling.begin(), values_for_scaling.end(),
                                                               25);
            const float upper = util::percentile_sorted<float>(values_for_scaling.begin(), values_for_scaling.end(),
                                                               75);*/
            const float lower = values_for_scaling[size_t(0.25*values_for_scaling.size())];
            const float upper = values_for_scaling[size_t(0.75*values_for_scaling.size())];
            data.set_range(std::max(lower - 2 * (upper - lower), values_for_scaling[0]),
                           std::min(values_for_scaling.back(), upper + 2 * (upper - lower)));
        }
        else data.set_range(0,0);
        if(type == constants::ErrorRate) data.set_range(0, std::min(5.0f, data.saxis().max()));

        std::string title = metrics.run_info().flowcell().barcode();
        if(title != "") title += " ";
        title += utils::to_description(type);
        data.set_title(title);

        std::string subtitle;
        if(metrics.run_info().flowcell().surface_count()>1)
            subtitle += options.surface_description() + " ";
        subtitle += options.cycle_description();
        if(logic::utils::is_channel_metric(type))
            subtitle += " " + options.channel_description(metrics.run_info().channels());
        if(logic::utils::is_base_metric(type))
            subtitle += " " + options.base_description();
        if(logic::utils::is_read_metric(type))
            subtitle += " " + options.read_description();
        data.set_subtitle(subtitle);
        data.set_label(utils::to_description(type));
    }
    /** Plot a flowcell map
     *
     * @ingroup plot_logic
     * @param metrics run metrics
     * @param metric_name specific metric value to plot by cycle
     * @param options options to filter the data
     * @param data output flowcell map
     */
    inline void plot_flowcell_map(model::metrics::run_metrics& metrics,
                                  const std::string& metric_name,
                                  const model::plot::filter_options& options,
                                  model::plot::flowcell_data& data)
                                  throw(model::invalid_filter_option, model::invalid_metric_type)
    {
        const constants::metric_type type = constants::parse<constants::metric_type>(metric_name);
        if(type == constants::UnknownMetricType)
            INTEROP_THROW(model::invalid_metric_type, "Unsupported metric type: " << metric_name);
        plot_flowcell_map(metrics, type, options, data);
    }
    /** List metric type names available for flowcell
     *
     * @param names destination vector to fill with metric type names
     */
    inline void list_flowcell_metrics(std::vector<std::string>& names)
    {
        std::vector<constants::metric_type> types;
        constants::list_enums(types);
        names.clear();
        names.reserve(types.size());
        for(size_t i=0;i<types.size();++i)
        {
            if(utils::to_feature(types[i]) == constants::UnknownMetricFeature) continue;
            names.push_back(constants::to_string(types[i]));
        }
    }


}}}}
