#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "Chip.h"
#include "Cube.h"
#include "LineManager.h"
#include "Preference.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/MultiPolygon.h"

#include "Affine.h"
#include "SpecialPixel.h"

int main () {
  Isis::Preference::Preferences(true);
  Isis::Chip chip(51,50);
  std::cout << "Test basics" << std::endl;
  std::cout << chip.Samples() << std::endl;
  std::cout << chip.Lines() << std::endl;

  chip.TackCube(453.5,568.5);
  std::cout << chip.TackSample() << std::endl;
  std::cout << chip.TackLine() << std::endl;

  std::cout << "Test chip-to-cube and cube-to-chip mapping" << std::endl;
  chip.SetChipPosition(chip.TackSample(),chip.TackLine());
  std::cout << chip.CubeSample() << std::endl;
  std::cout << chip.CubeLine() << std::endl;

  chip.SetChipPosition(1.0,1.0);
  std::cout << chip.CubeSample() << std::endl;
  std::cout << chip.CubeLine() << std::endl;

  chip.SetCubePosition(chip.CubeSample(),chip.CubeLine());
  std::cout << chip.ChipSample() << std::endl;
  std::cout << chip.ChipLine() << std::endl;


  std::cout << "Test assignment of chip data to constant" << std::endl;
  chip.SetAllValues(10.0);
  for (int i=1; i<=chip.Lines(); i++) {
    for (int j=1; j<=chip.Samples(); j++) {
      double value = chip.GetValue(j,i);
      if (value != 10.0) {
        std::cout << "bad constant (!= 10) at " << j << "," << i << std::endl;
      }
    }
  }

  std::cout << "Test loading chip data" << std::endl;
  for (int i=1; i<=chip.Lines(); i++) {
    for (int j=1; j<=chip.Samples(); j++) {
      chip.SetValue(j,i,(double) (i*100 + j));
    }
  }

  for (int i=1; i<=chip.Lines(); i++) {
    for (int j=1; j<=chip.Samples(); j++) {
      double value = chip.GetValue(j,i);
      if (value != (double) (i*100 + j)) {
        std::cout << "bad at " << j << "," << i << std::endl;
      }
    }
  }




  chip.SetValidRange(0.0,5050.0);
  std::cout << "Valid tests" << std::endl;
  std::cout << chip.IsValid(chip.Samples(),chip.Lines()) << std::endl;
  std::cout << chip.IsValid(chip.Samples()-1,chip.Lines()) << std::endl;
  std::cout << chip.IsValid(95.0) << std::endl;
  std::cout << chip.IsValid(99.99) << std::endl;

  std::cout << "Extract test" << std::endl;
  Isis::Chip sub = chip.Extract(4,3,chip.TackSample(),chip.TackLine());
  for (int i=1; i<=sub.Lines(); i++) {
    for (int j=1; j<=sub.Samples(); j++) {
      std::cout << sub.GetValue(j,i) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Test writing chip" << std::endl;
  chip.Write("junk.cub");

  Isis::Cube junk;
  junk.Open("junk.cub");
  Isis::LineManager line(junk);

  for (int i=1; i<=chip.Lines(); i++) {
    line.SetLine(i);
    junk.Read(line);
    for (int j=1; j<=chip.Samples(); j++) {
      double value = chip.GetValue(j,i);
      if (value != line[j-1]) {
        std::cout << "bad at " << j << "," << i << std::endl;
      }
    }
  }

  std::cout << "Test load chip from cube with rotation" << std::endl;
  chip.TackCube(26.0,25.0);
  chip.Load(junk,45.0);
  for (int i=1; i<=chip.Lines(); i++) {
    for (int j=1; j<=chip.Samples(); j++) {
      std::cout << std::setw(14) << chip.GetValue(j,i) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Test load chip from cube with rotation and clipping polygon " << std::endl;
  chip.TackCube(26.0,25.0);

  geos::geom::CoordinateSequence *pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (23.0, 22.0));
  pts->add (geos::geom::Coordinate (28.0, 22.0));
  pts->add (geos::geom::Coordinate (28.0, 27.0));
  pts->add (geos::geom::Coordinate (25.0, 28.0));
  pts->add (geos::geom::Coordinate (23.0, 22.0));
  std::vector<geos::geom::Geometry *> polys;
  geos::geom::GeometryFactory gf;
  polys.push_back (gf.createPolygon (gf.createLinearRing (pts),NULL));
  geos::geom::MultiPolygon* mPolygon = gf.createMultiPolygon (polys);

  chip.SetClipPolygon(*mPolygon);
  chip.Load(junk,45.0);
  for (int i=1; i<=chip.Lines(); i++) {
    for (int j=1; j<=chip.Samples(); j++) {
      std::cout << std::setw(14) << chip.GetValue(j,i) << " ";
    }
    std::cout << std::endl;
  }

  // Test affine transformation
  std::cout << "\nTesting Affine transformation extraction (-1,-1)...\n";
  Isis::Affine affine;
  affine.Translate(-1.0, -1.0);

  Isis::Chip mychip(51,50);          //  Needed because chip has poly clipping
  mychip.TackCube(26.0,25.0);
  mychip.Load(junk);
  mychip.SetChipPosition(mychip.TackSample(), mychip.TackLine());
  std::cout << "Cube Sample, Line = " << mychip.CubeSample() << "," 
            << mychip.CubeLine() << std::endl;
  Isis::Chip shift(25,25);
  mychip.Extract(shift, affine);
  shift.SetChipPosition(shift.TackSample(), shift.TackLine());
  std::cout << "Shift Cube Sample, Line = " << shift.CubeSample() << "," 
            << shift.CubeLine() << std::endl;

  Isis::Chip io = shift;
  io.TackCube(25.0, 24.0);
  io.Load(junk);
  io.SetChipPosition(io.TackSample(), io.TackLine());
  std::cout << "New Cube Sample, Line = " << io.CubeSample() << "," 
            << io.CubeLine() << std::endl;

  int ioNull(0), shiftNull(0);
  double sumDiff(0.0);
  for (int il = 1 ; il <= io.Lines() ; il++) {
    for (int is = 1 ; is <= io.Samples() ; is++) {
      if (Isis::IsSpecial(io.GetValue(is,il))) {
        ioNull++;
      }
      else if (Isis::IsSpecial(shift.GetValue(is,il))) {
        shiftNull++;
      } 
      else {
        sumDiff += io.GetValue(is,il) - shift.GetValue(is,il);
      }
    }
  }

  std::cout << "I/O Nulls:   " << ioNull << std::endl;
  std::cout << "Shift Nulls: " << shiftNull << std::endl;
  std::cout << "Sum Diff:    " << sumDiff << std::endl;

  std::cout << "\nTesting direct Affine Application...\n";
  Isis::Chip affchip(25,25);
  affchip.TackCube(25.0, 24.0);
  affchip.setTransform(io.getTransform());
  affchip.SetChipPosition(affchip.TackSample(), affchip.TackLine());
  std::cout << "Affine Cube Sample, Line = " << affchip.CubeSample() << "," 
            << affchip.CubeLine() << std::endl;

  std::cout << "\nTest reading with new Affine transform...\n";
  affchip.Load(junk, io.getTransform());
  ioNull = shiftNull = 0;
  sumDiff = 0.0;
  for (int il = 1 ; il <= io.Lines() ; il++) {
    for (int is = 1 ; is <= io.Samples() ; is++) {
      if (Isis::IsSpecial(io.GetValue(is,il))) {
        ioNull++;
      }
      else if (Isis::IsSpecial(affchip.GetValue(is,il))) {
        shiftNull++;
      } 
      else {
        sumDiff += io.GetValue(is,il) - affchip.GetValue(is,il);
      }
    }
  }

  std::cout << "I/O Nulls:   " << ioNull << std::endl;
  std::cout << "Shift Nulls: " << shiftNull << std::endl;
  std::cout << "Sum Diff:    " << sumDiff << std::endl;

  affchip.SetChipPosition(affchip.TackSample(), affchip.TackLine());
  std::cout << "Affine Cube loaded at Sample, Line = " << affchip.CubeSample() << "," 
            << affchip.CubeLine() << std::endl;

  junk.Close(true);

#if 0
  try {
    junk.Open("/work2/janderso/moc/ab102401.lev1.cub");
    chip.TackCube(453.0,567.0);
    chip.Load(junk);
  
    Isis::Cube junk2;
    junk2.Open("/work2/janderso/moc/ab102402.lev0.cub");
    Isis::Chip chip2(75,70);
    chip2.TackCube(166.0,567.0);
    chip2.Load(junk2,chip);
  
    chip.Write("junk3.cub");
    chip2.Write("junk4.cub");
  }
  catch (Isis::iException &e) {
    e.Report();
  }
#endif

  return 0;
}
