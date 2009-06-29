#ifndef ManualStretch_h
#define ManualStretch_h

#include "QStretch.h"

#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>

namespace Qisis {
  /**
   * @brief Manual stretch to input stretch pairs manually
   *  
   * This QStretch contains parameters to allow the user to 
   * manually add, edit and delete stretch pairs. 
   *
   * @ingroup Visualization Tools 
   *
   * @author 2009-05-01 Noah hilt
   * 
   * 
   */

  class ManualStretch : public QStretch {
    Q_OBJECT

  public:
    ManualStretch(const StretchTool *stretchTool);
    ManualStretch *clone();
    void connectTable(QTableWidget *widget);
    void disconnectTable(QTableWidget *widget);
    QGroupBox *getParameters();
    void setMinMax(double min, double max);

  protected slots:
    void changeInput(bool percent);
    void changeCurrentPair(int row, int col);
    void addPair();
    void editPair();
    void deletePair();


  private:
    QTableWidget *p_table; //!< The table widget that belongs to the AdvancedStretchTool

    QPushButton *p_addButton; //!< QPushButton to add a pair
    QPushButton *p_editButton; //!< QPushButton to edit a pair
    QPushButton *p_deleteButton; //!< QPushButton to delete a pair
    QRadioButton *p_dnButton; //!< QRadioButton to select DN values as input
    QRadioButton *p_percentButton; //!< QRadioButton to select percent values as input

    QLineEdit *p_inputEdit; //!< QLineEdit for input values
    QLineEdit *p_outputEdit; //!< QLineEdit for output values

    int p_selectedPair; //!< The index of the selected pair in the table

    bool p_editing; //!< Boolean for editing mode
  };
};

#endif

