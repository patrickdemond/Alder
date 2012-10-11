/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   User.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME User - User or "central" object
//
// .SECTION Description
//
// .SECTION See Also
// UserReader UserWriter
// 

#ifndef __User_h
#define __User_h

#include "ModelObject.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class User : public ModelObject
  {
  public:
    static User *New();
    vtkTypeMacro( User, ModelObject );

    void SetPassword( std::string );

    std::string name;
    std::string lastLogin;
    std::string createdOn;
    std::string hashedPassword;

  protected:
    User();
    ~User() {}

  private:
    User( const User& ); // Not implemented
    void operator=( const User& ); // Not implemented
  };
}

#endif
