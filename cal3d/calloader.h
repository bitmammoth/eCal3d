//****************************************************************************//
// loader.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_LOADER_H
#define CAL_LOADER_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "caldatasource.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreModel;
class CalCoreBone;
class CalCoreAnimation;
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreSubmesh;

enum
{
  LOADER_ROTATE_X_AXIS = 1,
  LOADER_INVERT_V_COORD = 2
};

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The loader class.
  *****************************************************************************/

class CAL3D_API CalLoader: public CalLoaderUserData
{
// constructors/destructor
public:
  CalLoader();
  virtual ~CalLoader();
	
// member functions
public:
  bool loadCoreAnimation(CalCoreAnimation *anim, const std::string& strFilename);
  bool loadCoreAnimation(CalCoreAnimation *anim, std::istream& inputStream, const std::string& strFilename);
  bool loadCoreAnimation(CalCoreAnimation *anim, void* inputBuffer, int len, const std::string& strFilename);

  bool loadCoreModel(CalCoreModel *model, const std::string& strFilename);
  bool loadCoreModel(CalCoreModel *model, const std::string& strFilename, const std::string& strFilenameM);

  // 1 file input
  bool loadCoreModel(CalCoreModel *model, std::istream& src, const std::string& strFilename);

  // 2 files input
  bool loadCoreModel(CalCoreModel *model, std::istream& src1, const std::string& strFilename1,
		                          std::istream& src2, const std::string& strFilename2);
  
  bool loadCoreModel(CalCoreModel *model, void* inputBuffer, int len, const std::string& strFilename);
  bool loadCoreModel(CalCoreModel *model, void* inputBuffer1, int len1, const std::string& strFilename1,
	                                  void* inputBuffer2, int len2, const std::string& strFilename2);

  static void setLoadingMode(int flags);
  
protected:
  static CalCoreBone *loadCoreBones(CalDataSource& dataSrc);
  static CalCoreKeyframe *loadCoreKeyframe(CalDataSource& dataSrc);
  static CalCoreSubmesh *loadCoreSubmesh(CalDataSource& dataSrc);
  static CalCoreTrack *loadCoreTrack(CalDataSource& dataSrc);
  static bool loadCoreAnimation(CalCoreAnimation *anim, CalDataSource& dataSrc);
  static bool loadCoreModel(CalCoreModel *model, CalDataSource& dataSrc);
  static bool loadCoreModel(CalCoreModel *model, CalDataSource& dataSrc1, CalDataSource& dataSrc2);

  static int loadingMode;
};

#endif

//****************************************************************************//
