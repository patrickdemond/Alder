/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Database.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME Database - The applications connection to the database
//
// .SECTION Description
//

#ifndef __Database_h
#define __Database_h

#include "ModelObject.h"

#include "vtkSmartPointer.h"

#include <iostream>
#include <vector>

class vtkMySQLDatabase;

namespace Alder
{
  class User;
  class Database : public ModelObject
  {
  public:
    static Database *New();
    vtkTypeMacro( Database, ModelObject );

    bool Connect(
      std::string name,
      std::string user,
      std::string pass,
      std::string host,
      int port );

    bool HasAdministrator();
    void SetAdministratorPassword( std::string password );
    bool IsAdministratorPassword( std::string password );

    void AddUser( std::string name );
    void AddUser( User* user );
    void RemoveUser( std::string name );
    void RemoveUser( User* user );
    std::vector< vtkSmartPointer<User> > GetUsers();

  protected:
    Database();
    ~Database() {}

    vtkSmartPointer<vtkMySQLDatabase> MySQLDatabase;

  private:
    Database( const Database& ); // Not implemented
    void operator=( const Database& ); // Not implemented
  };
}

#endif
