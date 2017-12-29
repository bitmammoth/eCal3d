//****************************************************************************//
// loader.cpp                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#undef DEBUG_LOADER

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "calloader.h"
#include "calerror.h"
#include "calvector.h"
#include "calmatrix.h"
#include "calquat.h"
#include "calcoremodel.h"
#include "calcorebone.h"
#include "calcoreanim.h"
#include "calcoretrack.h"
#include "calcorekey.h"
#include "calcoresub.h"
#include "buffersource.h"
#include "streamsource.h"

int CalLoader::loadingMode;
                                                                                                            
 /*****************************************************************************/
/** Sets optional flags which affect how the model is loaded into memory.
  *
  * This function sets the loading mode for all future loader calls.
  *
  * @param flags A boolean OR of any of the following flags
  *         \li LOADER_ROTATE_X_AXIS will rotate the mesh 90 degrees about the X axis,
  *             which has the effect of swapping Y/Z coordinates.
  *         \li LOADER_INVERT_V_COORD will substitute (1-v) for any v texture coordinate
  *             to eliminate the need for texture inversion after export.
  *
  *****************************************************************************/
void CalLoader::setLoadingMode(int flags)
{
  loadingMode = flags;
}

 /*****************************************************************************/
/** Constructs the loader instance.
  *
  * This function is the default constructor of the loader instance.
  *****************************************************************************/

CalLoader::CalLoader()
{
}

 /*****************************************************************************/
/** Destructs the loader instance.
  *
  * This function is the destructor of the loader instance.
  *****************************************************************************/

CalLoader::~CalLoader()
{
}

 /*****************************************************************************/
/** Loads a core animation.
  *****************************************************************************/

bool CalLoader::loadCoreAnimation(CalCoreAnimation *anim, const std::string& strFilename)
{
  // open the file
  std::ifstream file;
  file.open(strFilename.c_str(), std::ios::in | std::ios::binary);

  //make sure it was opened properly
  if(!file)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename);
    anim->destroy(); return false;
  }

  //make a new stream data source and use it to load the animation
  CalStreamSource streamSrc( file );

  bool coreanim = loadCoreAnimation(anim, streamSrc);

  //close the file
  file.close();

  return coreanim;
}

 /*****************************************************************************/
/** Loads a core animation instance.
  *
  * This function loads a core animation instance from an input stream.
  *
  * @param inputStream The stream to load the core animation instance from. This
  *                    stream should be initialized and ready to be read from.
  *
  * @return One of the following values:
  *         \li a pointer to the core animation
  *         \li \b 0 if an error happened
  *****************************************************************************/
                                                                                                            
bool CalLoader::loadCoreAnimation(CalCoreAnimation *anim, std::istream& inputStream, const std::string& strFilename)
{
   //Create a new istream data source and pass it on
   CalStreamSource streamSrc(inputStream);
   return loadCoreAnimation(anim, streamSrc);
}

bool CalLoader::loadCoreAnimation(CalCoreAnimation *anim, void* inputBuffer, int len, const std::string& strFilename)
{
  //Create a new buffer data source and pass it on
  CalBufferSource bufferSrc(inputBuffer);

  // std::istrstream file(data, len);
  return loadCoreAnimation(anim, bufferSrc);
}

bool CalLoader::loadCoreAnimation(CalCoreAnimation *anim, CalDataSource& dataSrc)
{
  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::ANIMATION_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    anim->destroy(); return false;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    anim->destroy(); return false;
  }

  // get the duration of the core animation
  float duration;
  if(!dataSrc.readFloat(duration))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    anim->destroy(); return false;
  }
  
  // check for a valid duration
  if(duration <= 0.0f)
  {
    CalError::setLastError(CalError::INVALID_ANIMATION_DURATION, __FILE__, __LINE__);
    anim->destroy(); return false;
  }

  // set the duration in the core animation instance
  anim->setDuration(duration);
  
  // read the number of tracks
  int trackCount;
  if(!dataSrc.readInteger(trackCount) || (trackCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    anim->destroy(); return false;
  }
  
  // load all core bones
  int trackId;
  for(trackId = 0; trackId < trackCount; ++trackId)
  {
    // load the core track
    CalCoreTrack *pCoreTrack;
    pCoreTrack = loadCoreTrack(dataSrc);
    if(pCoreTrack == 0) { anim->destroy(); return false; }
    anim->addCoreTrack(pCoreTrack);
  }
  
  return true;
}

 /*****************************************************************************/
