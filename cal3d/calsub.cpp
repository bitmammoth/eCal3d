//****************************************************************************//
// submesh.cpp                                                                //
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

#include "calsub.h"
#include "calerror.h"
#include "calcoresub.h"
#include "calmodel.h"


 /*****************************************************************************/
/** Constructs the submesh instance.
  *
  * This function is the default constructor of the submesh instance.
  *****************************************************************************/

CalSubmesh::CalSubmesh()
{
  m_pCoreSubmesh = 0;
}

CalSubmesh::~CalSubmesh()
{
}

 /*****************************************************************************/
/** Enables the storage of internal data (buffered mode).
  *
  * Submeshes can operate in one of two modes - buffered, or non-buffered.
  * In buffered mode, vertex data is transformed once, and stored in an
  * internal buffer.  The CalRenderer simply returns the stored data.
  * In non-buffered mode, the vertex data is not stored anywhere, which
  * means the CalRenderer must re-transform the data each time you call
  * the renderer.  Buffered mode is most efficient if you plan to call the
  * renderer multiple times per frame.  Non-buffered mode is more
  * efficient if you only plan to call the renderer once, since it
  * avoids the overhead of storing (and copying) a large buffer.  Note that
  * this is entirely a performance consideration - all Cal3D functions have
  * the same semantic behavior regardless of which mode the submeshes are in.
  * Note that certain Cal3D functions automatically force the submesh into
  * buffered mode.
  *
  *****************************************************************************/


void CalSubmesh::enableInternalData()
{
  // If internal data has already been enabled, do nothing.
  if (m_bInternalData) return;
  
  // Get the parameters of the core submesh.
  int vertexCount = m_pCoreSubmesh->getVertexCount();
  int textureCoordinateCount = m_pCoreSubmesh->getTextureCoordinateCount();

  // Reserve space for the main vectors.
  m_vectorVertex.reserve(vertexCount);
  m_vectorVertex.resize(vertexCount);
  m_vectorNormal.reserve(vertexCount);
  m_vectorNormal.resize(vertexCount);
  m_vectorvectorTangentSpace.reserve(textureCoordinateCount);
  m_vectorvectorTangentSpace.resize(textureCoordinateCount);
  
  // Reserve space for the tangent vectors, if enabled.
  int textureCoordinateId;
  for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
  {
    if (m_pCoreSubmesh->tangentsEnabled(textureCoordinateId))
    {
      m_vectorvectorTangentSpace[textureCoordinateId].reserve(vertexCount);
      m_vectorvectorTangentSpace[textureCoordinateId].resize(vertexCount);
    }
    else
    {
      m_vectorvectorTangentSpace[textureCoordinateId].clear();
    }
  }
  
  // Reserve 
  m_vectorPhysicalProperty.reserve(vertexCount);
  m_vectorPhysicalProperty.resize(vertexCount);

  // copy default values for vertex, normal, and physical property data
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = m_pCoreSubmesh->getVectorVertex();
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; vertexId++)
  {
    m_vectorVertex[vertexId] = vectorVertex[vertexId].position;
    m_vectorNormal[vertexId].x = vectorVertex[vertexId].nx * (1.0 / 127.0);
    m_vectorNormal[vertexId].y = vectorVertex[vertexId].ny * (1.0 / 127.0);
    m_vectorNormal[vertexId].z = vectorVertex[vertexId].nz * (1.0 / 127.0);
    m_vectorPhysicalProperty[vertexId].position = vectorVertex[vertexId].position;
    m_vectorPhysicalProperty[vertexId].positionOld = vectorVertex[vertexId].position;
  }
  
  // copy default values for tangent space data.
  for (textureCoordinateId = 0; textureCoordinateId<textureCoordinateCount; textureCoordinateId++)
  {
    std::vector<CalCoreSubmesh::TangentSpace>& vectorTangentSpace = m_pCoreSubmesh->getVectorTangentSpace(textureCoordinateId);
    if (m_pCoreSubmesh->tangentsEnabled(textureCoordinateId))
    {
      for (vertexId = 0; vertexId < vertexCount; vertexId++)
      {
	m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tangent.x = (vectorTangentSpace[vertexId].tx * (1.0/127.0));
	m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tangent.y = (vectorTangentSpace[vertexId].ty * (1.0/127.0));
	m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tangent.z = (vectorTangentSpace[vertexId].tz * (1.0/127.0));
	m_vectorvectorTangentSpace[textureCoordinateId][vertexId].crossFactor = vectorTangentSpace[vertexId].crossFactor;
      }
    }
  }

  // Set the internal data flag to true.
  m_bInternalData = true;
}

 /*****************************************************************************/
