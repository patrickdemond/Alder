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
     * Updates the Interview table with all existing interviews in Opal
     */
    static void UpdateInterviewData();

    /**
     * Returns whether this interview's exam and image data has been downloaded
     */
    bool HasExamData();
    bool HasImageData();

    /**
     * Updates all exam and image data associated with the interview from Opal
     * Note: exam data must be downloaded before image data
     */
    void UpdateExamData();
    void UpdateImageData();

    /**
     * Returns the neighbouring interview in UId/VisitDate order.
     */
    vtkSmartPointer<Interview> GetNeighbour( bool forward, bool loaded, bool unRated );
    vtkSmartPointer<Interview> GetNext( bool loaded, bool unRated )
    { return this->GetNeighbour( true, loaded, unRated ); }
    vtkSmartPointer<Interview> GetNextLoaded( bool unRated )
    { return this->GetNeighbour( true, true, unRated ); }
    vtkSmartPointer<Interview> GetNextUnLoaded( bool unRated )
    { return this->GetNeighbour( true, false, unRated ); }
    vtkSmartPointer<Interview> GetPrevious( bool loaded, bool unRated )
    { return this->GetNeighbour( false, loaded, unRated ); }
    vtkSmartPointer<Interview> GetPreviousLoaded( bool unRated )
    { return this->GetNeighbour( false, true, unRated ); }
    vtkSmartPointer<Interview> GetPreviousUnLoaded( bool unRated )
    { return this->GetNeighbour( false, false, unRated ); }

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
