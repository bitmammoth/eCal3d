//****************************************************************************//
// model.cpp                                                                  //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calerror.h"
#include "calmodel.h"
#include "calbone.h"
#include "calsub.h"
#include "calcoremodel.h"
#include "calcoreanim.h"
#include "calcoretrack.h"
#include "calcorebone.h"
#include "calcoresub.h"

 /*****************************************************************************/
/** Constructs the model instance.
  *
  * This function is the default constructor of the model instance.
  *****************************************************************************/

CalModel::CalModel(void)
{
  m_pCoreModel = 0;
  m_translation.clear();
  m_rotation.clear();
}

CalModel::~CalModel(void)
{
  assert(m_vectorBone.empty());
  assert(m_vectorSubmesh.empty());
}

CalModel *CalModel::Alloc(void) { return new CalModel; }
void CalModel::Free(CalModel *x) { delete x; }

 /*****************************************************************************/
/** Creates the model instance.
  *
  * This function creates the model instance based on a core model.
  *
  * @param pCoreModel A pointer to the core model on which this model instance
  *                   should be based on.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalModel::create(CalCoreModel *pCoreModel)
{
  if(pCoreModel == 0)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalModel::create");
    return false;
  }

  m_pCoreModel = pCoreModel;

  // get the number of submeshes
  int submeshCount = pCoreModel->getCoreSubmeshCount();

  // reserve space in the bone vector
  m_vectorSubmesh.reserve(submeshCount);

  // clone every core submesh
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; submeshId++)
  {
    CalSubmesh *pSubmesh = new CalSubmesh();
    if(pSubmesh == 0)
    {
      CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__, "CalModel::create");
      return false;
    }

    // create a submesh for every core submesh
    if(!pSubmesh->create(pCoreModel->getCoreSubmesh(submeshId), this))
    {
      delete pSubmesh;
      return false;
    }

    // insert submesh into submesh vector
    m_vectorSubmesh.push_back(pSubmesh);
  }

  // get the number of bones
  int boneCount = pCoreModel->getCoreBoneCount();
  
  // reserve space in the bone vector
  m_vectorBone.reserve(boneCount);
  m_vectorBone.resize(boneCount);
  m_vectorTransformMatrix.reserve(boneCount);
  m_vectorTransformMatrix.resize(boneCount);
  m_vectorTransformVector.reserve(boneCount);
  m_vectorTransformVector.resize(boneCount);
  
  // clone every core bone
  int boneId;
  for(boneId = 0; boneId < boneCount; boneId++)
  {
    CalBone &pBone = m_vectorBone[boneId];

    // create a bone for every core bone
    if(!pBone.create(pCoreModel->getCoreBone(boneId)))
    {
      return false;
    }
    
    // set skeleton in the bone instance
    pBone.setModel(this);
  }

  return true;
}

 /*****************************************************************************/
/** Destroys the model instance.
  *
  * This function destroys all data stored in the model instance and frees all
  * allocated memory.
  *****************************************************************************/

void CalModel::destroy(void)
{
  // destroy all submeshes
  std::vector<CalSubmesh *>::iterator iteratorSubmesh;
  for(iteratorSubmesh = m_vectorSubmesh.begin(); iteratorSubmesh != m_vectorSubmesh.end(); ++iteratorSubmesh)
  {
    (*iteratorSubmesh)->destroy();
    delete (*iteratorSubmesh);
  }
  m_vectorSubmesh.clear();

  // destroy all bones
  int boneCount = m_vectorBone.size();
  for (int boneId=0; boneId<boneCount; boneId++)
    m_vectorBone[boneId].destroy();
  m_vectorBone.clear();

  m_pCoreModel = 0;
}

 /*****************************************************************************/
/** Returns the number of submeshes.
  *
  * This function returns the number of submeshes in the mesh instance.
  *
  * @return The number of submeshes.
  *****************************************************************************/

int CalModel::getSubmeshCount(void)
{
  return m_vectorSubmesh.size();
}

 /*****************************************************************************/
/** Provides access to a submesh.
  *
  * This function returns the submesh with the given ID.
  *
  * @param id The ID of the submesh that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the submesh
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalSubmesh *CalModel::getSubmesh(int id)
{
  if((id < 0) || (id >= (int)m_vectorSubmesh.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalModel::getSubmesh");
    return 0;
  }

  return m_vectorSubmesh[id];
}

 /*****************************************************************************/
/** Returns the number of bones.
  *
  * This function returns the number of bones in the model instance.
  *
  * @return The number of submeshes.
  *****************************************************************************/

