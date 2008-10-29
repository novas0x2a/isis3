#ifndef QnetPointErrorFilter_h
#define QnetPointErrorFilter_h
/**
 * @file
 * $Date: 2008/08/06 17:35:12 $
 * $Revision: 1.3 $
 *
 *  Unless noted otherwise, the portions of Isis written by the USGS are public domain. See
 *  individual third-party library and package descriptions for intellectual property information,
 *  user agreements, and related information.
 *
 *  Although Isis has been used by the USGS, no warranty, expressed or implied, is made by the
 *  USGS as to the accuracy and functioning of such software and related material nor shall the
 *  fact of distribution constitute any such warranty, and no responsibility is assumed by the
 *  USGS in connection therewith.
 *
 *  For additional information, launch $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *  in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *  http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *  http://www.usgs.gov/privacy.html.
 */



#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPointErrorFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointErrorFilter (QWidget *parent=0);
      virtual void filter();

    public slots:

    private:
      QCheckBox *p_lessThanCB; 
      QCheckBox *p_greaterThanCB;  
      QLineEdit *p_lessErrorEdit;
      QLineEdit *p_greaterErrorEdit;

    private slots:
      void clearEdit();
  };
};

#endif
