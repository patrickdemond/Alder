/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAlderMySQLDatabase.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkAlderMySQLDatabase - maintain a connection to a MySQL database
//
// .SECTION Description
//
// This class provides a VTK interface to MySQL
// (http://www.mysql.com).  Unlike file-based databases like SQLite, you
// talk to MySQL through a client/server connection.  You must specify
// the hostname, (optional) port to connect to, username, password and
// database name in order to connect.
//
// This method has been copied from the VTK project to fix deficiencies
//
// .SECTION See Also
// vtkAlderMySQLQuery

#ifndef __vtkAlderMySQLDatabase_h
#define __vtkAlderMySQLDatabase_h

#include <vtkSQLDatabase.h>

class vtkAlderMySQLDatabasePrivate;
class vtkAlderMySQLQuery;
class vtkSQLQuery;
class vtkStringArray;

class vtkAlderMySQLDatabase : public vtkSQLDatabase
{
//BTX
  friend class vtkAlderMySQLQuery;
//ETX

public:
  vtkTypeMacro(vtkAlderMySQLDatabase, vtkSQLDatabase);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkAlderMySQLDatabase *New();

  // Description:
  // Open a new connection to the database.  You need to set the
  // filename before calling this function.  Returns true if the
  // database was opened successfully; false otherwise.
  bool Open( const char* password = 0 );

  // Description:
  // Close the connection to the database.
  void Close();

  // Description:
  // Return whether the database has an open connection
  bool IsOpen();

  // Description:
  // Return an empty query on this database.
  vtkSQLQuery* GetQueryInstance();

  // Description:
  // Get the list of tables from the database
  vtkStringArray* GetTables();

  // Description:
  // Get the list of fields for a particular table
  vtkStringArray* GetRecord(const char *table);

  // Description:
  // Return whether a feature is supported by the database.
  bool IsSupported(int feature);

  // Description:
  // Did the last operation generate an error
  bool HasError();

  // Description:
  // Get the last error text from the database
  const char* GetLastErrorText();

  // Description:
  // String representing database type (e.g. "mysql").
  vtkGetStringMacro(DatabaseType);

  // Description:
  // The database server host name.
  vtkSetStringMacro(HostName);
  vtkGetStringMacro(HostName);

  // Description:
  // The user name for connecting to the database server.
  vtkSetStringMacro(User);
  vtkGetStringMacro(User);

  // Description:
  // The user's password for connecting to the database server.
  vtkSetStringMacro(Password);

  // Description:
  // The name of the database to connect to.
  vtkSetStringMacro(DatabaseName);
  vtkGetStringMacro(DatabaseName);

  // Description:
  // Should automatic reconnection be enabled?
  // This defaults to true.
  // If you change its value, you must do so before any call to Open().
  vtkSetMacro(Reconnect,int);
  vtkGetMacro(Reconnect,int);
  vtkBooleanMacro(Reconnect,int);

  // Description:
  // The port used for connecting to the database.
  vtkSetClampMacro(ServerPort, int, 0, VTK_INT_MAX);
  vtkGetMacro(ServerPort, int);

  // Description:
  // Get the URL of the database.
  virtual vtkStdString GetURL();

  // Description:
  // Return the SQL string with the syntax of the preamble following a
  // "CREATE TABLE" SQL statement.
  // NB: this method implements the MySQL-specific IF NOT EXISTS syntax,
  // used when b = false.
  virtual vtkStdString GetTablePreamble( bool b ) { return b ? vtkStdString() :"IF NOT EXISTS "; }

  // Description:
  // Return the SQL string with the syntax to create a column inside a
  // "CREATE TABLE" SQL statement.
  // NB1: this method implements the MySQL-specific syntax:
  // \verbatim
  // `<column name>` <column type> <column attributes>
  // \endverbatim
  // NB2: if a column has type SERIAL in the schema, this will be turned
  // into INT NOT NULL AUTO_INCREMENT. Therefore, one should not pass
  // NOT NULL as an attribute of a column whose type is SERIAL.
  virtual vtkStdString GetColumnSpecification( vtkSQLDatabaseSchema* schema,
                                               int tblHandle,
                                               int colHandle );

  // Description:
  // Return the SQL string with the syntax to create an index inside a
  // "CREATE TABLE" SQL statement.
  // NB1: this method implements the MySQL-specific syntax:
  // \verbatim
  // <index type> [<index name>]  (`<column name 1>`,... )
  // \endverbatim
  // NB2: since MySQL supports INDEX creation within a CREATE TABLE statement,
  // skipped is always returned false.
  virtual vtkStdString GetIndexSpecification( vtkSQLDatabaseSchema* schema,
                                              int tblHandle,
                                              int idxHandle,
                                              bool& skipped );

  // Description:
  // Create a new database, optionally dropping any existing database of the same name.
  // Returns true when the database is properly created and false on failure.
  bool CreateDatabase( const char* dbName, bool dropExisting );

  // Description:
  // Drop a database if it exists.
  // Returns true on success and false on failure.
  bool DropDatabase( const char* dbName );

  // Description:
  // Overridden to determine connection parameters given the URL.
  // This is called by CreateFromURL() to initialize the instance.
  // Look at CreateFromURL() for details about the URL format.
  virtual bool ParseURL(const char* url);

protected:
  vtkAlderMySQLDatabase();
  ~vtkAlderMySQLDatabase();

private:
  // We want this to be private, a user of this class
  // should not be setting this for any reason
  vtkSetStringMacro(DatabaseType);

  vtkStringArray *Tables;
  vtkStringArray *Record;

  char* DatabaseType;
  char* HostName;
  char* User;
  char* Password;
  char* DatabaseName;
  int ServerPort;
  int Reconnect;

//BTX
  vtkAlderMySQLDatabasePrivate* const Private;
//ETX

  vtkAlderMySQLDatabase(const vtkAlderMySQLDatabase &); // Not implemented.
  void operator=(const vtkAlderMySQLDatabase &); // Not implemented.
};

#endif // __vtkAlderMySQLDatabase_h

