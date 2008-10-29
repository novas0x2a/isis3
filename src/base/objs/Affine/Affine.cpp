/**                                                                       
 * @file                                                                  
 * $Revision: 1.4 $                                                             
 * $Date: 2008/06/18 17:03:49 $                                                                 
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

#include <vector>
#include <iostream>
#include <string>
#include "jama_svd.h"

#include "Affine.h"
#include "PolynomialBivariate.h"
#include "LeastSquares.h"
#include "iException.h"
#include "Constants.h"

namespace Isis {
  /**
   * Constructs an Affine transform.  The default transform is 
   * the identity. 
   */
  Affine::Affine () {
    Identity();
  }

  //! Destroys the Affine object
  Affine::~Affine() {}

  /**
   * Set the forward and inverse affine transform to the identity. 
   * That is, xp = x and yp = y for all (x,y).
   */
  void Affine::Identity() {
    TNT::Array2D<double> ident(3,3);
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        ident[i][j] = 0.0;
      }
      ident[i][i] = 1.0;
    }

    p_matrix = ident;
    p_invmat = ident;
  }

  /** 
   * Given a set of coordinate pairs (n >= 3), compute the affine
   * transform that best fits the points.  If given exactly three 
   * coordinates that are not colinear, the fit will be guarenteed
   * to be exact through the points. 
   *  
   * @param x  The transformation x coordinates
   * @param y  The transformation y coordinates
   * @param xp The transformation xp coordinates
   * @param yp The transformation yp coordinates
   * @param n  The number of coordiante pairs
   * 
   * @throws Isis::iException::Math - Affine transform not invertible
   */
  void Affine::Solve (const double x[], const double y[], 
                      const double xp[], const double yp[], int n) {
    // We must solve two least squares equations
    PolynomialBivariate xpFunc(1);
    PolynomialBivariate ypFunc(1);
    LeastSquares xpLSQ(xpFunc);
    LeastSquares ypLSQ(ypFunc);

    // Push the knowns into the least squares class
    for (int i=0; i<n; i++) {
      std::vector<double> coord(2);
      coord[0] = x[i];
      coord[1] = y[i];
      xpLSQ.AddKnown(coord,xp[i]);
      ypLSQ.AddKnown(coord,yp[i]);
    }

    // Solve for A,B,C,D,E,F
    xpLSQ.Solve();
    ypLSQ.Solve();

    // Construct our affine matrix
    p_matrix[0][0] = xpFunc.Coefficient(1); // A
    p_matrix[0][1] = xpFunc.Coefficient(2); // B
    p_matrix[0][2] = xpFunc.Coefficient(0); // C
    p_matrix[1][0] = ypFunc.Coefficient(1); // D
    p_matrix[1][1] = ypFunc.Coefficient(2); // E
    p_matrix[1][2] = ypFunc.Coefficient(0); // F
    p_matrix[2][0] = 0.0;
    p_matrix[2][1] = 0.0;
    p_matrix[2][2] = 1.0;

    // Now compute the inverse affine matrix using singular value
    // decomposition A = USV'.  So invA = V invS U'.  Where ' represents
    // the transpose of a matrix and invS is S with the recipricol of the
    // diagonal elements
    JAMA::SVD<double> svd(p_matrix);

    TNT::Array2D<double> V;
    svd.getV(V);

    // The inverse of S is 1 over each diagonal element of S
    TNT::Array2D<double> invS;
    svd.getS(invS);
    for (int i=0; i<invS.dim1(); i++) {
      if (invS[i][i] == 0.0) {
        std::string msg = "Affine transform not invertible";
        throw iException::Message(iException::Math,msg,_FILEINFO_);
      }
      invS[i][i] = 1.0 / invS[i][i];
    }

    // Transpose U
    TNT::Array2D<double> U;
    svd.getU(U);
    TNT::Array2D<double> transU(U.dim2(),U.dim1());
    for (int r=0; r<U.dim1(); r++) {
      for (int c=0; c<U.dim2(); c++) {
        transU[c][r] = U[r][c];
      }
    }

    // Multiply stuff together to get the inverse of the affine
    TNT::Array2D<double> VinvS = TNT::matmult(V,invS);
    p_invmat = TNT::matmult(VinvS,transU);
  }

  /**
   * Apply a translation to the current affine transform
   * 
   * @param tx translatation to add to x'
   * @param ty translation to add to y'
   */
  void Affine::Translate (double tx, double ty) {
    TNT::Array2D<double> trans(3,3);
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        trans[i][j] = 0.0;
      }
      trans[i][i] = 1.0;
    }
    trans[0][2] = tx;
    trans[1][2] = ty;
    p_matrix = TNT::matmult(trans,p_matrix);
    
    trans[0][2] = -tx;
    trans[1][2] = -ty;
    p_invmat = TNT::matmult(p_invmat,trans);
  }

  /**
   * Apply a translation to the current affine transform
   * 
   * @param angle degrees of counterclockwise rotation
   */
  void Affine::Rotate(double angle) {
    TNT::Array2D<double> rot(3,3);
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        rot[i][j] = 0.0;
      }
      rot[i][i] = 1.0;
    }

    double angleRadians = angle * Isis::PI / 180.0;
    rot[0][0] = cos(angleRadians);
    rot[0][1] = -sin(angleRadians);
    rot[1][0] = sin(angleRadians);
    rot[1][1] = cos(angleRadians);
    p_matrix = TNT::matmult(rot,p_matrix);

    angleRadians = -angleRadians;
    rot[0][0] = cos(angleRadians);
    rot[0][1] = -sin(angleRadians);
    rot[1][0] = sin(angleRadians);
    rot[1][1] = cos(angleRadians);
    p_invmat = TNT::matmult(p_invmat,rot);
  }

  /**
   * Apply a scale to the current affine transform
   * 
   * @param scaleFactor The scale factor
   */
  void Affine::Scale (double scaleFactor) {
    TNT::Array2D<double> scale(3,3);
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        scale[i][j] = 0.0;
      }
      scale[i][i] = 1.0;
    }
    scale[0][0] = scaleFactor;
    scale[1][1] = scaleFactor;
    p_matrix = TNT::matmult(scale,p_matrix);
    
    scale[0][0] = scaleFactor;
    scale[1][1] = scaleFactor;
    p_invmat = TNT::matmult(p_invmat,scale);
  }

  /**
   * Compute (xp,yp) given (x,y).  Use the methods xp() and yp() to
   * obtain the results. 
   *  
   * @param x The transformation x factor
   * @param y The transformation y factor
   */
  void Affine::Compute(double x, double y) {
    p_x = x;
    p_y = y;
    p_xp = p_matrix[0][0] * x + p_matrix[0][1] * y + p_matrix[0][2];
    p_yp = p_matrix[1][0] * x + p_matrix[1][1] * y + p_matrix[1][2];
  }

  /**
   * Compute (x,y) given (xp,yp).  Use the methods x() and y() to
   * obtain the results. 
   *  
   * @param xp The inverse transformation xp factor
   * @param yp The inverse transformation yp factor
   */
  void Affine::ComputeInverse(double xp, double yp) {
    p_xp = xp;
    p_yp = yp;
    p_x = p_invmat[0][0] * xp + p_invmat[0][1] * yp + p_invmat[0][2];
    p_y = p_invmat[1][0] * xp + p_invmat[1][1] * yp + p_invmat[1][2];
  }
   
  /**
   * Return the affine coeffients for the entered variable (1 or 2).  The coefficients
   * are returned in a 3-dimensional vector 
   *  
   * @param var The coefficient vector index (1 or 2)
   */
  vector<double> Affine::Coefficients( int var ) {
    int index = var-1;
    vector <double> coef;
    coef.push_back(p_matrix[index][0]);
    coef.push_back(p_matrix[index][1]);
    coef.push_back(p_matrix[index][2]);
    return coef;
  }

  /**
   * Return the inverse affine coeffients for the entered variable (1 or 2).
   * The coefficients are returned in a 3-dimensional vector 
   *  
   * @param var The inverse coefficient vector index
   */
  vector<double> Affine::InverseCoefficients( int var ) {
     int index = var-1;
     vector <double> coef;
     coef.push_back(p_invmat[index][0]);
     coef.push_back(p_invmat[index][1]);
     coef.push_back(p_invmat[index][2]);
     return coef;
   }
} // end namespace isis
