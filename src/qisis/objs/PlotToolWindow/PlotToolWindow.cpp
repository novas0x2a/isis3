
#include "PlotToolWindow.h"

namespace Qisis {


   /**
   * This class was developed specifically to be used in conjuntion
   * with the PlotTool.  i.e. this is the PlotWindow for the
   * PlotTool.  This class handles items on the PlotWindow unique
   * to the PlotTool such as the vertical lines drawn on the plot
   * area called the band lines.
   */
  PlotToolWindow::PlotToolWindow (QString title,QWidget *parent) : 
    Qisis::PlotWindow(title, parent) {

    /*Plot line draws the vertical line(s) to indicate 
    which band(s) the user is looking at in the cube viewport*/
    p_grayBandLine = new QwtPlotMarker();
    p_redBandLine = new QwtPlotMarker();
    p_greenBandLine = new QwtPlotMarker();
    p_blueBandLine = new QwtPlotMarker();
    
    p_grayBandLine->setLineStyle(QwtPlotMarker::LineStyle(2));
    p_redBandLine->setLineStyle(QwtPlotMarker::LineStyle(2));
    p_greenBandLine->setLineStyle(QwtPlotMarker::LineStyle(2));
    p_blueBandLine->setLineStyle(QwtPlotMarker::LineStyle(2));

    QPen *pen = new QPen(Qt::white);
    pen->setWidth(1);
    p_grayBandLine->setLinePen(*pen);
    pen->setColor(Qt::red);
    p_redBandLine->setLinePen(*pen);
    pen->setColor(Qt::green);
    p_greenBandLine->setLinePen(*pen);
    pen->setColor(Qt::blue);
    p_blueBandLine->setLinePen(*pen);

    p_grayBandLine->hide();
    p_redBandLine->hide();
    p_greenBandLine->hide();
    p_blueBandLine->hide();

    p_grayBandLine->attach(this->p_plot);
    p_redBandLine->attach(this->p_plot);
    p_greenBandLine->attach(this->p_plot);
    p_blueBandLine->attach(this->p_plot);

    /*Setting the default to all lines hidden and all symbols visible.*/
    this->hideAllCurves();
    
  }

  /** 
   * This class needs to know which viewport the user is looking 
   * at so it can appropriately draw in the band lines. 
   * 
   * @param cvp
   */
  void PlotToolWindow::setViewport(CubeViewport *cvp){
    if(cvp == NULL) return;
    p_cvp = cvp;

  }

  /** 
   * This method actually draws in the vertical band line(s) on
   * the plot area.
   * 
   */
  void PlotToolWindow::drawBandMarkers(){
    if (p_cvp == NULL) return;
    if(!p_markersVisible) return;

    int redBand=0, greenBand=0, blueBand=0, grayBand=0;

    Isis::Cube *cube = p_cvp->cube();
    Isis::Pvl &pvl = *cube->Label();

    if (pvl.FindObject("IsisCube").HasGroup("BandBin")) {
      Isis::PvlGroup &bandBin = pvl.FindObject("IsisCube").FindGroup("BandBin");
      if (bandBin.HasKeyword("Center")) {
        p_wavelengths = bandBin.FindKeyword("Center");          
      }
    }
 
    if(p_cvp->isColor()) {
      redBand = p_cvp->redBand();
      greenBand = p_cvp->greenBand();
      blueBand = p_cvp->blueBand();
    } else {
      grayBand = p_cvp->grayBand();
    }


    /*This is were we need to set the x value to the band number.*/
    if(grayBand > 0) {
      if(p_plotType == "Wavelength") {
        p_grayBandLine->setXValue(p_wavelengths[grayBand-1]);
        
      }else{
        p_grayBandLine->setXValue(grayBand);
      }
      p_grayBandLine->show();
    } else {
      p_grayBandLine->hide();
    }

    if(redBand > 0) {

      if(p_plotType == "Wavelength") {
        p_redBandLine->setXValue(p_wavelengths[redBand-1]);
      }else{
          p_redBandLine->setXValue(redBand);
      }
      p_redBandLine->show();

    }else {
      p_redBandLine->hide();
   }

    if(greenBand > 0) {

      if(p_plotType == "Wavelength") {
        p_greenBandLine->setXValue(p_wavelengths[greenBand-1]);
      }else{
        p_greenBandLine->setXValue(greenBand);
      }
      p_greenBandLine->show();

    }else {
      p_greenBandLine->hide();
    }

    if(blueBand > 0) {

      if(p_plotType == "Wavelength") {
        p_blueBandLine->setXValue(p_wavelengths[blueBand-1]);
      }else{
        p_blueBandLine->setXValue(blueBand);
      }
      p_blueBandLine->show();

    }else {
      p_blueBandLine->hide();
    }

    this->p_plot->replot();

  }

  /** 
   * 
   * 
   * @param visible
   */
  void PlotToolWindow::setBandMarkersVisible(bool visible){
    p_markersVisible = visible;
  }

  /** 
   * 
   * 
   * 
   * @return bool
   */
  bool PlotToolWindow::bandMarkersVisible(){
    return p_markersVisible;
  }

