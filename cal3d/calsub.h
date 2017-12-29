//****************************************************************************//
// submesh.h                                                                  //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_SUBMESH_H
#define CAL_SUBMESH_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreSubmesh;
class CalModel;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The submesh class.
  *****************************************************************************/

class CAL3D_API CalSubmesh: public CalSubmeshUserData
{
  friend class CalModel;
  
// misc
public:
  /// The submesh PhysicalProperty.
  struct PhysicalProperty
  {
    CalVector position;
    CalVector positionOld;
    CalVector force;
  };

  /// The submesh TangentSpace.
  struct TangentSpace
  {
    CalVector tangent;
    float crossFactor;
  };
  
  /// The submesh Face.
  struct Face
  {
    int vertexId[3];
  };

// member variables
protected:
  CalModel *m_pModel;
  CalCoreSubmesh *m_pCoreSubmesh;
  std::vector<CalVector> m_vectorVertex;
  std::vector<CalVector> m_vectorNormal;
  std::vector<std::vector<TangentSpace> > m_vectorvectorTangentSpace;
  std::vector<Face> m_vectorFace;
  std::vector<PhysicalProperty> m_vectorPhysicalProperty;
  int m_vertexCount;
  int m_faceCount;
  bool m_bInternalData;
  float m_springTime;
  
  void updateVertices(void);
  void calculateSpringForces(float deltaTime);
  void calculateSpringVertices(float deltaTime);

// Because of Win32 DLL Heap Weirdness, Constructors/Destructor must be private. Use Alloc and Free.

 public:
  CalSubmesh();
  ~CalSubmesh();

// member functions	
 public:
  bool create(CalCoreSubmesh *pCoreSubmesh, CalModel *pModel);
  void destroy();
  CalCoreSubmesh *getCoreSubmesh();
  int getVertexCount();
  int getFaceCount();
  void updateSpringSystem(float t);
  bool hasInternalData();
  std::vector<CalVector>& getVectorVertex();
  std::vector<CalVector>& getVectorNormal();
  std::vector<TangentSpace>& getVectorTangentSpace(int textureCoordinateId);
  std::vector<PhysicalProperty>& getVectorPhysicalProperty();

// Functions to compute vertex positions based on bone positions.
  int calculateVertices(float *pVertexBuffer);
  int calculateNormals(float *pNormalBuffer);
  int calculateTangentSpaces(int channel, float *pTangentSpaceBuffer);
  int calculateVN(float *pVertexBuffer, float *pNormalBuffer);
  int calculateVNT(float *pVertexBuffer, float *pNormalBuffer, int channel, float *pTangentBuffer);

  int   *getBufferedFaces();
  float *getBufferedVertices();
  float *getBufferedNormals();
  float *getBufferedTangentSpaces(int mapId);
  float *getBufferedTextureCoordinates(int mapId);

  int getFaces(int *pFaceBuffer, int offset);
  void enableInternalData(void);
  void setLodLevel(float lodLevel);
};

#endif

//****************************************************************************//