int CalModel::getBoneCount()
{
  return m_vectorBone.size();
}

 /*****************************************************************************/
/** Provides access to a bone.
  *
  * This function returns the bone with the given ID.
  *
  * @param boneId The ID of the bone that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the bone
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalBone *CalModel::getBone(int boneId)
{
  return &(m_vectorBone[boneId]);
}

 /*****************************************************************************/
/** Given a bone name, returns the bone's ID.
  *
  * This function accepts a bone name, and returns the bone's ID.
  *
  * @param name The name of the bone that should be returned.
  *
  * @return One of the following values:
  *         \li the ID of the bone
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalModel::findBone(const std::string& name, int hint)
{
  // See if the hint helps.
  if ((hint >= 0) && (hint < m_vectorBone.size()))
    if (m_vectorBone[hint].getCoreBone()->getName().compare(name) == 0)
      return hint;
  
  // If not, do a brute scan.
  int boneId;
  int boneCount = m_vectorBone.size();
  for (boneId = 0; boneId < boneCount; boneId++)
  {
    if (m_vectorBone[boneId].getCoreBone()->getName().compare(name) == 0)
      return boneId;
  }
  
  // If not found at all, return (-1).
  return -1;
}

 /*****************************************************************************/
/** Sets the root translation.
  *
  * This function sets the root translation.
  *****************************************************************************/

void CalModel::setTranslation(const CalVector &translation)
{
  m_translation = translation;
}

 /*****************************************************************************/
/** Sets the root rotation.
  *
  * This function sets the root rotation.
  *****************************************************************************/

void CalModel::setRotation(const CalQuaternion &rotation)
{
  m_rotation = rotation;
}

 /*****************************************************************************/
/** Gets the root translation.
  *
  * This function returns the root translation.
  *****************************************************************************/

const CalVector &CalModel::getTranslation(void)
{
  return m_translation;
}

 /*****************************************************************************/
/** Gets the root rotation.
  *
  * This function returns the root rotation.
  *****************************************************************************/

const CalQuaternion &CalModel::getRotation(void)
{
  return m_rotation;
}

 /*****************************************************************************/
/** Clears the state of the skeleton instance.
  *
  * This function clears the state of the skeleton instance by recursively
  * clearing the states of its bones.
  *****************************************************************************/

void CalModel::clearState(void)
{
  int boneCount = m_vectorBone.size();
  for (int boneId=0; boneId<boneCount; boneId++)
    m_vectorBone[boneId].clearState();
}

 /*****************************************************************************/
/** Calculates the state of the skeleton instance.
  *
  * This function calculates the state of the skeleton instance by recursively
  * calculating the states of its bones.
  *****************************************************************************/

void CalModel::calculateState(void)
{
  // calculate all bone states of the skeleton
  int boneId;
  int boneCount = m_vectorBone.size();
  for (boneId = 0; boneId < boneCount; boneId++) {
    CalCoreBone *pCoreBone = m_pCoreModel->getCoreBone(boneId);
    if (pCoreBone->getParentId() == -1)
      m_vectorBone[boneId].calculateState();
  }

  // cache the transform matrix and transform vector for faster access.
  for (boneId = 0; boneId < boneCount; boneId++) {
    m_vectorTransformMatrix[boneId] = m_vectorBone[boneId].m_transformMatrix;
    m_vectorTransformVector[boneId] = m_vectorBone[boneId].m_transformVector;
  }
}

 /*****************************************************************************/
/** Saves the state of the skeleton instance.
  *
  * This function saves all the bone positions.  The saved bone positions
  * can then be blended back in to the animation at some future time.
  *****************************************************************************/

void CalModel::saveState(void)
{
  // save all bone states of the skeleton
  int boneCount = m_vectorBone.size();
  for (int boneId=0; boneId<boneCount; boneId++)
    m_vectorBone[boneId].saveState();
}

 /*****************************************************************************/
/** Blends the saved state of the skeleton back into the current pose.
  *
  * This function blends a saved position into the skeleton's state.
  *****************************************************************************/

void CalModel::blendSavedState(float weight)
{
  // blend the saved state for each bone.
  int boneCount = m_vectorBone.size();
  for (int boneId=0; boneId<boneCount; boneId++)
    m_vectorBone[boneId].blendSavedState(weight);
}

 /*****************************************************************************/
/** Locks the state of the skeleton instance.
  *
  * This function locks the state of the skeleton instance by recursively
  * locking the states of its bones.
  *****************************************************************************/

void CalModel::lockState(void)
{
  // lock all bone states of the skeleton
  int boneCount = m_vectorBone.size();
  for (int boneId=0; boneId<boneCount; boneId++)
    m_vectorBone[boneId].lockState();
}

 /*****************************************************************************/