/** Creates the submesh instance.
  *
  * This function creates the submesh instance based on a core submesh.
  *
  * @param pCoreSubmesh A pointer to the core submesh on which this submesh
  *                     instance should be based on.
  * @param pMesh A pointer to the mesh instance that the submesh is a part of.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSubmesh::create(CalCoreSubmesh *pCoreSubmesh, CalModel *pModel)
{
  if ((pCoreSubmesh == 0)||(pModel == 0))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalSubmesh::create");
    return false;
  }

  m_pCoreSubmesh = pCoreSubmesh;
  m_pModel = pModel;
  
  // reserve memory for the face vector
  m_vectorFace.reserve(m_pCoreSubmesh->getFaceCount());
  m_vectorFace.resize(m_pCoreSubmesh->getFaceCount());

  // set the initial lod level
  setLodLevel(1.0f);

  // check if the submesh instance must handle the vertex and normal data internally
  m_bInternalData = false;
  if(m_pCoreSubmesh->getSpringCount() > 0)
  {
    enableInternalData();
  }

  return true;
}

 /*****************************************************************************/
/** Destroys the submesh instance.
  *
  * This function destroys all data stored in the submesh instance and frees
  * all allocated memory.
  *****************************************************************************/

void CalSubmesh::destroy()
{
  m_pCoreSubmesh = 0;
}

 /*****************************************************************************/
/** Provides access to the core submesh.
  *
  * This function returns the core submesh on which this submesh instance is
  * based on.
  *
  * @return One of the following values:
  *         \li a pointer to the core submesh
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreSubmesh *CalSubmesh::getCoreSubmesh()
{
  return m_pCoreSubmesh;
}

 /*****************************************************************************/
/** Returns the number of faces.
  *
  * This function returns the number of faces in the submesh instance.
  *
  * @return The number of faces.
  *****************************************************************************/

int CalSubmesh::getFaceCount()
{
  return m_faceCount;
}

 /*****************************************************************************/
/** Provides access to the face data.
  *
  * This function returns the face data (vertex indices) of the submesh
  * instance. The LOD setting of the submesh instance is taken into account.
  *
  * @param pFaceBuffer A pointer to the user-provided buffer where the face
  *                    data is written to.
  *
  * @return The number of faces written to the buffer.
  *****************************************************************************/

int CalSubmesh::getFaces(int *pFaceBuffer, int offset)
{
  // copy the face vector to the face buffer
  int *src = (int*)&m_vectorFace[0];
  if (offset==0) {
    memcpy(pFaceBuffer, src, m_faceCount * sizeof(Face));
  } else {
    for (int i=0; i<m_faceCount*3; i++) pFaceBuffer[i]=src[i]+offset;
  }
  return m_faceCount;
}

 /*****************************************************************************/
/** Provides access to the face data.
  *
  * This function returns the face data (vertex indices) of the submesh
  * instance. The LOD setting of the submesh instance is taken into account.
  *
  * @return A pointer to a buffer containing the faces.  The data is
  *         guaranteed good until you modify the submesh.
  *****************************************************************************/

