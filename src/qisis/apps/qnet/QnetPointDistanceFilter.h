#ifndef QnetPointDistanceFilter_h
#define QnetPointDistanceFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPointDistanceFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointDistanceFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QLineEdit *p_lineEdit;
  };
};

#endif
