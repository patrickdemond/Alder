#ifndef __vtkAlderMySQLDatabasePrivate_h
#define __vtkAlderMySQLDatabasePrivate_h

#ifdef _WIN32
# include <winsock.h> // mysql.h relies on the typedefs from here
#endif

#include <mysql.h> // needed for MYSQL typedefs

class vtkAlderMySQLDatabasePrivate
{
public:
  vtkAlderMySQLDatabasePrivate() :
    Connection( NULL )
  {
  mysql_init( &this->NullConnection );
  }

  MYSQL NullConnection;
  MYSQL *Connection;
};

#endif // __vtkAlderMySQLDatabasePrivate_h
