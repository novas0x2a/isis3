#include <QHBoxLayout>
#include <QFontMetrics>
#include <QFileDialog>

#include "GuiFilenameParameter.h"
#include "UserInterface.h"

namespace Isis {

  GuiFilenameParameter::GuiFilenameParameter(QGridLayout *grid, UserInterface &ui,
                                             int group, int param) :
  GuiParameter(grid, ui, group, param) {

    //  QHBoxLayout *lo = new QHBoxLayout;
    //  grid->addLayout(lo, param, 2);

    p_lineEdit = new QLineEdit;
    connect(p_lineEdit,SIGNAL(textChanged(const QString &)),this,SIGNAL(ValueChanged()));
    //  lo->addWidget(p_lineEdit);
    grid->addWidget(p_lineEdit,param,2);

    p_fileButton = new QToolButton ();
    grid->addWidget(p_fileButton,param,3);  
    std::string file = Filename("$ISIS3DATA/base/icons/view_tree.png").Expanded();
    p_fileButton->setIconSize(QSize(22,22));
    p_fileButton->setIcon(QPixmap((iString)file));
    p_fileButton->setToolTip("Select file");
    QString fileButtonWhatsThisText = "<p><b>Function:</b> \
            Opens a file chooser window to select a file from</p>"; 
    p_fileButton->setWhatsThis(fileButtonWhatsThisText); 
    connect(p_fileButton,SIGNAL(clicked(bool)),this,SLOT(SelectFile()));

    if (p_ui->HelpersSize(group,param) != 0) {
      grid->addWidget(AddHelpers(p_lineEdit),param,4);
    }

    RememberWidget(p_lineEdit);
    RememberWidget(p_fileButton);

    p_type = FilenameWidget;
  }


  GuiFilenameParameter::~GuiFilenameParameter() {}


  void GuiFilenameParameter::Set (iString newValue) {
    p_lineEdit->setText ((iString)newValue);
  }


  iString GuiFilenameParameter::Value () {
    return p_lineEdit->text().toStdString();
  }

  /**
   *
   * @internal
   * @history  2007-05-16 Tracie Sucharski - For files located in CWD, do
   *                           not include path in the lineEdit.
   * @history  2007-06-05 Steven Koechle - Corrected problem where
   *                           output Filename was being opened
   *                           not saved. Defaulted to output mode
   *                           if not specified.
   */
  void GuiFilenameParameter::SelectFile() {
    // What directory do we look in?
    QString dir;
    if ((p_lineEdit->text().length() > 0) &&
        (p_lineEdit->text().toStdString() != p_ui->ParamInternalDefault(p_group,p_param))) {
      Isis::Filename f(p_lineEdit->text().toStdString());
      dir = (QString)(iString)f.Expanded();
    } else if (p_ui->ParamPath(p_group,p_param).length() > 0) {
      Isis::Filename f(p_ui->ParamPath(p_group,p_param));
      dir = (QString)(iString)f.Expanded();
    }

    // Set up the filter
    QString filter = (iString)p_ui->ParamFilter(p_group,p_param);
    filter += ";;Any(*)";

    // Get the filename
    QString s;
    if (p_ui->ParamFileMode(p_group,p_param) == "input") {
      s = QFileDialog::getOpenFileName(p_fileButton,"Select file",dir,filter);
    } else {
      s = QFileDialog::getSaveFileName(p_fileButton,"Select file",dir,filter);
    }
    if (s != "") {
      Isis::Filename f(s.toStdString());
      if (f.absoluteDir() == QDir::currentPath()) {
        s = (QString)(iString)f.Name();
      }
      Set(s.toStdString());
    }
  }
}

