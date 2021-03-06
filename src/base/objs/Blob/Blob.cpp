/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2009/02/18 16:21:50 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include <fstream>
#include <sstream>
#include <cstring>
#include "Blob.h"
#include "Pvl.h"
#include "Filename.h"
#include "iException.h"
#include "iException.h"
#include "iException.h"
#include "Message.h"

using namespace std;
namespace Isis {
 /**
  * Constructs a Blob object using a name and type.
  *
  * @param name The blob name
  * @param type The blob type
  */
  Blob::Blob (const std::string &name, const std::string &type) {
    p_blobName = name;
    p_buffer = NULL;
    p_labelFile = "";
    p_nbytes = 0;
    p_type = type;

    p_blobPvl.SetName(p_type);
    p_blobPvl += Isis::PvlKeyword("Name",p_blobName);
    p_blobPvl += Isis::PvlKeyword("StartByte",0);
    p_blobPvl += Isis::PvlKeyword("Bytes",0);
  }

 /**
  * Constructs a Blob object using a name, type, and reading Pvl values from a
  * file.
  *
  * @param name The blob name
  * @param type The blob type
  * @param file The filename to read from.
  */
  Blob::Blob (const std::string &name, const std::string &type, const std::string &file) {
    p_blobName = name;
    p_buffer = NULL;
    p_nbytes = 0;
    p_type = type;
    p_labelFile = Filename(file).Expanded();

    Read(file);
  }

  /**
   * This copies the blob object.
   * 
   * @param other 
   */
  Blob::Blob (const Blob& other) {
    p_blobPvl = other.p_blobPvl;
    p_blobName = other.p_blobName;
    p_startByte = other.p_startByte;
    p_nbytes = other.p_nbytes;
    p_type = other.p_type;
    p_detached = other.p_detached;
    p_labelFile = other.p_labelFile;

    p_buffer = NULL;

    if(other.p_buffer) {
      p_buffer = new char[p_nbytes];

      for(int i = 0; i < p_nbytes; i++) {
        p_buffer[i] = other.p_buffer[i];
      }
    }
  }

  //! Destroys the Blob object.
  Blob::~Blob() {
    if (p_buffer != NULL) delete [] p_buffer;
  }

  /**
   * This makes the two blob objects exactly the same (copies the blob)
   * 
   * @param other 
   * 
   * @return Blob& 
   */
  Blob &Blob::operator=(const Blob &other) {
    p_blobPvl = other.p_blobPvl;
    p_blobName = other.p_blobName;
    p_startByte = other.p_startByte;
    p_nbytes = other.p_nbytes;
    p_type = other.p_type;
    p_detached = other.p_detached;
    p_labelFile = other.p_labelFile;

    p_buffer = NULL;

    if(other.p_buffer) {
      p_buffer = new char[p_nbytes];

      for(int i = 0; i < p_nbytes; i++) {
        p_buffer[i] = other.p_buffer[i];
      }
    }

    return *this;
  }

 /**
  * This reads Pvl values from a specified file.
  *
  * @param file The filename to read from.
  *
  * @throws Isis::iException::Io - Unable to open file
  * @throws Isis::iException::Pvl - Invalid label format
  */
  void Blob::Read (const std::string &file) {
    // Expand the filename
    string temp(Filename(file).Expanded());

    // Open the file
    fstream istm;
    istm.open(temp.c_str(),std::ios::in);
    if (!istm) {
      string message = Isis::Message::FileOpen(temp);
      throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
    }

    // Get the pvl
    try {
      Isis::Pvl pvl;
      try {
        istm >> pvl;
      }
      catch (Isis::iException &e) {
        istm.close();
        string msg = "Invalid " + p_type + " label format";
        throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
      }

      // Check pvl and read from the stream
      Read(pvl,istm);
    }
    catch (Isis::iException &e) {
      istm.close();
      string msg = "Unable to open " + p_type + " [" + p_blobName +
                   "] in file [" + temp + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }

    istm.close();
  }

