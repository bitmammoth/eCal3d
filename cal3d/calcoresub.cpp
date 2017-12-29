//****************************************************************************//
// coresubmesh.cpp                                                            //
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

#include "calcoresub.h"

 /*****************************************************************************/
/** Constructs the core submesh instance.
  *
  * This function is the default constructor of the core submesh instance.
  *****************************************************************************/

CalCoreSubmesh::CalCoreSubmesh()
{
  m_coreMaterialThreadId = 0;
  m_lodCount = 0;
}

 /*****************************************************************************/
/** Destructs the core submesh instance.
  *
  * This function is the destructor of the core submesh instance.
  *****************************************************************************/

CalCoreSubmesh::~CalCoreSubmesh()
{
  assert(m_vectorFace.empty());
  assert(m_vectorVertex.empty());
  assert(m_vectorPhysicalProperty.empty());
  assert(m_vectorvectorTextureCoordinate.empty());
  assert(m_vectorSpring.empty());
}

 /*****************************************************************************/
/** Creates the core submesh instance.
  *
  * This function creates the core submesh instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::create()
{
  return true;
}

 /*****************************************************************************/
/** Destroys the core submesh instance.
  *
  * This function destroys all data stored in the core submesh instance and
  * frees all allocated memory.
  *****************************************************************************/

void CalCoreSubmesh::destroy()
{
  // destroy all data
  m_vectorFace.clear();
  m_vectorVertex.clear();
  m_vectorPhysicalProperty.clear();
  m_vectorvectorTextureCoordinate.clear();
  m_vectorSpring.clear();
}

 /*****************************************************************************/
/** Returns the ID of the core material thread.
  *
  * This function returns the ID of the core material thread of this core
  * submesh instance.
  *
  * @return The ID of the core material thread.
  *****************************************************************************/

int CalCoreSubmesh::getCoreMaterialThreadId()
{
  return m_coreMaterialThreadId;
}

 /*****************************************************************************/
/** Returns the number of faces.
  *
  * This function returns the number of faces in the core submesh instance.
  *
  * @return The number of faces.
  *****************************************************************************/

int CalCoreSubmesh::getFaceCount()
{
  return m_vectorFace.size();
}

 /*****************************************************************************/
/** Returns the number of LOD steps.
  *
  * This function returns the number of LOD steps in the core submesh instance.
  *
  * @return The number of LOD steps.
  *****************************************************************************/

int CalCoreSubmesh::getLodCount()
{
  return m_lodCount;
}

 /*****************************************************************************/
/** Returns the number of springs.
  *
  * This function returns the number of springs in the core submesh instance.
  *
  * @return The number of springs.
  *****************************************************************************/

int CalCoreSubmesh::getSpringCount()
{
  return m_vectorSpring.size();
}

 /*****************************************************************************/
/** Returns true if tangent vectors are enabled.
  *
  * This function returns true if the core submesh contains tangent vectors.
  *
  * @return True if tangent vectors are enabled.
  *****************************************************************************/

bool CalCoreSubmesh::tangentsEnabled(int mapId)
{
  if((mapId < 0) || (mapId >= (int)m_vectorTangentsEnabled.size())) return false;

  return m_vectorTangentsEnabled[mapId];
}

 /*****************************************************************************/
/** Enables or disables the storage of tangent spaces.
  *
  * This function enables or disables the storage of tangent space bases.
  *****************************************************************************/

bool CalCoreSubmesh::enableTangents(int mapId, bool enabled)
{
  if((mapId < 0) || (mapId >= (int)m_vectorTangentsEnabled.size())) return false;
  
  m_vectorTangentsEnabled[mapId] = enabled;
  
  if (enabled)
  {
    m_vectorvectorTangentSpace[mapId].reserve(m_vectorVertex.size());
    m_vectorvectorTangentSpace[mapId].resize(m_vectorVertex.size());
  }
  else
  {
    m_vectorvectorTangentSpace[mapId].clear();
  }
  return true;
}

 /*****************************************************************************/
/** Returns the face vector.
  *
  * This function returns the vector that contains all faces of the core submesh
  * instance.
  *
  * @return A reference to the face vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Face>& CalCoreSubmesh::getVectorFace()
{
  return m_vectorFace;
}

 /*****************************************************************************/
