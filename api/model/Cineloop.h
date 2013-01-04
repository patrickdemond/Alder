/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Cineloop.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Cineloop
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record for the Cineloop table
 */

#ifndef __Cineloop_h
#define __Cineloop_h

#include "ActiveRecordWithFile.h"

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Cineloop : public ActiveRecordWithFile
  {
  public:
    static Cineloop *New();
    vtkTypeMacro( Cineloop, ActiveRecordWithFile );
    std::string GetName() { return "Cineloop"; }

  protected:
    Cineloop() {}
    ~Cineloop() {}

  private:
    Cineloop( const Cineloop& ); // Not implemented
    void operator=( const Cineloop& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
