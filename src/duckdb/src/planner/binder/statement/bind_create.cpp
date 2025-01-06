#include "duckdb/catalog/catalog.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_search_path.hpp"
#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/function/scalar_macro_function.hpp"
#include "duckdb/function/table/table_scan.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/client_data.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/parser/constraints/foreign_key_constraint.hpp"
#include "duckdb/parser/constraints/list.hpp"
#include "duckdb/parser/constraints/unique_constraint.hpp"
#include "duckdb/parser/expression/constant_expression.hpp"
#include "duckdb/parser/expression/function_expression.hpp"
#include "duckdb/parser/expression/subquery_expression.hpp"
#include "duckdb/parser/parsed_data/create_index_info.hpp"
#include "duckdb/parser/parsed_data/create_macro_info.hpp"
#include "duckdb/parser/parsed_data/create_secret_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/parser/parsed_expression_iterator.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
#include "duckdb/parser/tableref/basetableref.hpp"
#include "duckdb/parser/tableref/table_function_ref.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/bound_query_node.hpp"
#include "duckdb/planner/expression/bound_cast_expression.hpp"
#include "duckdb/planner/expression/bound_columnref_expression.hpp"
#include "duckdb/planner/expression_binder/index_binder.hpp"
#include "duckdb/planner/expression_binder/select_binder.hpp"
#include "duckdb/planner/operator/logical_create.hpp"
#include "duckdb/planner/operator/logical_create_table.hpp"
#include "duckdb/planner/operator/logical_get.hpp"
#include "duckdb/planner/operator/logical_projection.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/planner/query_node/bound_select_node.hpp"
#include "duckdb/planner/tableref/bound_basetableref.hpp"
#include "duckdb/storage/storage_extension.hpp"
#include "duckdb/common/extension_type_info.hpp"

