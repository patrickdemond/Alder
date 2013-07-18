/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
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
      vtkVariant columnDefault = db->GetColumnDefault( this->GetName(), column );
      this->ColumnValues.insert( std::pair< std::string, vtkVariant >( column, columnDefault ) );
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
    
    Utilities::log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

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
        if( 0 != column.compare( "CreateTimestamp" ) && 0 != column.compare( "UpdateTimestamp" ) )
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
      if( 0 != it->first.compare( "Id" ) )
      {
        stream << ( first ? "" :  ", " ) << it->first
               << " = " << ( it->second.IsValid() ? query->EscapeString( it->second.ToString() ) : "NULL" );
        if( first ) first = false;
      }
    }

    // different sql based on whether the record already exists or not
    if( !this->Get( "Id" ).IsValid() || 0 == this->Get( "Id" ).ToInt() )
    {
      // add the CreateTimestamp column
      stream << ( first ? "" :  ", " ) << "CreateTimestamp = NULL";

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
             << " WHERE Id = " << query->EscapeString( this->Get( "Id" ).ToString() );
    }

    Utilities::log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    // if the record's Id isn't set, get it based on the auto-increment property of the Id column
    if( !this->Get( "Id" ).IsValid() || 0 == this->Get( "Id" ).ToInt() )
    {
      this->Set( "Id", this->GetLastInsertId() );
    }
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetLastInsertId()
  {
      Application *app = Application::GetInstance();
      std::stringstream stream;
      stream << "SELECT Max( Id ) FROM " << this->GetName();
      vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
          
      Utilities::log( "Getting last insert id for table: " + this->GetName() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      if( query->HasError() )
      {
        Utilities::log( query->GetLastErrorText() );
        throw std::runtime_error( "There was an error while trying to query the database." );
      }

      // only has one row
      query->NextRow();
      return query->DataValue( 0 ).ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Remove()
  {
    this->AssertPrimaryId();

    vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    std::stringstream stream;
    stream << "DELETE FROM " << this->GetName() << " "
           << "WHERE Id = " << query->EscapeString( this->Get( "Id" ).ToString() );
    Utilities::log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetCount( std::string recordType )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM " << recordType << " "
           << "WHERE " << this->GetName() << "Id = " << this->Get( "Id" ).ToString();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

    Utilities::log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
    
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

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::PrintSelf( ostream& os, vtkIndent indent )
  {
    this->Superclass::PrintSelf( os, indent );

    os << indent << "Initialized: " << ( this->Initialized ? "Yes" : "No" ) << endl;
    os << indent << "Column Values:" << endl;
    std::map< std::string, vtkVariant >::iterator it;
    for( it = this->ColumnValues.begin(); it != this->ColumnValues.end(); ++it )
    {
      os << indent.GetNextIndent() << it->first << ": " << it->second
         << ( it->second.IsValid() ? it->second.ToString() : "NULL" ) << endl;
    }
  }
}