/** Blends a core animation into the skeleton's state.
  *
  * This function blends a core animation into the skeleton's state.
  * To update a skeleton, one must call clearState, blendState,
  * lockState, and calculateState in that order.
  *****************************************************************************/

void CalModel::blendState(CalCoreAnimation *pCoreAnimation, float weight, float time)
{
  // get the duration of the core animation
  float duration;
  duration = pCoreAnimation->getDuration();
  
  // get the list of core tracks of above core animation
  std::list<CalCoreTrack *>& listCoreTrack = pCoreAnimation->getListCoreTrack();
  
  // loop through all core tracks of the core animation
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
  {
    // get the appropriate bone
    int boneId = findBone((*iteratorCoreTrack)->getCoreBoneName(), (*iteratorCoreTrack)->getCoreBoneHint());
    (*iteratorCoreTrack)->setCoreBoneHint(boneId);
    
    if (boneId >= 0)
    {
      CalBone &bone = m_vectorBone[boneId];

      // get the current translation and rotation
      CalVector orientation;
      CalQuaternion rotation;
      (*iteratorCoreTrack)->getState(time, duration, orientation, rotation);
      CalVector translation = orientation * bone.getCoreBone()->getLength();
      
      // blend the bone state with the new state
      bone.blendState(weight, translation, rotation);
    }
  }
}

 /*****************************************************************************/
/** Copies the entire state of another skeleton.
  *
  * This function copies the entire state of one skeleton into
  * another skeleton.  This is useful to synchronize two similar
  * objects.
  *****************************************************************************/

bool CalModel::mimicSkeleton(CalModel *pModel)
{
  // make sure the two bone vectors contain the same number of bones.
  if (pModel->m_vectorBone.size() != m_vectorBone.size())
  {
    CalError::setLastError(CalError::BONE_NOT_FOUND, __FILE__, __LINE__, "CalModel::mimicSkeleton");
    return false;
  }
  
  // copy all the bones states from the source skeleton.
  int boneId;
  for(boneId = 0; boneId < m_vectorBone.size(); boneId++)
  {
    m_vectorBone[boneId].mimicBone(&(pModel->m_vectorBone[boneId]));
    m_vectorTransformMatrix[boneId] = pModel->m_vectorTransformMatrix[boneId];
    m_vectorTransformVector[boneId] = pModel->m_vectorTransformVector[boneId];
  }
  
  // copy the base translation and rotation.
  m_translation = pModel->m_translation;
  m_rotation    = pModel->m_rotation;
  
  return true;
}

 /*****************************************************************************/
/** Provides access to the core model.
  *
  * This function returns the core model on which this model instance is based
  * on.
  *
  * @return One of the following values:
  *         \li a pointer to the core model
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreModel *CalModel::getCoreModel()
{
  return m_pCoreModel;
}

 /*****************************************************************************/
/** Sets the LOD level.
  *
  * This function sets the LOD level of all attached meshes.
  *
  * @param lodLevel The LOD level in the range [0.0, 1.0].
  *****************************************************************************/

void CalModel::setLodLevel(float lodLevel)
{
  // change lod level of every submesh
  int submeshId;
  for(submeshId = 0; submeshId < (int)m_vectorSubmesh.size(); submeshId++)
  {
    // set the lod level in the submesh
    m_vectorSubmesh[submeshId]->setLodLevel(lodLevel);
  }
}

 /*****************************************************************************/
/** Updates the spring system
  *
  * This function updates the spring system for a given amount of time.
  *
  * @param deltaTime The elapsed time in seconds since the last update.
  *****************************************************************************/

void CalModel::updateSpringSystem(float delta)
{
  int submeshCount = m_vectorSubmesh.size();
  for (int submeshId = 0; submeshId < submeshCount; submeshId++) {
    CalSubmesh *submesh = m_vectorSubmesh[submeshId];
    submesh->updateSpringSystem(delta);
  }
}

 /*****************************************************************************/
/** Updates the model instance.
  *
  * This function updates the model instance for a given amount of time.
  *
  * @param deltaTime The elapsed time in seconds since the last update.
  *****************************************************************************/

void CalModel::updateVertices(void)
{
  int submeshCount = m_vectorSubmesh.size();
  for (int submeshId = 0; submeshId < submeshCount; submeshId++) {
    CalSubmesh *submesh = m_vectorSubmesh[submeshId];
    if (submesh->hasInternalData()) submesh->updateVertices();
  }
}

//****************************************************************************//