namespace duckdb {

void Binder::BindSchemaOrCatalog(ClientContext &context, string &catalog, string &schema) {
	if (catalog.empty() && !schema.empty()) {
		// schema is specified - but catalog is not
		// try searching for the catalog instead
		auto &db_manager = DatabaseManager::Get(context);
		auto database = db_manager.GetDatabase(context, schema);
		if (database) {
			// we have a database with this name
			// check if there is a schema
			auto &search_path = *context.client_data->catalog_search_path;
			auto catalog_names = search_path.GetCatalogsForSchema(schema);
			if (catalog_names.empty()) {
				catalog_names.push_back(DatabaseManager::GetDefaultDatabase(context));
			}
			for (auto &catalog_name : catalog_names) {
				auto &catalog = Catalog::GetCatalog(context, catalog_name);
				if (catalog.CheckAmbiguousCatalogOrSchema(context, schema)) {
					throw BinderException(
					    "Ambiguous reference to catalog or schema \"%s\" - use a fully qualified path like \"%s.%s\"",
					    schema, catalog_name, schema);
				}
			}
			catalog = schema;
			schema = string();
		}
	}
}

void Binder::BindSchemaOrCatalog(string &catalog, string &schema) {
	BindSchemaOrCatalog(context, catalog, schema);
}

const string Binder::BindCatalog(string &catalog) {
	auto &db_manager = DatabaseManager::Get(context);
	optional_ptr<AttachedDatabase> database = db_manager.GetDatabase(context, catalog);
	if (database) {
		return db_manager.GetDatabase(context, catalog).get()->GetName();
	} else {
		return db_manager.GetDefaultDatabase(context);
	}
}

SchemaCatalogEntry &Binder::BindSchema(CreateInfo &info) {
	BindSchemaOrCatalog(info.catalog, info.schema);
	if (IsInvalidCatalog(info.catalog) && info.temporary) {
		info.catalog = TEMP_CATALOG;
	}
	auto &search_path = ClientData::Get(context).catalog_search_path;
	if (IsInvalidCatalog(info.catalog) && IsInvalidSchema(info.schema)) {
		auto &default_entry = search_path->GetDefault();
		info.catalog = default_entry.catalog;
		info.schema = default_entry.schema;
	} else if (IsInvalidSchema(info.schema)) {
		info.schema = search_path->GetDefaultSchema(info.catalog);
	} else if (IsInvalidCatalog(info.catalog)) {
		info.catalog = search_path->GetDefaultCatalog(info.schema);
	}
	if (IsInvalidCatalog(info.catalog)) {
		info.catalog = DatabaseManager::GetDefaultDatabase(context);
	}
	if (!info.temporary) {
		// non-temporary create: not read only
		if (info.catalog == TEMP_CATALOG) {
			throw ParserException("Only TEMPORARY table names can use the \"%s\" catalog", TEMP_CATALOG);
		}
	} else {
		if (info.catalog != TEMP_CATALOG) {
			throw ParserException("TEMPORARY table names can *only* use the \"%s\" catalog", TEMP_CATALOG);
		}
	}
	// fetch the schema in which we want to create the object
	auto &schema_obj = Catalog::GetSchema(context, info.catalog, info.schema);
	D_ASSERT(schema_obj.type == CatalogType::SCHEMA_ENTRY);
	info.schema = schema_obj.name;
	if (!info.temporary) {
		auto &properties = GetStatementProperties();
		properties.RegisterDBModify(schema_obj.catalog, context);
	}
	return schema_obj;
}

SchemaCatalogEntry &Binder::BindCreateSchema(CreateInfo &info) {
	auto &schema = BindSchema(info);
	if (schema.catalog.IsSystemCatalog()) {
		throw BinderException("Cannot create entry in system catalog");
	}
	return schema;
}

void Binder::SetCatalogLookupCallback(catalog_entry_callback_t callback) {
	entry_retriever.SetCallback(std::move(callback));
}

void Binder::BindCreateViewInfo(CreateViewInfo &base) {
	// bind the view as if it were a query so we can catch errors
	// note that we bind the original, and replace the original with a copy
	auto view_binder = Binder::CreateBinder(context);
	auto &dependencies = base.dependencies;
	auto &catalog = Catalog::GetCatalog(context, base.catalog);

	auto &db_config = DBConfig::GetConfig(context);
	bool should_create_dependencies = db_config.GetSetting<EnableViewDependenciesSetting>(context);
	if (should_create_dependencies) {
		view_binder->SetCatalogLookupCallback([&dependencies, &catalog](CatalogEntry &entry) {
			if (&catalog != &entry.ParentCatalog()) {
				// Don't register dependencies between catalogs
				return;
			}
			dependencies.AddDependency(entry);
		});
	}
	view_binder->can_contain_nulls = true;

	auto copy = base.query->Copy();
	auto query_node = view_binder->Bind(*base.query);
	base.query = unique_ptr_cast<SQLStatement, SelectStatement>(std::move(copy));
	if (base.aliases.size() > query_node.names.size()) {
		throw BinderException("More VIEW aliases than columns in query result");
	}
	base.types = query_node.types;
	base.names = query_node.names;
}

SchemaCatalogEntry &Binder::BindCreateFunctionInfo(CreateInfo &info) {
	auto &base = info.Cast<CreateMacroInfo>();

	auto &dependencies = base.dependencies;
	auto &catalog = Catalog::GetCatalog(context, info.catalog);
	auto &db_config = DBConfig::GetConfig(context);
	// try to bind each of the included functions
	unordered_set<idx_t> positional_parameters;
	for (auto &function : base.macros) {
		auto &scalar_function = function->Cast<ScalarMacroFunction>();
		if (scalar_function.expression->HasParameter()) {
			throw BinderException("Parameter expressions within macro's are not supported!");
		}
		vector<LogicalType> dummy_types;
		vector<string> dummy_names;
		auto parameter_count = function->parameters.size();
		if (positional_parameters.find(parameter_count) != positional_parameters.end()) {
			throw BinderException(
			    "Ambiguity in macro overloads - macro \"%s\" has multiple definitions with %llu parameters", base.name,
			    parameter_count);
		}
		positional_parameters.insert(parameter_count);

		// positional parameters
		for (auto &param_expr : function->parameters) {
			auto param = param_expr->Cast<ColumnRefExpression>();
			if (param.IsQualified()) {
				throw BinderException("Invalid parameter name '%s': must be unqualified", param.ToString());
			}
			dummy_types.emplace_back(LogicalType::SQLNULL);
			dummy_names.push_back(param.GetColumnName());
		}
		// default parameters
		for (auto &entry : function->default_parameters) {
			auto &val = entry.second->Cast<ConstantExpression>();
			dummy_types.push_back(val.value.type());
			dummy_names.push_back(entry.first);
		}
		auto this_macro_binding = make_uniq<DummyBinding>(dummy_types, dummy_names, base.name);
		macro_binding = this_macro_binding.get();

		// create a copy of the expression because we do not want to alter the original
		auto expression = scalar_function.expression->Copy();
		ExpressionBinder::QualifyColumnNames(*this, expression);

		// bind it to verify the function was defined correctly
		BoundSelectNode sel_node;
		BoundGroupInformation group_info;
		SelectBinder binder(*this, context, sel_node, group_info);
		bool should_create_dependencies = db_config.GetSetting<EnableMacroDependenciesSetting>(context);

		if (should_create_dependencies) {
			binder.SetCatalogLookupCallback([&dependencies, &catalog](CatalogEntry &entry) {
				if (&catalog != &entry.ParentCatalog()) {
					// Don't register any cross-catalog dependencies
					return;
				}
				// Register any catalog entry required to bind the macro function
				dependencies.AddDependency(entry);
			});
		}
		ErrorData error;
		try {
			error = binder.Bind(expression, 0, false);
			if (error.HasError()) {
				error.Throw();
			}
		} catch (const std::exception &ex) {
			error = ErrorData(ex);
		}
		// if we cannot resolve parameters we postpone binding until the macro function is used
		if (error.HasError() && error.Type() != ExceptionType::PARAMETER_NOT_RESOLVED) {
			error.Throw();
		}
	}

	return BindCreateSchema(info);
}

static bool IsValidUserType(optional_ptr<CatalogEntry> entry) {
	if (!entry) {
		return false;
	}
	return entry->Cast<TypeCatalogEntry>().user_type.id() != LogicalTypeId::INVALID;
}

void Binder::BindLogicalType(LogicalType &type, optional_ptr<Catalog> catalog, const string &schema) {
	if (type.id() != LogicalTypeId::USER) {
		// Recursive types, make sure to bind any nested user types recursively
		auto alias = type.GetAlias();
		auto ext_info = type.HasExtensionInfo() ? make_uniq<ExtensionTypeInfo>(*type.GetExtensionInfo()) : nullptr;

		switch (type.id()) {
		case LogicalTypeId::LIST: {
			auto child_type = ListType::GetChildType(type);
			BindLogicalType(child_type, catalog, schema);
			type = LogicalType::LIST(child_type);
		} break;
		case LogicalTypeId::MAP: {
			auto key_type = MapType::KeyType(type);
			BindLogicalType(key_type, catalog, schema);
			auto value_type = MapType::ValueType(type);
			BindLogicalType(value_type, catalog, schema);
			type = LogicalType::MAP(key_type, value_type);
		} break;
		case LogicalTypeId::ARRAY: {
			auto child_type = ArrayType::GetChildType(type);
			auto array_size = ArrayType::GetSize(type);
			BindLogicalType(child_type, catalog, schema);
			type = LogicalType::ARRAY(child_type, array_size);
		} break;
		case LogicalTypeId::STRUCT: {
			auto child_types = StructType::GetChildTypes(type);
			for (auto &child_type : child_types) {
				BindLogicalType(child_type.second, catalog, schema);
			}
			type = LogicalType::STRUCT(child_types);
		} break;
		case LogicalTypeId::UNION: {
			auto member_types = UnionType::CopyMemberTypes(type);
			for (auto &member_type : member_types) {
				BindLogicalType(member_type.second, catalog, schema);
			}
			type = LogicalType::UNION(member_types);
		} break;
		default:
			break;
		}

		// Set the alias and extension info back
		type.SetAlias(alias);
		type.SetExtensionInfo(std::move(ext_info));

		return;
	}

	// User type, bind the user type
	auto user_type_name = UserType::GetTypeName(type);
	auto user_type_schema = UserType::GetSchema(type);
	auto user_type_mods = UserType::GetTypeModifiers(type);

	bind_type_modifiers_function_t user_bind_modifiers_func = nullptr;

	if (catalog) {
		// The search order is:
		// 1) In the explicitly set schema (my_schema.my_type)
		// 2) In the same schema as the table
		// 3) In the same catalog
		// 4) System catalog

		optional_ptr<CatalogEntry> entry = nullptr;
		if (!user_type_schema.empty()) {
			entry = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, *catalog, user_type_schema, user_type_name,
			                                 OnEntryNotFound::RETURN_NULL);
		}
		if (!IsValidUserType(entry)) {
			entry = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, *catalog, schema, user_type_name,
			                                 OnEntryNotFound::RETURN_NULL);
		}
		if (!IsValidUserType(entry)) {
			entry = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, *catalog, INVALID_SCHEMA, user_type_name,
			                                 OnEntryNotFound::RETURN_NULL);
		}
		if (!IsValidUserType(entry)) {
			entry = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, INVALID_CATALOG, INVALID_SCHEMA, user_type_name,
			                                 OnEntryNotFound::THROW_EXCEPTION);
		}
		auto &type_entry = entry->Cast<TypeCatalogEntry>();
		type = type_entry.user_type;
		user_bind_modifiers_func = type_entry.bind_modifiers;
	} else {
		string type_catalog = UserType::GetCatalog(type);
		string type_schema = UserType::GetSchema(type);

		BindSchemaOrCatalog(context, type_catalog, type_schema);
		auto entry = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, type_catalog, type_schema, user_type_name);
		auto &type_entry = entry->Cast<TypeCatalogEntry>();
		type = type_entry.user_type;
		user_bind_modifiers_func = type_entry.bind_modifiers;
	}

	// Now we bind the inner user type
	BindLogicalType(type, catalog, schema);

	// Apply the type modifiers (if any)
	if (user_bind_modifiers_func) {
		// If an explicit bind_modifiers function was provided, use that to construct the type
		BindTypeModifiersInput input {context, type, user_type_mods};
		type = user_bind_modifiers_func(input);
	} else if (type.HasExtensionInfo()) {
		auto &ext_info = *type.GetExtensionInfo();

		// If the type already has modifiers, try to replace them with the user-provided ones if they are compatible
		// This enables registering custom types with "default" type modifiers that can be overridden, without
		// having to provide a custom bind_modifiers function
		auto type_mods_size = ext_info.modifiers.size();

		// Are we trying to pass more type modifiers than the type has?
		if (user_type_mods.size() > type_mods_size) {
			throw BinderException(
			    "Cannot apply '%d' type modifier(s) to type '%s' taking at most '%d' type modifier(s)",
			    user_type_mods.size(), user_type_name, type_mods_size);
		}

		// Deep copy the type so that we can replace the type modifiers
		type = type.DeepCopy();

		// Re-fetch the type modifiers now that we've deduplicated the ExtraTypeInfo
		auto &type_mods = type.GetExtensionInfo()->modifiers;

		// Replace them in order, casting if necessary
		for (idx_t i = 0; i < MinValue(type_mods.size(), user_type_mods.size()); i++) {
			auto &type_mod = type_mods[i];
			auto user_type_mod = user_type_mods[i];
			if (type_mod.type() == user_type_mod.type()) {
				type_mod = std::move(user_type_mod);
			} else if (user_type_mod.DefaultTryCastAs(type_mod.type())) {
				type_mod = std::move(user_type_mod);
			} else {
				throw BinderException("Cannot apply type modifier '%s' to type '%s', expected value of type '%s'",
				                      user_type_mod.ToString(), user_type_name, type_mod.type().ToString());
			}
		}
	} else if (!user_type_mods.empty()) {
		// We're trying to pass type modifiers to a type that doesnt have any
		throw BinderException("Type '%s' does not take any type modifiers", user_type_name);
	}
}