/** Returns the physical property vector.
  *
  * This function returns the vector that contains all physical properties of
  * the core submesh instance.
  *
  * @return A reference to the physical property vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::PhysicalProperty>& CalCoreSubmesh::getVectorPhysicalProperty()
{
  return m_vectorPhysicalProperty;
}

 /*****************************************************************************/
/** Returns the spring vector.
  *
  * This function returns the vector that contains all springs of the core
  * submesh instance.
  *
  * @return A reference to the spring vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Spring>& CalCoreSubmesh::getVectorSpring()
{
  return m_vectorSpring;
}

 /*****************************************************************************/
/** Returns the texture coordinate vector for a specific channel.
  *
  * This function returns the vector that contains texture coordinates
  * for a specific texture coordinate channel.
  *
  * @return A reference to the texture coordinate vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::TextureCoordinate> & CalCoreSubmesh::getVectorTextureCoordinate(int textureCoordinateId)
{
  return m_vectorvectorTextureCoordinate[textureCoordinateId];
}

 /*****************************************************************************/
/** Returns the tangent space vector for a specific channel.
  *
  * This function returns the vector that contains all tangent space bases
  * for a specific texture coordinate channel.
  *
  * @return A reference to the tangent space vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::TangentSpace>& CalCoreSubmesh::getVectorTangentSpace(int textureCoordinateId)
{
  return m_vectorvectorTangentSpace[textureCoordinateId];
}

 /*****************************************************************************/
/** Returns the texture coordinate vector-vector.
  *
  * This function returns the vector that contains all texture coordinate
  * vectors of the core submesh instance. This vector contains another vector
  * because there can be more than one texture map at each vertex.
  *
  * @return A reference to the texture coordinate vector-vector.
  *****************************************************************************/

std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> > & CalCoreSubmesh::getVectorVectorTextureCoordinate()
{
  return m_vectorvectorTextureCoordinate;
}

 /*****************************************************************************/
/** Returns the tangent space vector-vector.
  *
  * This function returns the vector that contains all tangent space bases of
  * the core submesh instance. This vector contains another vector
  * because there can be more than one texture map at each vertex.
  *
  * @return A reference to the tangent space vector-vector.
  *****************************************************************************/

std::vector<std::vector<CalCoreSubmesh::TangentSpace> >& CalCoreSubmesh::getVectorVectorTangentSpace()
{
  return m_vectorvectorTangentSpace;
}

 /*****************************************************************************/
/** Returns the Influence vector.
  *
  * This function returns the vector that contains the influences.
  * Note that each vertex has an "influenceCount" field.  Suppose that
  * the first vertex has an influenceCount of 3, and the second vertex has
  * an influenceCount of 2.  That means that the first 3 influences
  * in the influence array belong to the first vertex, and the next 2
  * influences belong to the second vertex.
  *
  * @return A reference to the influence array.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Influence>& CalCoreSubmesh::getVectorInfluence()
{
  return m_vectorInfluence;
}

 /*****************************************************************************/
/** Returns the vertex vector.
  *
  * This function returns the vector that contains all vertices of the core
  * submesh instance.
  *
  * @return A reference to the vertex vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Vertex>& CalCoreSubmesh::getVectorVertex()
{
  return m_vectorVertex;
}

 /*****************************************************************************/
/** Returns the LOD Control vector.
  *
  * This function returns the vector that contains all the LOD control information
  * of the core submesh instance.
  *
  * @return A reference to the LOD Control vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::LodControl>& CalCoreSubmesh::getVectorLodControl()
{
  return m_vectorLodControl;
}

 /*****************************************************************************/
/** Returns the number of vertices.
  *
  * This function returns the number of vertices in the core submesh instance.
  *
  * @return The number of vertices.
  *****************************************************************************/

int CalCoreSubmesh::getVertexCount()
{
  return m_vectorVertex.size();
}

 /*****************************************************************************/
/** Returns the number of texture coordinate channels.
  *
  * This function returns the number of texture coordinate channels in
  * the core submesh instance.
  *
  * @return The number of texture coordinate channels.
  *****************************************************************************/

int CalCoreSubmesh::getTextureCoordinateCount()
{
  return m_vectorvectorTextureCoordinate.size();
}

 /*****************************************************************************/
