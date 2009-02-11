#ifndef QnetNewPointDialog_h
#define QnetNewPointDialog_h

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

#include <vector>

namespace Qisis {
  /**
   * @internal
   *   @history 2008-11-26 Jeannie Walldren - Added functionality
   *            to show the last Point ID entered into a new point
   *            dialog box.
   */
  class QnetNewPointDialog : public QDialog {
    Q_OBJECT


    public:
      static QString lastPtIdValue;

      QnetNewPointDialog (QWidget *parent=0);

      QLineEdit *ptIdValue;
      void SetFiles (std::vector<std::string> &pointFiles);

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


