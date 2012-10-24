/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Cineloop.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Cineloop_h
#define __Cineloop_h

#include "ActiveRecord.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class Cineloop : public ActiveRecord
  {
  public:
    static Cineloop *New();
    vtkTypeMacro( Cineloop, ActiveRecord );

  protected:
    Cineloop() {}
    ~Cineloop() {}

    std::string GetName() { return "Cineloop"; }

  private:
    Cineloop( const Cineloop& ); // Not implemented
    void operator=( const Cineloop& ); // Not implemented
  };
}

#endif
