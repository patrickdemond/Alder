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
#include "vtkMySQLQuery.h"

#include <iostream>
#include <map>
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

    vtkSmartPointer<vtkMySQLQuery> GetQuery();

    std::vector<std::string> GetColumnNames( std::string table );
    std::string GetColumnDefault( std::string table, std::string column );
    bool IsColumnNullable( std::string table, std::string column );

  protected:
    Database();
    ~Database() {}

    void ReadInformationSchema();
    vtkSmartPointer<vtkMySQLDatabase> MySQLDatabase;
    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > > Columns;

  private:
    Database( const Database& ); // Not implemented
    void operator=( const Database& ); // Not implemented
  };
}

#endif