/** Loads a core bone instance.
  *
  * This function loads a core bone instance from a file stream.
  *
  * @param pCoreBone the bone to initialize.
  * @param file The file stream to load the core bone instance from.
  * @param strFilename The name of the file stream.
  *
  * @return One of the following values:
  *         \li a pointer to the core bone
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreBone *CalLoader::loadCoreBones(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  // get the name length of the bone
  int len;
  if(!dataSrc.readInteger(len) || (len < 1))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // read the name of the bone
  char *strBuffer;
  strBuffer = new char[len];
  dataSrc.readBytes(strBuffer, len);
  std::string strName;
  strName = strBuffer;
  delete [] strBuffer;
  
  // read the length of the bone.
  float length;
  dataSrc.readFloat(length);
  
  // get the translation of the bone
  float tx, ty, tz;
  dataSrc.readFloat(tx);
  dataSrc.readFloat(ty);
  dataSrc.readFloat(tz);

  // get the rotation of the bone
  float rx, ry, rz, rw;
  dataSrc.readFloat(rx);
  dataSrc.readFloat(ry);
  dataSrc.readFloat(rz);
  dataSrc.readFloat(rw);

  // get the bone space translation of the bone
  float txBoneSpace, tyBoneSpace, tzBoneSpace;
  dataSrc.readFloat(txBoneSpace);
  dataSrc.readFloat(tyBoneSpace);
  dataSrc.readFloat(tzBoneSpace);

  // get the bone space rotation of the bone
  float rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace;
  dataSrc.readFloat(rxBoneSpace);
  dataSrc.readFloat(ryBoneSpace);
  dataSrc.readFloat(rzBoneSpace);
  dataSrc.readFloat(rwBoneSpace);

  // get the parent bone id
  int parentId;
  dataSrc.readInteger(parentId);

  CalQuaternion rot(rx,ry,rz,rw);
  CalQuaternion rotbs(rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace);
  CalVector trans(tx,ty,tz);

  if (loadingMode & LOADER_ROTATE_X_AXIS)
  {
    if (parentId == -1) // only root bone necessary
    {
      // Root bone must have quaternion rotated
      float temp = (float)sqrt(2.0f)/2.0f;
      CalQuaternion x_axis_90(temp,0.0f,0.0f,temp);
      rot *= x_axis_90;
      // Root bone must have translation rotated also
      trans.set(tx,tz,ty);
    }
  }

  // check if an error happend
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }
  
  // allocate a new core bone instance
  CalCoreBone *pCoreBone;
  pCoreBone = new CalCoreBone();
  if(pCoreBone == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__, "CalLoader::loadCoreBones");
    return 0;
  }

  // create the core bone instance
  if(!pCoreBone->create(strName))
  {
    delete pCoreBone;
    return 0;
  }

  // set the parent of the bone
  pCoreBone->setParentId(parentId);

  // set all attributes of the bone
  pCoreBone->setLength(length);
  pCoreBone->setTranslation(trans);
  pCoreBone->setRotation(rot);
  pCoreBone->setTranslationBoneSpace(CalVector(txBoneSpace, tyBoneSpace, tzBoneSpace));
  pCoreBone->setRotationBoneSpace(rotbs);
  
  // read the number of children
  int childCount;
  if(!dataSrc.readInteger(childCount) || (childCount < 0))
  {
    pCoreBone->destroy();
    delete pCoreBone;
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // load all children ids
  for(; childCount > 0; childCount--)
  {
    int childId;
    if(!dataSrc.readInteger(childId) || (childId < 0))
    {
      pCoreBone->destroy();
      delete pCoreBone;
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return 0;
    }

    pCoreBone->addChildId(childId);
  }

  return pCoreBone;
}

 /*****************************************************************************/
