/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   OpalService.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "OpalService.h"

#include "Configuration.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

#include <sstream>
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( OpalService );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  OpalService::OpalService()
  {
    this->Username = "";
    this->Password = "";
    this->Host = "localhost";
    this->Port = 8843;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::Setup( std::string username, std::string password, std::string host, int port )
  {
    this->Username = username;
    this->Password = password;
    this->Host = host;
    this->Port = port;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > OpalService::GetIdentifiers( std::string dataSource, std::string table )
  {
    Json::Value root;
    Json::Reader reader;
    
    std::stringstream stream;
    stream << "./opal.py --opal https://" << this->Host << ":" << this->Port
           << " --user " << this->Username 
           << " --password " << this->Password
           << " --ws /datasource/" << dataSource
           << "/table/" << table << "/entities";
    std::string result = exec( stream.str().c_str() );
    if( 0 == result.length() )
      throw std::runtime_error( "Invalid response from Opal service" );
    else if( !reader.parse( result.c_str(), root ) )
      throw std::runtime_error( "Unable to parse result from Opal service" );
    
    std::vector< std::string > list;
    for( int i = 0; i < root.size(); ++i )
    {
      std::string identifier = root[i].get( "identifier", "" ).asString();
      if( 0 < identifier.length() ) list.push_back( identifier );
    }

    // Opal doesn't sort results, do so now
    std::sort( list.begin(), list.end() );
    return list;
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::GetValue( std::string dataSource, std::string table, std::string variable )
  {
  }
}
