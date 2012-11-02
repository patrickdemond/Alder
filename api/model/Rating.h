/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Rating.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Rating_h
#define __Rating_h

#include "ActiveRecord.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class Rating : public ActiveRecord
  {
  public:
    static Rating *New();
    vtkTypeMacro( Rating, ActiveRecord );
    std::string GetName() { return "Rating"; }

  protected:
    Rating() {}
    ~Rating() {}

  private:
    Rating( const Rating& ); // Not implemented
    void operator=( const Rating& ); // Not implemented
  };
}

#endif