/** Loads a core keyframe instance.
  *
  * This function loads a core keyframe instance from a file stream.
  *
  * @param file The file stream to load the core keyframe instance from.
  * @param strFilename The name of the file stream.
  *
  * @return One of the following values:
  *         \li a pointer to the core keyframe
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreKeyframe *CalLoader::loadCoreKeyframe(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  // get the time of the keyframe
  float time;
  dataSrc.readFloat(time);

  // get the orientation of the bone
  float tx, ty, tz;
  dataSrc.readFloat(tx);
  dataSrc.readFloat(ty);
  dataSrc.readFloat(tz);

  // get the rotation of the bone
  float rx, ry, rz, rw;
  dataSrc.readFloat(rx);
  dataSrc.readFloat(ry);
  dataSrc.readFloat(rz);
  dataSrc.readFloat(rw);

  // check if an error happend
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // allocate a new core keyframe instance
  CalCoreKeyframe *pCoreKeyframe;
  pCoreKeyframe = new CalCoreKeyframe();
  if(pCoreKeyframe == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__, "CalLoader::loadCoreKeyframe");
    return 0;
  }

  // create the core keyframe instance
  if(!pCoreKeyframe->create())
  {
    delete pCoreKeyframe;
    return 0;
  }

  // set all attributes of the keyframe
  pCoreKeyframe->setTime(time);
  pCoreKeyframe->setOrientation(CalVector(tx, ty, tz));
  pCoreKeyframe->setRotation(CalQuaternion(rx, ry, rz, rw));

  return pCoreKeyframe;
}

 /*****************************************************************************/
/** Loads a core model.
  *****************************************************************************/

bool CalLoader::loadCoreModel(CalCoreModel *model, const std::string& strFilename)
{
  // open the file
  std::ifstream file;
  file.open(strFilename.c_str(), std::ios::in | std::ios::binary);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename);
    model->destroy(); return false;
  }
  
  //make a new stream data source and use it to load the animation
  CalStreamSource streamSrc( file );

  bool coremodel = loadCoreModel(model, streamSrc);

  //close the file
  file.close();

  return coremodel;
}

bool CalLoader::loadCoreModel(CalCoreModel *model, void* inputBuffer, int len, const std::string& strFilename)
{
  //Create a new buffer data source and pass it on
  CalBufferSource bufferSrc(inputBuffer);
  return loadCoreModel(model, bufferSrc);
}

bool CalLoader::loadCoreModel(CalCoreModel *model, CalDataSource& dataSrc)
{
  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::MODEL_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    model->destroy(); return false;
  }
  
  // read the number of bones
  int boneCount;
  if(!dataSrc.readInteger(boneCount) || (boneCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }
  
  // load all core bones
  int boneId;
  for(boneId = 0; boneId < boneCount; ++boneId)
  {
    // load the core bone
    CalCoreBone *pCoreBone;
    pCoreBone = loadCoreBones(dataSrc);
    if(pCoreBone == 0) { model->destroy(); return false; }
    
    // set the core skeleton of the core bone instance
    pCoreBone->setCoreModel(model);

    // add the core bone to the core skeleton instance
    model->m_vectorCoreBone.push_back(pCoreBone);
  }

  // get the number of submeshes
  int submeshCount;
  dataSrc.readInteger(submeshCount);
  
  // check if an error happend
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }

  // load all core submeshes
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; submeshId++)
  {
    // load the core submesh
    CalCoreSubmesh *pCoreSubmesh;
    pCoreSubmesh = loadCoreSubmesh(dataSrc);
    if(pCoreSubmesh == 0) { model->destroy(); return false; }

    // add the core submesh to the core mesh instance
    model->m_vectorCoreSubmesh.push_back(pCoreSubmesh);
  }

  return true;
}

bool CalLoader::loadCoreModel(CalCoreModel *model, const std::string& strFilename1, const std::string& strFilename2)
{
  // open the first file
  std::ifstream file1;
  file1.open(strFilename1.c_str(), std::ios::in | std::ios::binary);
  if(!file1)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename1);
    model->destroy(); return false;
  }

  // open the second file
  std::ifstream file2;
  file2.open(strFilename2.c_str(), std::ios::in | std::ios::binary);
  if(!file2)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename2);
    model->destroy(); return false;
  }
  
  CalStreamSource streamSrc1( file1 );
  CalStreamSource streamSrc2( file2 );

  bool result = loadCoreModel(model, streamSrc1, streamSrc2);
  file1.close();
  file2.close();
  return result;
}

bool CalLoader::loadCoreModel(CalCoreModel *model,
			      void* inputBuffer1, int len1, const std::string& strFilename1,
			      void* inputBuffer2, int len2, const std::string& strFilename2)
{
  CalBufferSource bufferSrc1(inputBuffer1);
  CalBufferSource bufferSrc2(inputBuffer2);
  return loadCoreModel(model, bufferSrc1, bufferSrc2);
}

