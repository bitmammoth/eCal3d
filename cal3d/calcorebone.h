//****************************************************************************//
// corebone.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_COREBONE_H
#define CAL_COREBONE_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"
#include "calquat.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreModel;

 /*****************************************************************************/
/** The core bone class.
  *****************************************************************************/

class CAL3D_API CalCoreBone: public CalCoreBoneUserData
{
// member variables
protected:
  std::string m_strName;
  CalCoreModel *m_pCoreModel;
  int m_parentId;
  std::list<int> m_listChildId;
  float m_length;
  CalVector m_translation;
  CalQuaternion m_rotation;
  CalVector m_translationAbsolute;
  CalQuaternion m_rotationAbsolute;
  CalVector m_translationBoneSpace;
  CalQuaternion m_rotationBoneSpace;
  
// constructors/destructor
public:
  CalCoreBone();
  virtual ~CalCoreBone();
	
// member functions
public:
  bool addChildId(int childId);
  void calculateState();
  bool create(const std::string& strName);
  void destroy();
  std::list<int>& getListChildId();
  const std::string& getName();
  int getParentId();
  float getLength();
  const CalQuaternion& getRotation();
  const CalQuaternion& getRotationAbsolute();
  const CalQuaternion& getRotationBoneSpace();
  const CalVector& getTranslation();
  const CalVector& getTranslationAbsolute();
  const CalVector& getTranslationBoneSpace();
  void setCoreModel(CalCoreModel *pCoreModel);
  void setLength(float f);
  void setParentId(int parentId);
  void setRotation(const CalQuaternion& rotation);
  void setRotationBoneSpace(const CalQuaternion& rotation);
  void setTranslation(const CalVector& translation);
  void setTranslationBoneSpace(const CalVector& translation);
};

#endif

//****************************************************************************//
