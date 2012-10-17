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

#include "vtkMySQLQuery.h"

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
  ActiveRecord::~ActiveRecord()
  {
    this->DeleteColumnValues();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::DeleteColumnValues()
  {
    std::map<std::string,vtkVariant*>::iterator it;
    for( it = this->ColumnValues.begin(); it != this->ColumnValues.end(); ++it )
    {
      if( it->second )
      {
        delete it->second;
        it->second = NULL;
      }
    }
    this->ColumnValues.clear();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Initialize()
  {
    this->DeleteColumnValues();

    Database *db = Application::GetInstance()->GetDB();
    std::vector<std::string>::iterator it;
    std::vector<std::string> columns = db->GetColumnNames( this->GetName() );

    // When first creating an active record we want the ColumnValues ivar to have an empty
    // value for every column in the active record's table.  We use mysql's information_schema
    // database for this purpose.  This is all implemented by the Database model
    for( it = columns.begin(); it != columns.end(); ++it )
    {
      std::string columnName = *it;
      std::string columnDefault = db->GetColumnDefault( this->GetName(), columnName );
      vtkVariant *v = db->IsColumnNullable( this->GetName(), columnName ) &&
                      0 == columnDefault.length()
                    ? NULL
                    : new vtkVariant( columnDefault );
      this->ColumnValues.insert( std::pair< std::string, vtkVariant* >( columnName, v ) );
    }

    this->Initialized = true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecord::Load( std::map< std::string, std::string > map )
  {
    this->DeleteColumnValues();

    Database *db = Application::GetInstance()->GetDB();
    vtkSmartPointer<vtkMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
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
        std::string columnName = query->GetFieldName( c );
        if( 0 != columnName.compare( "create_timestamp" ) && 0 != columnName.compare( "update_timestamp" ) )
        {
          vtkVariant *v = db->IsColumnNullable( this->GetName(), columnName ) &&
                          0 == query->DataValue( c ).ToString().length()
                        ? NULL
                        : new vtkVariant( query->DataValue( c ) );
          this->ColumnValues.insert( std::pair< std::string, vtkVariant* >( columnName, v ) );
        }
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
    vtkSmartPointer<vtkMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::map< std::string, vtkVariant* >::iterator it;
    std::stringstream stream;

    bool first = true;
    for( it = this->ColumnValues.begin(); it != this->ColumnValues.end(); ++it )
    {
      if( 0 != it->first.compare( "id" ) )
      {
        stream << ( first ? "" :  ", " ) << it->first
               << " = " << ( it->second ? query->EscapeString( it->second->ToString() ) : "NULL" );
        if( first ) first = false;
      }
    }

    // different sql based on whether the record already exists or not
    if( 0 == this->Get( "id" )->ToString().length() )
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
             << " WHERE id = " << query->EscapeString( this->Get( "id" )->ToString() );
    }

    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Remove()
  {
    // make sure the record has an id
    vtkVariant *id = this->Get( "id" );
    if( !id || 0 == id->ToString().length() )
      throw std::runtime_error( "Tried to remove record with no id" );

    vtkSmartPointer<vtkMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::stringstream stream;
    stream << "DELETE FROM " << this->GetName() << " "
           << "WHERE id = " << query->EscapeString( this->Get( "id" )->ToString() );
    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< vtkSmartPointer< ActiveRecord > > ActiveRecord::GetAll()
  {
    std::vector< vtkSmartPointer< ActiveRecord > > list;
    std::stringstream stream;
    stream << "SELECT id FROM " << this->GetName();
    vtkSmartPointer<vtkMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    vtkDebugSQLMacro( << stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    while( query->NextRow() )
    {
      // TODO: create a new instance of the child class
      vtkSmartPointer< ActiveRecord > record = vtkSmartPointer< ActiveRecord >::Take(
        ActiveRecord::SafeDownCast( Application::GetInstance()->Create( this->GetName() ) ) );
      record->Load( "id", query->DataValue( 0 ).ToString() );
      list.push_back( record );
    }

    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkVariant* ActiveRecord::Get( std::string column )
  {
    // make sure the record is initialized
    if( !this->Initialized ) this->Initialize();

    std::map< std::string, vtkVariant* >::iterator pair = this->ColumnValues.find( column );
    if( this->ColumnValues.end() == pair )
    {
      std::stringstream error;
      error << "Tried to get column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    return pair->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::SetVariant( std::string column, vtkVariant *value )
  {
    // make sure the record is initialized
    if( !this->Initialized ) this->Initialize();

    std::map< std::string, vtkVariant* >::iterator pair = this->ColumnValues.find( column );
    if( this->ColumnValues.end() == pair )
    {
      std::stringstream error;
      error << "Tried to set column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    // if it exists, delete the old value
    if( pair->second ) delete pair->second;
    pair->second = value;
  }
}