  // Read blob from an open file (probably a cube)
  void Blob::Read (Isis::Pvl &pvl, std::fstream &istm) {
    try {
      Find(pvl);
      ReadInit();
      if (p_detached != "") {
        fstream dstm;
        dstm.open(p_detached.c_str(),std::ios::in);
        if (!dstm) {
          string message = Isis::Message::FileOpen(p_detached);
          throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
        }
        ReadData(dstm);
      }
      else {
        ReadData(istm);
      }
    }
    catch (Isis::iException &e) {
      string msg = "Unable to read " + p_type + " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }
  }

  // Search PVL for the desire blob
  void Blob::Find (Isis::Pvl &pvl) {
    bool found = false;
    try {
      // Search for the blob name
      Isis::iString blobName = p_blobName;
      blobName.UpCase();
      for (int o=0; o<pvl.Objects(); o++) {
        Isis::PvlObject &obj = pvl.Object(o);
        if (obj.IsNamed(p_type)) {
          Isis::iString curName = (string) obj["Name"];
          curName.UpCase();
          if (blobName == curName) {
            p_blobPvl = obj;
            found = true;
            break;
          }
          else {
            if (p_type == "OriginalLabel" && curName == "ORIGINALLABEL") {
              p_blobPvl = obj;
              found = true;
              break;
            }
          }     
        }
      }
    }
    catch (Isis::iException &e) {
      string msg = "Invalid " + p_type + " label format";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }

    // Did we find it?
    if (!found) {
      string msg = "Unable to find " + p_type + " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Ok the blob exists so we need to prep for reading the binary data
    try {
      p_startByte = p_blobPvl["StartByte"];
      p_nbytes = p_blobPvl["Bytes"];
      p_detached = "";
      if (p_blobPvl.HasKeyword("^"+p_type)) {
        string path = "";
        if (p_labelFile != "") {
          path = Filename(p_labelFile).Path() + "/";
        }
        p_detached = path + (std::string) p_blobPvl["^"+p_type];
        p_blobPvl.DeleteKeyword("^"+p_type);
      }
    }
    catch (Isis::iException &e) {
      string msg = "Invalid " + p_type + " label format";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }
  }

 /**
  * Read binary data from an input stream into the Blob.
  *
  * @param stream The input stream to read from.
  *
  * @throws Isis::iException::Io - Error reading data from stream
  */
  void Blob::ReadData (std::fstream &stream) {
    // Read the binary data
    if (p_buffer != NULL) delete [] p_buffer;
    p_buffer = new char[p_nbytes];

    streampos sbyte = p_startByte - 1;
    stream.seekg(sbyte,std::ios::beg);
    if (!stream.good()) {
      string msg = "Error preparing to read data from " + p_type +
                    " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }

    stream.read(p_buffer,p_nbytes);
    if (!stream.good()) {
      string msg = "Error reading data from " + p_type + " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }
  }

 /**
  * Write the blob data out to a file.
  *
  * @param file The filename to write to.
  *
  * @throws Isis::iException::Io - Unable to open file
  * @throws Isis::iException::Io - Error preparing to write data to file
  * @throws Isis::iException::Io - Error creating file
  */
  void Blob::Write(const std::string &file) {
    // Determine the size of the label and write it out
    try {
      WriteInit();
      Isis::Pvl pvl;
      pvl.AddObject(p_blobPvl);
      ostringstream os;
      os << pvl << endl;
      os.seekp(0,std::ios::end);
      Isis::BigInt nbytes = (Isis::BigInt) os.tellp() + (Isis::BigInt) 64;
      p_startByte = nbytes + 1 + 1; // 1-based;
      pvl.FindObject(p_type)["StartByte"] = p_startByte;
      pvl.FindObject(p_type)["Bytes"] = p_nbytes;
      pvl.Write(file);

      // Prepare and write the binary data
      fstream stream;
      ios::openmode flags = std::ios::in | std::ios::binary | std::ios::out;
      stream.open(file.c_str(),flags);
      if (!stream) {
        string message = "Unable to open [" + file + "]";
        throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
      }

      streampos sbyte = p_startByte - 1;
      stream.seekp(sbyte,std::ios::beg);
      if (!stream.good()) {
        stream.close();
        string msg = "Error preparing to write data to " +
                     p_type + " [" + p_blobName + "]";
        throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
      }

      WriteData(stream);
      stream.close();
    }
    catch (Isis::iException &e) {
      string msg = "Unable to create " + p_type + " file [" + file + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }
  }

 /**
  * Write the blob data out to a Pvl object.
  * @param pvl The pvl object to update
  * @param stm stream to write data to
  * @param detachedFilename If the stream is detached from the labels give
  * the name of the file
  */
  void Blob::Write(Isis::Pvl &pvl, std::fstream &stm,
                   const std::string &detachedFilename) {
    // Handle 64-bit I/O
    WriteInit();

    // Find out where they wanted to write the blob
    streampos sbyte = stm.tellp();
    sbyte += 1;

    // Find out where the end-of-file is
    stm.seekp(0,std::ios::end);
    streampos eofbyte = stm.tellp();
    eofbyte += 1;

    // Handle detached blobs
    if (detachedFilename != "") {
      p_blobPvl += Isis::PvlKeyword("^"+p_type,detachedFilename);
    }

    // See if the blob is already in the file
    p_blobPvl["StartByte"] = (Isis::BigInt) sbyte;
    p_blobPvl["Bytes"] = p_nbytes;

    bool found = false;
    for (int i=0; i<pvl.Objects(); i++) {
      if (pvl.Object(i).Name() == p_blobPvl.Name()) {
        PvlObject &obj = pvl.Object(i);
        if ((string) obj["Name"] == (string) p_blobPvl["Name"]) {
          found = true;

          Isis::BigInt oldSbyte = obj["StartByte"];
          int oldNbytes = (int) obj["Bytes"];

          // Does it fit in the old space
          if (p_nbytes <= oldNbytes) {
            p_blobPvl["StartByte"] = obj["StartByte"];
            sbyte = oldSbyte;
          }

          // Was the old space at the end of the file
          else if (((oldSbyte + oldNbytes) == eofbyte) &&
                   (eofbyte >= sbyte)) {
            p_blobPvl["StartByte"] = obj["StartByte"];
            sbyte = oldSbyte;
          }

          // Put it at the requested position/end of the file
          else {
            // Leave this here for clarity
          }

          obj = p_blobPvl;
        }
      }
    }

    // Didn't find the same blob so add it to the labels
    if (!found) {
      pvl.AddObject(p_blobPvl);
    }

    stm.seekp((Isis::BigInt) sbyte-(Isis::BigInt)1);
    WriteData(stm);

    // Handle detached blobs
    if (detachedFilename != "") {
      p_blobPvl.DeleteKeyword("^"+p_type);
    }
  }

 /**
  * Writes blob data to a stream
  *
  * @param stream Output steam blob data will be written to
  *
  * @throws Isis::iException::Io - Error writing data to stream
  */
  void Blob::WriteData(std::fstream &stream) {
    stream.write(p_buffer,p_nbytes);
    if (!stream.good()) {
      string msg = "Error writing data to " + p_type + " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }
  }

 /**
  * Checks pvl object and returns whether or not it is a Blob
  *
  * @param obj Pvl object
  *
  * @return bool Returns true if the object is a blob, and false if it is not
  */
  bool IsBlob(Isis::PvlObject &obj) {
    if (obj.IsNamed("TABLE")) return true;
    return false;
  }
} // end namespace isis

