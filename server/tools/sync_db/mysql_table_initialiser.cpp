/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "mysql_table_initialiser.hpp"

#include "dbmgr_mysql/helper_types.hpp"


/**
 *	This class is used to help with performing an ALTER TABLE command on an
 *	existing table in the database.
 *
 *	It collects all the column additions, drops and modification requests
 *	and prints debugging information prior to execution. The collection of all
 *	operations into a single SQL command allows for a single fast DB operation
 *	rather than many slow operations.
 */
class AlterTableHelper
{
public:
	AlterTableHelper( const std::string tableName );

	void addColumn( const std::string & columnName,
					const std::string & columnType );

	void dropColumn( const std::string & columnName );

	void modifyColumn( const std::string & columnName,
						const std::string & columnType );

	uint execute( MySql & connection, bool isVerbose = false );

	uint numAlteredItems() const
		{ return numAdditions_ + numDrops_ + numModifications_; }

private:
	std::string tableName_;

	uint numAdditions_;
	uint numDrops_;
	uint numModifications_;

	std::stringstream stringStream_;
};


/**
 *	Constructor.
 *
 *	@param tableName  The name of the table the ALTER TABLE command will
 *	                  operate on.
 */
AlterTableHelper::AlterTableHelper( const std::string tableName ) :
	tableName_( tableName ),
	numAdditions_( 0 ),
	numDrops_( 0 ),
	numModifications_( 0 )
{
	stringStream_ << "ALTER TABLE " << tableName;
}


/**
 *	This method appends an ADD COLUMN request to the ALTER TABLE command.
 *
 *	@param columnName  The name of the column to add.
 *	@param columnType  The DB column type of the column to add (eg, INT)
 */
void AlterTableHelper::addColumn( const std::string & columnName,
	const std::string & columnType )
{
	if (this->numAlteredItems())
	{
		stringStream_ << ",";
	}

	stringStream_ << " ADD COLUMN " << columnName << " " << columnType;

	++numAdditions_;
}


/**
 *	This method appends a DROP COLUMN request to the ALTER TABLE command.
 *
 *	@param columnName  The name of the column to drop.
 */
void AlterTableHelper::dropColumn( const std::string & columnName )
{
	if (this->numAlteredItems())
	{
		stringStream_ << ",";
	}

	stringStream_ << " DROP COLUMN " << columnName;

	++numDrops_;
}


/**
 *	This method appends a MODIFY COLUMN request to the ALTER TABLE command.
 *
 *	@param columnName  The name of the column to modify.
 *	@param columnType  The DB column type the column will be changed to
 *	                   (eg, INT).
 */
void AlterTableHelper::modifyColumn( const std::string & columnName,
	const std::string & columnType )
{
	if (this->numAlteredItems())
	{
		stringStream_ << ",";
	}

	stringStream_ << " MODIFY COLUMN " << columnName << " " << columnType;

	++numModifications_;
}


/**
 *	This method performs the ALTER TABLE on the provided MySQL connection.
 *
 *	@param connection  The MySql connection to perform the ALTER TABLE on.
 *	@param isVerbose   Flag indicating whether to log more information or not.
 *
 *	@returns The number of columns that were added, deleted and modified.
 */
uint AlterTableHelper::execute( MySql & connection, bool isVerbose )
{
	if (this->numAlteredItems())
	{
		if (isVerbose)
		{
			INFO_MSG( "\tAdding %u columns into table %s\n",
						numAdditions_, tableName_.c_str() );
			INFO_MSG( "\tDeleting %u columns from table %s\n",
						numDrops_, tableName_.c_str() );
			INFO_MSG( "\tUpdating %u columns from table %s\n",
						numModifications_, tableName_.c_str() );

			DEBUG_MSG( "SQL: %s\n", stringStream_.str().c_str() );
		}
		connection.execute( stringStream_.str() );
	}

	return this->numAlteredItems();
}



/**
 *	Constructor.
 *
 *	@param con       The MySql connection to use for initialising.
 *	@param allowNew  Should we allow new column and tables be created.
 */
