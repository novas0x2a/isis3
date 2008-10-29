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
