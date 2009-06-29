#include "ManualStretch.h"

#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>

#include "StretchTool.h"

namespace Qisis {
  /**
   * ManualStretch Constructor. StretchTool can not be null. 
   *  
   * Initializes the stretch, QButtons, and sets the name of 
   * this QStretch to be 'Manual' 
   * 
   * @param stretchTool 
   */
  ManualStretch::ManualStretch(const StretchTool *stretchTool) : QStretch(stretchTool, "Manual") {
     p_editButton = new QPushButton("Edit");
     p_deleteButton = new QPushButton("Delete");
     p_editButton->setEnabled(false);
     p_deleteButton->setEnabled(false);

     connect(p_editButton, SIGNAL(clicked()), this, SLOT(editPair()));
     connect(p_deleteButton, SIGNAL(clicked()), this, SLOT(deletePair()));
  }

  /**
   * Creates a new ManualStretch with this ManualStretch's parameters 
   * and returns it.
   * 
   * 
   * @return ManualStretch* 
   */
  ManualStretch *ManualStretch::clone() {
    ManualStretch *stretch = new ManualStretch(p_stretchTool);
    stretch->setMinMax(p_min, p_max);
    stretch->p_stretch = this->p_stretch;
    return stretch;
  }

  /**
   * Returns the QGroupBox holding parameters.
   * 
   * 
   * @return QGroupBox* 
   */
  QGroupBox *ManualStretch::getParameters() {
    //If the parameters box hasn't been created yet, create it
    if(p_parametersBox == NULL) {
      p_parametersBox = new QGroupBox("Manual Parameters");
  
      QLabel *inputLabel = new QLabel("Input");
      QLabel *outputLabel = new QLabel("Output");
      p_dnButton = new QRadioButton("DN");
      p_percentButton = new QRadioButton("%");
      connect(p_percentButton, SIGNAL(toggled(bool)), this, SLOT(changeInput(bool)));

      p_dnButton->setChecked(true);
      p_inputEdit = new QLineEdit();
      p_outputEdit = new QLineEdit();
      p_addButton = new QPushButton("Add");
      connect(p_addButton, SIGNAL(clicked()), this, SLOT(addPair()));
  
      QGridLayout *layout = new QGridLayout();
      layout->addWidget(inputLabel, 0, 0, 1, 2);
      layout->addWidget(outputLabel, 0, 2, 1, 2);
      layout->addWidget(p_dnButton, 1, 0);
      layout->addWidget(p_percentButton, 1, 1);
      layout->addWidget(p_inputEdit, 2, 0, 1, 2);
      layout->addWidget(p_outputEdit, 2, 2, 1, 2);
      layout->addWidget(p_addButton, 3, 2, 1, 2);
      layout->addWidget(p_editButton, 4, 0, 1, 2);
      layout->addWidget(p_deleteButton, 4, 2, 1, 2);
      layout->setRowStretch(5, 1);
      p_parametersBox->setLayout(layout);
    }

    return p_parametersBox;
  }

  /**
   * Sets the min/max.
   * 
   * @param min 
   * @param max 
   */
  void ManualStretch::setMinMax(double min, double max) {
    QStretch::setMinMax(min, max);
    if(p_stretch.Pairs() == 0) {
      p_stretch.AddPair(min, 0);
      p_stretch.AddPair(max, 255);
    }
    emit(update());
  }

  /**
   * Sets the table widget's selection mode so that pairs can be selected. 
   *  
   * This method is called when this is set to be the active stretch. 
   * 
   * @param widget 
   */
  void ManualStretch::connectTable(QTableWidget *widget) {
    p_table = widget;
    p_table->setSelectionMode(QAbstractItemView::SingleSelection);
    p_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(p_table, SIGNAL(cellClicked(int, int)), this, SLOT(changeCurrentPair(int, int)));
  }

  /**
   * Sets the table widget's selection mode so that pairs can not be selected. 
   *  
   * This method is called when this is the active stretch and is deactivated.
   * 
   * @param widget 
   */
  void ManualStretch::disconnectTable(QTableWidget *widget) {
    disconnect(p_table, SIGNAL(cellClicked(int, int)), this, SLOT(changeCurrentPair(int, int)));
    widget->setSelectionMode(QAbstractItemView::NoSelection);
    widget->setSelectionBehavior(QAbstractItemView::SelectItems);
  }

  /**
   * This method is called when the radio button changes between DN and percent 
   * and will translate the input value accordingly.
   * 
   * @param percent 
   */
  void ManualStretch::changeInput(bool percent) {
    //There is no input so return
    if(p_inputEdit->text().isEmpty()) return;

    bool ok;
    double input = p_inputEdit->text().toDouble(&ok);
    //The input could not be converted to a double
    if(!ok) return;

    if(percent) {
      input = p_stretchTool->getHistogramPercent(input);
    }
    else {
      input = p_stretchTool->getHistogramDN(input);
    }

    p_inputEdit->setText(QString("%1").arg(input));
  }

