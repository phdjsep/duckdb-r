//-------------------------------------------------------------------------
//                         DuckDB
//
//
// duckdb/common/enums/metrics_type.hpp
// 
// This file is automatically generated by scripts/generate_metric_enums.py
// Do not edit this file manually, your changes will be overwritten
//-------------------------------------------------------------------------

#include "duckdb/common/enums/metric_type.hpp"
namespace duckdb {

profiler_settings_t MetricsUtils::GetOptimizerMetrics() {
    return {
        MetricsType::OPTIMIZER_EXPRESSION_REWRITER,
        MetricsType::OPTIMIZER_FILTER_PULLUP,
        MetricsType::OPTIMIZER_FILTER_PUSHDOWN,
        MetricsType::OPTIMIZER_CTE_FILTER_PUSHER,
        MetricsType::OPTIMIZER_REGEX_RANGE,
        MetricsType::OPTIMIZER_IN_CLAUSE,
        MetricsType::OPTIMIZER_JOIN_ORDER,
        MetricsType::OPTIMIZER_DELIMINATOR,
        MetricsType::OPTIMIZER_UNNEST_REWRITER,
        MetricsType::OPTIMIZER_UNUSED_COLUMNS,
        MetricsType::OPTIMIZER_STATISTICS_PROPAGATION,
        MetricsType::OPTIMIZER_COMMON_SUBEXPRESSIONS,
        MetricsType::OPTIMIZER_COMMON_AGGREGATE,
        MetricsType::OPTIMIZER_COLUMN_LIFETIME,
        MetricsType::OPTIMIZER_BUILD_SIDE_PROBE_SIDE,
        MetricsType::OPTIMIZER_LIMIT_PUSHDOWN,
        MetricsType::OPTIMIZER_TOP_N,
        MetricsType::OPTIMIZER_COMPRESSED_MATERIALIZATION,
        MetricsType::OPTIMIZER_DUPLICATE_GROUPS,
        MetricsType::OPTIMIZER_REORDER_FILTER,
        MetricsType::OPTIMIZER_JOIN_FILTER_PUSHDOWN,
        MetricsType::OPTIMIZER_EXTENSION,
        MetricsType::OPTIMIZER_MATERIALIZED_CTE,
    };
}

profiler_settings_t MetricsUtils::GetPhaseTimingMetrics() {
    return {
        MetricsType::ALL_OPTIMIZERS,
        MetricsType::CUMULATIVE_OPTIMIZER_TIMING,
        MetricsType::PLANNER,
        MetricsType::PLANNER_BINDING,
        MetricsType::PHYSICAL_PLANNER,
        MetricsType::PHYSICAL_PLANNER_COLUMN_BINDING,
        MetricsType::PHYSICAL_PLANNER_RESOLVE_TYPES,
        MetricsType::PHYSICAL_PLANNER_CREATE_PLAN,
    };
}

MetricsType MetricsUtils::GetOptimizerMetricByType(OptimizerType type) {
    switch(type) {
        case OptimizerType::EXPRESSION_REWRITER:
            return MetricsType::OPTIMIZER_EXPRESSION_REWRITER;
        case OptimizerType::FILTER_PULLUP:
            return MetricsType::OPTIMIZER_FILTER_PULLUP;
        case OptimizerType::FILTER_PUSHDOWN:
            return MetricsType::OPTIMIZER_FILTER_PUSHDOWN;
        case OptimizerType::CTE_FILTER_PUSHER:
            return MetricsType::OPTIMIZER_CTE_FILTER_PUSHER;
        case OptimizerType::REGEX_RANGE:
            return MetricsType::OPTIMIZER_REGEX_RANGE;
        case OptimizerType::IN_CLAUSE:
            return MetricsType::OPTIMIZER_IN_CLAUSE;
        case OptimizerType::JOIN_ORDER:
            return MetricsType::OPTIMIZER_JOIN_ORDER;
        case OptimizerType::DELIMINATOR:
            return MetricsType::OPTIMIZER_DELIMINATOR;
        case OptimizerType::UNNEST_REWRITER:
            return MetricsType::OPTIMIZER_UNNEST_REWRITER;
        case OptimizerType::UNUSED_COLUMNS:
            return MetricsType::OPTIMIZER_UNUSED_COLUMNS;
        case OptimizerType::STATISTICS_PROPAGATION:
            return MetricsType::OPTIMIZER_STATISTICS_PROPAGATION;
        case OptimizerType::COMMON_SUBEXPRESSIONS:
            return MetricsType::OPTIMIZER_COMMON_SUBEXPRESSIONS;
        case OptimizerType::COMMON_AGGREGATE:
            return MetricsType::OPTIMIZER_COMMON_AGGREGATE;
        case OptimizerType::COLUMN_LIFETIME:
            return MetricsType::OPTIMIZER_COLUMN_LIFETIME;
        case OptimizerType::BUILD_SIDE_PROBE_SIDE:
            return MetricsType::OPTIMIZER_BUILD_SIDE_PROBE_SIDE;
        case OptimizerType::LIMIT_PUSHDOWN:
            return MetricsType::OPTIMIZER_LIMIT_PUSHDOWN;
        case OptimizerType::SAMPLING_PUSHDOWN:
            return MetricsType::OPTIMIZER_SAMPLING_PUSHDOWN;
        case OptimizerType::TOP_N:
            return MetricsType::OPTIMIZER_TOP_N;
        case OptimizerType::COMPRESSED_MATERIALIZATION:
            return MetricsType::OPTIMIZER_COMPRESSED_MATERIALIZATION;
        case OptimizerType::DUPLICATE_GROUPS:
            return MetricsType::OPTIMIZER_DUPLICATE_GROUPS;
        case OptimizerType::REORDER_FILTER:
            return MetricsType::OPTIMIZER_REORDER_FILTER;
        case OptimizerType::JOIN_FILTER_PUSHDOWN:
            return MetricsType::OPTIMIZER_JOIN_FILTER_PUSHDOWN;
        case OptimizerType::EXTENSION:
            return MetricsType::OPTIMIZER_EXTENSION;
        case OptimizerType::MATERIALIZED_CTE:
            return MetricsType::OPTIMIZER_MATERIALIZED_CTE;
		case OptimizerType::EMPTY_RESULT_PULLUP:
			return MetricsType::OPTIMIZER_EMPTY_RESULT_PULLUP;
       default:
            throw InternalException("OptimizerType %s cannot be converted to a MetricsType", EnumUtil::ToString(type));
    };
}

OptimizerType MetricsUtils::GetOptimizerTypeByMetric(MetricsType type) {
    switch(type) {
        case MetricsType::OPTIMIZER_EXPRESSION_REWRITER:
            return OptimizerType::EXPRESSION_REWRITER;
        case MetricsType::OPTIMIZER_FILTER_PULLUP:
            return OptimizerType::FILTER_PULLUP;
        case MetricsType::OPTIMIZER_FILTER_PUSHDOWN:
            return OptimizerType::FILTER_PUSHDOWN;
        case MetricsType::OPTIMIZER_CTE_FILTER_PUSHER:
            return OptimizerType::CTE_FILTER_PUSHER;
        case MetricsType::OPTIMIZER_REGEX_RANGE:
            return OptimizerType::REGEX_RANGE;
        case MetricsType::OPTIMIZER_IN_CLAUSE:
            return OptimizerType::IN_CLAUSE;
        case MetricsType::OPTIMIZER_JOIN_ORDER:
            return OptimizerType::JOIN_ORDER;
        case MetricsType::OPTIMIZER_DELIMINATOR:
            return OptimizerType::DELIMINATOR;
        case MetricsType::OPTIMIZER_UNNEST_REWRITER:
            return OptimizerType::UNNEST_REWRITER;
        case MetricsType::OPTIMIZER_UNUSED_COLUMNS:
            return OptimizerType::UNUSED_COLUMNS;
        case MetricsType::OPTIMIZER_STATISTICS_PROPAGATION:
            return OptimizerType::STATISTICS_PROPAGATION;
        case MetricsType::OPTIMIZER_COMMON_SUBEXPRESSIONS:
            return OptimizerType::COMMON_SUBEXPRESSIONS;
        case MetricsType::OPTIMIZER_COMMON_AGGREGATE:
            return OptimizerType::COMMON_AGGREGATE;
        case MetricsType::OPTIMIZER_COLUMN_LIFETIME:
            return OptimizerType::COLUMN_LIFETIME;
        case MetricsType::OPTIMIZER_BUILD_SIDE_PROBE_SIDE:
            return OptimizerType::BUILD_SIDE_PROBE_SIDE;
        case MetricsType::OPTIMIZER_LIMIT_PUSHDOWN:
            return OptimizerType::LIMIT_PUSHDOWN;
        case MetricsType::OPTIMIZER_TOP_N:
            return OptimizerType::TOP_N;
        case MetricsType::OPTIMIZER_COMPRESSED_MATERIALIZATION:
            return OptimizerType::COMPRESSED_MATERIALIZATION;
        case MetricsType::OPTIMIZER_DUPLICATE_GROUPS:
            return OptimizerType::DUPLICATE_GROUPS;
        case MetricsType::OPTIMIZER_REORDER_FILTER:
            return OptimizerType::REORDER_FILTER;
        case MetricsType::OPTIMIZER_JOIN_FILTER_PUSHDOWN:
            return OptimizerType::JOIN_FILTER_PUSHDOWN;
        case MetricsType::OPTIMIZER_EXTENSION:
            return OptimizerType::EXTENSION;
        case MetricsType::OPTIMIZER_MATERIALIZED_CTE:
            return OptimizerType::MATERIALIZED_CTE;
		case MetricsType::OPTIMIZER_EMPTY_RESULT_PULLUP:
    		return OptimizerType::EMPTY_RESULT_PULLUP;
    default:
            return OptimizerType::INVALID;
    };
}

bool MetricsUtils::IsOptimizerMetric(MetricsType type) {
    switch(type) {
        case MetricsType::OPTIMIZER_EXPRESSION_REWRITER:
        case MetricsType::OPTIMIZER_FILTER_PULLUP:
        case MetricsType::OPTIMIZER_FILTER_PUSHDOWN:
        case MetricsType::OPTIMIZER_CTE_FILTER_PUSHER:
        case MetricsType::OPTIMIZER_REGEX_RANGE:
        case MetricsType::OPTIMIZER_IN_CLAUSE:
        case MetricsType::OPTIMIZER_JOIN_ORDER:
        case MetricsType::OPTIMIZER_DELIMINATOR:
        case MetricsType::OPTIMIZER_UNNEST_REWRITER:
        case MetricsType::OPTIMIZER_UNUSED_COLUMNS:
        case MetricsType::OPTIMIZER_STATISTICS_PROPAGATION:
        case MetricsType::OPTIMIZER_COMMON_SUBEXPRESSIONS:
        case MetricsType::OPTIMIZER_COMMON_AGGREGATE:
        case MetricsType::OPTIMIZER_COLUMN_LIFETIME:
        case MetricsType::OPTIMIZER_BUILD_SIDE_PROBE_SIDE:
        case MetricsType::OPTIMIZER_LIMIT_PUSHDOWN:
        case MetricsType::OPTIMIZER_SAMPLING_PUSHDOWN:
        case MetricsType::OPTIMIZER_TOP_N:
        case MetricsType::OPTIMIZER_COMPRESSED_MATERIALIZATION:
        case MetricsType::OPTIMIZER_DUPLICATE_GROUPS:
        case MetricsType::OPTIMIZER_REORDER_FILTER:
        case MetricsType::OPTIMIZER_JOIN_FILTER_PUSHDOWN:
        case MetricsType::OPTIMIZER_EXTENSION:
        case MetricsType::OPTIMIZER_MATERIALIZED_CTE:
		case MetricsType::OPTIMIZER_EMPTY_RESULT_PULLUP:
            return true;
        default:
            return false;
    };
}

bool MetricsUtils::IsPhaseTimingMetric(MetricsType type) {
    switch(type) {
        case MetricsType::ALL_OPTIMIZERS:
        case MetricsType::CUMULATIVE_OPTIMIZER_TIMING:
        case MetricsType::PLANNER:
        case MetricsType::PLANNER_BINDING:
        case MetricsType::PHYSICAL_PLANNER:
        case MetricsType::PHYSICAL_PLANNER_COLUMN_BINDING:
        case MetricsType::PHYSICAL_PLANNER_RESOLVE_TYPES:
        case MetricsType::PHYSICAL_PLANNER_CREATE_PLAN:
            return true;
        default:
            return false;
    };
}

} // namespace duckdb
