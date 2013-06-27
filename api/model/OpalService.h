/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   OpalService.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class OpalService
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief Class for interacting with Opal
 * 
 * This class provides a programming interface to Opal's RESTful interface by using the
 * curl library.  A description of Opal can be found
 * <a href="http://www.obiba.org/?q=node/63">here</a>.
 */

#ifndef __OpalService_h
#define __OpalService_h

#include "ModelObject.h"

#include "vtkSmartPointer.h"
#include "vtkAlderMySQLQuery.h"

#include <iostream>
#include <json/reader.h>
#include <map>
#include <vector>

class vtkAlderMySQLOpalService;

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class OpalService : public ModelObject
  {
  public:
    static OpalService *New();
    vtkTypeMacro( OpalService, ModelObject );

    /**
     * Defines connection parameters to use when communicating with the Opal server
     */
    void Setup(
      std::string username,
      std::string password,
      std::string host,
      int port = 8843,
      int timeout = 10 );

    vtkGetMacro( Port, int );
    vtkSetMacro( Port, int );

    vtkGetMacro( Timeout, int );
    vtkSetMacro( Timeout, int );

    /**
     * Returns a list of all identifiers in a particular data source and table
     * @param dataSource string
     * @param table string
     */
    std::vector< std::string > GetIdentifiers( std::string dataSource, std::string table );

    /**
     * Returns all variables for all identifiers limited by the offset and limit parameters
     * @param dataSource string
     * @param table string
     * @param offset int The offset to begin the list at.
     * @param limit int The limit of how many key/value pairs to return
     */
    std::map< std::string, std::map< std::string, std::string > > GetRows(
      std::string dataSource, std::string table,
      int offset = 0, int limit = 100 );

    /**
     * Returns all variables for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     */
    std::map< std::string, std::string > GetRow(
      std::string dataSource, std::string table, std::string identifier );

    /**
     * Returns all values for a particular variable limited by the offset and limit parameters
     * @param dataSource string
     * @param table string
     * @param variable string
     * @param offset int The offset to begin the list at.
     * @param limit int The limit of how many key/value pairs to return
     */
    std::map< std::string, std::string > GetColumn(
      std::string dataSource, std::string table, std::string variable,
      int offset = 0, int limit = 100 );

    /**
     * Returns a particular variable for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     * @param variable string
     */
    std::string GetValue(
      std::string dataSource, std::string table, std::string identifier, std::string variable );

    /**
     * Returns an array variable for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     * @param variable string
     */
    std::vector< std::string > GetValues(
      std::string dataSource, std::string table, std::string identifier, std::string variable );

    /**
     * Returns a particular variable for a given identifier
     * @param dataSource string
     * @param fileName string
     * @param table string
     * @param identifier string
     * @param variable string
     * @param integer position
     */
    void SaveFile(
      std::string fileName,
      std::string dataSource,
      std::string table,
      std::string identifier,
      std::string variable,
      int position = -1 );

  protected:
    OpalService();
    ~OpalService() {}

    /**
     * Returns the response provided by Opal for a given service path, or if fileName is not
     * empty then writes the response to the given filename (returning an empty json value.
     * @param servicePath string
     * @param fileName string
     * @throws runtime_error
     */
    virtual Json::Value Read( std::string servicePath, std::string fileName = "" );

    std::map< std::string,std::map< std::string,std::map< std::string, std::string > > > Columns;
    std::string Username;
    std::string Password;
    std::string Host;
    int Port;
    int Timeout;

  private:
    OpalService( const OpalService& ); // Not implemented
    void operator=( const OpalService& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
