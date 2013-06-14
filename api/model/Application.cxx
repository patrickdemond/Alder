/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Application.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "Application.h"

#include "Configuration.h"
#include "Database.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "OpalService.h"
#include "Rating.h"
#include "User.h"

#include "vtkObjectFactory.h"
#include "vtkVariant.h"

#include <stdexcept>

namespace Alder
{
  Application* Application::Instance = NULL; // set the initial application

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::Application()
  {
    this->Config = Configuration::New();
    this->DB = Database::New();
    this->Opal = OpalService::New();
    this->ResetApplication();

    // populate the constructor and class name registries with all active record classes
    this->ConstructorRegistry["Exam"] = &createInstance<Exam>;
    this->ClassNameRegistry["Exam"] = typeid(Exam).name();
    this->ConstructorRegistry["Image"] = &createInstance<Image>;
    this->ClassNameRegistry["Image"] = typeid(Image).name();
    this->ConstructorRegistry["Interview"] = &createInstance<Interview>;
    this->ClassNameRegistry["Interview"] = typeid(Interview).name();
    this->ConstructorRegistry["Rating"] = &createInstance<Rating>;
    this->ClassNameRegistry["Rating"] = typeid(Rating).name();
    this->ConstructorRegistry["User"] = &createInstance<User>;
    this->ClassNameRegistry["User"] = typeid(User).name();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::~Application()
  {
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

    if( NULL != this->ActiveInterview )
    {
      this->ActiveInterview->Delete();
      this->ActiveInterview = NULL;
    }

    if( NULL != this->ActiveInterview )
    {
      this->ActiveInterview->Delete();
      this->ActiveInterview = NULL;
    }

    if( NULL != this->ActiveImage )
    {
      this->ActiveImage->Delete();
      this->ActiveImage = NULL;
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
  std::string Application::GetUnmangledClassName( std::string mangledName )
  {
    std::map< std::string, std::string >::iterator it;
    for( it = this->ClassNameRegistry.begin(); it != this->ClassNameRegistry.end(); ++it )
      if( it->second == mangledName ) return it->first;
    
    throw std::runtime_error( "Tried to unmangle class name which isn't registered." );
    return ""; // this will never happen because of the throw
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

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::ResetApplication()
  {
    this->SetActiveUser( NULL );
    this->SetActiveInterview( NULL );
    this->SetActiveImage( NULL );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveUser( User *user )
  {
    if( user != this->ActiveUser )
    {
      if( this->ActiveUser ) this->ActiveUser->UnRegister( this );
      this->ActiveUser = user;
      if( this->ActiveUser ) 
      {
        this->ActiveUser->Register( this );

        // get the user's last active interview
        vtkSmartPointer< Interview > interview;
        if( this->ActiveUser->GetRecord( interview ) ) this->SetActiveInterview( interview );
      }
      this->Modified();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveInterview( Interview *interview )
  {
    if( interview != this->ActiveInterview )
    {
      if( this->ActiveInterview ) this->ActiveInterview->UnRegister( this );
      this->ActiveInterview = interview;
      if( this->ActiveInterview )
      {
        this->ActiveInterview->Register( this );
        this->SetActiveImage( NULL );
      }

      // if there is an active user, save the active interview
      if( this->ActiveUser )
      {
        if( interview ) this->ActiveUser->Set( "InterviewId", interview->Get( "Id" ).ToInt() );
        else this->ActiveUser->SetNull( "InterviewId" );
        this->ActiveUser->Save();
      }
      this->Modified();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkCxxSetObjectMacro( Application, ActiveImage, Image );
}
