#ifndef QnetNewMeasureDialog_h
#define QnetNewMeasureDialog_h

#include <QDialog>

class QLabel;
class QListWidget;
class QPushButton;

#include "ControlPoint.h"

#include <vector>

namespace Qisis {
  class QnetNewMeasureDialog : public QDialog {
    Q_OBJECT

    public:
      QnetNewMeasureDialog (QWidget *parent=0);
      void SetFiles (const Isis::ControlPoint &point,
                     std::vector<std::string> &pointFiles);

      QListWidget *fileList;

    private:

      QPushButton *p_okButton;

      std::vector<std::string> *p_pointFiles;

    private slots:
      void enableOkButton(const QString &text);

  };
};

#endif


