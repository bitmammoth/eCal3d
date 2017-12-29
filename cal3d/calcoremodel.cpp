//****************************************************************************//
// coremodel.cpp                                                              //
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

#include "calcoremodel.h"
#include "calcorebone.h"
#include "calcoresub.h"
#include "calerror.h"
#include "calloader.h"
#include "calsaver.h"

 /*****************************************************************************/
/** Constructs the core model instance.
  *
  * This function is the default constructor of the core model instance.
  *****************************************************************************/

CalCoreModel::CalCoreModel()
{
}

CalCoreModel::~CalCoreModel()
{
  assert(m_vectorCoreBone.empty());
  assert(m_vectorCoreSubmesh.empty());
}

CalCoreModel *CalCoreModel::Alloc(void) { return new CalCoreModel; }
void CalCoreModel::Free(CalCoreModel *x) { delete x; }

 /*****************************************************************************/
/** Creates the core model instance.
  *
  * This function creates the core model instance.
  *
  * @param strName A string that should be used as the name of the core model
  *                instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::create(const char *strName)
{
  m_strName = strName;

  return true;
}

 /*****************************************************************************/
/** Destroys the core model instance.
  *
  * This function destroys all data stored in the core model instance and frees
  * all allocated memory.
  *****************************************************************************/

void CalCoreModel::destroy(void)
{
  m_strName.erase();
  
  // destroy all core bones
  std::vector<CalCoreBone *>::iterator iteratorCoreBone;
  for(iteratorCoreBone = m_vectorCoreBone.begin(); iteratorCoreBone != m_vectorCoreBone.end(); ++iteratorCoreBone)
  {
    (*iteratorCoreBone)->destroy();
    delete (*iteratorCoreBone);
  }
  m_vectorCoreBone.clear();
  
  // destroy all core submeshes
  std::vector<CalCoreSubmesh *>::iterator iteratorCoreSubmesh;
  for(iteratorCoreSubmesh = m_vectorCoreSubmesh.begin(); iteratorCoreSubmesh != m_vectorCoreSubmesh.end(); ++iteratorCoreSubmesh)
  {
    (*iteratorCoreSubmesh)->destroy();
    delete (*iteratorCoreSubmesh);
  }
  m_vectorCoreSubmesh.clear();
}

 /*****************************************************************************/
/** Returns the number of core bones.
  *
  * This function returns the number of core bones.
  *
  * @return The number of core bones.
  *****************************************************************************/

int CalCoreModel::getCoreBoneCount()
{
  return m_vectorCoreBone.size();
}

 /*****************************************************************************/
/** Provides access to a core bone.
  *
  * This function returns the core bone with the given ID.
  *
  * @param coreBoneId The ID of the core bone that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the core bone
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreBone *CalCoreModel::getCoreBone(int coreBoneId)
{
  if((coreBoneId < 0) || (coreBoneId >= (int)m_vectorCoreBone.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalCoreModel::getCoreBone");
    return 0;
  }

  return m_vectorCoreBone[coreBoneId];
}

 /*****************************************************************************/
/** Returns the ID of a specified core bone.
  *
  * This function returns the ID of a specified core bone.
  *
  * @param strName The name of the core bone that should be returned.
  *
  * @return One of the following values:
  *         \li the \b ID of the core bone
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::getCoreBoneId(const std::string& strName)
{
  int boneId;
  for(boneId = 0; boneId < (int)m_vectorCoreBone.size(); boneId++)
  {
    if(m_vectorCoreBone[boneId]->getName() == strName) return boneId;
  }

  return -1;
}

 /*****************************************************************************/
/** Adds a core bone.
  *
  * This function adds a new, blank core bone to the core skeleton instance.
  *
  * @return One of the following values:
  *         \li the assigned bone \b ID of the added core bone
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::addCoreBone(const std::string& strName)
{
  // Allocate a bone ID.
  int boneId = m_vectorCoreBone.size();
  
  // Create the core bone
  CalCoreBone *pCoreBone = new CalCoreBone();
  pCoreBone->create(strName);
  pCoreBone->setCoreModel(this);

  // Push it onto the core bone vector.
  m_vectorCoreBone.push_back(pCoreBone);
  return boneId;
}

 /*****************************************************************************/
/** Calculates the current state.
  *
  * This function calculates the current state of the core skeleton instance by
  * calculating all the core bone states.
  *****************************************************************************/

void CalCoreModel::calculateState()
{
  // calculate all bone states of the skeleton
  for (int boneId=0; boneId < m_vectorCoreBone.size(); boneId++)
  {
    CalCoreBone *pCoreBone = m_vectorCoreBone[boneId];
    if (pCoreBone->getParentId() == -1)
    {
      pCoreBone->calculateState();
    }
  }
}

 /*****************************************************************************/
/** Returns the number of core submeshes.
  *
  * This function returns the number of core submeshes in the core mesh
  * instance.
  *
  * @return The number of core submeshes.
  *****************************************************************************/

int CalCoreModel::getCoreSubmeshCount()
{
  return m_vectorCoreSubmesh.size();
}

 /*****************************************************************************/
/** Provides access to a core submesh.
  *
  * This function returns the core submesh with the given ID.
  *
  * @param id The ID of the core submesh that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the core submesh
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreSubmesh *CalCoreModel::getCoreSubmesh(int id)
{
  if((id < 0) || (id >= (int)m_vectorCoreSubmesh.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalCoreModel::getCoreSubmesh");
    return 0;
  }

  return m_vectorCoreSubmesh[id];
}

 /*****************************************************************************/
/** Adds a core submesh.
  *
  * This function adds a core submesh to the core mesh instance.
  * The core submesh so added is initially empty.
  *
  * @return One of the following values:
  *         \li the assigned submesh \b ID of the added core submesh
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::addCoreSubmesh(void)
{
  // get next bone id
  int submeshId = m_vectorCoreSubmesh.size();

  // create the core submesh
  CalCoreSubmesh *pCoreSubmesh = new CalCoreSubmesh;
  pCoreSubmesh->create();
  
  // push the core submesh onto the core submesh list.
  m_vectorCoreSubmesh.push_back(pCoreSubmesh);
  return submeshId;
}

//****************************************************************************//
