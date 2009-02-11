#ifndef QnetPointMeasureFilter_h
#define QnetPointMeasureFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  /**
   * Defines the Measure Type filter for the QnetNavTool's Points 
   * section.  The user may choose which types of measures the 
   * points contain. This class is designed to remove points from 
   * the current filtered list that do not contain any measures of 
   * the selected type. 
   * 
   * @internal
   *   @history 2009-01-08 Jeannie Walldren - Modified filter()
   *                          method to remove new filter points
   *                          from the existing filtered list.
   */
  class QnetPointMeasureFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointMeasureFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QCheckBox *p_unmeasured;
      QCheckBox *p_manual;
      QCheckBox *p_estimated;
      QCheckBox *p_automatic; 
      QCheckBox *p_validatedManual; 
      QCheckBox *p_validatedAutomatic;
  };
};

#endif
