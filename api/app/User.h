/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   User.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __User_h
#define __User_h

#include "ActiveRecord.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class User : public ActiveRecord
  {
  public:
    static User *New();
    vtkTypeMacro( User, ActiveRecord );

    virtual void ResetPassword();
    virtual bool IsPassword( std::string );

  protected:
    User() {}
    ~User() {}

    std::string GetName() { return "User"; }
    void SetVariant( std::string column, vtkVariant *value );

  private:
    User( const User& ); // Not implemented
    void operator=( const User& ); // Not implemented
  };
}

#endif
