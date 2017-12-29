//****************************************************************************//
// saver.cpp                                                                  //
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

#include "calsaver.h"
#include "calerror.h"
#include "calvector.h"
#include "calquat.h"
#include "calcoremodel.h"
#include "calcorebone.h"
#include "calcoreanim.h"
#include "calcoretrack.h"
#include "calcorekey.h"
#include "calcoresub.h"

 /*****************************************************************************/
/** Constructs the saver instance.
  *
  * This function is the default constructor of the saver instance.
  *****************************************************************************/

CalSaver::CalSaver()
{
}

 /*****************************************************************************/
/** Destructs the saver instance.
  *
  * This function is the destructor of the saver instance.
  *****************************************************************************/

CalSaver::~CalSaver()
{
}

 /*****************************************************************************/
/** Saves a core animation instance.
  *
  * This function saves a core animation instance to a file.
  *
  * @param strFilename The name of the file to save the core animation instance
  *                    to.
  * @param pCoreAnimation A pointer to the core animation instance that should
  *                       be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreAnimation(const std::string& strFilename, CalCoreAnimation *pCoreAnimation)
{
  // open the file
  std::ofstream file;
  file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write magic tag
  file.write((char *)&Cal::ANIMATION_FILE_MAGIC, sizeof(Cal::ANIMATION_FILE_MAGIC));
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write version info
  file.write((char *)&Cal::CURRENT_FILE_VERSION, sizeof(Cal::CURRENT_FILE_VERSION));
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the duration of the core animation
  float duration;
  duration = pCoreAnimation->getDuration();

  file.write((char *)&duration, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // get core track list
  std::list<CalCoreTrack *>& listCoreTrack = pCoreAnimation->getListCoreTrack();

  // write the number of tracks
  int trackCount;
  trackCount = listCoreTrack.size();

  file.write((char *)&trackCount, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return 0;
  }

  // write all core bones
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
  {
    // save core track
    if(!saveCoreTrack(file, strFilename, *iteratorCoreTrack))
    {
      return false;
    }
  }

  // explicitly close the file
  file.close();

  return true;
}

 /*****************************************************************************/
/** Saves a core bone instance.
  *
  * This function saves a core bone instance to a file stream.
  *
  * @param file The file stream to save the core bone instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreBone A pointer to the core bone instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreBones(std::ofstream& file, const std::string& strFilename, CalCoreBone *pCoreBone)
{
  if(!file)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the name of the bone
  int len;
  len = pCoreBone->getName().size() + 1;
  file.write((char *)&len, 4);
  file.write(pCoreBone->getName().c_str(), len);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the length of the bone.
  float length = pCoreBone->getLength();
  file.write((char *)&length, 4);

  // write the translation of the bone
  const CalVector& translation = pCoreBone->getTranslation();
  file.write((char *)&translation[0], 4);
  file.write((char *)&translation[1], 4);
  file.write((char *)&translation[2], 4);

  // write the rotation of the bone
  const CalQuaternion& rotation = pCoreBone->getRotation();
  file.write((char *)&rotation[0], 4);
  file.write((char *)&rotation[1], 4);
  file.write((char *)&rotation[2], 4);
  file.write((char *)&rotation[3], 4);

  // write the translation of the bone
  const CalVector& translationBoneSpace = pCoreBone->getTranslationBoneSpace();
  file.write((char *)&translationBoneSpace[0], 4);
  file.write((char *)&translationBoneSpace[1], 4);
  file.write((char *)&translationBoneSpace[2], 4);

  // write the rotation of the bone
  const CalQuaternion& rotationBoneSpace = pCoreBone->getRotationBoneSpace();
  file.write((char *)&rotationBoneSpace[0], 4);
  file.write((char *)&rotationBoneSpace[1], 4);
  file.write((char *)&rotationBoneSpace[2], 4);
  file.write((char *)&rotationBoneSpace[3], 4);

  // write the parent bone id
  int parentId;
  parentId = pCoreBone->getParentId();
  file.write((char *)&parentId, 4);

  // check if an error happend
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // get children list
  std::list<int>& listChildId = pCoreBone->getListChildId();

  // write the number of children
  int childCount;
  childCount = listChildId.size();

  file.write((char *)&childCount, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write all children ids
  std::list<int>::iterator iteratorChildId;
  for(iteratorChildId = listChildId.begin(); iteratorChildId != listChildId.end(); ++iteratorChildId)
  {
    // get child id
    int childId;
    childId = *iteratorChildId;

    // write child id
    file.write((char *)&childId, 4);
    if(!file)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core keyframe instance.
  *
  * This function saves a core keyframe instance to a file stream.
  *
  * @param file The file stream to save the core keyframe instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreKeyframe A pointer to the core keyframe instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreKeyframe(std::ofstream& file, const std::string& strFilename, CalCoreKeyframe *pCoreKeyframe)
{
  if(!file)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the time of the keyframe
  float time;
  time = pCoreKeyframe->getTime();
  file.write((char *)&time, 4);

  // write the orientation of the keyframe
  const CalVector& translation = pCoreKeyframe->getOrientation();
  file.write((char *)&translation[0], 4);
  file.write((char *)&translation[1], 4);
  file.write((char *)&translation[2], 4);

  // write the rotation of the keyframe
  const CalQuaternion& rotation = pCoreKeyframe->getRotation();
  file.write((char *)&rotation[0], 4);
  file.write((char *)&rotation[1], 4);
  file.write((char *)&rotation[2], 4);
  file.write((char *)&rotation[3], 4);

  // check if an error happend
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  return true;
}


 /*****************************************************************************/