TableInitialiser::TableInitialiser( MySql & con, bool allowNew,
		const std::string & characterSet, const std::string & collation ) :
	TableInspector( con ),
	allowNew_( allowNew ),
	characterSet_( characterSet ),
	collation_( collation )
{
}


/**
 *	This method creates an index on the given column in the given table
 * 	according to colInfo.indexType. Most of the time this would be
 * 	IndexTypeNone.
 */
void TableInitialiser::createIndex( const std::string & tableName,
	const std::string & colName,
	const TableMetaData::ColumnInfo & colInfo )
{
	switch (colInfo.indexType)
	{
	case INDEX_TYPE_NONE:
		break;

	case INDEX_TYPE_PRIMARY:
		// A bit dodgy, but this is created when we create the table
		// and it cannot be added or deleted afterwards.
		break;

	case INDEX_TYPE_NAME:
	{
		// __kyl__ (24/7/2006) Super dodgy way of working out the
		// size of the index. If it is a VARCHAR field then we use
		// the size of the field. If it is any other type of field,
		// then we make the index size 255.
		const char * indexLengthConstraint = "";
		if (colInfo.columnType.fieldType != MYSQL_TYPE_VAR_STRING)
		{	// Not VARCHAR field. Set index length to 255.
			indexLengthConstraint = "(255)";
		}
		std::string indexName = generateIndexName( colName );

		try
		{
			connection_.execute( "CREATE UNIQUE INDEX " + indexName +
				" ON " + tableName + " (" + colName +
				indexLengthConstraint + ")" );
		}
		catch (...)
		{
			ERROR_MSG( "TableInitialiser::createIndex: Failed to "
					"create name index on column '%s.%s'. Please "
					"ensure all that values in the column are unique "
					"before attempting to create a name index.\n",
					tableName.c_str(), colName.c_str() );
			throw;
		}
	}
	break;

	case INDEX_TYPE_PARENT_ID:
		connection_.execute( "CREATE INDEX "PARENTID_INDEX_NAME" ON " +
				tableName + " (" + colName + ")" );
		break;

	default:
		CRITICAL_MSG( "createEntityTableIndex: Unknown index type %d\n",
				colInfo.indexType );
		break;
	}
}


/**
 *	This method deletes an index in the given table according to indexType.
 *	This is the evil twin of createEntityTableIndex().
 *
 * 	@param tableName  The name of the table to remove the index from.
 * 	@param colName    The name of the column the index is associated with.
 * 	@param indexType  The type of index to remove.
 */
void TableInitialiser::removeIndex( const std::string & tableName,
	const std::string & colName,
	ColumnIndexType indexType )
{
	try
	{
		switch (indexType)
		{
			case INDEX_TYPE_NONE:
				break;
			case INDEX_TYPE_PRIMARY:
				// Can't delete primary index.
				break;
			case INDEX_TYPE_NAME:
				{
					std::string indexName = generateIndexName( colName );

					connection_.execute( "ALTER TABLE " + tableName +
							" DROP INDEX " + indexName );
				}
				break;
			case INDEX_TYPE_PARENT_ID:
				connection_.execute( "ALTER TABLE " + tableName +
						" DROP INDEX "PARENTID_INDEX_NAME );
				break;
			default:
				CRITICAL_MSG( "TableInitialiser::removeIndex: "
						"Unknown index type %d\n", indexType );
				break;
		}
	}
	catch (std::exception& e)
	{
		// Shouldn't really happen, but it's not fatal so we shouldn't die.
		ERROR_MSG( "TableInitialiser::removeIndex: %s\n", e.what() );
	}
}


/*
 *	Override from TableInspector.
 */
