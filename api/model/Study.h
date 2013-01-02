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

    /**
     * Returns the next study in UId order.
     */
    vtkSmartPointer<Study> GetNext();

    /**
     * Makes the current record the next record in UId order.
     */
    void Next();

    /**
     * Returns the previous study in UId order.
     */
    vtkSmartPointer<Study> GetPrevious();

    /**
     * Makes the current record the previous record in UId order.
     */
    void Previous();

    /**
     * Returns a vector of all UIds alphabetically ordered
     */
    static std::vector< std::string > GetUIdList();

    /**
     * Convenience method to determine how many images this study has
     */
    int GetImageCount();

    /**
     * Returns whether a user has rated all images associated with the study.
     * If the study has no images this method returns true.
     */
    bool IsRatedBy( User* user );

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
