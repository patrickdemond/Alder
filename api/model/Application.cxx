/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Application.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "Application.h"

#include "Configuration.h"
#include "Cineloop.h"
#include "Database.h"
#include "Image.h"
#include "OpalService.h"
#include "Rating.h"
#include "Series.h"
#include "Study.h"
#include "User.h"

#include "vtkObjectFactory.h"
#include "vtkVariant.h"
#include "vtkMedicalImageViewer.h"

namespace Alder
{
  vtkCxxSetObjectMacro( Application, ActiveUser, User );
  vtkCxxSetObjectMacro( Application, ActiveStudy, Study );

  Application* Application::Instance = NULL; // set the initial application

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::Application()
  {
    this->Viewer = vtkMedicalImageViewer::New();
    this->Viewer->SetImageToSinusoid();
    this->Config = Configuration::New();
    this->DB = Database::New();
    this->Opal = OpalService::New();
    this->ActiveUser = NULL;
    this->ActiveStudy = NULL;

    // populate the factory with all active record classes
    this->Factory["Cineloop"] = &createInstance<Cineloop>;
    this->Factory["Image"] = &createInstance<Image>;
    this->Factory["Rating"] = &createInstance<Rating>;
    this->Factory["Series"] = &createInstance<Series>;
    this->Factory["Study"] = &createInstance<Study>;
    this->Factory["User"] = &createInstance<User>;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::~Application()
  {
    if( NULL != this->Viewer )
    {
      this->Viewer->Delete();
      this->Viewer = NULL;
    }

    if( NULL != this->Config )
    {
      this->Config->Delete();
      this->Config = NULL;
    }

    if( NULL != this->DB )
    {
      this->DB->Delete();
      this->DB = NULL;
    }

    if( NULL != this->Opal )
    {
      this->Opal->Delete();
      this->Opal = NULL;
    }

    if( NULL != this->ActiveUser )
    {
      this->ActiveUser->Delete();
      this->ActiveUser = NULL;
    }

    if( NULL != this->ActiveStudy )
    {
      this->ActiveStudy->Delete();
      this->ActiveStudy = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  // this must be implemented instead of the standard new macro since the New()
  // method is protected (ie: we do not want an instantiator new function)
  Application* Application::New()
  {
    vtkObject* ret = vtkObjectFactory::CreateInstance( "Application" );
    return ret ? static_cast< Application* >( ret ) : new Application;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application* Application::GetInstance()
  {
    if( NULL == Application::Instance ) Application::Instance = Application::New();
    return Application::Instance;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::DeleteInstance()
  {
    if( NULL != Application::Instance )
    {
      Application::Instance->Delete();
      Application::Instance = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ReadConfiguration( std::string filename )
  {
    // make sure the file exists
    ifstream ifile( filename.c_str() );
    if( !ifile ) return false;
    return this->Config->Read( filename );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ConnectToDatabase()
  {
    std::string name = this->Config->GetValue( "Database", "Name" );
    std::string user = this->Config->GetValue( "Database", "Username" );
    std::string pass = this->Config->GetValue( "Database", "Password" );
    std::string host = this->Config->GetValue( "Database", "Host" );
    std::string port = this->Config->GetValue( "Database", "Port" );

    // make sure the database and user names are provided
    if( 0 == name.length() || 0 == user.length() )
    {
      cerr << "ERROR: database name, user name and password must be included in "
           << "configuration file" << endl;
      return false;
    }

    // defaint host and port
    if( 0 == host.length() ) host = "localhost";
    if( 0 == port.length() ) port = "3306";

    return this->DB->Connect( name, user, pass, host, vtkVariant( port ).ToInt() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetupOpalService()
  {
    // setup the opal service
    std::string user = this->Config->GetValue( "Opal", "Username" );
    std::string pass = this->Config->GetValue( "Opal", "Password" );
    std::string host = this->Config->GetValue( "Opal", "Host" );
    std::string port = this->Config->GetValue( "Opal", "Port" );
    this->Opal->Setup( user, pass, host, vtkVariant( port ).ToInt() );
  }
}
