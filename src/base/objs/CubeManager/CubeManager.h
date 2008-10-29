#ifndef CubeManager_h
#define CubeManager_h

#include <QString>
#include <QMap>

/*
 *   Unless noted otherwise, the portions of Isis written by the
 *   USGS are public domain. See individual third-party library
 *   and package descriptions for intellectual property
 *   information,user agreements, and related information.
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
namespace Isis {
  class Cube;

  /**
   * @brief Class for quick re-accessing of cubes based on file name
   * 
   * This class holds cubes in static memory for reading.
   * This is helpful to prevent reading of the same cube many times. Files will remain
   * opened for reading, this is not for use with a cube that will ever be written
   * to. The pointers returned from this class are DANGEROUS, because they may be 
   * deleted at any time, and other objects may be manipulating them. Use this 
   * class sparingly. 
   * 
   * @author 2008-05-20 Steven Lambright 
   *  
   * @internal 
   *   @author 2008-06-19 Steven Lambright Added CleanUp methods
   *   @history 2008-08-19 Steven Koechle - Removed Geos includes
   */
  class CubeManager  {
    public:
      /**
       * This method calls the private method OpenCube() 
       *  
       * @see OpenCube 
       * 
       * @param cubeFilename Filename of the cube to be opened
       * 
       * @return Cube* Pointer to the cube (guaranteed not null)
       */
      static Cube *Open(const std::string &cubeFilename) { return p_instance.OpenCube(cubeFilename); }

      /**
       * This method calls CleanCubes(const std::string &cubeFilename) 
       *  
       * @see CleanCubes(const std::string &cubeFilename)
       * 
       * @param cubeFilename The filename of the cube to destroy from memory 
       */
      static void CleanUp(const std::string &cubeFilename) { p_instance.CleanCubes(cubeFilename); }

      /**
       * This method calls CleanCubes()
       *  
       * @see CleanCubes 
       */
      static void CleanUp() { p_instance.CleanCubes(); };

    protected:
      //! This is the constructor... nothing needs done
      CubeManager() {};
      ~CubeManager();

      void CleanCubes(const std::string &cubeFilename);
      void CleanCubes();

      //! There is always one instance of CubeManager around
      static CubeManager p_instance;

      Cube *OpenCube(const std::string &cubeFilename);

      //! This keeps track of the open cubes
      QMap<QString, Cube *> p_cubes;
  };
}

#endif
