/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Modality.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Modality
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 * 
 * @brief An active record for the Modality table
 */

#ifndef __Modality_h
#define __Modality_h

#include "ActiveRecord.h"

#include <iostream>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Modality : public ActiveRecord
  {
  public:
    static Modality *New();
    vtkTypeMacro( Modality, ActiveRecord );
    std::string GetName() const { return "Modality"; }

  protected:
    Modality() {}
    ~Modality() {}

  private:
    Modality( const Modality& ); // Not implemented
    void operator=( const Modality& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