/** Reserves memory for the vertices, faces and texture coordinates.
  *
  * This function reserves memory for the vertices, faces, texture coordinates
  * and springs of the core submesh instance.  It allocates exactly enough memory
  * to hold the number of elements specified, with no waste.
  *
  * @param vertexCount The number of vertices that this core submesh instance
  *                    should be able to hold.
  * @param textureCoordinateCount The number of texture coordinates that this
  *                               core submesh instance should be able to hold.
  * @param faceCount The number of faces that this core submesh instance should
  *                  be able to hold.
  * @param springCount The number of springs that this core submesh instance
  *                  should be able to hold.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::reserve(int vertexCount, int textureCoordinateCount, int faceCount, int springCount)
{
  int oldTextureCoordinateCount = m_vectorvectorTextureCoordinate.size();

  // reserve the space needed in all the vectors
  m_vectorVertex.reserve(vertexCount);
  m_vectorVertex.resize(vertexCount);
  
  m_vectorLodControl.reserve(vertexCount);
  m_vectorLodControl.resize(vertexCount);
  
  m_vectorTangentsEnabled.reserve(textureCoordinateCount);
  m_vectorTangentsEnabled.resize(textureCoordinateCount);
  
  m_vectorvectorTangentSpace.reserve(textureCoordinateCount);
  m_vectorvectorTangentSpace.resize(textureCoordinateCount);
  
  m_vectorvectorTextureCoordinate.reserve(textureCoordinateCount);
  m_vectorvectorTextureCoordinate.resize(textureCoordinateCount);
  
  int textureCoordinateId;
  for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId)
  {
    m_vectorvectorTextureCoordinate[textureCoordinateId].reserve(vertexCount);
    m_vectorvectorTextureCoordinate[textureCoordinateId].resize(vertexCount);

    if (textureCoordinateId >= oldTextureCoordinateCount)
    {
      m_vectorTangentsEnabled[textureCoordinateId] = false;
    }
    
    if (m_vectorTangentsEnabled[textureCoordinateId])
    {
      m_vectorvectorTangentSpace[textureCoordinateId].reserve(vertexCount);
      m_vectorvectorTangentSpace[textureCoordinateId].resize(vertexCount);
    }
    else
    {
      m_vectorvectorTangentSpace[textureCoordinateId].clear();
    }
  }
  
  m_vectorFace.reserve(faceCount);
  m_vectorFace.resize(faceCount);

  m_vectorSpring.reserve(springCount);
  m_vectorSpring.resize(springCount);

  // reserve the space for the physical properties if we have springs in the core submesh instance
  if(springCount > 0)
  {
    m_vectorPhysicalProperty.reserve(vertexCount);
    m_vectorPhysicalProperty.resize(vertexCount);
  }
  else
  {
    m_vectorPhysicalProperty.clear();
  }

  return true;
}

 /*****************************************************************************/
