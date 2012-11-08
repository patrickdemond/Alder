/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Exam.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Exam
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record for the Exam table
 */

#ifndef __Exam_h
#define __Exam_h

#include "ActiveRecord.h"

#include <iostream>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Exam : public ActiveRecord
  {
  public:
    static Exam *New();
    vtkTypeMacro( Exam, ActiveRecord );
    std::string GetName() { return "Exam"; }

  protected:
    Exam() {}
    ~Exam() {}

  private:
    Exam( const Exam& ); // Not implemented
    void operator=( const Exam& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
