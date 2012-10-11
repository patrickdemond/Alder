/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
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
  User::User()
  {
    this->name = "";
    this->hashedPassword = "";
    this->lastLogin = "";
    this->createdOn = "";
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void User::SetPassword( std::string password )
  {
    Alder::hashString( password, this->hashedPassword );
  }
}