unique_ptr<LogicalOperator> DuckCatalog::BindCreateIndex(Binder &binder, CreateStatement &stmt,
                                                         TableCatalogEntry &table, unique_ptr<LogicalOperator> plan) {
	D_ASSERT(plan->type == LogicalOperatorType::LOGICAL_GET);
	auto create_index_info = unique_ptr_cast<CreateInfo, CreateIndexInfo>(std::move(stmt.info));
	IndexBinder index_binder(binder, binder.context);
	return index_binder.BindCreateIndex(binder.context, std::move(create_index_info), table, std::move(plan), nullptr);
}

BoundStatement Binder::Bind(CreateStatement &stmt) {
	BoundStatement result;
	result.names = {"Count"};
	result.types = {LogicalType::BIGINT};

	auto catalog_type = stmt.info->type;
	auto &properties = GetStatementProperties();
	switch (catalog_type) {
	case CatalogType::SCHEMA_ENTRY: {
		auto &base = stmt.info->Cast<CreateInfo>();
		auto catalog = BindCatalog(base.catalog);
		properties.RegisterDBModify(Catalog::GetCatalog(context, catalog), context);
		result.plan = make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_SCHEMA, std::move(stmt.info));
		break;
	}
	case CatalogType::VIEW_ENTRY: {
		auto &base = stmt.info->Cast<CreateViewInfo>();
		// bind the schema
		auto &schema = BindCreateSchema(*stmt.info);
		BindCreateViewInfo(base);
		result.plan = make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_VIEW, std::move(stmt.info), &schema);
		break;
	}
	case CatalogType::SEQUENCE_ENTRY: {
		auto &schema = BindCreateSchema(*stmt.info);
		result.plan =
		    make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_SEQUENCE, std::move(stmt.info), &schema);
		break;
	}
	case CatalogType::TABLE_MACRO_ENTRY: {
		auto &schema = BindCreateSchema(*stmt.info);
		result.plan =
		    make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_MACRO, std::move(stmt.info), &schema);
		break;
	}
	case CatalogType::MACRO_ENTRY: {
		auto &schema = BindCreateFunctionInfo(*stmt.info);
		auto logical_create =
		    make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_MACRO, std::move(stmt.info), &schema);
		result.plan = std::move(logical_create);
		break;
	}
	case CatalogType::INDEX_ENTRY: {
		auto &create_index_info = stmt.info->Cast<CreateIndexInfo>();

		// Plan the table scan.
		TableDescription table_description(create_index_info.catalog, create_index_info.schema,
		                                   create_index_info.table);
		auto table_ref = make_uniq<BaseTableRef>(table_description);
		auto bound_table = Bind(*table_ref);
		if (bound_table->type != TableReferenceType::BASE_TABLE) {
			throw BinderException("can only create an index on a base table");
		}

		auto &table_binding = bound_table->Cast<BoundBaseTableRef>();
		auto &table = table_binding.table;
		if (table.temporary) {
			stmt.info->temporary = true;
		}
		properties.RegisterDBModify(table.catalog, context);

		// create a plan over the bound table
		auto plan = CreatePlan(*bound_table);
		if (plan->type != LogicalOperatorType::LOGICAL_GET) {
			throw BinderException("Cannot create index on a view!");
		}

		result.plan = table.catalog.BindCreateIndex(*this, stmt, table, std::move(plan));
		break;
	}
	case CatalogType::TABLE_ENTRY: {
		auto bound_info = BindCreateTableInfo(std::move(stmt.info));
		auto root = std::move(bound_info->query);

		// create the logical operator
		auto &schema = bound_info->schema;
		auto create_table = make_uniq<LogicalCreateTable>(schema, std::move(bound_info));
		if (root) {
			// CREATE TABLE AS
			properties.return_type = StatementReturnType::CHANGED_ROWS;
			create_table->children.push_back(std::move(root));
		}
		result.plan = std::move(create_table);
		break;
	}
	case CatalogType::TYPE_ENTRY: {
		auto &schema = BindCreateSchema(*stmt.info);
		auto &create_type_info = stmt.info->Cast<CreateTypeInfo>();
		result.plan = make_uniq<LogicalCreate>(LogicalOperatorType::LOGICAL_CREATE_TYPE, std::move(stmt.info), &schema);

		auto &catalog = Catalog::GetCatalog(context, create_type_info.catalog);
		auto &dependencies = create_type_info.dependencies;
		auto dependency_callback = [&dependencies, &catalog](CatalogEntry &entry) {
			if (&catalog != &entry.ParentCatalog()) {
				// Don't register any cross-catalog dependencies
				return;
			}
			dependencies.AddDependency(entry);
		};
		if (create_type_info.query) {
			// CREATE TYPE mood AS ENUM (SELECT 'happy')
			auto query_obj = Bind(*create_type_info.query);
			auto query = std::move(query_obj.plan);
			create_type_info.query.reset();

			auto &sql_types = query_obj.types;
			if (sql_types.size() != 1) {
				// add cast expression?
				throw BinderException("The query must return a single column");
			}
			if (sql_types[0].id() != LogicalType::VARCHAR) {
				// push a projection casting to varchar
				vector<unique_ptr<Expression>> select_list;
				auto ref = make_uniq<BoundColumnRefExpression>(sql_types[0], query->GetColumnBindings()[0]);
				auto cast_expr = BoundCastExpression::AddCastToType(context, std::move(ref), LogicalType::VARCHAR);
				select_list.push_back(std::move(cast_expr));
				auto proj = make_uniq<LogicalProjection>(GenerateTableIndex(), std::move(select_list));
				proj->AddChild(std::move(query));
				query = std::move(proj);
			}

			result.plan->AddChild(std::move(query));
		} else if (create_type_info.type.id() == LogicalTypeId::USER) {
			SetCatalogLookupCallback(dependency_callback);
			// two cases:
			// 1: create a type with a non-existent type as source, Binder::BindLogicalType(...) will throw exception.
			// 2: create a type alias with a custom type.
			// eg. CREATE TYPE a AS INT; CREATE TYPE b AS a;
			// We set b to be an alias for the underlying type of a
			auto type_entry_p = entry_retriever.GetEntry(CatalogType::TYPE_ENTRY, schema.catalog.GetName(), schema.name,
			                                             UserType::GetTypeName(create_type_info.type));
			D_ASSERT(type_entry_p);
			auto &type_entry = type_entry_p->Cast<TypeCatalogEntry>();
			create_type_info.type = type_entry.user_type;
		} else {
			SetCatalogLookupCallback(dependency_callback);
			// This is done so that if the type contains a USER type,
			// we register this dependency
			auto preserved_type = create_type_info.type;
			BindLogicalType(create_type_info.type);
			create_type_info.type = preserved_type;
		}
		break;
	}
	case CatalogType::SECRET_ENTRY: {
		CatalogTransaction transaction = CatalogTransaction(Catalog::GetSystemCatalog(context), context);
		properties.return_type = StatementReturnType::QUERY_RESULT;
		return SecretManager::Get(context).BindCreateSecret(transaction, stmt.info->Cast<CreateSecretInfo>());
	}
	default:
		throw InternalException("Unrecognized type!");
	}
	properties.return_type = StatementReturnType::NOTHING;
	properties.allow_stream_result = false;
	return result;
}

} // namespace duckdb
