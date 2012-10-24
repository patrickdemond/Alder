/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Series.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Series_h
#define __Series_h

#include "ActiveRecord.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class Series : public ActiveRecord
  {
  public:
    static Series *New();
    vtkTypeMacro( Series, ActiveRecord );

  protected:
    Series() {}
    ~Series() {}

    std::string GetName() { return "Series"; }

  private:
    Series( const Series& ); // Not implemented
    void operator=( const Series& ); // Not implemented
  };
}

#endif