int *CalSubmesh::getBufferedFaces()
{
  return (int*)&(m_vectorFace[0]);
}

 /*****************************************************************************/
/** Calculates transformed vertex, normal, and tangent data.
  *
  * This is a special-case function that handles the situation where you
  * need to compute the vertices, normals, and exactly one tangent space.
  * It is substantially faster than computing each part separately, since
  * certain common subexpressions can be eliminated when computing all three.
  *
  * @param pSubmesh A pointer to the submesh from which the vertex data should
  *                 be calculated and returned.
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  * @param pNormalBuffer A pointer to the user-provided buffer where the normal
  *                      data is written to.
  * @param textureCoordinateId The identifier of a texture coordinate channel
  *                      for which transformed tangent space data is needed.
  * @param pTangentBuffer A pointer to the user-provided buffer where the tangent
  *                      data is written to.
  *
  * @return The number of vertices written to each buffer.
  *****************************************************************************/

int CalSubmesh::calculateVNT(float *pVertexBuffer, float *pNormalBuffer,
			      int textureCoordinateId, float *pTangentBuffer)
{
#undef CALCULATE_VERTICES
#undef CALCULATE_NORMALS
#undef CALCULATE_TANGENTS
#define CALCULATE_VERTICES 1
#define CALCULATE_NORMALS 1
#define CALCULATE_TANGENTS 1
#include "calphysop.h"
}

 /*****************************************************************************/
/** Calculates the transformed vertex data.
  *
  * This is a special-case function that handles the situation where you
  * need to compute the vertices and normals. It is substantially faster than
  * computing each part separately, since certain common subexpressions can be
  * eliminated when computing both.
  *
  * @param pSubmesh A pointer to the submesh from which the vertex data should
  *                 be calculated and returned.
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  *
  * @return The number of vertices written to the buffer.
  *****************************************************************************/

int CalSubmesh::calculateVN(float *pVertexBuffer, float *pNormalBuffer)
{
#undef CALCULATE_VERTICES
#undef CALCULATE_NORMALS
#undef CALCULATE_TANGENTS
#define CALCULATE_VERTICES 1
#define CALCULATE_NORMALS 1
#define CALCULATE_TANGENTS 0
#include "calphysop.h"
}

 /*****************************************************************************/
/** Calculates the transformed vertex data.
  *
  * This function calculates and returns the transformed vertex data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the vertex data should
  *                 be calculated and returned.
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  *
  * @return The number of vertices written to the buffer.
  *****************************************************************************/

int CalSubmesh::calculateVertices(float *pVertexBuffer)
{
#undef CALCULATE_VERTICES
#undef CALCULATE_NORMALS
#undef CALCULATE_TANGENTS
#define CALCULATE_VERTICES 1
#define CALCULATE_NORMALS 0
#define CALCULATE_TANGENTS 0
#include "calphysop.h"
}

 /*****************************************************************************/
/** Calculates the transformed normal data.
  *
  * This function calculates and returns the transformed normal data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the normal data should
  *                 be calculated and returned.
  * @param pNormalBuffer A pointer to the user-provided buffer where the normal
  *                      data is written to.
  *
  * @return The number of normals written to the buffer.
  *****************************************************************************/

int CalSubmesh::calculateNormals(float *pNormalBuffer)
{
#undef CALCULATE_VERTICES
#undef CALCULATE_NORMALS
#undef CALCULATE_TANGENTS
#define CALCULATE_VERTICES 0
#define CALCULATE_NORMALS 1
#define CALCULATE_TANGENTS 0
#include "calphysop.h"
}

 /*****************************************************************************/
/** Calculates the transformed tangent space data.
  *
  * This function calculates and returns the transformed tangent space data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the tangent data should
  *                 be calculated and returned.
  * @param textureCoordinateId The texture coordinate channel for which you
  *                 need the tangent data.
  * @param pTangentSpaceBuffer A pointer to the user-provided buffer where
  *                  the tangent space data is written to.
  *
  * @return The number of tangent spaces written to the buffer.
  *****************************************************************************/

