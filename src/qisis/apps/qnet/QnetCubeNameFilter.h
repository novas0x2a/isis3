#ifndef QnetCubeNameFilter_h
#define QnetCubeNameFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetCubeNameFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetCubeNameFilter (QWidget *parent=0);
      virtual void filter();

    private:
     QLineEdit *p_cubeNameEdit;     
  };
};

#endif
