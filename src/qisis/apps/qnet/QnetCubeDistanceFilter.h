#ifndef QnetCubeDistanceFilter_h
#define QnetCubeDistanceFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetCubeDistanceFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetCubeDistanceFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QLineEdit *p_distEdit;
  };
};

#endif
