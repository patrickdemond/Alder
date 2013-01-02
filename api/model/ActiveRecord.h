/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   ActiveRecord.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class ActiveRecord
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief Abstract base class for all active record classes
 * 
 * ActiveRecord provides a programming interface to the database.  All classes
 * which correspond directly to a table (and named exactly the same way) must
 * extend this class.
 */

#ifndef __ActiveRecord_h
#define __ActiveRecord_h

#include "ModelObject.h"

#include "Application.h"
#include "Database.h"

#include "vtkAlderMySQLQuery.h"
#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include <map>
#include <stdexcept>
#include <typeinfo>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class ActiveRecord : public ModelObject
  {
  public:
    vtkTypeMacro( ActiveRecord, ModelObject );

    /**
     * Returns whether this record has a particular column
     */
    bool ColumnNameExists( std::string column );

    //@{
    /**
     * Loads a specific record from the database.  Input parameters must include the values
     * of a primary or unique key in the corresponding table.
     * @throws runtime_error
     */
    bool Load( std::string key, std::string value )
    {
      return this->Load( std::pair< std::string, std::string >( key, value ) );
    }
    bool Load( std::pair< std::string, std::string > pair )
    {
      std::map< std::string, std::string > map;
      map.insert( pair );
      return this->Load( map );
    }
    virtual bool Load( std::map< std::string, std::string > map );
    //@}

    /**
     * Saves the record's current values to the database.  If the record was not loaded
     * then a new record will be inserted into the database.
     */
    virtual void Save();

    /**
     * Removes the current record from the database.
     * @throws runtime_error
     */
    virtual void Remove();
    
    /**
     * Provides a list of all records which exist in a table.
     * @param list vector An existing vector to put all records into.
     */
    template< class T > static void GetAll( std::vector< vtkSmartPointer< T > > *list )
    { // we have to implement this here because of the template
      Application *app = Application::GetInstance();
      // get the class name of T, return error if not found
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      std::stringstream stream;
      stream << "SELECT Id FROM " << type;
      vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

      query->SetQuery( stream.str().c_str() );
      query->Execute();

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take( T::SafeDownCast( T::New() ) );
        record->Load( "Id", query->DataValue( 0 ).ToString() );
        list->push_back( record );
      }
    }

    /**
     * Provides a list of all records which are related to this record by foreign key.
     * @param list vector An existing vector to put all records into.
     */
    template< class T > void GetList( std::vector< vtkSmartPointer< T > > *list )
    {
      Application *app = Application::GetInstance();
      // get the class name of T, return error if not found
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      std::stringstream stream;
      stream << "SELECT Id FROM " << type << " "
             << "WHERE " << this->GetName() << "Id = " << this->Get( "Id" ).ToString();
      vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

      vtkDebugSQLMacro( << stream.str() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take(
          T::SafeDownCast( app->Create( type ) ) );
        record->Load( "Id", query->DataValue( 0 ).ToString() );
        list->push_back( record );
      }
    }
    
    /**
     * Returns the number of records which are related to this record by foreign key.
     * @param std::string recordType The associated table name.
     */
    int GetCount( std::string recordType );

    /**
     * Get the value of any column in the record.
     * @throws runtime_error
     */
    virtual vtkVariant Get( std::string column );

    /**
     * Get the record which has a foreign key in this table.
     * Note: the record returned must be deleted by the recipient
     * @throws runtime_error
     */
    virtual ActiveRecord* GetRecord( std::string column, std::string columnName = "" );

    /**
     * Set the value of any column in the record.
     * Note: this will only affect the active record in memory, to update the database
     * Save() needs to be called.
     * If you wish to set the value to NULL then use the SetNull() method instead of Set()
     */
    template <class T> void Set( std::string column, T value )
    { this->SetVariant( column, vtkVariant( value ) ); }
    void SetNull( std::string column )
    { this->SetVariant( column, vtkVariant() ); }

    //@{
    /** 
     * Defines whether or not to print all SQL statements to cout
     */
    vtkGetMacro( DebugSQL, bool );
    vtkSetMacro( DebugSQL, bool );
    vtkBooleanMacro( DebugSQL, bool );
    //@}

    /**
     * Must be extended by every child class.
     * Its value is always the name of the class (identical case)
     */
    virtual std::string GetName() = 0;

  protected:
    ActiveRecord();
    ~ActiveRecord() {}

    /**
     * Sets up the record with default values for all table columns
     */
    void Initialize();

    /**
     * Runs a check to make sure the record exists in the database
     * @throws runtime_error
     */
    inline void AssertPrimaryId()
    {
      vtkVariant id = this->Get( "Id" );
      if( !id.IsValid() || 0 == id.ToInt() )
        throw std::runtime_error( "Assert failed: primary id for record is not set" );
    }

    /**
     * Internal method used by Set()
     * @throws runtime_error
     */
    virtual void SetVariant( std::string column, vtkVariant value );

    std::map<std::string,vtkVariant> ColumnValues;
    bool DebugSQL;
    bool Initialized;

  private:
    ActiveRecord( const ActiveRecord& ); // Not implemented
    void operator=( const ActiveRecord& ); // Not implemented
    void DeleteColumnValues();
  };
}

/** @} end of doxygen group */

#endif
