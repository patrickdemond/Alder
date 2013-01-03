/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
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
  Json::Value OpalService::Read( std::string servicePath )
  {
    Json::Value root;
    Json::Reader reader;
    
    std::stringstream stream;
    stream << "./opal.py --opal https://" << this->Host << ":" << this->Port
           << " --user " << this->Username 
           << " --password " << this->Password
           << " --ws \"" << servicePath << "\"";
    std::string result = exec( stream.str().c_str() );
    if( 0 == result.length() )
      throw std::runtime_error( "Invalid response from Opal service" );
    else if( !reader.parse( result.c_str(), root ) )
      throw std::runtime_error( "Unable to parse result from Opal service" );

    return root;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > OpalService::GetIdentifiers( std::string dataSource, std::string table )
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table << "/entities";
    Json::Value root = this->Read( stream.str() );
    
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
  std::map< std::string, std::string > OpalService::GetValueList(
      std::string dataSource, std::string table, std::string variable, int offset, int limit )
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSets?offset=" << offset << "&limit=" << limit
           << "&select=name().eq('" << variable << "')";
    Json::Value root = this->Read( stream.str() );
    
    std::map< std::string, std::string > list;
    for( int i = 0; i < root["valueSets"].size(); ++i )
    {
      std::string identifier = root["valueSets"][i].get( "identifier", "" ).asString();
      std::string value = root["valueSets"][i]["values"][0].get( "value", "" ).asString();
      if( 0 < identifier.length() && 0 < value.length() ) list[identifier] = value;
    }

    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::GetValue( std::string dataSource, std::string table, std::string variable )
  {
  }
}