bool CalLoader::loadCoreModel(CalCoreModel *model, CalDataSource& dataSrc1, CalDataSource& dataSrc2)
{
  // check if this is a valid file
  char magic[4];
  if(!dataSrc1.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::SKELETON_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc1.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    model->destroy(); return false;
  }
  
  // read the number of bones
  int boneCount;
  if(!dataSrc1.readInteger(boneCount) || (boneCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }
  
  // load all core bones
  int boneId;
  for(boneId = 0; boneId < boneCount; ++boneId)
  {
    // load the core bone
    CalCoreBone *pCoreBone;
    pCoreBone = loadCoreBones(dataSrc1);
    if(pCoreBone == 0) { model->destroy(); return false; }

    // set the core skeleton of the core bone instance
    pCoreBone->setCoreModel(model);

    // add the core bone to the core skeleton instance
    model->m_vectorCoreBone.push_back(pCoreBone);
  }

  // calculate state of the core skeleton
  model->calculateState();

  
  // check if this is a valid file
  if(!dataSrc2.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::MESH_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }

  // check if the version is compatible with the library
  if(!dataSrc2.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    model->destroy(); return false;
  }
  
  // get the number of submeshes
  int submeshCount;
  dataSrc2.readInteger(submeshCount);
  
  // check if an error happend
  if(!dataSrc2.ok())
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    model->destroy(); return false;
  }

  // load all core submeshes
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; submeshId++)
  {
    // load the core submesh
    CalCoreSubmesh *pCoreSubmesh;
    pCoreSubmesh = loadCoreSubmesh(dataSrc2);
    if(pCoreSubmesh == 0) { model->destroy(); return false; }

    // add the core submesh to the core mesh instance
    model->m_vectorCoreSubmesh.push_back(pCoreSubmesh);
  }

  return true;
}

 /*****************************************************************************/
