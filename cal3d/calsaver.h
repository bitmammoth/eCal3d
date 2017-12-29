//****************************************************************************//
// saver.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_SAVER_H
#define CAL_SAVER_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreModel;
class CalCoreBone;
class CalCoreAnimation;
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreSubmesh;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The loader class.
  *****************************************************************************/

class CAL3D_API CalSaver: public CalSaverUserData
{
// constructors/destructor
public:
  CalSaver();
  virtual ~CalSaver();
  
// member functions
public:
  bool saveCoreAnimation(const std::string& strFilename, CalCoreAnimation *pCoreAnimation);
  bool saveCoreModel(const std::string& strFilename, CalCoreModel *pCoreModel);

protected:
  bool saveCoreBones(std::ofstream& file, const std::string& strFilename, CalCoreBone *pCoreBone);
  bool saveCoreKeyframe(std::ofstream& file, const std::string& strFilename, CalCoreKeyframe *pCoreKeyframe);
  bool saveCoreSubmesh(std::ofstream& file, const std::string& strFilename, CalCoreSubmesh *pCoreSubmesh);
  bool saveCoreTrack(std::ofstream& file, const std::string& strFilename, CalCoreTrack *pCoreTrack);
};

#endif

//****************************************************************************//
