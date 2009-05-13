#include <sstream>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextEdit>
#include <QDialog>
#include <QString>
#include <QToolButton>
#include <QMenu>
#include <QDir>

#include "GuiCubeParameter.h"
#include "Filename.h"
#include "UserInterface.h"
#include "GuiInputAttribute.h"
#include "GuiOutputAttribute.h"
#include "Cube.h"
#include "System.h"
#include "iException.h"
#include "Pvl.h"
#include "Application.h"


namespace Isis {

  GuiCubeParameter::GuiCubeParameter(QGridLayout *grid, UserInterface &ui,
                                     int group, int param) :
  GuiParameter(grid, ui, group, param) {

    p_lineEdit = new QLineEdit;
    connect(p_lineEdit,SIGNAL(textChanged(const QString &)),this,SIGNAL(ValueChanged()));
    grid->addWidget(p_lineEdit,param,2);

    p_menu = new QMenu ();

    QAction *fileAction = new QAction(this);
    fileAction->setText("Select Cube");
    connect(fileAction,SIGNAL(triggered(bool)),this,SLOT(SelectFile()));
    p_menu->addAction(fileAction);

    QAction *attAction = new QAction(this);
    attAction->setText("Change Attributes ...");
    connect(attAction,SIGNAL(triggered(bool)),this,SLOT(SelectAttribute()));
    p_menu->addAction(attAction);

    QAction *viewAction = new QAction(this);
    viewAction->setText("View cube");
    connect(viewAction,SIGNAL(triggered(bool)),this,SLOT(ViewCube()));
    p_menu->addAction(viewAction);

    QAction *labAction = new QAction(this);
    labAction->setText("View labels");
    connect(labAction,SIGNAL(triggered(bool)),this,SLOT(ViewLabel()));
    p_menu->addAction(labAction);

    QAction *action = new QAction(this);
    std::string file = Filename("$ISIS3DATA/base/icons/view_tree.png").Expanded();
    action->setIcon(QPixmap((iString)file));
    connect(action,SIGNAL(triggered(bool)),this,SLOT(SelectFile()));

    p_optButton = new QToolButton ();
    p_optButton->setIconSize(QSize(22,22));
    p_optButton->setIcon(QPixmap((iString)file));
    p_optButton->setMenu(p_menu);
    p_optButton->setPopupMode(QToolButton::MenuButtonPopup);
    p_optButton->setDefaultAction(action);
    p_optButton->setToolTip("Select file");
    QString optButtonWhatsThisText = "<p><b>Function:</b> \
            Opens a file chooser window to select a file from</p> <p>\
            <b>Hint: </b> Click the arrow for more cube parameter options</p>"; 
    p_optButton->setWhatsThis(optButtonWhatsThisText); 
    grid->addWidget(p_optButton,param,3);

    if (p_ui->HelpersSize(group,param) != 0) {
      grid->addWidget(AddHelpers(p_lineEdit),param,4);
    }

    // Set up widgets for enabling and disabling
    RememberWidget(p_lineEdit);
    RememberWidget(p_optButton);

    p_type = CubeWidget;
  }

  GuiCubeParameter::~GuiCubeParameter() {
    delete p_menu;
  }

  void GuiCubeParameter::Set (iString newValue) {
    p_lineEdit->setText (newValue.c_str());
  }

  iString GuiCubeParameter::Value () {
    return p_lineEdit->text().toStdString();
  }