int CalSubmesh::calculateTangentSpaces(int textureCoordinateId, float *pTangentBuffer)
{
#undef CALCULATE_VERTICES
#undef CALCULATE_NORMALS
#undef CALCULATE_TANGENTS
#define CALCULATE_VERTICES 0
#define CALCULATE_NORMALS 0
#define CALCULATE_TANGENTS 1
#include "calphysop.h"
}

 /*****************************************************************************/
/** Calculates the forces on each unbound vertex.
  *
  * This function calculates the forces on each unbound vertex of a specific
  * submesh.
  *
  * @param deltaTime The elapsed time in seconds since the last calculation.
  *****************************************************************************/

void CalSubmesh::calculateSpringForces(float deltaTime)
{
  // get the physical property vector of the core submesh
  std::vector<CalCoreSubmesh::PhysicalProperty>& vectorCorePhysicalProperty = m_pCoreSubmesh->getVectorPhysicalProperty();
  
  // loop through all the vertices
  int vertexId;
  for(vertexId = 0; vertexId < (int)m_vectorVertex.size(); vertexId++)
  {
    // get the physical property of the vertex
    PhysicalProperty& physicalProperty = m_vectorPhysicalProperty[vertexId];

    // get the physical property of the core vertex
    CalCoreSubmesh::PhysicalProperty& corePhysicalProperty = vectorCorePhysicalProperty[vertexId];

    // only take vertices with a weight > 0 into account
    if(corePhysicalProperty.weight > 0.0f)
    {
      physicalProperty.force.set(0.0f, 0.5f, corePhysicalProperty.weight * -98.1f);
    }
  }
}

 /*****************************************************************************/
/** Calculates the vertices influenced by the spring system instance.
  *
  * This function calculates the vertices influenced by the spring system
  * instance.
  *
  * @param deltaTime The elapsed time in seconds since the last calculation.
  *****************************************************************************/

void CalSubmesh::calculateSpringVertices(float deltaTime)
{
  // get the physical property vector of the core submesh
  std::vector<CalCoreSubmesh::PhysicalProperty>& vectorCorePhysicalProperty = m_pCoreSubmesh->getVectorPhysicalProperty();

  // loop through all the vertices
  int vertexId;
  for(vertexId = 0; vertexId < (int)m_vectorVertex.size(); vertexId++)
  {
    // get the vertex
    CalVector& vertex = m_vectorVertex[vertexId];

    // get the physical property of the vertex
    CalSubmesh::PhysicalProperty& physicalProperty = m_vectorPhysicalProperty[vertexId];

    // get the physical property of the core vertex
    CalCoreSubmesh::PhysicalProperty& corePhysicalProperty = vectorCorePhysicalProperty[vertexId];

    // store current position for later use
    CalVector position;
    position = physicalProperty.position;

    // only take vertices with a weight > 0 into account
    if(corePhysicalProperty.weight > 0.0f)
    {
      // do the Verlet step
      physicalProperty.position += (position - physicalProperty.positionOld) * 0.99f + physicalProperty.force / corePhysicalProperty.weight * deltaTime * deltaTime;
    }
    else
    {
      physicalProperty.position = m_vectorVertex[vertexId];
    }

    // make the current position the old one
    physicalProperty.positionOld = position;

    // set the new position of the vertex
    vertex = physicalProperty.position;

    // clear the accumulated force on the vertex
    physicalProperty.force.clear();
  }

  // get the spring vector of the core submesh
  std::vector<CalCoreSubmesh::Spring>& vectorSpring = m_pCoreSubmesh->getVectorSpring();

  // iterate a few times to relax the constraints
  int iterationCount;
#define ITERATION_COUNT 2
  for(iterationCount = 0; iterationCount < ITERATION_COUNT; iterationCount++)
  {
    // loop through all the springs
    std::vector<CalCoreSubmesh::Spring>::iterator iteratorSpring;
    for(iteratorSpring = vectorSpring.begin(); iteratorSpring != vectorSpring.end(); ++iteratorSpring)
    {
      // get the spring
      CalCoreSubmesh::Spring& spring = *iteratorSpring;

      // compute the difference between the two spring vertices
      CalVector distance;
      distance = m_vectorVertex[spring.vertexId[1]] - m_vectorVertex[spring.vertexId[0]];

      // get the current length of the spring
      float length;
      length = distance.length();

      if(length > 0.0f)
      {
        float factor[2];
        factor[0] = (length - spring.idleLength) / length;
        factor[1] = factor[0];

        if(vectorCorePhysicalProperty[spring.vertexId[0]].weight > 0.0f)
        {
          factor[0] /= 2.0f;
          factor[1] /= 2.0f;
        }
        else
        {
          factor[0] = 0.0f;
        }

        if(vectorCorePhysicalProperty[spring.vertexId[1]].weight <= 0.0f)
        {
          factor[0] *= 2.0f;
          factor[1] = 0.0f;
        }

        m_vectorVertex[spring.vertexId[0]] += distance * factor[0];
        m_vectorPhysicalProperty[spring.vertexId[0]].position = m_vectorVertex[spring.vertexId[0]];

        m_vectorVertex[spring.vertexId[1]] -= distance * factor[1];
        m_vectorPhysicalProperty[spring.vertexId[1]].position = m_vectorVertex[spring.vertexId[1]];
      }
    }
  }
}

 /*****************************************************************************/
