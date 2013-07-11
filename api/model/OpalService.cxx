/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   OpalService.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "OpalService.h"

#include "Application.h"
#include "Configuration.h"
#include "Utilities.h"

#include "vtkCommand.h"
#include "vtkObjectFactory.h"

#include <sstream>
#include <stdexcept>
#include </usr/include/curl/curl.h>
#include </usr/include/curl/stdcheaders.h>
#include </usr/include/curl/easy.h>

namespace Alder
{
  // initialize the configureEventSent static ivar
  bool OpalService::configureEventSent = false;
  bool OpalService::progressCheck = true;

  // this function is used by curl to send progress signals
  int OpalService::curlProgressCallback(
    void *notUsed, double downTotal, double downNow, double upTotal, double upNow )
  {
    Application *app = Application::GetInstance();

    if( !app->GetAbortFlag() )
    {
      bool global = false;
      // send the configure event if it hasn't been sent yet
      if( !OpalService::configureEventSent  )
      {
        // send a pair, the first argument is that this is the local progress, the second to set the mode
        bool showBusy = OpalService::progressCheck ? (0.0 == downTotal) : false;
        std::pair<bool, bool> configureProgress = std::pair<bool, bool>( global, showBusy );
        app->InvokeEvent( vtkCommand::ConfigureEvent, static_cast<void *>( &configureProgress ) );
        OpalService::configureEventSent = true;
        return 0;
      }

      // if the downTotal is 0 then we can't send a progress event
      std::pair<bool, double> progress =
        std::pair<bool, double>( global, ( 0.0 == downTotal ? downTotal : downNow / downTotal ) );
       
      app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );
    }

