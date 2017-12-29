//****************************************************************************//
// model.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_MODEL_H
#define CAL_MODEL_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"
#include "calbone.h"
#include "calquat.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreModel;
class CalCoreAnimation;
class CalBone;
class CalSubmesh;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The model class.
  *****************************************************************************/

class CAL3D_API CalModel: public CalModelUserData
{
  friend class CalSubmesh;
  friend CalModel *CalModelNew(void);
  
// member variables
protected:
  CalCoreModel *m_pCoreModel;
  CalVector m_translation;
  CalQuaternion m_rotation;
  std::vector<CalBone> m_vectorBone;
  std::vector<CalMatrix> m_vectorTransformMatrix;
  std::vector<CalVector> m_vectorTransformVector;
  std::vector<CalSubmesh *> m_vectorSubmesh;
  
// constructors/destructor
public: 
  CalModel();
  virtual ~CalModel();
  static CalModel *Alloc(void);
  static void Free(CalModel *x);
  
// member functions
public:
  bool create(CalCoreModel *pCoreModel);
  void destroy(void);
  CalCoreModel *getCoreModel(void);
  void setLodLevel(float lodLevel);

  // State queries
  const CalVector &getTranslation(void);
  const CalQuaternion &getRotation(void);
  int getBoneCount(void);
  CalBone *getBone(int boneId);
  int findBone(const std::string& name, int hint=(-1));
  
  // functions to set the pose using animations.
  void setTranslation(const CalVector &translation);
  void setRotation(const CalQuaternion &rotation);
  void clearState(void);
  void blendState(CalCoreAnimation *pCoreAnimation, float weight, float time);
  void blendSavedState(float weight);
  void lockState(void);
  void saveState(void);
  void calculateState(void);
  
  // function to set the pose by copying another model.
  bool mimicSkeleton(CalModel *pModel);
  
  // function to update the spring system
  void updateSpringSystem(float delta);
  
  // function to update the vertices.
  void updateVertices(void);
  
  // functions to loop over the submeshes.
  int getSubmeshCount(void);
  CalSubmesh *getSubmesh(int id);
};


#endif

//****************************************************************************//