/** Saves a core model instance.
  *
  * This function saves a core model instance to a file.
  *
  * @param strFilename The name of the file to save the core mesh instance to.
  * @param pCoreMesh A pointer to the core mesh instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreModel(const std::string& strFilename, CalCoreModel *pCoreModel)
{
  // open the file
  std::ofstream file;
  file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write magic tag
  file.write((char *)&Cal::MODEL_FILE_MAGIC, sizeof(Cal::MODEL_FILE_MAGIC));
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write version info
  file.write((char *)&Cal::CURRENT_FILE_VERSION, sizeof(Cal::CURRENT_FILE_VERSION));
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the number of bones
  int boneCount;
  boneCount = pCoreModel->getCoreBoneCount();

  file.write((char *)&boneCount, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write all core bones
  int boneId;
  for(boneId = 0; boneId < boneCount; boneId++)
  {
    // write the core bone
    if(!saveCoreBones(file, strFilename, pCoreModel->getCoreBone(boneId)))
    {
      return false;
    }
  }

  // write the number of submeshes
  int submeshCount;
  submeshCount = pCoreModel->getCoreSubmeshCount();

  file.write((char *)&submeshCount, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write all core submeshes
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; submeshId++)
  {
    // write the core submesh
    if(!saveCoreSubmesh(file, strFilename, pCoreModel->getCoreSubmesh(submeshId)))
    {
      return false;
    }
  }

  // explicitly close the file
  file.close();

  return true;
}


 /*****************************************************************************/
