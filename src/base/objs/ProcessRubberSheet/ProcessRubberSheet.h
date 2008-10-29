#ifndef ProcessRubberSheet_h
#define ProcessRubberSheet_h
/**
 * @file
 * $Revision: 1.1.1.1 $
 * $Date: 2006/10/31 23:18:09 $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.
 *
 *   For additional information, launch 
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website, 
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */                                                                    

#include "Process.h"
#include "Buffer.h"
#include "Transform.h"
#include "Interpolator.h"
#include "Portal.h"
#include "TileManager.h"

namespace Isis {
/**                                                                       
 * @brief Derivative of Process, designed for geometric transformations                
 *                                                                        
 * This is the processing class for geometric transformations of cube data. 
 * Objects of this class can be used to apply rubber sheet transformations from 
 * one space to another, such as converting from one map projection to another 
 * or from instrument space to ground space. Each pixel position in the output 
 * cube will be processed by querying a transformer to find what input pixel  
 * should be used and an interpolator to find the value of the pixel. Any 
 * application using this class must supply a Transform object and an 
 * Interpolator object. This class allows only one input cube and one output 
 * cube.                                     
 *                                                                        
 * @ingroup HighLevelCubeIO
 *                                                                        
 * @author 2002-10-22 Stuart Sides                                                                           
 *                                                                        
 * @internal  
 *  @history 2003-02-13 Stuart Sides - Created a unit test for the object.
 *  @history 2003-03-31 Stuart Sides - Added false argument to IsisError.report  
 *                                     call in the unit test.
 *  @history 2003-05-16 Stuart Sides - Modified schema from astrogeology... 
 *                                     isis.astrogeology...
 *  @history 2003-05-28 Stuart Sides - Added a new member function to allow an  
 *                                     application to be notified when the 
 *                                     current band number of the output cube 
 *                                     changes (BandChange).
 *  @history 2003-10-23 Jeff Anderson - Modified StartProcess method to use a 
 *                                      quadtree mechanism to speed processing
 *                                      of cubes. Essentially it generates an 
 *                                      internalized file
 *  @history 2004-03-09 Stuart Sides - Modified quadtree mechanism to not give 
 *                                     up on a quad unless none of the edge
 *                                     pixels transform. This needs to be 
 *                                     refined even more.
 *  @history 2005-02-11 Elizabeth Ribelin - Modified file to support Doxygen 
 *                                          documentation
 *  @history 2006-03-28 Tracie Sucharski - Modified the StartProcess to
 *                                         process multi-band cubes using the
 *                                         same tile map if there is no Band
 *                                         dependent function.
 *  @history 2006-04-04 Tracie Sucharski - Added ForceTile method and two
 *                                         private variables to hold the 
 *                                         sample/line of the input cube position
 *                                         which will force tile containing that
 *                                         position to be processed in ProcessQuad
 *                                         even if all 4 corners of tile are bad.
 *                                                                        
 *  @todo 2005-02-11 Stuart Sides - finish documentation and add coded and  
 *                                 implementation example to class documentation 
 */                                                                       

  class ProcessRubberSheet : public Isis::Process {
    private:
      void (*p_bandChangeFunct) (const int band);

      double p_lineMap[128][128];
      double p_sampMap[128][128];

      double p_forceSamp;
      double p_forceLine;
                                                             
      typedef struct Quad {
        public:
          int slineTile;
          int ssampTile;
          int sline;
          int ssamp;
          int eline;
          int esamp;
      };
      
      void ProcessQuad (std::vector<Quad *> &quadTree, Isis::Transform &trans,
                        double lineMap[128][128], double sampMap[128][128]);
      void SplitQuad (std::vector<Quad *> &quadTree);
      void SlowQuad (std::vector<Quad *> &quadTree, Isis::Transform &trans,
                     double lineMap[128][128], double sampMap[128][128]);
      double Det4x4 (double m[4][4]);
      double Det3x3 (double m[3][3]);
  
      // SlowGeom method is never used but saved for posterity
      void SlowGeom (Isis::TileManager &otile, Isis::Portal &iportal, 
                     Isis::Transform &trans, Isis::Interpolator &interp);
      void QuadTree (Isis::TileManager &otile, Isis::Portal &iportal, 
                     Isis::Transform &trans, Isis::Interpolator &interp,
                     bool useLastTileMap);
    public:

      //! Constructs a RubberSheet object
      ProcessRubberSheet() {
        p_bandChangeFunct = NULL;
        p_forceSamp = Isis::Null;
        p_forceLine = Isis::Null;
      };

      //! Destroys the RubberSheet object.
      ~ProcessRubberSheet() {};
  
      // Line Processing method for one input and output cube
      void StartProcess (Isis::Transform &trans, Isis::Interpolator &interp);
  
      // Register a function to be called when the band number changes
      void BandChange (void (*funct)(const int band));

      void ForceTile (double Samp,double Line) {
        p_forceSamp = Samp;
        p_forceLine = Line;
      }
    };
};

#endif
