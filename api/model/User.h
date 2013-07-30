/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   User.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class User
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 * 
 * @brief An active record for the User table
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
  class User : public ActiveRecord
  {
  public:
    static User *New();
    vtkTypeMacro( User, ActiveRecord );

    virtual void ResetPassword();
    virtual bool IsPassword( std::string );
    static std::string GetDefaultPassword() { return "password"; }
    std::string GetName() const { return "User"; }

  protected:
    User() {}
    ~User() {}

    virtual void SetVariant( std::string column, vtkVariant value );

  private:
    User( const User& ); // Not implemented
    void operator=( const User& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