bool TableInitialiser::onNeedNewTable( const std::string & tableName,
		const TableMetaData::NameToColInfoMap & columns )
{
	if (!allowNew_)
	{
		NOTICE_MSG( "\tWitholding table creation: %s\n", tableName.c_str() );
		return true;
	}

	INFO_MSG("\tCreating table %s\n", tableName.c_str());
	connection_.execute( "CREATE TABLE IF NOT EXISTS " + tableName +
			" (id BIGINT AUTO_INCREMENT, PRIMARY KEY idKey (id)) "
			"ENGINE="MYSQL_ENGINE_TYPE );

	// __kyl__ (28/7/2005) We can't create a table with no columns so
	// we create one with the id column even though it may not be
	// needed. We'll delete the id column later on.
	bool deleteIDCol = false;
	TableMetaData::NameToColInfoMap newColumns( columns );
	TableMetaData::NameToColInfoMap::iterator idIter =
			newColumns.find( ID_COLUMN_NAME );
	if (idIter != newColumns.end())
	{
		newColumns.erase( idIter );
	}
	else
	{
		deleteIDCol = true;
	}

	AlterTableHelper alterTableHelper( tableName );

	this->addColumns( tableName, newColumns, alterTableHelper, false );

	// delete unnecessary ID column that we created table with.
	if (deleteIDCol)
	{
		alterTableHelper.dropColumn( ID_COLUMN_NAME );
	}

	if (alterTableHelper.execute( connection_ ))
	{
		this->initialiseColumns( tableName, newColumns, true );
	}

	return true;
}


/*
 *	Override from TableInspector.
 */
bool TableInitialiser::onExistingTable( const std::string & tableName )
{
	if (characterSet_.empty())
	{
		// If not set, leave as-is.
		return true;
	}

	std::stringstream ss;
	ss << "ALTER TABLE " << tableName <<
			" CONVERT TO CHARACTER SET " <<
			MySqlEscapedString( connection_, characterSet_ );

	if (!collation_.empty())
	{
		ss << " COLLATE " << MySqlEscapedString( connection_, collation_ );
	}

	connection_.execute( ss.str() );

	return true;
}


/*
 *	Override from TableInspector.
 */
bool TableInitialiser::onNeedUpdateTable( const std::string & tableName,
		const TableMetaData::NameToColInfoMap & obsoleteColumns,
		const TableMetaData::NameToColInfoMap & newColumns,
		const TableMetaData::NameToUpdatedColInfoMap & updatedColumns )
{
	AlterTableHelper alterTableHelper( tableName );

	if (allowNew_)
	{
		this->addColumns( tableName, newColumns, alterTableHelper, true );
	}
	this->dropColumns( tableName, obsoleteColumns, alterTableHelper, true );
	this->updateColumns( tableName, updatedColumns, alterTableHelper, true );

	if (alterTableHelper.execute( connection_, /*isVerbose*/ true ))
	{
		// Re-create any indexes that may be needed
		// New column indexes
		if (allowNew_)
		{
			this->initialiseColumns( tableName, newColumns, true );
		}

		// Updated column indexes
		this->initialiseColumns( tableName, updatedColumns, false );
	}
	else
	{
		INFO_MSG( "No entity definition modifications required.\n" );
	}

	return true;
}


/**
 *	This method initialises all the columns in the provided list.
 *
 *	@param tableName  The name of the table to initialise.
 *	@param columns    The column descriptions to initialise the table with.
 *	@param shouldApplyDefaultValue  Flag to indicate whether the columns should
 *	                                be initialised with the default type value.
 */
template < class COLUMN_MAP >
void TableInitialiser::initialiseColumns( const std::string & tableName,
	COLUMN_MAP & columns, bool shouldApplyDefaultValue )
{
	class COLUMN_MAP::const_iterator iter = columns.begin();
	while (iter != columns.end())
	{
		this->initialiseColumn( tableName, iter->first, iter->second,
									shouldApplyDefaultValue );
		++iter;
	}
}


/**
 *	This method creates any required indexes and sets the default column value
 *	for the column if MySQL doesn't support DEFAULT for that type (eg: BLOB).
 *
 *	@see initialiseColumns.
 *
 *	@param tableName   The name of the table the column to initialise is in.
 *	@param columnName  The name of the column to initialise.
 *	@param columnInfo  The full column description.
 *	@param shouldApplyDefaultValue  Flag to indicate whether the columns should
 *	                                be initialised with the default type value.
 */
