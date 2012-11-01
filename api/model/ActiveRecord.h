/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   ActiveRecord.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __ActiveRecord_h
#define __ActiveRecord_h

#include "ModelObject.h"

#include "Application.h"
#include "Database.h"

#include "vtkMySQLQuery.h"
#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include <map>
#include <typeinfo>
#include <vector>

namespace Alder
{
  class ActiveRecord : public ModelObject
  {
  public:
    vtkTypeMacro( ActiveRecord, ModelObject );
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
    virtual void Save();
    virtual void Remove();
    template< class T > static void GetAll( std::vector< vtkSmartPointer< T > > *list )
    { // we have to implement this here because of the template
      Application *app = Application::GetInstance();
      // get the class name of T, return error if not found
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      std::stringstream stream;
      stream << "SELECT id FROM " << type;
      vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();

      vtkDebugSQLWithoutObjectMacro( << stream.str() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take(
          T::SafeDownCast( app->Create( type ) ) );
        record->Load( "id", query->DataValue( 0 ).ToString() );
        list->push_back( record );
      }
    }

    template< class T > void GetList( std::vector< vtkSmartPointer< T > > *list )
    {
      Application *app = Application::GetInstance();
      // get the class name of T, return error if not found
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      std::stringstream stream;
      stream << "SELECT id FROM " << type << " "
             << "WHERE " << this->GetName() << "_id = " << this->Get( "id" )->ToString();
      vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();

      vtkDebugSQLMacro( << stream.str() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take(
          T::SafeDownCast( app->Create( type ) ) );
        record->Load( "id", query->DataValue( 0 ).ToString() );
        list->push_back( record );
      }
    }

    // Get table column values
    virtual vtkVariant* Get( std::string column );

    // Set table column values
    template <class T> void Set( std::string column, T value )
    { this->SetVariant( column, new vtkVariant( value ) ); }

    vtkGetMacro( DebugSQL, bool );
    vtkSetMacro( DebugSQL, bool );
    vtkBooleanMacro( DebugSQL, bool );

  protected:
    ActiveRecord();
    ~ActiveRecord();

    std::string Name() const { return typeid( *this ).name(); }
    virtual std::string GetName() = 0;
    void Initialize();
    virtual void SetVariant( std::string column, vtkVariant *value );

    std::map<std::string,vtkVariant*> ColumnValues;
    bool DebugSQL;
    bool Initialized;

  private:
    ActiveRecord( const ActiveRecord& ); // Not implemented
    void operator=( const ActiveRecord& ); // Not implemented
    void DeleteColumnValues();
  };
}
#endif