/** Updates the spring system for the submesh.
  *
  * @param t The amount of time to elapse for the spring system.
  *****************************************************************************/

void CalSubmesh::updateSpringSystem(float t)
{
  m_springTime = t;
}

 /*****************************************************************************/
/** Updates the buffered vertex data of a submesh.
  *
  * This function updates the buffered data of a specific submesh.
  * First, it tries to find a highly-optimized function to calculate the
  * data. If it can't find one, it will use the slower general-case functions.
  * If the submesh doesn't buffer vertices (that is, if internal data has not
  * been enabled), this is a no-op.
  *
  * @param submesh The submesh whose buffered vertex data needs to be updated.
  *****************************************************************************/

void CalSubmesh::updateVertices(void)
{
  // If this submesh does not store internal data, there's nothing to do.
  if (!m_bInternalData) return;
  
  // Count the tangent spaces.
  int tangentSpaceCount = 0;
  int tangentSpaceIndex;
  int textureCoordinateId;
  for (textureCoordinateId = 0; textureCoordinateId < m_pCoreSubmesh->getTextureCoordinateCount(); textureCoordinateId++)
  {    
    if (m_pCoreSubmesh->tangentsEnabled(textureCoordinateId))
    {
      tangentSpaceIndex = textureCoordinateId;
      tangentSpaceCount++;
    }
  }
  
  if (tangentSpaceCount == 1)
  {
    // If there's exactly one tangent space, use calculateVNT
    std::vector<CalSubmesh::TangentSpace> &vectorTangentSpace = getVectorTangentSpace(tangentSpaceIndex);
    calculateVNT((float*)&(m_vectorVertex[0]), (float *)&(m_vectorNormal[0]), 
		 tangentSpaceIndex, (float *)&(vectorTangentSpace[0]));
  } else {
    // Use this code for every other case:
    calculateVN((float *)&(m_vectorVertex[0]), (float *)&(m_vectorNormal[0]));
    for (textureCoordinateId = 0; textureCoordinateId < m_pCoreSubmesh->getTextureCoordinateCount(); textureCoordinateId++)
    {
      if (m_pCoreSubmesh->tangentsEnabled(textureCoordinateId))
	{
	  calculateTangentSpaces(textureCoordinateId, (float *)&(m_vectorvectorTangentSpace[textureCoordinateId][0]));
	}
    }
  }
  
  if (m_pCoreSubmesh->getSpringCount() > 0)
  {
    calculateSpringForces(m_springTime);
    calculateSpringVertices(m_springTime);
    m_springTime = 0.0;
  }
}

 /*****************************************************************************/
