#ifndef QnetPointIdFilter_h
#define QnetPointIdFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  /**
   * Defines the Point ID filter for the QnetNavTool's Points 
   * section.  The user must enter a string. This class is 
   * designed to remove points from the current filtered list 
   * whose PointId keyword values do not contain the string. 
   * 
   * @internal
   *   @history 2009-01-08 Jeannie Walldren - Modified filter()
   *                          method to remove new filter points
   *                          from the existing filtered list.
   *                          
   */

  class QnetPointIdFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointIdFilter (QWidget *parent=0);
      virtual void filter();

    private:
     QLineEdit *p_pointIdEdit;     
  };
};

#endif
