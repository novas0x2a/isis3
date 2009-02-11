#ifndef Qisis_TrackTool_h
#define Qisis_TrackTool_h

#include "Tool.h"

class QLabel;
class QStatusBar;
namespace Qisis {
  class TrackTool : public Tool {
    Q_OBJECT

    public:
      TrackTool (QStatusBar *parent);

    public slots:
      virtual void mouseMove(QPoint p);
      virtual void mouseLeave();

    protected:
      void addConnections(CubeViewport *cvp);
      void removeConnections(CubeViewport *cvp);

    private slots:
      void locateCursor ();

    private:
      void updateLabels(QPoint p);
      void clearLabels();

      QStatusBar *p_sbar;   //!< Status bar
      QLabel *p_sampLabel;  //!< Sample label
      QLabel *p_lineLabel;  //!< Line label
      QLabel *p_latLabel;   //!< Lat label
      QLabel *p_lonLabel;   //!< Lon label
      QLabel *p_grayLabel;  //!< Gray label
      QLabel *p_redLabel;   //!< Red label
      QLabel *p_grnLabel;   //!< Green label
      QLabel *p_bluLabel;   //!< Blue label
  };
};

#endif
