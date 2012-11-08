/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   User.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class User
 *
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 *
 * @brief User record.
 *
 * User is a child class of ActiveRecord that represents
 * an entry in the User table of the Alder database.  
 *
 */

#ifndef __User_h
#define __User_h

#include "ActiveRecord.h"

#include <iostream>

/**
 * @addtogroup Alder
 * @{
 */

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
    static std::string GetDefaultPassword() { return "password"; }
    std::string GetName() { return "User"; }

  protected:
    User() {}
    ~User() {}

    void SetVariant( std::string column, vtkVariant *value );

  private:
    User( const User& ); // Not implemented
    void operator=( const User& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
