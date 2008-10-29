#ifndef QnetPolygonSizeFilter_h
#define QnetPolygonSizeFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPolygonSizeFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPolygonSizeFilter (QWidget *parent=0);
    virtual void filter();

    private:
      QRadioButton *p_lessThanRB; 
      QRadioButton *p_greaterThanRB;
      QLineEdit *p_lineEdit;
  };
};

#endif