/** Provides access to the vertex data.
  *
  * This function returns the vertex data of the selected mesh/submesh.
  *
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  *
  * @return The number of vertices written to the buffer.
  *****************************************************************************/

//int CalSubmesh::getVertices(float *pVertexBuffer)
//{
//  // check if the submesh handles vertex data internally
//  if(m_bInternalData)
//  {
//    // copy the internal vertex data to the provided vertex buffer
//    memcpy(pVertexBuffer, &m_vectorVertex[0], m_vertexCount * sizeof(CalVector));
//
//    return m_vertexCount;
//  }
//
//  // submesh does not handle the vertex data internally, so let the physique calculate it now
//  return m_pMesh->m_pModel->getPhysique()->calculateVertices(this, pVertexBuffer);
//}

 /*****************************************************************************/
/** Provides access to the vertex data.
  *
  * This function returns the vertex data of the selected mesh/submesh.
  * Note: this puts the relevant submesh into buffered mode.
  *
  * @return A pointer to a buffer containing the vertices.  The data is
  *         guaranteed good until you select a different submesh or modify
  *         the submesh.
  *****************************************************************************/

float *CalSubmesh::getBufferedVertices()
{
  if(!m_bInternalData) {
    enableInternalData();
    updateVertices();
  }
  return &(m_vectorVertex[0].x);
}

 /*****************************************************************************/
/** Provides access to the normal data.
  *
  * This function returns the normal data of the selected mesh/submesh.
  *
  * @param pNormalBuffer A pointer to the user-provided buffer where the normal
  *                      data is written to.
  *
  * @return The number of normals written to the buffer.
  *****************************************************************************/

//int CalSubmesh::getNormals(float *pNormalBuffer)
//{
//  // check if the submesh handles vertex data internally
//  if(m_bInternalData)
//  {
//    // copy the internal normal data to the provided normal buffer
//    memcpy(pNormalBuffer, &m_vectorNormal[0], m_vertexCount * sizeof(CalVector));
//    
//    return m_vertexCount;
//  }
//  
//  // submesh does not handle the vertex data internally, so let the physique calculate it now
//  return m_pMesh->m_pModel->getPhysique()->calculateNormals(this, pNormalBuffer);
//}

 /*****************************************************************************/
/** Provides access to the normal data.
  *
  * This function returns the normal data of the selected mesh/submesh.
  * Note: this puts the relevant submesh into buffered mode.
  *
  * @return A pointer to a buffer containing the normals.  The data is
  *         guaranteed good until you select a different submesh or modify
  *         the submesh.
  *****************************************************************************/

float *CalSubmesh::getBufferedNormals()
{
  if(!m_bInternalData) {
    enableInternalData();
    updateVertices();
  }
  return &(m_vectorNormal[0].x);
}

 /*****************************************************************************/
/** Provides access to the tangent space data.
  *
  * This function returns the tangent space data of the selected mesh/submesh.
  *
  * @param pTangentBuffer A pointer to the user-provided buffer where the tangent
  *                      space data is written to.
  *
  * @return The number of tangent spaces written to the buffer.
  *****************************************************************************/

//int CalSubmesh::getTangentSpaces(int mapId, float *pTangentSpaceBuffer)
//{
//  // if there are no tangents, return zero.
//  if (!m_pCoreSubmesh->tangentsEnabled(mapId)) return 0;
//  
//  // check if the submesh handles vertex data internally
//  if(m_bInternalData)
//  {
//    // copy the internal tangent data to the provided tangent buffer
//    memcpy(pTangentSpaceBuffer, &m_vectorvectorTangentSpace[mapId][0], m_vertexCount * sizeof(CalSubmesh::TangentSpace));
//    return tangentCount;
//  }
//  
//  // submesh does not handle the vertex data internally, so let the physique calculate it now
//  return m_pMesh->m_pModel->getPhysique()->calculateTangentSpaces(this, mapId, pTangentSpaceBuffer);
//}

 /*****************************************************************************/
