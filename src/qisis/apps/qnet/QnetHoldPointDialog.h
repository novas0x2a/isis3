#ifndef QnetHoldPointDialog_h
#define QnetHoldPointDialog_h

#include <QDialog>

class QListWidget;
class QRadioButton;
class QPushButton;

#include "ControlPoint.h"

#include <vector>

namespace Qisis {
  /**
   * Dialog box to help user choose how to determine lat/lon/rad 
   * for selected hold point. 
   *
   * @internal
   *   @history 2008-12-29 Jeannie Walldren - Changed name from
   *            QnetGroundPointDialog
   */
  class QnetHoldPointDialog : public QDialog {
    Q_OBJECT

    public:
      QnetHoldPointDialog (QWidget *parent=0);
      void setPoint (Isis::ControlPoint &point);


    signals:
      void holdPoint (Isis::ControlPoint &point);
      
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




