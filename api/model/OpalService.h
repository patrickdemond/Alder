/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   OpalService.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME OpalService - The applications connection to the database
//
// .SECTION Description
//

#ifndef __OpalService_h
#define __OpalService_h

#include "ModelObject.h"

#include "vtkSmartPointer.h"
#include "vtkMySQLQuery.h"

#include <iostream>
#include <json/reader.h>
#include <map>
#include <vector>

class vtkMySQLOpalService;

namespace Alder
{
  class User;
  class OpalService : public ModelObject
  {
  public:
    static OpalService *New();
    vtkTypeMacro( OpalService, ModelObject );
    void Setup( std::string username, std::string password, std::string host, int port );
    std::vector< std::string > GetIdentifiers( std::string dataSource, std::string table );
    std::map< std::string, std::string > GetValueList(
      std::string dataSource, std::string table, std::string variable, int offset = 0, int limit = 100 );
    void GetValue( std::string dataSource, std::string table, std::string variable );

  protected:
    OpalService();
    ~OpalService() {}

    virtual Json::Value Read( std::string servicePath );

    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > > Columns;
    std::string Username;
    std::string Password;
    std::string Host;
    int Port;

  private:
    OpalService( const OpalService& ); // Not implemented
    void operator=( const OpalService& ); // Not implemented
  };
}

#endif
