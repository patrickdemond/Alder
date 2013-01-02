/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Modality.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Modality
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
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
    std::string GetName() { return "Modality"; }

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