/** Resizes the arrays that hold vertices, faces and texture coordinates.
  *
  * This function resizes the arrays that hold vertices, faces, texture
  * coordinates and springs of the core submesh instance.  It sets aside extra
  * memory to make future "resize" operations fast. 
  *
  * @param vertexCount The number of vertices that this core submesh instance
  *                    should be able to hold.  If (-1), do not change the size
  *                    of the vertex array.
  * @param textureCoordinateCount The number of texture coordinates that this
  *                               core submesh instance should be able to hold.
  *                               If (-1), do not change the number of texture
  *                               coordinate channels.
  * @param faceCount The number of faces that this core submesh instance should
  *                  be able to hold.  If (-1), do not change the number of faces.
  * @param springCount The number of springs that this core submesh instance
  *                  should be able to hold.  If (-1), do not change the number of
  *                  springs.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::resize(int vertexCount, int textureCoordinateCount, int faceCount, int springCount)
{
  int oldTextureCoordinateCount = m_vectorvectorTextureCoordinate.size();

  int vertexReserve = 16;
  int textureCoordinateReserve = 16;
  int faceReserve = 16;
  int springReserve = 16;

  if (vertexCount            < 0) vertexCount = m_vectorVertex.size();
  if (textureCoordinateCount < 0) textureCoordinateCount = m_vectorvectorTextureCoordinate.size();
  if (faceCount              < 0) faceCount = m_vectorFace.size();
  if (springCount            < 0) springCount = m_vectorSpring.size();
  
  while (vertexReserve            < vertexCount)            vertexReserve<<=1;
  while (textureCoordinateReserve < textureCoordinateCount) textureCoordinateReserve<<=1;
  while (faceReserve              < faceCount)              faceReserve<<=1;
  while (springReserve            < springCount)            springReserve<<=1;

  // reserve the space needed in all the vectors
  m_vectorVertex.reserve(vertexReserve);
  m_vectorVertex.resize(vertexCount);

  m_vectorLodControl.reserve(vertexReserve);
  m_vectorLodControl.resize(vertexCount);
  
  m_vectorTangentsEnabled.reserve(textureCoordinateReserve);
  m_vectorTangentsEnabled.resize(textureCoordinateCount);
  
  m_vectorvectorTangentSpace.reserve(textureCoordinateReserve);
  m_vectorvectorTangentSpace.resize(textureCoordinateCount);
  
  m_vectorvectorTextureCoordinate.reserve(textureCoordinateReserve);
  m_vectorvectorTextureCoordinate.resize(textureCoordinateCount);
  
  int textureCoordinateId;
  for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
  {
    m_vectorvectorTextureCoordinate[textureCoordinateId].reserve(vertexReserve);
    m_vectorvectorTextureCoordinate[textureCoordinateId].resize(vertexCount);

    if (textureCoordinateId >= oldTextureCoordinateCount)
    {
      m_vectorTangentsEnabled[textureCoordinateId] = false;
    }
    
    if (m_vectorTangentsEnabled[textureCoordinateId])
    {
      m_vectorvectorTangentSpace[textureCoordinateId].reserve(vertexReserve);
      m_vectorvectorTangentSpace[textureCoordinateId].resize(vertexCount);
    }
    else
    {
      m_vectorvectorTangentSpace[textureCoordinateId].clear();
    }
  }
  
  m_vectorFace.reserve(faceReserve);
  m_vectorFace.resize(faceCount);

  m_vectorSpring.reserve(springReserve);
  m_vectorSpring.resize(springCount);

  // reserve the space for the physical properties if we have springs in the core submesh instance
  if(springCount > 0)
  {
    m_vectorPhysicalProperty.reserve(vertexReserve);
    m_vectorPhysicalProperty.resize(vertexCount);
  }
  else
  {
    m_vectorPhysicalProperty.clear();
  }

  return true;
}

 /*****************************************************************************/
/** Sets the ID of the core material thread.
  *
  * This function sets the ID of the core material thread of the core submesh
  * instance.
  *
  * @param coreMaterialThreadId The ID of the core material thread that should
  *                             be set.
  *****************************************************************************/

void CalCoreSubmesh::setCoreMaterialThreadId(int coreMaterialThreadId)
{
  m_coreMaterialThreadId = coreMaterialThreadId;
}

 /*****************************************************************************/
/** Sets a specified face.
  *
  * This function sets a specified face in the core submesh instance.
  *
  * @param faceId  The ID of the face.
  * @param face The face that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setFace(int faceId, const Face& face)
{
  if((faceId < 0) || (faceId >= (int)m_vectorFace.size())) return false;

  m_vectorFace[faceId] = face;

  return true;
}

 /*****************************************************************************/
/** Sets the number of LOD steps.
  *
  * This function sets the number of LOD steps of the core submesh instance.
  *
  * @param lodCount The number of LOD steps that should be set.
  *****************************************************************************/

void CalCoreSubmesh::setLodCount(int lodCount)
{
  m_lodCount = lodCount;
}

 /*****************************************************************************/
/** Sets a specified physical property.
  *
  * This function sets a specified physical property in the core submesh
  * instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param physicalProperty The physical property that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setPhysicalProperty(int vertexId, const PhysicalProperty& physicalProperty)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorPhysicalProperty.size())) return false;

  m_vectorPhysicalProperty[vertexId] = physicalProperty;

  return true;
}

 /*****************************************************************************/
/** Sets a specified spring.
  *
  * This function sets a specified spring in the core submesh instance.
  *
  * @param springId  The ID of the spring.
  * @param spring The spring that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setSpring(int springId, const Spring& spring)
{
  if((springId < 0) || (springId >= (int)m_vectorSpring.size())) return false;

  m_vectorSpring[springId] = spring;

  return true;
}

 /*****************************************************************************/
