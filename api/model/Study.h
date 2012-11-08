/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Study.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Study
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record for the Study table
 */

#ifndef __Study_h
#define __Study_h

#include "ActiveRecord.h"

#include <iostream>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Study : public ActiveRecord
  {
  public:
    static Study *New();
    vtkTypeMacro( Study, ActiveRecord );
//    static void UpdateData();
//    static std::vector< std::string > GetIdentifierList();
    std::string GetName() { return "Study"; }

  protected:
    Study() {}
    ~Study() {}

  private:
    Study( const Study& ); // Not implemented
    void operator=( const Study& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
