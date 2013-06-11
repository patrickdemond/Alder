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
#include </usr/include/curl/curl.h>
#include </usr/include/curl/stdcheaders.h>
#include </usr/include/curl/easy.h>

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
  Json::Value OpalService::Read( std::string servicePath, std::string fileName )
  {
    bool toFile = 0 < fileName.length();
    FILE *file;
    CURL *curl;
    std::stringstream urlStream;
    std::string credentials, url, result;
    struct curl_slist *headers = NULL;
    CURLcode res;
    Json::Value root;
    Json::Reader reader;

    // encode the credentials
    Utilities::base64String( this->Username + ":" + this->Password, credentials );
    credentials = "Authorization:X-Opal-Auth " + credentials;

    urlStream << "https://" << this->Host << ":" << this->Port << "/ws" + servicePath;
    url = urlStream.str();

    curl = curl_easy_init();
    if( !curl ) 
      throw std::runtime_error( "Unable to create cURL connection to Opal" );

    // put the credentials in a header
    headers = curl_slist_append( headers, credentials.c_str() );

    // if we are writing to a file, open it
    if( toFile ) file = fopen( fileName.c_str(), "wb" );

    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToFile );
    else curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToString );

    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEDATA, file );
    else curl_easy_setopt( curl, CURLOPT_WRITEDATA, &result );

    curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
    curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    res = curl_easy_perform( curl );

    // clean up
    curl_slist_free_all( headers );
    curl_easy_cleanup( curl );

    if( 0 != res )
    {
      std::stringstream stream;
      stream << "Received cURL error " << res << " when attempting to contact Opal";
      throw std::runtime_error( stream.str().c_str() );
    }

    if( !toFile )
    {
      if( 0 == result.length() )
        throw std::runtime_error( "Empty response from Opal service" );
      else if( !reader.parse( result.c_str(), root ) )
        throw std::runtime_error( "Unable to parse result from Opal service" );
    }

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
