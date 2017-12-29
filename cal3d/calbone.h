//****************************************************************************//
// bone.h                                                                     //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_BONE_H
#define CAL_BONE_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"
#include "calquat.h"
#include "calmatrix.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreBone;
class CalModel;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The bone class.
  *****************************************************************************/

class CAL3D_API CalBone: public CalBoneUserData
{
  friend class CalModel;
  
// member variables
protected:
  CalCoreBone *m_pCoreBone;
  CalModel *m_pModel;
  float m_accumulatedWeight;
  float m_accumulatedWeightAbsolute;
  CalVector m_translation;
  CalQuaternion m_rotation;
  CalVector m_translationSaved;
  CalQuaternion m_rotationSaved;
  CalVector m_translationAbsolute;
  CalQuaternion m_rotationAbsolute;
  CalVector m_translationBoneSpace;
  CalQuaternion m_rotationBoneSpace;
  CalVector m_transformVector;
  CalMatrix m_transformMatrix;
  
// constructors/destructor
public:
  CalBone();
  virtual ~CalBone();
  
// member functions
public:
  void blendState(float weight, const CalVector& translation, const CalQuaternion& rotation);
  void calculateState();
  void clearState();
  void saveState();
  void blendSavedState(float weight);
  bool create(CalCoreBone *pCoreBone);
  void destroy();
  CalCoreBone *getCoreBone();
  const CalQuaternion& getRotation();
  const CalQuaternion& getRotationAbsolute();
  const CalQuaternion& getRotationBoneSpace();
  const CalVector& getTranslation();
  const CalVector& getTranslationAbsolute();
  const CalVector& getTranslationBoneSpace();
  void lockState();
  void mimicBone(CalBone *bone);
  void setModel(CalModel *pModel);
};

#endif

//****************************************************************************//