/** Saves a core submesh instance.
  *
  * This function saves a core submesh instance to a file stream.
  *
  * @param file The file stream to save the core submesh instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreSubmesh A pointer to the core submesh instance that should be
  *                     saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreSubmesh(std::ofstream& file, const std::string& strFilename, CalCoreSubmesh *pCoreSubmesh)
{
  if(!file)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
    return false;
  }

  // write the core material thread id
  int coreMaterialThreadId;
  coreMaterialThreadId = pCoreSubmesh->getCoreMaterialThreadId();
  file.write((char *)&coreMaterialThreadId, 4);

  // check if an error happend
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // get the vertex, face, physical property, and spring vector
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pCoreSubmesh->getVectorVertex();
  std::vector<CalCoreSubmesh::Face>& vectorFace = pCoreSubmesh->getVectorFace();
  std::vector<CalCoreSubmesh::PhysicalProperty>& vectorPhysicalProperty = pCoreSubmesh->getVectorPhysicalProperty();
  std::vector<CalCoreSubmesh::Spring>& vectorSpring = pCoreSubmesh->getVectorSpring();
  std::vector<CalCoreSubmesh::LodControl>& vectorLodControl = pCoreSubmesh->getVectorLodControl();
  
  // write the number of vertices, faces, level-of-details, springs, and maps
  int vertexCount;
  vertexCount = vectorVertex.size();
  file.write((char *)&vertexCount, 4);

  int faceCount;
  faceCount = vectorFace.size();
  file.write((char *)&faceCount, 4);

  int lodCount;
  lodCount = pCoreSubmesh->getLodCount();
  file.write((char *)&lodCount, 4);

  int springCount;
  springCount = pCoreSubmesh->getSpringCount();
  file.write((char *)&springCount, 4);

  int textureCoordinateCount;
  textureCoordinateCount = pCoreSubmesh->getTextureCoordinateCount();
  file.write((char *)&textureCoordinateCount, 4);

  // Write the tangent-space enabled flags.
  int textureCoordinateId;
  for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
  {
    bool enabled = pCoreSubmesh->tangentsEnabled(textureCoordinateId);
    file.write((char *)&enabled, 1);
  }
  
  // check if an error happend
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }
  
  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >
    &vectorvectorTextureCoordinate = pCoreSubmesh->getVectorVectorTextureCoordinate();
  std::vector<std::vector<CalCoreSubmesh::TangentSpace> >
    &vectorvectorTangentSpace = pCoreSubmesh->getVectorVectorTangentSpace();

  // Get the influence vector.
  std::vector<CalCoreSubmesh::Influence>& vectorInfluence = pCoreSubmesh->getVectorInfluence();
  int nextInfluence = 0;

  // write all vertices
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; vertexId++)
  {
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];
    CalCoreSubmesh::LodControl& lodcontrol = vectorLodControl[vertexId];
    
    // write the vertex data
    file.write((char *)&vertex.position.x, 4);
    file.write((char *)&vertex.position.y, 4);
    file.write((char *)&vertex.position.z, 4);
    char nxyz[3];
    nxyz[0] = vertex.nx;
    nxyz[1] = vertex.ny;
    nxyz[2] = vertex.nz;
    file.write((char *)nxyz, 3);
    
    // write the LOD control information.
    int collapseId = lodcontrol.collapseId;
    int faceCollapseCount = lodcontrol.faceCollapseCount;
    file.write((char *)&collapseId, 4);
    file.write((char *)&faceCollapseCount, 4);
    
    // write all texture coordinates of this vertex
    for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; textureCoordinateId++)
    {
      CalCoreSubmesh::TextureCoordinate& textureCoordinate = vectorvectorTextureCoordinate[textureCoordinateId][vertexId];

      // write the texture coordinate data
      file.write((char *)&textureCoordinate.u, 4);
      file.write((char *)&textureCoordinate.v, 4);
      
      if (pCoreSubmesh->tangentsEnabled(textureCoordinateId))
      {
	CalCoreSubmesh::TangentSpace& tangentSpace = vectorvectorTangentSpace[textureCoordinateId][vertexId];
	char tanspace[4];
	tanspace[0] = tangentSpace.tx;
	tanspace[1] = tangentSpace.ty;
	tanspace[2] = tangentSpace.tz;
	tanspace[3] = tangentSpace.crossFactor;
	file.write((char *)tanspace, 4);
      }
      
      // check if an error happend
      if(!file)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
      }
    }
    
    // write the number of influences
    int influenceCount = vertex.influenceCount;
    file.write((char *)&influenceCount, 4);
    
    // check if an error happend
    if(!file)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
      return false;
    }
    
     // write all influences of this vertex
    int influenceId;
    for(influenceId = 0; influenceId < vertex.influenceCount; influenceId++)
    {
      CalCoreSubmesh::Influence& influence = vectorInfluence[nextInfluence + influenceId];

      // write the influence data
      file.write((char *)&influence.boneId, 4);
      file.write((char *)&influence.weight, 4);

      // check if an error happend
      if(!file)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
      }
    }
    nextInfluence += vertex.influenceCount;

    // save the physical property of the vertex if there are springs in the core submesh
    if(springCount > 0)
    {
      // write the physical property of this vertex if there are springs in the core submesh
      CalCoreSubmesh::PhysicalProperty& physicalProperty = vectorPhysicalProperty[vertexId];

      // write the physical property data
      file.write((char *)&physicalProperty.weight, 4);

      // check if an error happend
      if(!file)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
      }
    }
  }

  // write all springs
  int springId;
  for(springId = 0; springId < springCount; springId++)
  {
    CalCoreSubmesh::Spring& spring = vectorSpring[springId];

    // write the spring data
    file.write((char *)&spring.vertexId[0], 4);
    file.write((char *)&spring.vertexId[1], 4);
    file.write((char *)&spring.springCoefficient, 4);
    file.write((char *)&spring.idleLength, 4);

    // check if an error happend
    if(!file)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
      return false;
    }
  }

  // write all faces
  int faceId;
  for(faceId = 0; faceId < faceCount; faceId++)
  {
    CalCoreSubmesh::Face& face = vectorFace[faceId];

    // write the face data
    file.write((char *)&face.vertexId[0], 4);
    file.write((char *)&face.vertexId[1], 4);
    file.write((char *)&face.vertexId[2], 4);

    // check if an error happend
    if(!file)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core track instance.
  *
  * This function saves a core track instance to a file stream.
  *
  * @param file The file stream to save the core track instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreTrack A pointer to the core track instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreTrack(std::ofstream& file, const std::string& strFilename, CalCoreTrack *pCoreTrack)
{
  if(!file)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
    return false;
  }
  
  // write the name of the bone
  int len;
  len = pCoreTrack->getCoreBoneName().size() + 1;
  file.write((char *)&len, 4);
  file.write(pCoreTrack->getCoreBoneName().c_str(), len);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }
  
  // get core keyframe map
  std::map<float, CalCoreKeyframe *>& mapCoreKeyframe = pCoreTrack->getMapCoreKeyframe();

  // read the number of keyframes
  int keyframeCount;
  keyframeCount = mapCoreKeyframe.size();

  file.write((char *)&keyframeCount, 4);
  if(!file)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
    return false;
  }

  // save all core keyframes
  std::map<float, CalCoreKeyframe *>::iterator iteratorCoreKeyframe;
  for(iteratorCoreKeyframe = mapCoreKeyframe.begin(); iteratorCoreKeyframe != mapCoreKeyframe.end(); ++iteratorCoreKeyframe)
  {
    // save the core keyframe
    if(!saveCoreKeyframe(file, strFilename, iteratorCoreKeyframe->second))
    {
      return false;
    }
  }

  return true;
}

//****************************************************************************//
