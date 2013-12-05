/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderDicomTagWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderDicomTagWidget_h
#define __QAlderDicomTagWidget_h

#include <QWidget>

class QAlderDicomTagWidgetPrivate;

class QAlderDicomTagWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  QAlderDicomTagWidget( QWidget* parent = 0 );
  virtual ~QAlderDicomTagWidget();

public Q_SLOTS:
  virtual void updateTableWidget( QString );

protected:
  QScopedPointer<QAlderDicomTagWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(QAlderDicomTagWidget);
  Q_DISABLE_COPY(QAlderDicomTagWidget);
};

#endif
