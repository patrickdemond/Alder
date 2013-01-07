/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Interview
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record for the Interview table
 */

#ifndef __Interview_h
#define __Interview_h

#include "ActiveRecord.h"

#include <iostream>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Interview : public ActiveRecord
  {
  public:
    static Interview *New();
    vtkTypeMacro( Interview, ActiveRecord );
    std::string GetName() { return "Interview"; }

    /**
     * Returns the next interview in UId/VisitDate order.
     */
    vtkSmartPointer<Interview> GetNext();

    /**
     * Makes the current record the next record in UId/VisitDate order.
     */
    void Next();

    /**
     * Returns the previous interview in UId/VisitDate order.
     */
    vtkSmartPointer<Interview> GetPrevious();

    /**
     * Makes the current record the previous record in UId/VisitDate order.
     */
    void Previous();

    /**
     * Convenience method to determine how many images this interview has
     */
    int GetImageCount();

    /**
     * Returns whether a user has rated all images associated with the interview.
     * If the interview has no images this method returns true.
     */
    bool IsRatedBy( User* user );

  protected:
    Interview() {}
    ~Interview() {}

    /**
     * Returns a vector of all UId/VisitDate pairs ordered by UId then VisitDate
     */
    static std::vector< std::pair< std::string, std::string > > GetUIdVisitDateList();

  private:
    Interview( const Interview& ); // Not implemented
    void operator=( const Interview& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
