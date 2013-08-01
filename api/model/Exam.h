/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Exam
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
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
    std::string GetName() const { return "Exam"; }

    /**
     * Returns whether this exam's image data has been downloaded
     */
    bool HasImageData();

    /**
     * Updates all image data associated with the exam from Opal
     */
    void UpdateImageData();

    /**
     * Returns whether a user has rated all images associated with the exam.
     * If the exam has no images this method returns true.
     */
    bool IsRatedBy( User* user );

    /**
     * Retrieves an image from Opal.
     * @throws exception 
     */
    bool RetrieveImage( std::string type, std::string variable, std::string UId,
                        std::map<std::string, vtkVariant> settings,
                        std::string suffix, std::string sideVariable = "" );

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