/** Loads a core submesh instance.
  *
  * This function loads a core submesh instance from a file stream.
  *
  * @param file The file stream to load the core submesh instance from.
  * @param strFilename The name of the file stream.
  *
  * @return One of the following values:
  *         \li a pointer to the core submesh
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreSubmesh *CalLoader::loadCoreSubmesh(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  // get the material thread id of the submesh
  int coreMaterialThreadId;
  dataSrc.readInteger(coreMaterialThreadId);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: coreMaterialThreadId: %d\n", coreMaterialThreadId);
#endif

  // get the number of vertices, faces, level-of-details, springs, and texture coordinates
  int vertexCount;
  dataSrc.readInteger(vertexCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: vertexCount: %d\n", vertexCount);
#endif

  int faceCount;
  dataSrc.readInteger(faceCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: faceCount: %d\n", faceCount);
#endif

  int lodCount;
  dataSrc.readInteger(lodCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: lodCount: %d\n", lodCount);
#endif

  int springCount;
  dataSrc.readInteger(springCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: springCount: %d\n", springCount);
#endif

  int textureCoordinateCount;
  dataSrc.readInteger(textureCoordinateCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: textureCoordinateCount: %d\n", textureCoordinateCount);
#endif

  // check if an error happend
  if(!dataSrc.ok())
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // allocate a new core submesh instance
  CalCoreSubmesh *pCoreSubmesh;
  pCoreSubmesh = new CalCoreSubmesh();
  if(pCoreSubmesh == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__, "CalLoader::loadCoreSubmesh");
    return 0;
  }

  // create the core submesh instance
  if(!pCoreSubmesh->create())
  {
    delete pCoreSubmesh;
    return 0;
  }

  // set the LOD step count
  pCoreSubmesh->setLodCount(lodCount);

  // set the core material id
  pCoreSubmesh->setCoreMaterialThreadId(coreMaterialThreadId);

  // reserve memory for all the submesh data
  if(!pCoreSubmesh->reserve(vertexCount, textureCoordinateCount, faceCount, springCount))
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    pCoreSubmesh->destroy();
    delete pCoreSubmesh;
    return 0;
  }
  
  // load the tangent space enable flags.
  int textureCoordinateId;
  for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
  {
    char ena; bool enabled = false;
    dataSrc.readBytes((void *)&ena, 1);
    if(ena) enabled = true;
    pCoreSubmesh->enableTangents(textureCoordinateId, enabled);
#ifdef DEBUG_LOADER
    printf("loadCoreSubMesh: enableTangents(%d): %d %c (should be 0 / non-0)\n", textureCoordinateId, enabled, ena);
#endif
  }

  // Get the influence vector.
  std::vector<CalCoreSubmesh::Influence>& vectorInfluence = pCoreSubmesh->getVectorInfluence();
  
  // Get the vertex vector.
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pCoreSubmesh->getVectorVertex();
  
  // Get the tangent space vectors
  std::vector<std::vector<CalCoreSubmesh::TangentSpace> >& vectorvectorTangentSpace =
    pCoreSubmesh->getVectorVectorTangentSpace();
  
  // load all vertices and their influences
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; vertexId++)
  {
    // The vertex we're setting.
    CalCoreSubmesh::Vertex &vertex = vectorVertex[vertexId];
    
    // load data of the vertex
    dataSrc.readFloat(vertex.position.x);
    dataSrc.readFloat(vertex.position.y);
    dataSrc.readFloat(vertex.position.z);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: vertex.position: %f %f %f\n", vertex.position.x, vertex.position.y, vertex.position.z);
#endif

    char nxyz[3];
    dataSrc.readBytes(nxyz, 3);
    vertex.nx = nxyz[0];
    vertex.ny = nxyz[1];
    vertex.nz = nxyz[2];
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: vertex. nx ny nz: %d %d %d\n", vertex.nx, vertex.ny, vertex.nz);
#endif

    // Load the LOD control information.
    int faceCollapseCount, collapseId;
    dataSrc.readInteger(collapseId);
    dataSrc.readInteger(faceCollapseCount);
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: collapseId faceCollapseCount: %d %d\n", collapseId, faceCollapseCount);
#endif

    pCoreSubmesh->setLodControl(vertexId, faceCollapseCount, collapseId);
    
    // check if an error happend
    if(!dataSrc.ok())
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      pCoreSubmesh->destroy();
      delete pCoreSubmesh;
      return 0;
    }
    
    // load all texture coordinates of the vertex
    int textureCoordinateId;
    for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
    {
      CalCoreSubmesh::TextureCoordinate textureCoordinate;

      // load data of the influence
      dataSrc.readFloat(textureCoordinate.u);
      dataSrc.readFloat(textureCoordinate.v);
#ifdef DEBUG_LOADER
      printf("loadCoreSubMesh: textureCoordinate(%d): %f %f\n", textureCoordinateId,textureCoordinate.u, textureCoordinate.v);
#endif

      pCoreSubmesh->setTextureCoordinate(vertexId, textureCoordinateId, textureCoordinate);
      
      if (pCoreSubmesh->tangentsEnabled(textureCoordinateId))
      {
	char tanspace[4];
	dataSrc.readBytes(tanspace, 4);
	vectorvectorTangentSpace[textureCoordinateId][vertexId].tx = tanspace[0];
	vectorvectorTangentSpace[textureCoordinateId][vertexId].ty = tanspace[1];
	vectorvectorTangentSpace[textureCoordinateId][vertexId].tz = tanspace[2];
	vectorvectorTangentSpace[textureCoordinateId][vertexId].crossFactor = tanspace[3];
#ifdef DEBUG_LOADER
        printf("loadCoreSubMesh: vectorTangentSpace(%d, %d): %d %d %d %d\n", textureCoordinateId, vertexId, tanspace[0], tanspace[1], tanspace[2], tanspace[3]);
#endif
      }
      
      // check if an error happend
      if(!dataSrc.ok())
      {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        pCoreSubmesh->destroy();
        delete pCoreSubmesh;
        return 0;
      }
    }

    // get the number of influences
    int influenceCount;
    dataSrc.readInteger(influenceCount);
    vertex.influenceCount = influenceCount;
#ifdef DEBUG_LOADER
    printf("loadCoreSubMesh: influenceCount: %d\n", influenceCount);
#endif
    
    // check if an error happend
    if(!dataSrc.ok())
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      pCoreSubmesh->destroy();
      delete pCoreSubmesh;
      return 0;
    }
    
    int firstInfluence = vectorInfluence.size();
    vectorInfluence.resize(firstInfluence + vertex.influenceCount);
    
    // load all influences of the vertex
    int influenceId;
    for(influenceId = 0; influenceId < vertex.influenceCount; influenceId++)
    {
      // load data of the influence
      dataSrc.readInteger(vectorInfluence[firstInfluence + influenceId].boneId);
      dataSrc.readFloat(vectorInfluence[firstInfluence + influenceId].weight);

      // check if an error happend
      if(!dataSrc.ok())
      {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        pCoreSubmesh->destroy();
        delete pCoreSubmesh;
        return 0;
      }
    }
    
    // load the physical property of the vertex if there are springs in the core submesh
    if(springCount > 0)
    {
      CalCoreSubmesh::PhysicalProperty physicalProperty;

      // load data of the physical property
      dataSrc.readFloat(physicalProperty.weight);

      // check if an error happend
      if(!dataSrc.ok())
      {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        pCoreSubmesh->destroy();
        delete pCoreSubmesh;
        return 0;
      }

      // set the physical property in the core submesh instance
      pCoreSubmesh->setPhysicalProperty(vertexId, physicalProperty);
    }
  }

  // Pack the influence vector.
  vectorInfluence.reserve(vectorInfluence.size());
  
  // load all springs
  int springId;
  for(springId = 0; springId < springCount; springId++)
  {
    CalCoreSubmesh::Spring spring;

    // load data of the spring
    dataSrc.readInteger(spring.vertexId[0]);
    dataSrc.readInteger(spring.vertexId[1]);
    dataSrc.readFloat(spring.springCoefficient);
    dataSrc.readFloat(spring.idleLength);

    // check if an error happend
    if(!dataSrc.ok())
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      pCoreSubmesh->destroy();
      delete pCoreSubmesh;
      return 0;
    }

    // set spring in the core submesh instance
    pCoreSubmesh->setSpring(springId, spring);
  }

  // load all faces
  int faceId;
  for(faceId = 0; faceId < faceCount; faceId++)
  {
    CalCoreSubmesh::Face face;

    // load data of the face
    dataSrc.readInteger(face.vertexId[0]);
    dataSrc.readInteger(face.vertexId[1]);
    dataSrc.readInteger(face.vertexId[2]);

    // check if an error happend
    if(!dataSrc.ok())
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      pCoreSubmesh->destroy();
      delete pCoreSubmesh;
      return 0;
    }

    // set face in the core submesh instance
    pCoreSubmesh->setFace(faceId, face);
  }
#ifdef DEBUG_LOADER
  printf("loadCoreSubMesh: DONE!!!!!!\n\n\n\n\n");
#endif
    

  return pCoreSubmesh;
}

 /*****************************************************************************/
