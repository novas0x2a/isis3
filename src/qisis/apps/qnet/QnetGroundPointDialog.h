#ifndef QnetGroundPointDialog_h
#define QnetGroundPointDialog_h

#include <QDialog>

class QListWidget;
class QRadioButton;
class QPushButton;

#include "ControlPoint.h"

#include <vector>

namespace Qisis {
  class QnetGroundPointDialog : public QDialog {
    Q_OBJECT

    public:
      QnetGroundPointDialog (QWidget *parent=0);
      void setPoint (Isis::ControlPoint &point);


    signals:
      void groundPoint (Isis::ControlPoint &point);
      
    private:

      QRadioButton *p_avg;
      QRadioButton *p_select;
      QListWidget *p_fileList;

      QPushButton *p_okButton;

      Isis::ControlPoint *p_point;

    private slots:
      void selectMeasures ();
      void accept ();
  };
};

#endif




