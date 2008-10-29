#ifndef QnetNewPointDialog_h
#define QnetNewPointDialog_h

#include <QDialog>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

#include <vector>

namespace Qisis {
  class QnetNewPointDialog : public QDialog {
    Q_OBJECT

    public:
      QnetNewPointDialog (QWidget *parent=0);
      void SetFiles (std::vector<std::string> &pointFiles);

      QLineEdit *ptIdValue;
      QListWidget *fileList;

    private:

      QLabel *p_ptIdLabel;
      QPushButton *p_okButton;

      std::vector<std::string> *p_pointFiles;

    private slots:
      void enableOkButton(const QString &text);

  };
};

#endif


