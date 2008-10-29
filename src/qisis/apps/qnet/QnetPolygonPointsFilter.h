#ifndef QnetPolygonPointsFilter_h
#define QnetPolygonPointsFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPolygonPointsFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPolygonPointsFilter (QWidget *parent=0);
    virtual void filter();

    private:
      QRadioButton *p_lessThanRB;
      QRadioButton *p_greaterThanRB; 
      QSpinBox *p_pointSpin;
  };
};

#endif
