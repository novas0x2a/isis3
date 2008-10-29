#ifndef Isis_GuiFilenameParameter_h
#define Isis_GuiFilenameParameter_h

#include <QLineEdit>
#include <QToolButton>

#include "GuiParameter.h"


namespace Isis {

//  class QTextEdit;

  class GuiFilenameParameter : public GuiParameter {

    Q_OBJECT

    public:

      GuiFilenameParameter (QGridLayout *grid, UserInterface &ui,
                        int group, int param);
      ~GuiFilenameParameter ();

      iString Value ();

      void Set (iString newValue);

    private:
      QLineEdit *p_lineEdit;
      QToolButton *p_fileButton;

    private slots:
      void SelectFile();

  };
};



#endif

