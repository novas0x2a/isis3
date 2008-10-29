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