/** Loads a core track instance.
  *
  * This function loads a core track instance from a data source.
  *
  * @param dataSrc The data source to load the core track instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core track
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreTrack *CalLoader::loadCoreTrack(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // BUG BUG BUG - Code SHOULD use readString section below rather than the rest, but
  // readString crashes and I dont have time to putz with it...
  // get the name length of the bone
  int len;
  if(!dataSrc.readInteger(len) || (len < 1))
  //std::string strName;
  // if(!dataSrc.readString(strName))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // read the name of the bone
  char *strBuffer;
  strBuffer = new char[len];
  dataSrc.readBytes(strBuffer, len);
  std::string strName;
  strName = strBuffer;
  delete [] strBuffer;

  // END BUG

  // allocate a new core track instance
  CalCoreTrack *pCoreTrack;
  pCoreTrack = new CalCoreTrack();
  if(pCoreTrack == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // create the core track instance
  if(!pCoreTrack->create())
  {
    delete pCoreTrack;
    return 0;
  }

  // link the core track to the appropriate core bone
  pCoreTrack->setCoreBoneName(strName);

  // read the number of keyframes
  int keyframeCount;
  if(!dataSrc.readInteger(keyframeCount) || (keyframeCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // load all core keyframes
  int keyframeId;
  for(keyframeId = 0; keyframeId < keyframeCount; ++keyframeId)
  {
    // load the core keyframe
    CalCoreKeyframe *pCoreKeyframe;
    pCoreKeyframe = loadCoreKeyframe(dataSrc);
    if(pCoreKeyframe == 0)
    {
      pCoreTrack->destroy();
      delete pCoreTrack;
      return 0;
    }

//    if (loadingMode & LOADER_ROTATE_X_AXIS)
//    {
//      // Check for anim rotation
//      if (!coreBoneId)  // root bone
//      {
//        // rotate root bone quaternion
//        CalQuaternion rot = pCoreKeyframe->getRotation();
//        float temp = (float)sqrt(2.0f)/2.0f;
//        CalQuaternion x_axis_90(temp,0.0f,0.0f,temp);
//        rot *= x_axis_90;
//        pCoreKeyframe->setRotation(rot);
//        // rotate root bone displacement
//        CalVector vec = pCoreKeyframe->getTranslation();
//        temp = vec.y;
//        vec.y = vec.z;
//        vec.z = temp;
//        pCoreKeyframe->setTranslation(vec);
//      }
//    }

    // add the core keyframe to the core track instance
    pCoreTrack->addCoreKeyframe(pCoreKeyframe);
  }

  return pCoreTrack;
}

//****************************************************************************//
