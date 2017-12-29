//****************************************************************************//
// coreanimation.cpp                                                          //
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

#include "calcoreanim.h"
#include "calcoretrack.h"

 /*****************************************************************************/
/** Constructs the core animation instance.
  *
  * This function is the default constructor of the core animation instance.
  *****************************************************************************/

CalCoreAnimation::CalCoreAnimation()
{
}

CalCoreAnimation::~CalCoreAnimation()
{
  assert(m_listCoreTrack.empty());
}

CalCoreAnimation *CalCoreAnimation::Alloc(void) { return new CalCoreAnimation; }
void CalCoreAnimation::Free(CalCoreAnimation *x) { delete x; }

 /*****************************************************************************/
/** Adds a core track.
  *
  * This function adds a core track to the core animation instance.
  *
  * @param pCoreTrack A pointer to the core track that should be added.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreAnimation::addCoreTrack(CalCoreTrack *pCoreTrack)
{
  m_listCoreTrack.push_back(pCoreTrack);

  return true;
}

 /*****************************************************************************/
/** Creates the core animation instance.
  *
  * This function creates the core animation instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreAnimation::create(const char *strName)
{
  m_strName = strName;
  
  return true;
}

 /*****************************************************************************/
/** Destroys the core animation instance.
  *
  * This function destroys all data stored in the core animation instance and
  * frees all allocated memory.
  *****************************************************************************/

void CalCoreAnimation::destroy()
{
  // destroy all core tracks
  while(!m_listCoreTrack.empty())
  {
    CalCoreTrack *pCoreTrack;
    pCoreTrack = m_listCoreTrack.front();
    m_listCoreTrack.pop_front();

    pCoreTrack->destroy();
    delete pCoreTrack;
  }
}

 /*****************************************************************************/
/** Returns the duration.
  *
  * This function returns the duration of the core animation instance.
  *
  * @return The duration in seconds.
  *****************************************************************************/

float CalCoreAnimation::getDuration()
{
  return m_duration;
}

 /*****************************************************************************/
/** Returns the core track list.
  *
  * This function returns the list that contains all core tracks of the core
  * animation instance.
  *
  * @return A reference to the core track list.
  *****************************************************************************/

std::list<CalCoreTrack *>& CalCoreAnimation::getListCoreTrack()
{
  return m_listCoreTrack;
}

 /*****************************************************************************/
/** Sets the duration.
  *
  * This function sets the duration of the core animation instance.
  *
  * @param duration The duration in seconds that should be set.
  *****************************************************************************/

void CalCoreAnimation::setDuration(float duration)
{
  m_duration = duration;
}

//****************************************************************************//
