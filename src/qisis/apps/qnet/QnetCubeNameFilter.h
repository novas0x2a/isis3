#ifndef QnetCubeNameFilter_h
#define QnetCubeNameFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  /**
   * Defines the Name filter for the QnetNavTool's Cubes 
   * section.  The user must enter a string. This class is 
   * designed to remove cubes from the current filtered list 
   * whose filename does not contain the string. 
   * 
   * @internal
   *   @history 2009-01-08 Jeannie Walldren - Modified filter()
   *                          method to create new filtered list
   *                          from images in the existing filtered
   *                          list.
   */
  class QnetCubeNameFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetCubeNameFilter (QWidget *parent=0);
      virtual void filter();

    private:
     QLineEdit *p_cubeNameEdit;     
  };
};

#endif
