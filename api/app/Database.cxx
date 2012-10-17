/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Database.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "Database.h"

#include "Configuration.h"
#include "User.h"
#include "Utilities.h"

#include "vtkMySQLDatabase.h"
#include "vtkMySQLToTableReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkMySQLQuery.h"
#include "vtkTable.h"
#include "vtkVariant.h"

#include <sstream>
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Database );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Database::Database()
  {
    this->MySQLDatabase = vtkSmartPointer<vtkMySQLDatabase>::New();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::Connect(
    std::string name,
    std::string user,
    std::string pass,
    std::string host,
    int port )
  {
    // set the database parameters using the configuration object
    this->MySQLDatabase->SetDatabaseName( name.c_str() );
    this->MySQLDatabase->SetUser( user.c_str() );
    this->MySQLDatabase->SetHostName( host.c_str() );
    this->MySQLDatabase->SetServerPort( port );
    bool success = this->MySQLDatabase->Open( pass.c_str() );
    this->ReadInformationSchema();

    return success;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Database::ReadInformationSchema()
  {
    vtkSmartPointer<vtkMySQLQuery> query = this->GetQuery();

    std::stringstream stream; 
    // the following query's first column MUST be table_name (index 0) and second column
    // MUST be table_column (index 1)
    stream << "SELECT table_name, column_name, column_type, data_type, column_default, is_nullable "
           << "FROM information_schema.columns "
           << "WHERE table_schema = " << query->EscapeString( this->MySQLDatabase->GetDatabaseName() ) << " "
           << "AND column_name != 'update_timestamp' "
           << "AND column_name != 'create_timestamp' "
           << "ORDER BY table_name, ordinal_position";
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    
    std::string tableName = "";
    std::map< std::string,std::map< std::string, std::string > > tableMap;
    while( query->NextRow() )
    {
      // if we are starting a new table save the old one and start over
      if( 0 != tableName.compare( query->DataValue( 0 ).ToString() ) )
      {
        if( 0 != tableName.length() ) this->Columns.insert(
          std::pair< std::string, std::map< std::string,std::map< std::string, std::string > > >(
            tableName, tableMap ) );
        tableName = query->DataValue( 0 ).ToString();
        tableMap.clear();
      }

      // get this column's details
      std::map< std::string, std::string > columnMap;
      for( int c = 2; c < query->GetNumberOfFields(); ++c )
        columnMap.insert( std::pair< std::string, std::string >(
          query->GetFieldName( c ), query->DataValue( c ).ToString() ) );

      // add the column to the current table
      std::string columnName = query->DataValue( 1 ).ToString();
      tableMap.insert( std::pair< std::string, std::map< std::string, std::string > >(
        columnName, columnMap ) );
    }

    // save the last table
    if( 0 != tableName.length() ) this->Columns.insert(
      std::pair< std::string, std::map< std::string,std::map< std::string, std::string > > >(
        tableName, tableMap ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> Database::GetColumnNames( std::string table )
  {
    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > >::iterator tablePair;
    tablePair = this->Columns.find( table );
    if( this->Columns.end() == tablePair )
    {
      std::stringstream error;
      error << "Tried to get column names for table \"" << table << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string,std::map< std::string, std::string > > tableMap = tablePair->second;
    std::vector<std::string> columns;
    std::map< std::string,std::map< std::string, std::string > >::iterator it;
    for( it = tableMap.begin(); it != tableMap.end(); ++it ) columns.push_back( it->first );

    return columns;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Database::GetColumnDefault( std::string table, std::string column )
  {
    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > >::iterator
      tablePair = this->Columns.find( table );
    if( this->Columns.end() == tablePair )
    {
      std::stringstream error;
      error << "Tried to get default column value from table \"" << table << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string,std::map< std::string, std::string > >::iterator
      columnPair = tablePair->second.find( column );
    if( tablePair->second.end() == columnPair )
    {
      std::stringstream error;
      error << "Tried to get default column value for \""
            << table << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string, std::string > columnMap = columnPair->second;
    return columnMap.find( "column_default" )->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::IsColumnNullable( std::string table, std::string column )
  {
    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > >::iterator
      tablePair = this->Columns.find( table );
    if( this->Columns.end() == tablePair )
    {
      std::stringstream error;
      error << "Tried to get column nullable from table \"" << table << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string,std::map< std::string, std::string > >::iterator
      columnPair = tablePair->second.find( column );
    if( tablePair->second.end() == columnPair )
    {
      std::stringstream error;
      error << "Tried to get column nullable for \""
            << table << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string, std::string > columnMap = columnPair->second;
    return 0 == columnMap.find( "is_nullable" )->second.compare( "YES" );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<vtkMySQLQuery> Database::GetQuery()
  {
    return vtkSmartPointer<vtkMySQLQuery>::Take(
      vtkMySQLQuery::SafeDownCast( this->MySQLDatabase->GetQueryInstance() ) );
  }

/*
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< vtkSmartPointer<User> > Database::GetUsers()
  {
    std::vector< vtkSmartPointer<User> > users;

    // get users from the database and populate a vector of User objects
    vtkSmartPointer<vtkMySQLToTableReader> reader = vtkSmartPointer<vtkMySQLToTableReader>::New();
    reader->SetDatabase( this->MySQLDatabase );
    reader->SetTableName( "user" );
    reader->Update();
    vtkTable *table = reader->GetOutput();
    for( vtkIdType row = 0; row < table->GetNumberOfRows(); ++row )
    {
      vtkSmartPointer<User> user = vtkSmartPointer<User>::New();
      user->name = table->GetValueByName( row, "name" ).ToString();
      user->lastLogin = table->GetValueByName( row, "last_login" ).ToString();
      user->createdOn = table->GetValueByName( row, "create_timestamp" ).ToString();
      users.push_back( user );
    }

    return users;
  }
*/
}
