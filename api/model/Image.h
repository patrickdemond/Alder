/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Image.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Image_h
#define __Image_h

#include "ActiveRecord.h"

#include <iostream>

namespace Alder
{
//  class Session;
  class Image : public ActiveRecord
  {
  public:
    static Image *New();
    vtkTypeMacro( Image, ActiveRecord );
    std::string GetName() { return "Image"; }

  protected:
    Image() {}
    ~Image() {}

  private:
    Image( const Image& ); // Not implemented
    void operator=( const Image& ); // Not implemented
  };
}

#endif
