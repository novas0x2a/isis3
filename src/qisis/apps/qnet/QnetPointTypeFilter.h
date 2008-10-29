#ifndef QnetPointTypeFilter_h
#define QnetPointTypeFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPointTypeFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointTypeFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QRadioButton *p_ground;
      QRadioButton *p_ignore;
      QRadioButton *p_held;
  };
};

#endif