/** Provides access to the tangent data.
  *
  * This function returns the tangent data of the selected mesh/submesh.
  * Note: this puts the relevant submesh into buffered mode.
  *
  * @return A pointer to a buffer containing the tangents.  The data is
  *         guaranteed good until you select a different submesh or modify
  *         the submesh.
  *****************************************************************************/

float *CalSubmesh::getBufferedTangentSpaces(int mapId)
{
  // check if the map id is valid
  if(!m_pCoreSubmesh->tangentsEnabled(mapId))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "CalSubmesh::getBufferedTangentSpaces");
    return 0;
  }
  
  // Return the requested array
  if(!m_bInternalData) {
    enableInternalData();
    updateVertices();
  }
  return (float*)&(m_vectorvectorTangentSpace[mapId][0]);
}

 /*****************************************************************************/
/** Provides access to the texture coordinate data.
  *
  * This function returns the texture coordinate data for a given map of the
  * selected mesh/submesh.
  *
  * @param mapId The ID of the map to get the texture coordinate data from.
  * @param pTextureCoordinateBuffer A pointer to the user-provided buffer where
  *                    the texture coordinate data is written to.
  *
  * @return The number of texture coordinates written to the buffer.
  *****************************************************************************/

//int CalSubmesh::getTextureCoordinates(int mapId, float *pTextureCoordinateBuffer)
//{
//  // Get the texture coordinate vector
//  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& 
//    vectorvectorTextureCoordinate = m_pCoreSubmesh->getVectorVectorTextureCoordinate();
//
//  // check if the map id is valid
//  if((mapId < 0) || (mapId >= (int)vectorvectorTextureCoordinate.size()))
//  {
//    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
//    return -1;
//  }
//
//  // get the number of texture coordinates to return
//  int textureCoordinateCount;
//  textureCoordinateCount = m_pSelectedSubmesh->getVertexCount();
//
//  // copy the texture coordinate vector to the face buffer
//  memcpy(pTextureCoordinateBuffer, &vectorvectorTextureCoordinate[mapId][0], textureCoordinateCount * sizeof(CalCoreSubmesh::TextureCoordinate));
//
//  return textureCoordinateCount;
//}

 /*****************************************************************************/
/** Provides access to the texture coordinate data.
  *
  * This function returns the texture coordinate data for a given map of the
  * selected mesh/submesh.
  *
  * @param mapId The ID of the map to get the texture coordinate data from.
  *
  * @return A pointer to a buffer containing the texture coordinates. The data
  *         is guaranteed good until you select a different submesh or modify
  *         the submesh.
  *****************************************************************************/

float *CalSubmesh::getBufferedTextureCoordinates(int mapId)
{
  // check if the map id is valid
  
  if ((mapId < 0) || (mapId >= m_pCoreSubmesh->m_vectorvectorTextureCoordinate.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "");
    return 0;
  }

  // copy the texture coordinate vector to the face buffer
  return (float*)&(m_pCoreSubmesh->m_vectorvectorTextureCoordinate[mapId][0]);
}

 /*****************************************************************************/
/** Returns the normal vector.
  *
  * This function returns the vector that contains all normals of the submesh
  * instance.
  *
  * @return A reference to the normal vector.
  *****************************************************************************/

std::vector<CalVector>& CalSubmesh::getVectorNormal()
{
  return m_vectorNormal;
}

 /*****************************************************************************/