/** Sets a specified texture coordinate.
  *
  * This function sets a specified texture coordinate in the core submesh
  * instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param textureCoordinateId  The ID of the texture coordinate.
  * @param textureCoordinate The texture coordinate that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setTextureCoordinate(int vertexId, int textureCoordinateId, const TextureCoordinate& textureCoordinate)
{
  if((textureCoordinateId < 0) || (textureCoordinateId >= (int)m_vectorvectorTextureCoordinate.size())) return false;
  if((vertexId < 0) || (vertexId >= (int)m_vectorvectorTextureCoordinate[textureCoordinateId].size())) return false;

  m_vectorvectorTextureCoordinate[textureCoordinateId][vertexId] = textureCoordinate;

  return true;
}

 /*****************************************************************************/
/** Sets a specified vertex and normal
  *
  * This function sets a specified vertex and normal in the core submesh instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param position The position of the vertex
  * @param normal The normal of the vertex
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setVertex(int vertexId, const CalVector &position, const CalVector &normal)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;

  m_vectorVertex[vertexId].position = position;
  int nx = normal.x * 127.5;
  int ny = normal.y * 127.5;
  int nz = normal.z * 127.5;
  if (nx > 127) nx = 127; if (nx < -127) nx = -127;
  if (ny > 127) ny = 127; if (ny < -127) ny = -127;
  if (nz > 127) nz = 127; if (nz < -127) nz = -127;
  m_vectorVertex[vertexId].nx = nx;
  m_vectorVertex[vertexId].ny = ny;
  m_vectorVertex[vertexId].nz = nz;
  return true;
}

 /*****************************************************************************/
/** Sets the influence count of a vertex.
  *
  * This function sets the influence count of a vertex.
  *
  * @param vertexId  The ID of the vertex.
  * @param influenceCount The number of influences for the vertex.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setInfluenceCount(int vertexId, int influenceCount)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;
  if((influenceCount < 0) || (influenceCount > 127)) return false;

  m_vectorVertex[vertexId].influenceCount = influenceCount;
  
  return true;
}

 /*****************************************************************************/
/** Sets the LOD control parameters of a vertex.
  *
  * This function sets the LOD control parameters of a vertex.
  *
  * @param vertexId  The ID of the vertex.
  * @param faceCollapseCount The face collapse count.
  * @param collapseId The collapse ID.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setLodControl(int vertexId, int faceCollapseCount, int collapseId)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;

  m_vectorLodControl[vertexId].faceCollapseCount = faceCollapseCount;
  m_vectorLodControl[vertexId].collapseId = collapseId;
  
  return true;
}

 /*****************************************************************************/
/** Sets the tangent vector associated with a specified texture coordinate pair.
  *
  * This function sets the tangent vector associated with a specified
  * texture coordinate pair in the core submesh instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param textureCoordinateId The ID of the texture coordinate channel.
  * @param tangent   The tangent vector that should be stored.
  * @param crossFactor The cross-product factor that should be stored.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setTangentSpace(int vertexId, int textureCoordinateId, const CalVector& tangent, float crossFactor)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;
  if((textureCoordinateId < 0) || (textureCoordinateId >= (int)m_vectorvectorTextureCoordinate.size())) return false;
  
  int tx = tangent.x * 127.5;
  int ty = tangent.y * 127.5;
  int tz = tangent.z * 127.5;
  if (tx > 127) tx = 127; if (tx < -127) tx = -127;
  if (ty > 127) ty = 127; if (ty < -127) ty = -127;
  if (tz > 127) tz = 127; if (tz < -127) tz = -127;
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tx = tx;
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].ty = ty;
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tz = tz;
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].crossFactor = crossFactor;
  return true;
}

 /*****************************************************************************/
/** Gets the user data of a specified vertex
  *
  * This function gets the userdata of a specified vertex in the core submesh instance.
  *
  * @param vertexId  The ID of the vertex.
  *
  * @return One of the following values:
  *         \li \b a pointer to the userdata
  *         \li \b zero if an error occurred
  *****************************************************************************/

CalCoreVertexUserData *CalCoreSubmesh::getVertexUserData(int vertexId)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return 0;

  return &(m_vectorVertex[vertexId]);
}

 /*****************************************************************************/
/** Gets the texture coordinate set as an array of floats.
  *****************************************************************************/


float *CalCoreSubmesh::getTextureCoordinates(int mapId)
{
  if ((mapId < 0) || (mapId >= m_vectorvectorTextureCoordinate.size()))
  {
    return 0;
  }
  return &(m_vectorvectorTextureCoordinate[mapId][0].u);
}
