#ifndef Isis_GuiParameter_h
#define Isis_GuiParameter_h

#include <QObject>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include "iString.h"

namespace Isis {
  class UserInterface;
  class GuiParameter : public QObject {

    Q_OBJECT

    public:

      GuiParameter (QGridLayout *grid, UserInterface &ui, int group, int param);
      virtual ~GuiParameter ();

      //! Return the name of the parameter
      iString Name() const { return p_name; };

      void SetToDefault ();

      void SetToCurrent ();

      virtual iString Value () = 0;

      virtual void Set (iString newValue) = 0;

      void SetEnabled (bool enabled);

      //! Is the parameter enabled
      bool IsEnabled () const { return p_label->isEnabled(); }

      virtual bool IsModified();

      void Update();

      void RememberWidget(QWidget *w);

      QWidget *AddHelpers(QObject *lo);

      virtual std::vector<std::string> Exclusions();

      enum ParameterType { IntegerWidget, DoubleWidget, StringWidget,
                           ListWidget, FilenameWidget, CubeWidget,
                           BooleanWidget };
      ParameterType Type() { return p_type; };

    protected:
      int p_group;
      int p_param;
      iString p_name;
      UserInterface *p_ui;

      QLabel *p_label;

      QList<QWidget *> p_widgetList;

      ParameterType p_type;

    private:
      QMenu *p_helperMenu;

    signals:
      void ValueChanged();
      void HelperTrigger(const QString &);

  };
};

#endif