  /**
   * Gets an input/output file from a GUI filechooser or typed in
   * filename.
   * 
   * @internal
   * @history  2007-05-16 Tracie Sucharski - For cubes located in CWD, do
   *                           not include path in the lineEdit.
   * @history  2007-06-05 Steven Koechle - Corrected problem where
   *                           output cube was being opened not
   *                           saved.
   */
  void GuiCubeParameter::SelectFile() {
    // What directory do we look in?
    QString dir;
    if ((p_lineEdit->text().length() > 0) &&
        (p_lineEdit->text().toStdString() != p_ui->ParamInternalDefault(p_group,p_param))) {
      Isis::Filename f(p_lineEdit->text().toStdString());
      dir = (QString)(iString)f.Expanded();
    } else if (p_ui->ParamPath(p_group,p_param).length() > 0) {
      Isis::Filename f(p_ui->ParamPath(p_group,p_param));
      dir = (QString)(iString)Filename(p_ui->ParamPath(p_group,p_param)).Expanded();
    }

    // Set up the filter
    QString filter = (iString)p_ui->ParamFilter(p_group,p_param);
    if(filter.isEmpty()) {
      filter = "Any(*)";
    } else {
      filter += ";;Any(*)";
    }


    // Get the filename
    QString s;
    if (p_ui->ParamFileMode(p_group,p_param) == "input") {
      s = QFileDialog::getOpenFileName(p_optButton,"Select file",dir,filter);
    } else {
      s = QFileDialog::getSaveFileName(p_optButton,"Select file",dir,filter);
    }

    if (s != "") {
      Isis::Filename f(s.toStdString());
      if (f.absoluteDir() == QDir::currentPath()) {
        s = (QString)(iString)f.Name();
      }
      Set(s.toStdString());
    }
  }

  void GuiCubeParameter::SelectAttribute() {
    if (p_ui->ParamFileMode(p_group,p_param) == "input") {
      Isis::CubeAttributeInput att(p_lineEdit->text().toStdString());
      std::string curAtt = att.BandsStr();
      std::string newAtt;
      int status = GuiInputAttribute::GetAttributes (curAtt,newAtt,
                                                     p_ui->ParamName(p_group,p_param),
                                                     p_optButton);
      if ((status == 1) && (curAtt != newAtt)) {
        Isis::Filename f(p_lineEdit->text().toStdString());
        p_lineEdit->setText((iString)(f.Expanded() + newAtt));
      }
    } else {
      Isis::CubeAttributeOutput att("+" + p_ui->PixelType(p_group,p_param));
      bool allowProp = att.PropagatePixelType();
      att.Set(p_lineEdit->text().toStdString());

      std::string curAtt;
      att.Write(curAtt);
      std::string newAtt;
      int status = GuiOutputAttribute::GetAttributes (curAtt,newAtt,
                                                      p_ui->ParamName(p_group,p_param),
                                                      allowProp,
                                                      p_optButton);
      if ((status == 1) && (curAtt != newAtt)) {
        Isis::Filename f(p_lineEdit->text().toStdString());
        p_lineEdit->setText((iString)(f.Expanded() + newAtt));
      }
    }

    return;
  }

  void GuiCubeParameter::ViewCube() {
    try {
      // Make sure the user entered a value
      if (IsModified()) {
        std::string cubeName = Value();

        // Check to make sure the cube can be opened
        Isis::Cube temp;
        temp.Open(cubeName);
        temp.Close();

        // Open the cube in Qview
        std::string command = "qview " + cubeName + " &";
        Isis::System(command);
      }
      // Throw an error if no cube name was entered
      else {
        std::string msg = "You must enter a cube name to open";
        throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
      }
    } catch (Isis::iException &e) {
      Isis::iApp->GuiReportError(e);
    }
  }

  void GuiCubeParameter::ViewLabel() {
    try {
      // Make sure the user entered a value
      if (IsModified()) {
        std::string cubeName = Value();

        // Check to make sure the cube can be opened
        Isis::Cube temp;
        temp.Open(cubeName);

        // Get the label and write it out to the log
        Isis::Pvl *label = temp.Label();
        Isis::Application::GuiLog(*label);

        // Close the cube
        temp.Close();

      } else {
        std::string msg = "You must enter a cube name to open";
        throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
      }
    } catch (Isis::iException &e) {
      Isis::iApp->GuiReportError(e);
    }

  }
}

