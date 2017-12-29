//****************************************************************************//
// coremodel.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_COREMODEL_H
#define CAL_COREMODEL_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"

class CalCoreSubmesh;
class CalCoreBone;

 /*****************************************************************************/
/** The core model class.
  *****************************************************************************/

class CAL3D_API CalCoreModel: public CalCoreModelUserData
{
  friend class CalLoader;
  friend class CalSaver;
  
// member variables
protected:
  std::string                   m_strName;
  std::vector<CalCoreBone *>    m_vectorCoreBone;
  std::vector<CalCoreSubmesh *> m_vectorCoreSubmesh;
  
// constructors/destructor
public:
  CalCoreModel();
  virtual ~CalCoreModel();
  static CalCoreModel *Alloc(void);
  static void Free(CalCoreModel *x);
 
// construction/destruction
public:
  bool create(const char *strName);
  void destroy(void);

// Constructing and scanning the skeleton.
public:
  int getCoreBoneCount(void);
  CalCoreBone *getCoreBone(int coreBoneId);
  int getCoreBoneId(const std::string& strName);
  int addCoreBone(const std::string& strName);
  void calculateState(void);

// Constructing and scanning the submeshes.
  int getCoreSubmeshCount();
  CalCoreSubmesh *getCoreSubmesh(int id);
  int addCoreSubmesh(void);
};

#endif

//****************************************************************************//
