//****************************************************************************//
// calglobal.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_GLOBAL_H
#define CAL_GLOBAL_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

// autoconf/automake includes
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// platform dependent includes
#include "calplatform.h"

// standard includes
#include <cstdlib>
#include <cmath>

// debug includes
#include <cassert>

// STL includes
#include <strstream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>

// global Cal3D constants
namespace Cal
{
  const char MODEL_FILE_MAGIC[4]     = { 'C', 'D', 'F', '\0' };
  const char SKELETON_FILE_MAGIC[4]  = { 'C', 'S', 'F', '\0' };
  const char ANIMATION_FILE_MAGIC[4] = { 'C', 'A', 'F', '\0' };
  const char MESH_FILE_MAGIC[4]      = { 'C', 'M', 'F', '\0' };
  const char MATERIAL_FILE_MAGIC[4]  = { 'C', 'R', 'F', '\0' };
  
  // library version
  const int LIBRARY_VERSION = 710;

  // file versions
  const int CURRENT_FILE_VERSION = LIBRARY_VERSION;
  const int EARLIEST_COMPATIBLE_FILE_VERSION = 700;

  // empty string
  const std::string strNull;
};

class CAL3D_API CalNullUserData
{
};

class CAL3D_API CalBasicUserData
{
 private:
  void *m_UserData;
 public:
  CalBasicUserData(void) { m_UserData=0; }
  void *getUserData(void) { return m_UserData; }
  void  setUserData(void *p) { m_UserData=p; }
};

#define CalCoreAnimationUserData   CalNullUserData
#define CalCoreBoneUserData        CalBasicUserData
#define CalCoreKeyframeUserData    CalNullUserData
#define CalCoreModelUserData       CalBasicUserData
#define CalCoreSubmeshUserData     CalNullUserData
#define CalCoreTrackUserData       CalNullUserData
#define CalCoreVertexUserData      CalNullUserData
#define CalCoreMapUserData         CalBasicUserData
#define CalAnimationUserData       CalNullUserData
#define CalBoneUserData            CalNullUserData
#define CalLoaderUserData          CalNullUserData
#define CalMixerUserData           CalNullUserData
#define CalModelUserData           CalBasicUserData
#define CalPhysiqueUserData        CalNullUserData
#define CalRendererUserData        CalNullUserData
#define CalSaverUserData           CalNullUserData
#define CalSpringSystemUserData    CalNullUserData
#define CalSubmeshUserData         CalNullUserData

#ifdef CALUSERDATA
#include <caluserdata.h>
#endif
#ifdef CXUSERDATA
#include <cx-userdata.h>
#endif
#ifdef CVUSERDATA
#include <cv-userdata.h>
#endif


#endif
