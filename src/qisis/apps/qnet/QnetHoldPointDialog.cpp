#include <QtGui>

#include <QMessageBox>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>


#include <algorithm>

#include "QnetHoldPointDialog.h"
#include "SerialNumberList.h"
#include "ControlPoint.h"
#include "Camera.h"

#include "qnet.h"

using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {

  QnetHoldPointDialog::QnetHoldPointDialog (QWidget *parent) : QDialog (parent) {

    p_avg = new QRadioButton("Average Measures");
    p_avg->setChecked(true);
    p_select = new QRadioButton("Select Measures");
    p_select->setChecked(false);
    connect(p_avg,SIGNAL(clicked()),this,SLOT(selectMeasures()));
    connect(p_select,SIGNAL(clicked()),this,SLOT(selectMeasures()));

    QLabel *listLabel = new QLabel("Select Files:");

    p_fileList = new QListWidget();
    p_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    p_fileList->setEnabled(false);

    //  Create OK & Cancel buttons
    p_okButton = new QPushButton("OK");
    p_okButton->setDefault(true);
    p_okButton->setEnabled(true);
    QPushButton *cancelButton = new QPushButton("Cancel");
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(p_okButton);
    buttonLayout->addWidget(cancelButton);

    connect(p_okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(p_avg);
    vLayout->addWidget(p_select);
    vLayout->addWidget(listLabel);
    vLayout->addWidget(p_fileList);
    vLayout->addLayout(buttonLayout);

    setLayout(vLayout);
    setWindowTitle("Create Hold ControlPoint");

  }

  void QnetHoldPointDialog::setPoint (Isis::ControlPoint &point) {
    p_point = &point;

    //  Find all files for this point
    for (int i=0; i<p_point->Size(); i++) {
      QListWidgetItem *item = new QListWidgetItem(p_fileList);
      string fname = g_serialNumberList->Filename((*p_point)[i].CubeSerialNumber());
      item->setText(QString(fname.c_str()));
    }
  }



  void QnetHoldPointDialog::selectMeasures () {
    if (p_select->isChecked()) {
      p_fileList->setEnabled(true);
    }
    else {
      p_fileList->setEnabled(false);
    }
  }


  void QnetHoldPointDialog::accept () {

    if (p_avg->isChecked()) {
      p_point->ComputeApriori();
    }
    else {
      int i = p_fileList->currentRow();
      Isis::Camera *cam = (*p_point)[i].Camera();
      if (cam->SetImage((*p_point)[i].Sample(),(*p_point)[i].Line())) {
        double lat = cam->UniversalLatitude();
        double lon = cam->UniversalLongitude();
        double rad = cam->LocalRadius();
        p_point->SetUniversalGround(lat,lon,rad);
      }
      else {
        QString msg = "Cannot compute lat/lon for this control point " +
                      QString::fromStdString(p_point->Id()) + " pick another point.";
        QMessageBox::information((QWidget *)parent(),"Error",msg);
        return;
      }

    }

    p_point->SetHeld(true);
    emit holdPoint (*p_point);

    QDialog::accept();
  }

}

