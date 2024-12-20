//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/planner/table_filter.hpp"
#include "duckdb/planner/filter/null_filter.hpp"
#include "duckdb/planner/filter/constant_filter.hpp"
#include "duckdb/planner/filter/conjunction_filter.hpp"
#include "duckdb/planner/filter/struct_filter.hpp"
#include "duckdb/planner/filter/optional_filter.hpp"

namespace duckdb {

void TableFilter::Serialize(Serializer &serializer) const {
	serializer.WriteProperty<TableFilterType>(100, "filter_type", filter_type);
}

unique_ptr<TableFilter> TableFilter::Deserialize(Deserializer &deserializer) {
	auto filter_type = deserializer.ReadProperty<TableFilterType>(100, "filter_type");
	unique_ptr<TableFilter> result;
	switch (filter_type) {
	case TableFilterType::CONJUNCTION_AND:
		result = ConjunctionAndFilter::Deserialize(deserializer);
		break;
	case TableFilterType::CONJUNCTION_OR:
		result = ConjunctionOrFilter::Deserialize(deserializer);
		break;
	case TableFilterType::CONSTANT_COMPARISON:
		result = ConstantFilter::Deserialize(deserializer);
		break;
	case TableFilterType::IS_NOT_NULL:
		result = IsNotNullFilter::Deserialize(deserializer);
		break;
	case TableFilterType::IS_NULL:
		result = IsNullFilter::Deserialize(deserializer);
		break;
	case TableFilterType::OPTIONAL_FILTER:
		result = OptionalFilter::Deserialize(deserializer);
		break;
	case TableFilterType::STRUCT_EXTRACT:
		result = StructFilter::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of TableFilter!");
	}
	return result;
}

void ConjunctionAndFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<unique_ptr<TableFilter>>>(200, "child_filters", child_filters);
}

unique_ptr<TableFilter> ConjunctionAndFilter::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<ConjunctionAndFilter>(new ConjunctionAndFilter());
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<TableFilter>>>(200, "child_filters", result->child_filters);
	return std::move(result);
}

void ConjunctionOrFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<unique_ptr<TableFilter>>>(200, "child_filters", child_filters);
}

unique_ptr<TableFilter> ConjunctionOrFilter::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<ConjunctionOrFilter>(new ConjunctionOrFilter());
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<TableFilter>>>(200, "child_filters", result->child_filters);
	return std::move(result);
}

void ConstantFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
	serializer.WriteProperty<ExpressionType>(200, "comparison_type", comparison_type);
	serializer.WriteProperty<Value>(201, "constant", constant);
}

unique_ptr<TableFilter> ConstantFilter::Deserialize(Deserializer &deserializer) {
	auto comparison_type = deserializer.ReadProperty<ExpressionType>(200, "comparison_type");
	auto constant = deserializer.ReadProperty<Value>(201, "constant");
	auto result = duckdb::unique_ptr<ConstantFilter>(new ConstantFilter(comparison_type, constant));
	return std::move(result);
}

void IsNotNullFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
}

unique_ptr<TableFilter> IsNotNullFilter::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<IsNotNullFilter>(new IsNotNullFilter());
	return std::move(result);
}

void IsNullFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
}

unique_ptr<TableFilter> IsNullFilter::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<IsNullFilter>(new IsNullFilter());
	return std::move(result);
}

void OptionalFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
	serializer.WritePropertyWithDefault<unique_ptr<TableFilter>>(200, "child_filter", child_filter);
}

unique_ptr<TableFilter> OptionalFilter::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<OptionalFilter>(new OptionalFilter());
	deserializer.ReadPropertyWithDefault<unique_ptr<TableFilter>>(200, "child_filter", result->child_filter);
	return std::move(result);
}

void StructFilter::Serialize(Serializer &serializer) const {
	TableFilter::Serialize(serializer);
	serializer.WritePropertyWithDefault<idx_t>(200, "child_idx", child_idx);
	serializer.WritePropertyWithDefault<string>(201, "child_name", child_name);
	serializer.WritePropertyWithDefault<unique_ptr<TableFilter>>(202, "child_filter", child_filter);
}

unique_ptr<TableFilter> StructFilter::Deserialize(Deserializer &deserializer) {
	auto child_idx = deserializer.ReadPropertyWithDefault<idx_t>(200, "child_idx");
	auto child_name = deserializer.ReadPropertyWithDefault<string>(201, "child_name");
	auto child_filter = deserializer.ReadPropertyWithDefault<unique_ptr<TableFilter>>(202, "child_filter");
	auto result = duckdb::unique_ptr<StructFilter>(new StructFilter(child_idx, std::move(child_name), std::move(child_filter)));
	return std::move(result);
}

} // namespace duckdb
