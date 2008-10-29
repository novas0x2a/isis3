#ifndef QnetCubePointsFilter_h
#define QnetCubePointsFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetCubePointsFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetCubePointsFilter (QWidget *parent=0);
      virtual void filter();

    private:
     QRadioButton *p_lessThanRB; 
     QRadioButton *p_greaterThanRB;
     QLineEdit *p_pointEdit;     
  };
};

#endif
