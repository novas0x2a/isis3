#ifndef QnetPointImagesFilter_h
#define QnetPointImagesFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  class QnetPointImagesFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetPointImagesFilter (QWidget *parent=0);
      virtual void filter();

    private:
      QRadioButton *p_lessThanRB;
      QRadioButton *p_greaterThanRB; 
      QLineEdit *p_imageEdit;
  };
};

#endif
