/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   ModelObject.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __ModelObject_h
#define __ModelObject_h

#include "vtkObject.h"
#include "vtkSetGet.h"

namespace Alder
{
  class ModelObject : public vtkObject
  {
  public:
    vtkTypeMacro( ModelObject, vtkObject );

  protected:
    ModelObject();
    ~ModelObject() {}

  private:
    ModelObject( const ModelObject& ); // Not implemented
    void operator=( const ModelObject& ); // Not implemented
  };
}

#endif