    return app->GetAbortFlag() ? 1 : 0;
  }

  vtkStandardNewMacro( OpalService );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  OpalService::OpalService()
  {
    this->Username = "";
    this->Password = "";
    this->Host = "localhost";
    this->Port = 8843;
    this->Timeout = 10;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::Setup(
    std::string username, std::string password, std::string host, int port, int timeout )
  {
    this->Username = username;
    this->Password = password;
    this->Host = host;
    this->Port = port;
    this->Timeout = timeout;
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
    Application *app = Application::GetInstance();

    // encode the credentials
    Utilities::base64String( this->Username + ":" + this->Password, credentials );
    credentials = "Authorization:X-Opal-Auth " + credentials;

    urlStream << "https://" << this->Host << ":" << this->Port << "/ws" + servicePath;
    url = urlStream.str();
    Utilities::log( "Querying Opal: " + url );

    curl = curl_easy_init();
    if( !curl ) 
      throw std::runtime_error( "Unable to create cURL connection to Opal" );

    // put the credentials in a header and the option to return data in json format
    headers = curl_slist_append( headers, "Accept: application/json" );
    headers = curl_slist_append( headers, credentials.c_str() );

    // if we are writing to a file, open it
    if( toFile ) 
    {
      file = fopen( fileName.c_str(), "wb" );

      if( NULL == file )
      {
        std::stringstream stream;
        cout << servicePath << endl; 
        stream << "Unable to open file \"" << fileName << "\" for writing." << endl;
        throw std::runtime_error( stream.str().c_str() );
      }
    }

    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToFile );
    else curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToString );

    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEDATA, file );
    else curl_easy_setopt( curl, CURLOPT_WRITEDATA, &result );

    curl_easy_setopt( curl, CURLOPT_TIMEOUT, this->Timeout );
    curl_easy_setopt( curl, CURLOPT_SSLVERSION, 3 );
    curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 0L );
    curl_easy_setopt( curl, CURLOPT_PROGRESSFUNCTION, OpalService::curlProgressCallback );
    curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );

    // we are using the local progress bar for curl progress, not the global one
    bool global = false;

    // invoke the start event using the local progress bar
    app->InvokeEvent( vtkCommand::StartEvent, static_cast<void *>( &global ) );
     
    // if set, the configure event will be performed during the first callback within curl progress
    res = curl_easy_perform( curl );

    // invoke the end event using the local progress bar
    app->InvokeEvent( vtkCommand::EndEvent, static_cast<void *>( &global ) );

    // clean up
    curl_slist_free_all( headers );
    curl_easy_cleanup( curl );
    if( toFile ) fclose( file );

    if( 0 != res )
    {
      // don't display abort errors (code 42) when the user initiated the abort
      if( !( CURLE_ABORTED_BY_CALLBACK == res && app->GetAbortFlag() ) )
      {
        std::stringstream stream;
        stream << "Received cURL error " << res << " when attempting to contact Opal: ";
        stream << curl_easy_strerror( res );
        throw std::runtime_error( stream.str().c_str() );
      }
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
  std::map< std::string, std::map< std::string, std::string > > OpalService::GetRows(
    std::string dataSource, std::string table, int offset, int limit )
  {
    std::map< std::string, std::map< std::string, std::string > > list;
    std::string identifier, key, value;
    std::stringstream stream;

    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSets?offset=" << offset << "&limit=" << limit;
    Json::Value root = this->Read( stream.str() );
    
    for( int i = 0; i < root["valueSets"].size(); ++i )
    {
      identifier = root["valueSets"][i].get( "identifier", "" ).asString();

      if( 0 < identifier.length() )
      {
        std::map< std::string, std::string > map;
        for( int j = 0; j < root["valueSets"][i]["values"].size(); ++j )
        {
          key = root["variables"][j].asString();
          value = root["valueSets"][i]["values"][j].get( "value", "" ).asString();
          map[key] = value;
        }
        list[identifier] = map;
      }
    }

    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map< std::string, std::string > OpalService::GetRow(
    std::string dataSource, std::string table, std::string identifier )
  {
    std::map< std::string, std::string > map;
    std::string key, value;
    std::stringstream stream;

    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier;
    Json::Value root = this->Read( stream.str() );
    
    identifier = root["valueSets"][0].get( "identifier", "" ).asString();

    if( 0 < identifier.length() )
    {
      for( int j = 0; j < root["valueSets"][0]["values"].size(); ++j )
      {
        key = root["variables"][j].asString();
        value = root["valueSets"][0]["values"][j].get( "value", "" ).asString();
        map[key] = value;
      }
    }

    return map;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map< std::string, std::string > OpalService::GetColumn(
    std::string dataSource, std::string table, std::string variable, int offset, int limit )
  {
    std::map< std::string, std::string > map;
    std::string identifier, value;
    std::stringstream stream;

    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSets?offset=" << offset << "&limit=" << limit
           << "&select=name().eq('" << variable << "')";
    Json::Value root = this->Read( stream.str() );
    
    for( int i = 0; i < root["valueSets"].size(); ++i )
    {
      identifier = root["valueSets"][i].get( "identifier", "" ).asString();

      if( 0 < identifier.length() )
      {
        value = root["valueSets"][i]["values"][0].get( "value", "" ).asString();
        map[identifier] = value;
      }
    }

    return map;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string OpalService::GetValue(
    std::string dataSource, std::string table, std::string identifier, std::string variable )
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier << "/variable/" << variable;
    return this->Read( stream.str() ).get( "value", "" ).asString();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > OpalService::GetValues(
    std::string dataSource, std::string table, std::string identifier, std::string variable )
  {
    std::vector< std::string > retValues;
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier << "/variable/" << variable;

    // loop through the values array and get all the values
    Json::Value values = this->Read( stream.str() ).get( "values", "" );

    for( int i = 0; i < values.size(); ++i )
      retValues.push_back( values[i].get( "value", "" ).asString() );

    return retValues;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::SaveFile(
    std::string fileName,
    std::string dataSource,
    std::string table,
    std::string identifier,
    std::string variable,
    int position )
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier << "/variable/" << variable << "/value";

    // add on the position
    if( 0 <= position ) stream << "?pos=" << position;

    this->Read( stream.str(), fileName );
  }
}
