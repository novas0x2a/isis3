#ifndef Isis_GuiCubeParameter_h
#define Isis_GuiCubeParameter_h

#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include "GuiParameter.h"


namespace Isis {

//  class QTextEdit;

  class GuiCubeParameter : public GuiParameter {

    Q_OBJECT

    public:

      GuiCubeParameter (QGridLayout *grid, UserInterface &ui,
                        int group, int param);
      ~GuiCubeParameter ();

      iString Value ();

      void Set (iString newValue);

    private:
      QLineEdit *p_lineEdit;
      QToolButton *p_optButton;
      QMenu *p_menu;

    private slots:
      void SelectFile();
      void SelectAttribute();
      void ViewCube();
      void ViewLabel();
  };
};



#endif

