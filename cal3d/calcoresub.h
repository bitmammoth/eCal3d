//****************************************************************************//
// coresubmesh.h                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_CORESUBMESH_H
#define CAL_CORESUBMESH_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calglobal.h"
#include "calvector.h"

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The core submesh class.
  *****************************************************************************/

class CAL3D_API CalCoreSubmesh: public CalCoreSubmeshUserData
{
  friend class CalSubmesh;

// misc
public:
  /// The core submesh TextureCoordinate.
  struct TextureCoordinate
  {
    float u, v;
  };
  
  struct TangentSpace
  {
    char tx,ty,tz;     // To get the tangent, divide by 127.0
    char crossFactor;  // To get the binormal, use ((N x T) * crossFactor)
  };
  
  /// The core submesh Influence.
  struct Influence
  {
    int boneId;
    float weight;
  };
  
  /// The core submesh PhysicalProperty.
  struct PhysicalProperty
  {
    float weight;
  };
  
  /// The core submesh Vertex.
  struct Vertex: public CalCoreVertexUserData
  {
    CalVector position;
    char nx,ny,nz;
    char influenceCount;
  };
  
  // The LOD control information.
  struct LodControl
  {
    int faceCollapseCount;
    int collapseId;
  };

  /// The core submesh Face.
  struct Face
  {
    int vertexId[3];
  };

  /// The core submesh Spring.
  struct Spring
  {
    int vertexId[2];
    float springCoefficient;
    float idleLength;
  };

// member variables
protected:
  std::vector<Vertex> m_vectorVertex;
  std::vector<bool> m_vectorTangentsEnabled;
  std::vector<std::vector<TangentSpace> > m_vectorvectorTangentSpace;
  std::vector<std::vector<TextureCoordinate> > m_vectorvectorTextureCoordinate;
  std::vector<PhysicalProperty> m_vectorPhysicalProperty;
  std::vector<Face> m_vectorFace;
  std::vector<Spring> m_vectorSpring;
  std::vector<Influence> m_vectorInfluence;
  std::vector<LodControl> m_vectorLodControl;
  int m_coreMaterialThreadId;
  int m_lodCount;

// constructors/destructor
public:
  CalCoreSubmesh();
  virtual ~CalCoreSubmesh();

// member functions	
public:
  bool create();
  void destroy();
  int getCoreMaterialThreadId();
  int getFaceCount();
  int getLodCount();
  int getSpringCount();
  int getVertexCount();
  int getTextureCoordinateCount();
  float *getTextureCoordinates(int textureCoordinateId);
  std::vector<Face>& getVectorFace();
  std::vector<PhysicalProperty>& getVectorPhysicalProperty();
  std::vector<Spring>& getVectorSpring();
  std::vector<std::vector<TangentSpace> >& getVectorVectorTangentSpace();
  std::vector<std::vector<TextureCoordinate> >& getVectorVectorTextureCoordinate();
  std::vector<Influence>& getVectorInfluence();
  std::vector<Vertex>& getVectorVertex();
  std::vector<TangentSpace>& getVectorTangentSpace(int textureCoordinateId);
  std::vector<TextureCoordinate>& getVectorTextureCoordinate(int textureCoordinateId);
  std::vector<LodControl>& getVectorLodControl();
  bool tangentsEnabled(int mapId);
  bool enableTangents(int mapId, bool enabled);
  bool reserve(int vertexCount, int textureCoordinateCount, int faceCount, int springCount);
  bool resize(int vertexCount, int textureCoordinateCount, int faceCount, int springCount);
  void setCoreMaterialThreadId(int coreMaterialThreadId);
  bool setFace(int faceId, const Face& face);
  void setLodCount(int lodCount);
  bool setPhysicalProperty(int vertexId, const PhysicalProperty& physicalProperty);
  bool setSpring(int springId, const Spring& spring);
  bool setTextureCoordinate(int vertexId, int textureCoordinateId, const TextureCoordinate& textureCoordinate);
  bool setTangentSpace(int vertexId, int textureCoordinateId, const CalVector &tangent, float crossFactor);
  bool setLodControl(int vertexId, int faceCollapseCount, int collapseId);
  bool setVertex(int vertexId, const CalVector &position, const CalVector &normal);
  bool setInfluenceCount(int vertexId, int influenceCount);
  CalCoreVertexUserData *getVertexUserData(int vertexId);
};

#endif

//****************************************************************************//
