/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Database.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Database
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief Class for interacting with the database
 * 
 * This class provides methods to interact with the database.  It includes
 * metadata such as information about every column in every table.  A single
 * instance of this class is created and managed by the Application singleton
 * and it is primarily used by active records.
 */

#ifndef __Database_h
#define __Database_h

#include "ModelObject.h"

#include "vtkAlderMySQLQuery.h"
#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include <iostream>
#include <map>
#include <vector>

class vtkAlderMySQLDatabase;

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class Database : public ModelObject
  {
  public:
    static Database *New();
    vtkTypeMacro( Database, ModelObject );

    /**
     * Connects to a database given connection parameters
     * @param name string
     * @param user string
     * @param pass string
     * @param host string
     * @param port int
     */
    bool Connect(
      std::string name,
      std::string user,
      std::string pass,
      std::string host,
      int port );

    /**
     * Returns a vtkAlderMySQLQuery object for performing queries
     * This method should only be used by Model objects.
     */
    vtkSmartPointer<vtkAlderMySQLQuery> GetQuery();

    /**
     * Returns a list of column names for a given table
     * @param table string
     * @throws runtime_error
     */
    std::vector<std::string> GetColumnNames( std::string table );

    /**
     * Returns the default value for a table's column
     * @param table string
     * @param column string
     * @throws runtime_error
     */
    vtkVariant GetColumnDefault( std::string table, std::string column );

    /**
     * Returns whether a table's column value may be null
     * @param table string
     * @param column string
     * @throws runtime_error
     */
    bool IsColumnNullable( std::string table, std::string column );

    /**
     * Returns whether a table's column is a foreign key
     * NOTE: there's no way to get this information from the information schema so instead
     *       this method uses the convention that all foreign keys end in "Id"
     * @param table string
     * @param column string
     * @throws runtime_error
     */
    bool IsColumnForeignKey( std::string table, std::string column );

  protected:
    Database();
    ~Database() {}

    /**
     * An internal method which is called once to read all table metadata from the
     * information_schema database.
     */
    void ReadInformationSchema();
    vtkSmartPointer<vtkAlderMySQLDatabase> MySQLDatabase;
    std::map< std::string,std::map< std::string,std::map< std::string, vtkVariant > > > Columns;

  private:
    Database( const Database& ); // Not implemented
    void operator=( const Database& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
