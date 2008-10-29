#ifndef QnetPointRangeFilter_h
#define QnetPointRangeFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPointRangeFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointRangeFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QLineEdit *p_minlat;
      QLineEdit *p_maxlat;
      QLineEdit *p_minlon;
      QLineEdit *p_maxlon;
  };
};

#endif
