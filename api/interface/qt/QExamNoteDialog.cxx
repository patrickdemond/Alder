/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QExamNoteDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QExamNoteDialog.h"
#include "ui_QExamNoteDialog.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"

#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include <QAbstractButton>
#include <QPlainTextEdit>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QExamNoteDialog::QExamNoteDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QExamNoteDialog;
  this->ui->setupUi( this );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QExamNoteDialog::~QExamNoteDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QExamNoteDialog::close()
{
  // save whatever is in the text edit to the exam before closing
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;

    if( image->GetRecord( exam ) )
    {
      exam->Set( "Note", this->ui->plainTextEdit->toPlainText().toStdString() );
      exam->Save();
    }
  }

  QDialog::close();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QExamNoteDialog::updateInterface()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    vtkSmartPointer< Alder::Interview > interview;

    if( image->GetRecord( exam ) && exam->GetRecord( interview ) )
    {
      // set the title
      QString title = exam->Get( "Modality" ).ToString().c_str();
      title += " notes for ";
      title += interview->Get( "UId" ).ToString().c_str();
      title += " (";
      title += interview->Get( "VisitDate" ).ToString().c_str();
      title += ")";
      this->ui->label->setText( title );

      // set the text
      vtkVariant note = exam->Get( "Note" );
      if( note.IsValid() ) this->ui->plainTextEdit->setPlainText( note.ToString().c_str() );
    }
  }
}
