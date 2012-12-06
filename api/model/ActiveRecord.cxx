/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   ActiveRecord.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "ActiveRecord.h"

#include "Application.h"
#include "Database.h"

#include "vtkAlderMySQLQuery.h"

#include <sstream>
#include <stdexcept>
#include <vector>

namespace Alder
{
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ActiveRecord::ActiveRecord()
  {
    this->DebugSQL = false;
    this->Initialized = false;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecord::ColumnNameExists( std::string column )
  {
    // make sure the record is initialized
    if( !this->Initialized ) this->Initialize();

    std::map< std::string, vtkVariant >::iterator pair = this->ColumnValues.find( column );
    return this->ColumnValues.end() != pair;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Initialize()
  {
    this->ColumnValues.clear();

    Database *db = Application::GetInstance()->GetDB();
    std::vector<std::string>::iterator it;
    std::vector<std::string> columns = db->GetColumnNames( this->GetName() );

    // When first creating an active record we want the ColumnValues ivar to have an empty
    // value for every column in the active record's table.  We use mysql's information_schema
    // database for this purpose.  This is all implemented by the Database model
    for( it = columns.begin(); it != columns.end(); ++it )
    {
      std::string column = *it;
      std::string columnDefault = db->GetColumnDefault( this->GetName(), column );
      this->ColumnValues.insert( std::pair< std::string, vtkVariant >( column, vtkVariant( columnDefault ) ) );
    }

    this->Initialized = true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecord::Load( std::map< std::string, std::string > map )
  {
    this->ColumnValues.clear();

    Database *db = Application::GetInstance()->GetDB();
    vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::map< std::string, std::string >::iterator it;

    // create an sql statement using the provided map
    std::stringstream stream;
    stream << "SELECT * FROM " << this->GetName();
    for( it = map.begin(); it != map.end(); ++it )
      stream << ( map.begin() == it ? " WHERE " : " AND " )
             << it->first << " = " << query->EscapeString( it->second );
    
    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    bool first = true;
    while( query->NextRow() )
    {
      // make sure we only have one record, throw exception if not
      if( !first )
      {
        std::stringstream error;
        error << "Loading " << this->GetName() << " record resulted in multiple rows";
        throw std::runtime_error( error.str() );
      }

      for( int c = 0; c < query->GetNumberOfFields(); ++c )
      {
        std::string column = query->GetFieldName( c );
        if( 0 != column.compare( "create_timestamp" ) && 0 != column.compare( "update_timestamp" ) )
          this->ColumnValues.insert( std::pair< std::string, vtkVariant >( column, query->DataValue( c ) ) );
      }

      if( first ) first = false;
    }

    // if we didn't find a row then first is still true
    this->Initialized = !first;
    return !first;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Save()
  {
    vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::map< std::string, vtkVariant >::iterator it;
    std::stringstream stream;

    bool first = true;
    for( it = this->ColumnValues.begin(); it != this->ColumnValues.end(); ++it )
    {
      if( 0 != it->first.compare( "id" ) )
      {
        stream << ( first ? "" :  ", " ) << it->first
               << " = " << ( it->second.IsValid() ? query->EscapeString( it->second.ToString() ) : "NULL" );
        if( first ) first = false;
      }
    }

    // different sql based on whether the record already exists or not
    if( !this->Get( "id" ).IsValid() )
    {
      // add the create_timestamp column
      stream << ( first ? "" :  ", " ) << "create_timestamp = NULL";

      // add a new record
      std::string s = stream.str();
      stream.str( "" );
      stream << "INSERT INTO " << this->GetName() << " SET " << s;
    }
    else
    {
      // update the existing record
      std::string s = stream.str();
      stream.str( "" );
      stream << "UPDATE " << this->GetName() << " SET " << s
             << " WHERE id = " << query->EscapeString( this->Get( "id" ).ToString() );
    }

    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Remove()
  {
    this->AssertPrimaryId();

    vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::stringstream stream;
    stream << "DELETE FROM " << this->GetName() << " "
           << "WHERE id = " << query->EscapeString( this->Get( "id" ).ToString() );
    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetCount( std::string recordType )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM " << recordType << " "
           << "WHERE " << this->GetName() << "_id = " << this->Get( "id" ).ToString();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    
    // only has one row
    query->NextRow();
    return query->DataValue( 0 ).ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkVariant ActiveRecord::Get( std::string column )
  {
    // make sure the column exists
    if( !this->ColumnNameExists( column ) )
    {
      std::stringstream error;
      error << "Tried to get column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string, vtkVariant >::iterator pair = this->ColumnValues.find( column );
    return pair->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ActiveRecord* ActiveRecord::GetRecord( std::string table, std::string column )
  {
    // if no column name was provided, use the default (table name followed by _id)
    if( column.empty() )
    {
      column = toLower( table );
      column += "_id";
    }

    // test to see if correct foreign key exists
    if( !this->ColumnNameExists( column ) )
    {
      std::stringstream error;
      error << "Tried to get \"" << table << "\" record but column \"" << column << "\" doesn't exist";
      throw std::runtime_error( error.str() );
    }

    ActiveRecord *record = NULL;
    vtkVariant v = this->Get( column );
    if( v.IsValid() )
    { // only create the record if the foreign key is not null
      record = ActiveRecord::SafeDownCast( Application::GetInstance()->Create( table ) );
      record->Load( "id", this->Get( column ).ToString() );
    }

    return record;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::SetVariant( std::string column, vtkVariant value )
  {
    // make sure the column exists
    if( !this->ColumnNameExists( column ) )
    {
      std::stringstream error;
      error << "Tried to set column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    std::map< std::string, vtkVariant >::iterator pair = this->ColumnValues.find( column );
    pair->second = value;
  }
}