void TableInitialiser::initialiseColumn( const std::string & tableName,
	const std::string & columnName,
	const TableMetaData::ColumnInfo & columnInfo,
	bool shouldApplyDefaultValue )
{
	this->createIndex( tableName, columnName, columnInfo );

	// For any columns that were unable to have a default value applied
	// to the column description, update the columns now with the
	// default data.
	if (shouldApplyDefaultValue &&
		!columnInfo.columnType.isDefaultValueSupported())
	{
		std::stringstream ss;
		ss << "UPDATE " << tableName << " SET " << columnName
			<< "='" << MySqlEscapedString( connection_,
						columnInfo.columnType.defaultValue )
			<< '\'';
		connection_.execute( ss.str() );
	}
}


/*
 *	Override from TableInspector.
 */
bool TableInitialiser::onNeedDeleteTables( const StrSet & tableNames )
{
	StrSet::const_iterator iter = tableNames.begin();
	while (iter != tableNames.end())
	{
		INFO_MSG( "\tDeleting table %s\n", iter->c_str() );
		connection_.execute( "DROP TABLE " + *iter );
		++iter;
	}

	return true;
}


/**
 *	This method adds columns to an existing table.
 *
 *	@param tableName  The name of the table to add the columns into.
 *	@param columns    The set of columns to add to the table.
 *	@param helper     The helper to use for optimising the column additions.
 *	@param shouldPrintInfo  Flag indicating how verbose to be while working.
 */
void TableInitialiser::addColumns( const std::string & tableName,
		const TableMetaData::NameToColInfoMap & columns,
		AlterTableHelper & helper, bool shouldPrintInfo )
{
	TableMetaData::NameToColInfoMap::const_iterator iter = columns.begin();
	while (iter != columns.end())
	{
		if (shouldPrintInfo)
		{
			TRACE_MSG( "\tPreparing to update table %s, adding column %s\n",
						tableName.c_str(), iter->first.c_str() );
		}

		helper.addColumn( iter->first,
					iter->second.columnType.getAsString(
						connection_, iter->second.indexType ) );

		++iter;
	}

	return;
}


/**
 *	This method removes columns from an existing table.
 *
 *	@param tableName  The name of the table to remove the columns from.
 *	@param columns    The set of columns to remove from the table.
 *	@param helper  The helper to use for optimising the column removal.
 *	@param shouldPrintInfo  Flag indicating how verbose to be while working.
 */
void TableInitialiser::dropColumns( const std::string & tableName,
		const TableMetaData::NameToColInfoMap & columns,
		AlterTableHelper & helper, bool shouldPrintInfo )
{

	// Prepare the columns to be dropped
	TableMetaData::NameToColInfoMap::const_iterator iter = columns.begin();
	while (iter != columns.end())
	{
		if (shouldPrintInfo)
		{
			TRACE_MSG( "\tPreparing to update table %s, deleting column %s\n",
					tableName.c_str(), iter->first.c_str() );
		}

		this->removeIndex( tableName, iter->first, iter->second.indexType );

		helper.dropColumn( iter->first );

		++iter;
	}
}


/**
 *	This method modifies existing columns in a table if the associated entity
 *	definition has changed.
 *
 *	@param tableName  The name of the table to modify columns in.
 *	@param columns    The set of columns to modify.
 *	@param helper  The helper to use for optimising the column modification.
 *	@param shouldPrintInfo  Flag indicating how verbose to be while working.
 */
void TableInitialiser::updateColumns( const std::string & tableName,
		const TableMetaData::NameToUpdatedColInfoMap & columns,
		AlterTableHelper & helper, bool shouldPrintInfo )
{
	// Update changed columns
	TableMetaData::NameToUpdatedColInfoMap::const_iterator iter =
															columns.begin();
	while (iter != columns.end())
	{
		std::string columnTypeStr =
				iter->second.columnType.getAsString( connection_,
						iter->second.indexType );

		if (shouldPrintInfo)
		{
			TRACE_MSG( "\tPreparing to update table %s, modifying type of "
						"column %s to %s (%sindexed)\n", tableName.c_str(),
				iter->first.c_str(), columnTypeStr.c_str(),
				(iter->second.indexType == INDEX_TYPE_NONE) ?
					"non-": "" );
		}

		this->removeIndex( tableName, iter->first, iter->second.oldIndexType );

		helper.modifyColumn( iter->first, columnTypeStr );

		++iter;
	}
}

// mysql_table_initialiser.cpp
