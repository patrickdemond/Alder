/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QStudyNoteDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QStudyNoteDialog.h"
#include "ui_QStudyNoteDialog.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "Study.h"

#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include <QAbstractButton>
#include <QPlainTextEdit>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QStudyNoteDialog::QStudyNoteDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QStudyNoteDialog;
  this->ui->setupUi( this );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QStudyNoteDialog::~QStudyNoteDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QStudyNoteDialog::close()
{
  // save whatever is in the text edit to the study before closing
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    vtkSmartPointer< Alder::Study > study;

    if( image->GetRecord( exam ) && exam->GetRecord( study ) )
    {
      study->Set( "Note", this->ui->plainTextEdit->toPlainText().toStdString() );
      study->Save();
    }
  }

  QDialog::close();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QStudyNoteDialog::updateInterface()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    vtkSmartPointer< Alder::Study > study;
    vtkSmartPointer< Alder::Interview > interview;

    if( image->GetRecord( exam ) && exam->GetRecord( study ) && study->GetRecord( interview ) )
    {
      // set the title
      QString title = study->Get( "Modality" ).ToString().c_str();
      title += " notes for ";
      title += interview->Get( "UId" ).ToString().c_str();
      title += " (";
      title += interview->Get( "VisitDate" ).ToString().c_str();
      title += ")";
      this->ui->label->setText( title );

      // set the text
      vtkVariant note = study->Get( "Note" );
      if( note.IsValid() ) this->ui->plainTextEdit->setPlainText( note.ToString().c_str() );
    }
  }
}
