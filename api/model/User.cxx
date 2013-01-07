/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   User.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "User.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( User );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void User::SetVariant( std::string column, vtkVariant value )
  {
    if( 0 == column.compare( "Password" ) && value.IsValid() )
    { // if we are setting the password override the parent so that we can hash
      std::string hashedPassword;
      Utilities::hashString( value.ToString(), hashedPassword );
      value = vtkVariant( hashedPassword );
    }

    this->Superclass::SetVariant( column, value );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void User::ResetPassword()
  {
    this->Set( "Password", User::GetDefaultPassword() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool User::IsPassword( std::string password )
  {
    // first hash the password argument
    std::string hashedPassword;
    Utilities::hashString( password, hashedPassword );
    return 0 == hashedPassword.compare( this->Get( "Password" ).ToString() );
  }
}
