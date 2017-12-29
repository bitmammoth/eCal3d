//****************************************************************************//
// coretrack.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_CORETRACK_H
#define CAL_CORETRACK_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"
#include "calquat.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreBone;
class CalCoreKeyframe;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The core track class.
  *****************************************************************************/

class CAL3D_API CalCoreTrack: public CalCoreTrackUserData
{
// member variables
protected:
  int m_coreBoneHint;
  std::string m_coreBoneName;
  std::map<float, CalCoreKeyframe *> m_mapCoreKeyframe;

// constructors/destructor
public:
  CalCoreTrack();
  virtual ~CalCoreTrack();

// member functions	
public:
  bool addCoreKeyframe(CalCoreKeyframe *pCoreKeyframe);
  bool create();
  void destroy();
  int getCoreBoneHint();
  void setCoreBoneHint(int coreBoneId);
  std::string& getCoreBoneName(void);
  void setCoreBoneName(const std::string& name);
  std::map<float, CalCoreKeyframe *>& getMapCoreKeyframe();
  bool getState(float time, float duration, CalVector& orientation, CalQuaternion& rotation);
};

#endif

//****************************************************************************//
