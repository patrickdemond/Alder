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
#include "vtkSQLQuery.h"
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
    return this->MySQLDatabase->Open( pass.c_str() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::HasAdministrator()
  {
    vtkSmartPointer<vtkSQLQuery> query =
      vtkSmartPointer<vtkSQLQuery>::Take( this->MySQLDatabase->GetQueryInstance() );
    query->SetQuery( "SELECT COUNT(*) AS total FROM user WHERE name = 'administrator'" );
    query->Execute();
    query->NextRow();
    vtkVariant v = query->DataValue( 0 );
    return 1 == v.ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Database::SetAdministratorPassword( std::string password )
  {
    vtkSmartPointer<vtkSQLQuery> query =
      vtkSmartPointer<vtkSQLQuery>::Take( this->MySQLDatabase->GetQueryInstance() );
    std::string hashedPassword;
    Alder::hashString( password, hashedPassword );
    std::string sql = "REPLACE INTO user SET created_timestamp = NULL, name = 'Administrator', password = '";
    sql += hashedPassword;
    sql += "'";
    query->SetQuery( sql.c_str() );
    query->Execute();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::IsAdministratorPassword( std::string password )
  {
    // first hash the password argument
    std::string hashedPassword;
    Alder::hashString( password, hashedPassword );

    vtkSmartPointer<vtkSQLQuery> query =
      vtkSmartPointer<vtkSQLQuery>::Take( this->MySQLDatabase->GetQueryInstance() );
    query->SetQuery( "SELECT password FROM user WHERE name = 'Administrator'" );
    query->Execute();
    query->NextRow(); // we can only have one row
    std::string currentHashedPassword = query->DataValue( 0 ).ToString();
    
    return hashedPassword == currentHashedPassword;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< vtkSmartPointer<Alder::User> > Database::GetUsers()
  {
    std::vector< vtkSmartPointer<Alder::User> > users;

    // get users from the database and populate a vector of User objects
    vtkSmartPointer<vtkMySQLToTableReader> reader = vtkSmartPointer<vtkMySQLToTableReader>::New();
    reader->SetDatabase( this->MySQLDatabase );
    reader->SetTableName( "user" );
    reader->Update();
    vtkTable *table = reader->GetOutput();
    for( vtkIdType row = 0; row < table->GetNumberOfRows(); ++row )
    {
      vtkSmartPointer<Alder::User> user = vtkSmartPointer<Alder::User>::New();
      user->name = table->GetValueByName( row, "name" ).ToString();
      user->lastLogin = table->GetValueByName( row, "last_login" ).ToString();
      user->createdOn = table->GetValueByName( row, "created_timestamp" ).ToString();
      users.push_back( user );
    }

    return users;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Database::AddUser( std::string name )
  {
    vtkSmartPointer<Alder::User> user = vtkSmartPointer<Alder::User>::New();
    user->name = name;
    user->SetPassword( "password" ); // new users always have the password "password"
    this->AddUser( user );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Database::AddUser( User *user )
  {
    vtkSmartPointer<vtkSQLQuery> query =
      vtkSmartPointer<vtkSQLQuery>::Take( this->MySQLDatabase->GetQueryInstance() );
    
    // make sure user name isn't blank
    if( 0 == user->name.length() ) throw std::runtime_error( "Trying to create user without username" );

    // make sure password isn't blank
    if( 0 == user->hashedPassword.length() ) throw std::runtime_error( "Trying to create user without password" );

    std::stringstream stream;
    stream << "INSERT INTO user ( created_timestamp, name, password ) VALUES ( NULL,'"
           << user->name << "','" << user->hashedPassword << "' )";
    query->SetQuery( stream.str().c_str() );
    query->Execute();
  }
}