  /** 
   * This method is called from PlotTool.  This enables the users
   * to hide or show the vertical line(s) on the plot which
   * represent the color bands or the black/white band.
   * 
   */
  void PlotToolWindow::showHideLines(){
    if(p_cvp->isColor()) {
      p_blueBandLine->setVisible(!p_markersVisible);
      p_redBandLine->setVisible(!p_markersVisible);
      p_greenBandLine->setVisible(!p_markersVisible);
    }else {
      p_grayBandLine->setVisible(!p_markersVisible);
    }

    p_markersVisible = !p_markersVisible;

    this->p_plot->replot();
  }

  /** 
   * 
   * 
   * @param plotType
   */
  void PlotToolWindow::setPlotType(QString plotType){
    p_plotType = plotType;
    
  }

  /** 
   * 
   * 
   * @param autoScale
   */
  void PlotToolWindow::setAutoScaleOption(bool autoScale){
    p_autoScale = autoScale;
  }

  /**
   * Fills in the table with the data from the current curves
   * in the plotWindow  -- overridden function from PlotWindow
   */ 
  void PlotToolWindow::fillTable(){
    if(p_tableWindow == NULL) return;
	
    p_axisTitle = p_plot->axisTitle(QwtPlot::xBottom).text().toStdString();

    if(p_axisTitle == "Band" || p_axisTitle == "Wavelength") {
      fillTableSpectral();
    } else {
      p_tableWindow->table()->setColumnCount(0);
      fillTableSpatial();
	  
    }

  }

  /** 
   * 
   * 
   */
  void PlotToolWindow::fillTableSpatial(){
    PlotWindow::fillTable();
  }

  /** 
   * 
   * 
   */
  void PlotToolWindow::fillTableSpectral(){
    /*resize rows if needed*/
    unsigned int rows = p_tableWindow->table()->rowCount();
	
    if(rows != p_plotCurves[p_plotCurves.size()-1]->data().size()) {
      int diff = p_plotCurves[p_plotCurves.size()-1]->data().size() - rows;
      if(diff > 0) p_tableWindow->table()->setRowCount(diff);
	  
      for(int i = 1; i <= abs(diff); i++){
	
        if(diff > 0){
          p_tableWindow->table()->insertRow(rows + i);
        } else {
          p_tableWindow->table()->removeRow(rows - i);
        } 

      }
    }

    //write X axis
    if(p_tableWindow->table()->columnCount() == 0){

      p_tableWindow->addToTable (true,p_plot->axisTitle(QwtPlot::xBottom).text()
                                 ,p_plot->axisTitle(QwtPlot::xBottom).text(),0);

      p_header = p_tableWindow->table()->horizontalHeaderItem(0)->text().toStdString();

      /* Add min/max/average/sigma*/
      //min
      p_tableWindow->addToTable (true,p_plotCurves[2]->title().text(),
                                       p_plotCurves[2]->title().text());
      //max
      p_tableWindow->addToTable (true,p_plotCurves[3]->title().text(),
                                       p_plotCurves[3]->title().text());

      //average
      p_tableWindow->addToTable (true,p_plotCurves[4]->title().text(),
                                       p_plotCurves[4]->title().text());
      //sigma
      p_tableWindow->addToTable (true,"Sigma","Sigma");
	        
            
    } else if(p_header.compare(p_axisTitle)) {

        p_tableWindow->table()->removeColumn(0);
        p_tableWindow->listWidget()->takeItem(0);
        p_tableWindow->addToTable (true,p_plot->axisTitle(QwtPlot::xBottom).text()
                                   ,p_plot->axisTitle(QwtPlot::xBottom).text(), 0);
        p_header = p_tableWindow->table()->horizontalHeaderItem(0)->text().toStdString();

    }

    for(int c = 0; c< p_plotCurves.size(); c++){  
      
      /*rows*/
      for(unsigned int r = 0; r<p_plotCurves[c]->data().size(); r++){  
        /*creates a widget item with the data from the curves.*/

        if(!p_plotCurves[c]->title().text().contains("Sigma")){

          QTableWidgetItem *item = new QTableWidgetItem
                            (Isis::iString(p_plotCurves[c]->data().y(r)).ToQt());
          p_tableWindow->table()->setItem(r, c-1, item);
		  
  
          QTableWidgetItem *xAxisItem = new QTableWidgetItem
                            (Isis::iString(p_plotCurves[c]->data().x(r)).ToQt());
          p_tableWindow->table()->setItem(r, 0, xAxisItem);

        }

      }/*end rows */
      

    }/*end columns*/

    /*Adding the std. dev. to the table.*/
    for(unsigned int s = 0; s < p_stdDevArray.size(); s++) {
      QTableWidgetItem *item = new QTableWidgetItem (QString::number (p_stdDevArray[s]));
      p_tableWindow->table()->setItem(s, 4, item);

    }
  }

  /** 
   * Gives us access to the standard deviation array so we can 
   * display it in the table. 
   * 
   * @param stdDevArray
   */
  void PlotToolWindow::setStdDev (std::vector<double> stdDevArray){
    p_stdDevArray = stdDevArray;
  }

}