  /**
   * When an item has been selected in the table widget, select the row and set 
   * the current pair to that row.
   * 
   * @param row 
   * @param col 
   */
  void ManualStretch::changeCurrentPair(int row, int col) {
    //If the row is valid, set the selected row and enable edit/delete buttons
    if(row >= 0) {
      p_editButton->setEnabled(true);
      p_deleteButton->setEnabled(true);
  
      p_table->selectRow(row);
  
      p_selectedPair = row;
    }
    //Otherwise disable them
    else {
      p_editButton->setEnabled(false);
      p_deleteButton->setEnabled(false);

      p_selectedPair = -1;
    }

    //If we were editing, make sure to set the mode back to add
    if(p_editing) {
      p_inputEdit->clear();
      p_outputEdit->clear();
      p_editing = false;
      p_addButton->setText("Add");
    }
  }

  /**
   * Check input and output for validity and adds a new pair if valid. 
   * 
   */
  void ManualStretch::addPair() {
    //There is no input/output so return
    if(p_inputEdit->text().isEmpty() || p_outputEdit->text().isEmpty()) return;

    bool ok;
    double input = p_inputEdit->text().toDouble(&ok);
    double output = p_outputEdit->text().toDouble(&ok);

    //If the input/output fields can not be converted to doubles, return
    if(!ok) return;

    //If the input mode is percent, check for validity and get the percentage from the AdvancedStretchTool
    if(p_percentButton->isChecked()) {
      //Percent must be between 0 and 100
      if(input < 0.0 || input > 100.0) {
        QMessageBox::critical(p_stretchTool->getDialog(), "Warning", "Percent value must be between 0 and 100.");
        p_inputEdit->clear();
        return;
      }

      input = p_stretchTool->getHistogramDN(input);
    }

    //If there are no pairs, just add it
    if(p_stretch.Pairs() == 0) {
      p_stretch.AddPair(input, output);
    }
    //Otherwise we need to insert the pair into the correct index
    else {
      bool added = false;
      QList<QPair<double, double> > oldPairs;

      //If editing, just replace the pair to be edited with the new one
      if(p_editing) {
        for(int i = 0; i < p_stretch.Pairs(); i++) {
          if(i == p_selectedPair) {
            oldPairs.append(qMakePair(input, output));
          }
          else {
            oldPairs.append(qMakePair(p_stretch.Input(i), p_stretch.Output(i)));
          }
        }

        p_editing = false;
        p_addButton->setText("Add");
      }
      //Otherwise search for a valid index and insert new pair
      else {
        for(int i = 0; i < p_stretch.Pairs(); i++) {
          if(!added && input < p_stretch.Input(i)) {
            added = true;
            oldPairs.append(qMakePair(input, output));
          }
  
          if(!added && input == p_stretch.Input(i)) {
            added = true;
            oldPairs.append(qMakePair(input, output));
          }
          else {
            oldPairs.append(qMakePair(p_stretch.Input(i), p_stretch.Output(i)));
          }
        }
        if(!added) {
          oldPairs.append(qMakePair(input, output));
        }
      }
  
      //Clear the old pairs and add the new sorted pairs
      p_stretch.ClearPairs();
      for(int i = 0; i < oldPairs.size(); i++) {
        p_stretch.AddPair(oldPairs.at(i).first, oldPairs.at(i).second);
      }
    }

    //Clear the input/output fields and emit an update
    p_inputEdit->clear();
    p_outputEdit->clear();
    emit(update());
  }

  /**
   * Edit the selected pair - populates the input/output fields with the pair's
   * data and sets the mode to edit.
   * 
   */
  void ManualStretch::editPair() {
    QTableWidgetItem *inputItem = p_table->item(p_selectedPair, 0);
    QTableWidgetItem *outputItem = p_table->item(p_selectedPair, 1);

    if(p_percentButton->isChecked()) {
      double input = inputItem->text().toDouble();
      input = p_stretchTool->getHistogramPercent(input);

      p_inputEdit->setText(QString("%1").arg(input));
    }
    else {
      p_inputEdit->setText(inputItem->text());
    }

    p_outputEdit->setText(outputItem->text());

    p_editing = true;
    p_addButton->setText("Ok");
  }

  /**
   * Deletes the selected pair.
   * 
   */
  void ManualStretch::deletePair() {
    p_table->removeRow(p_selectedPair);

    QList<QPair<double, double> > oldPairs;
    for(int i = 0; i < p_stretch.Pairs(); i++) {
      if(i == p_selectedPair) continue;
      oldPairs.append(qMakePair(p_stretch.Input(i), p_stretch.Output(i)));
    }

    p_stretch.ClearPairs();

    for(int i = 0; i < oldPairs.count(); i++) {
      p_stretch.AddPair(oldPairs.at(i).first, oldPairs.at(i).second);
    }

    //If we were editing, make sure to set the mode back to add
    if(p_editing) {
      p_inputEdit->clear();
      p_outputEdit->clear();
      p_editing = false;
      p_addButton->setText("Add");
    }

    changeCurrentPair(p_table->currentRow(), 0);

    emit(update());
  }
}
