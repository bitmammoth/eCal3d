//****************************************************************************//
// corekeyframe.cpp                                                           //
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

#include "calcorekey.h"

 /*****************************************************************************/
/** Constructs the core keyframe instance.
  *
  * This function is the default constructor of the core keyframe instance.
  *****************************************************************************/

CalCoreKeyframe::CalCoreKeyframe()
{
  m_time = 0.0f;
}

 /*****************************************************************************/
/** Destructs the core keyframe instance.
  *
  * This function is the destructor of the core keyframe instance.
  *****************************************************************************/

CalCoreKeyframe::~CalCoreKeyframe()
{
}

 /*****************************************************************************/
/** Creates the core keyframe instance.
  *
  * This function creates the core keyframe instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreKeyframe::create()
{
  return true;
}

 /*****************************************************************************/
/** Destroys the core keyframe instance.
  *
  * This function destroys all data stored in the core keyframe instance and
  * frees all allocated memory.
  *****************************************************************************/

void CalCoreKeyframe::destroy()
{
}

 /*****************************************************************************/
/** Returns the rotation.
  *
  * This function returns the rotation of the core keyframe instance.
  *
  * @return The rotation as quaternion.
  *****************************************************************************/

const CalQuaternion& CalCoreKeyframe::getRotation()
{
  return m_rotation;
}

 /*****************************************************************************/
/** Returns the time.
  *
  * This function returns the time of the core keyframe instance.
  *
  * @return The time in seconds.
  *****************************************************************************/

float CalCoreKeyframe::getTime()
{
  return m_time;
}

 /*****************************************************************************/
/** Returns the orientation.
  *
  * This function returns the orientation of the core keyframe instance.
  *
  * @return The orientation as vector.
  *****************************************************************************/

const CalVector& CalCoreKeyframe::getOrientation()
{
  return m_orientation;
}

 /*****************************************************************************/
/** Sets the rotation.
  *
  * This function sets the rotation of the core keyframe instance.
  *
  * @param rotation The rotation as quaternion.
  *****************************************************************************/

void CalCoreKeyframe::setRotation(const CalQuaternion& rotation)
{
  m_rotation = rotation;
}

 /*****************************************************************************/
/** Sets the time.
  *
  * This function sets the time of the core keyframe instance.
  *
  * @param rotation The time in seconds.
  *****************************************************************************/

void CalCoreKeyframe::setTime(float time)
{
  m_time = time;
}

 /*****************************************************************************/
/** Sets the orientation.
  *
  * This function sets the orientation of the core keyframe instance.
  *
  * @param rotation The orientation as vector.
  *****************************************************************************/

void CalCoreKeyframe::setOrientation(const CalVector& orientation)
{
  m_orientation = orientation;
}

//****************************************************************************//