/** Returns the tangent space vector.
  *
  * This function returns the vector that contains all tangent space bases
  * for the given texture coordinate channel.
  *
  * @return A reference to the tangent vector.
  *****************************************************************************/

std::vector<CalSubmesh::TangentSpace>& CalSubmesh::getVectorTangentSpace(int textureCoordinateId)
{
  return m_vectorvectorTangentSpace[textureCoordinateId];
}

 /*****************************************************************************/
/** Returns the physical property vector.
  *
  * This function returns the vector that contains all physical properties of
  * the submesh instance.
  *
  * @return A reference to the physical property vector.
  *****************************************************************************/

std::vector<CalSubmesh::PhysicalProperty>& CalSubmesh::getVectorPhysicalProperty()
{
  return m_vectorPhysicalProperty;
}

 /*****************************************************************************/
/** Returns the vertex vector.
  *
  * This function returns the vector that contains all vertices of the submesh
  * instance.
  *
  * @return A reference to the vertex vector.
  *****************************************************************************/

std::vector<CalVector>& CalSubmesh::getVectorVertex()
{
  return m_vectorVertex;
}

 /*****************************************************************************/
/** Returns the number of vertices.
  *
  * This function returns the number of vertices in the submesh instance.
  *
  * @return The number of vertices.
  *****************************************************************************/

int CalSubmesh::getVertexCount()
{
  return m_vertexCount;
}

 /*****************************************************************************/
/** Returns if the submesh instance handles vertex data internally.
  *
  * This function returns wheter the submesh instance handles vertex data
  * internally.
  *
  * @return One of the following values:
  *         \li \b true if vertex data is handled internally
  *         \li \b false if not
  *****************************************************************************/

bool CalSubmesh::hasInternalData()
{
  return m_bInternalData;
}

 /*****************************************************************************/
/** Sets the LOD level.
  *
  * This function sets the LOD level of the submesh instance.
  *
  * @param lodLevel The LOD level in the range [0.0, 1.0].
  *****************************************************************************/

void CalSubmesh::setLodLevel(float lodLevel)
{
  // clamp the lod level to [0.0, 1.0]
  if(lodLevel < 0.0f) lodLevel = 0.0f;
  if(lodLevel > 1.0f) lodLevel = 1.0f;

  // get the lod count of the core submesh
  int lodCount;
  lodCount = m_pCoreSubmesh->getLodCount();

  // calculate the target lod count
  lodCount = (int)((1.0f - lodLevel) * lodCount);

  // calculate the new number of vertices
  m_vertexCount = m_pCoreSubmesh->getVertexCount() - lodCount;

  // get face vector of the core submesh
  std::vector<CalCoreSubmesh::Face>& vectorFace = m_pCoreSubmesh->getVectorFace();

  // get face vector of the core submesh
  std::vector<CalCoreSubmesh::LodControl>& vectorLodControl = m_pCoreSubmesh->getVectorLodControl();
  
  // calculate the new number of faces
  m_faceCount = vectorFace.size();

  int vertexId;
  for(vertexId = vectorLodControl.size() - 1; vertexId >= m_vertexCount; vertexId--)
  {
    m_faceCount -= vectorLodControl[vertexId].faceCollapseCount;
  }
  
  // fill the face vector with the collapsed vertex ids
  int faceId;
  for(faceId = 0; faceId < m_faceCount; faceId++)
  {
    int vertexId;
    for(vertexId = 0; vertexId < 3; vertexId++)
    {
      // get the vertex id
      int collapsedVertexId;
      collapsedVertexId = vectorFace[faceId].vertexId[vertexId];

      // collapse the vertex id until it fits into the current lod level
      while(collapsedVertexId >= m_vertexCount) collapsedVertexId = vectorLodControl[collapsedVertexId].collapseId;

      // store the collapse vertex id in the submesh face vector
      m_vectorFace[faceId].vertexId[vertexId] = collapsedVertexId;
    }
  }
}

//****************************************************************************//



