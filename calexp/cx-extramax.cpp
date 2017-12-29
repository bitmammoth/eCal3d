//////////////////////////////////////////////////////////////////
//
// ExtraMax - Some simple functions layered on top of the MAX API,
// designed to make access to MAX data structures a little easier.
//
//////////////////////////////////////////////////////////////////

#include "max.h"
#include "bipexp.h"
#include "phyexp.h"
#include "cx-extramax.h"
#if MAX_RELEASE >= 4000
#include "iparamb2.h"
#include "iskin.h"
#endif

Matrix3 ExtraMax_Transpose(Matrix3& matrix)
{
  float transpose[4][3];
  
  int row;
  for(row = 0; row < 3; row++)
    {
      int column;
      for(column = 0; column < 3; column++)
	{
	  transpose[column][row] = matrix.GetAddr()[row][column];
	}
    }
  
  int column;
  for(column = 0; column < 3; column++)
    {
      transpose[3][column] = 0;
    }
  
  Matrix3 transposeMatrix(transpose);
  transposeMatrix.NoTrans();
  return transposeMatrix;
}

bool ExtraMax_IsBone(INode *pNode)
{
  // check for root node
  if(pNode->IsRootNode()) return false;

  // check for bone node
  ObjectState os;
  os = pNode->EvalWorldState(0);
  if(os.obj->ClassID() == Class_ID(BONE_CLASS_ID, 0)) return true;

#if MAX_RELEASE >= 4000
  if(os.obj->ClassID() == BONE_OBJ_CLASSID) return true;
#endif

  if(os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0)) return false;

  // check for biped node
  Control *pControl;
  pControl = pNode->GetTMController();
  if((pControl->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) || (pControl->ClassID() == BIPBODY_CONTROL_CLASS_ID)) return true;
  
  return false;
}

bool ExtraMax_IsMesh(INode *pNode)
{
  // check for root node
  if(pNode->IsRootNode()) return false;

  // check for mesh node
  ObjectState os;
  os = pNode->EvalWorldState(0);
  if(os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) return true;

  return false;
}

Modifier *ExtraMax_FindPhysiqueModifier(INode *pINode)
{
  // get the object reference of the node
  Object *pObject;
  pObject = pINode->GetObjectRef();
  if(pObject == 0) return 0;
  
  // loop through all derived objects
  while(pObject->SuperClassID() == GEN_DERIVOB_CLASS_ID)
    {
      IDerivedObject *pDerivedObject;
      pDerivedObject = static_cast<IDerivedObject *>(pObject);
      
      // loop through all modifiers
      int stackId;
      for(stackId = 0; stackId < pDerivedObject->NumModifiers(); stackId++)
	{
	  // get the modifier
	  Modifier *pModifier;
	  pModifier = pDerivedObject->GetModifier(stackId);
	  
	  // check if we found the physique modifier
	  if(pModifier->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)) return pModifier;
	}
      
      // continue with next derived object
      pObject = pDerivedObject->GetObjRef();
    }
  
  return 0;
}


Modifier *ExtraMax_FindSkinModifier(INode *pINode)
{
#if MAX_RELEASE >= 4000
  // get the object reference of the node
  Object *pObject;
  pObject = pINode->GetObjectRef();
  if(pObject == 0) return 0;
  
  // loop through all derived objects
  while(pObject->SuperClassID() == GEN_DERIVOB_CLASS_ID)
    {
      IDerivedObject *pDerivedObject;
      pDerivedObject = static_cast<IDerivedObject *>(pObject);
      
      // loop through all modifiers
      int stackId;
      for(stackId = 0; stackId < pDerivedObject->NumModifiers(); stackId++)
	{
	  // get the modifier
	  Modifier *pModifier;
	  pModifier = pDerivedObject->GetModifier(stackId);
	  
	  // check if we found the skin modifier
	  if(pModifier->ClassID() == SKIN_CLASSID) return pModifier;
	}
      
      // continue with next derived object
      pObject = pDerivedObject->GetObjRef();
    }
  
#endif
  return 0;
}

void ExtraMax_AlphaSortNodes(INode **bones, int bonecount)
{
  for (int i=0; i<bonecount; i++) {
    for (int j=i+1; j<bonecount; j++) {
      INode *bone1 = bones[i];
      INode *bone2 = bones[j];
      char name1[256];
      char name2[256];
      strncpy(name1, bone1->GetName(), 255);
      strncpy(name2, bone2->GetName(), 255);
      if (strcmp(name1,name2) > 0) { bones[i]=bone2; bones[j]=bone1; }
    }
  }
}

Point3 ExtraMax_GetVertexNormal(Mesh *mesh, int faceId, int faceVertexId)
{
  // get the face and the vertex.
  Face *pFace = &(mesh->faces[faceId]);
  int vertexId = pFace->v[faceVertexId];
  
  // get the "rendered" vertex
  RVertex *pRVertex = mesh->getRVertPtr(vertexId);
  
  // If the normal is specified, return it.
  if(pRVertex->rFlags & SPECIFIED_NORMAL) return pRVertex->rn.getNormal();
  
  // get the number of normals
  int normalCount = pRVertex->rFlags & NORCT_MASK;

  // If only one normal is found, return it.
  if (normalCount == 1) return pRVertex->rn.getNormal();
  
  // Try to use a smoothing group to get the normal.
  if(normalCount > 0) {
    DWORD smGroup = pFace->smGroup;
    if (smGroup != 0) {
      int normalId;
      for(normalId = 0; normalId < normalCount; normalId++)
	if(pRVertex->ern[normalId].getSmGroup() & smGroup)
	  return pRVertex->ern[normalId].getNormal();
    }
  }
  
  // if all fails, return the face normal
  return mesh->getFaceNormal(faceId);
}


Point3 ExtraMax_GetTexCoordDirect(Mesh *mesh, int faceId, int faceVertexId, int channel)
{
  Point3 result;
  if ((channel==0)&&(mesh->numTVerts > 0)) {
    result = mesh->tVerts[mesh->tvFace[faceId].t[faceVertexId]];
  } else if ((mesh->maps != 0)&&(mesh->mapSupport(channel))) {
    TVFace *pTVFace = mesh->mapFaces(channel);
    UVVert *pUVVert = mesh->mapVerts(channel);
    result = pUVVert[pTVFace[faceId].t[faceVertexId]];
  } else {
    result.x = 0.0;
    result.y = 0.0;
    result.z = 0.0;
  }
  return result;
}

void ExtraMax_GetPhysiqueInfluences(Modifier *physique, INode *node, int vertexId,
				    INode **bones, float *weights, int *nbones)
{
  // Initialize everything to zero.
  
  IPhysiqueExport   *pPhysiqueExport = 0;
  IPhyContextExport *pContextExport = 0;
  IPhyVertexExport  *pVertexExport = 0;
  int vertexType = 0;
  *nbones = 0;

  // create a physique export interface
  pPhysiqueExport = (IPhysiqueExport *)physique->GetInterface(I_PHYINTERFACE);
  if (pPhysiqueExport==0) goto cleanup;

  // create a context export interface
  pContextExport = (IPhyContextExport *)pPhysiqueExport->GetContextInterface(node);
  if (pContextExport==0) goto cleanup;

  // set the flags in the context export interface
  pContextExport->ConvertToRigid(TRUE);
  pContextExport->AllowBlending(TRUE);
  
  // get the vertex export interface
  pVertexExport = (IPhyVertexExport *)pContextExport->GetVertexInterface(vertexId);
  if (pVertexExport==0) goto cleanup;

  // get the vertex type
  vertexType = pVertexExport->GetVertexType();

  // handle the specific vertex type
  if(vertexType == RIGID_TYPE) {
    // typecast to rigid vertex
    IPhyRigidVertex *pTypeVertex = (IPhyRigidVertex *)pVertexExport;

    bones[0] = pTypeVertex->GetNode();
	if (bones[0]) {
	    weights[0] = 1.0;
		*nbones = 1;
	} else {
		*nbones = 0;
	}
    goto cleanup;
  }
  
  if(vertexType == RIGID_BLENDED_TYPE) {
    // typecast to blended vertex
    IPhyBlendedRigidVertex *pTypeVertex = (IPhyBlendedRigidVertex *)pVertexExport;

    // loop through all influencing bones
    int nodeId, xnodeId;
    *nbones = pTypeVertex->GetNumberNodes();
    for(nodeId = 0; nodeId < (*nbones); nodeId++) {
      bones[nodeId] = pTypeVertex->GetNode(nodeId);
      weights[nodeId] = pTypeVertex->GetWeight(nodeId);
    }
    
    // collapse redundancies. this may lead to zero-weight bones.
    // we rely on the threshold to eliminate these.
    for (nodeId = 0; nodeId < (*nbones); nodeId++) {
      for (xnodeId = nodeId+1; xnodeId < (*nbones); xnodeId++) {
	if (bones[nodeId] == bones[xnodeId]) {
	  weights[nodeId] += weights[xnodeId];
	  weights[xnodeId] = 0.0;
	}
      }
    }

    goto cleanup;
  }

cleanup:
  
  pPhysiqueExport->ReleaseContextInterface(pContextExport);
  physique->ReleaseInterface(I_PHYINTERFACE, pPhysiqueExport);
}

void ExtraMax_GetSkinInfluences(Modifier *skin, INode *node, int vertexId,
				INode **bones, float *weights, int *nbones)
{
  *nbones = 0;
#if MAX_RELEASE >= 4000
  ISkin            *pSkin = 0;
  ISkinContextData *pSkinContextData = 0;

  // create a skin interface
  pSkin = (ISkin*)(skin->GetInterface(I_SKIN));
  if (pSkin == 0) goto cleanup;

  // create a skin context data interface
  pSkinContextData = (ISkinContextData *)pSkin->GetContextInterface(node);
  if (pSkinContextData == 0) goto cleanup;

  skin->ReleaseInterface(I_SKIN, pSkin);

  // loop through all influencing bones
  *nbones = 0;
  int nodeId, xnodeId;
  for(nodeId = 0; nodeId < pSkinContextData->GetNumAssignedBones(vertexId); nodeId++) {
    int boneId = pSkinContextData->GetAssignedBone(vertexId, nodeId);
    if(boneId < 0) continue;
    bones[*nbones] = pSkin->GetBone(boneId);
    weights[*nbones] = pSkinContextData->GetBoneWeight(vertexId, nodeId);
    (*nbones)++;
  }
      
  // collapse redundancies. this may lead to zero-weight bones.
  // we rely on the threshold to eliminate these.
  for (nodeId = 0; nodeId < (*nbones); nodeId++) {
    for (xnodeId = nodeId+1; xnodeId < (*nbones); xnodeId++) {
      if (bones[nodeId] == bones[xnodeId]) {
	weights[nodeId] += weights[xnodeId];
	weights[xnodeId] = 0.0;
      }
    }
  }

  // release all interfaces
 cleanup:
  skin->ReleaseInterface(I_SKIN, pSkin);
#endif
}


bool ExtraMax_GetTranslationAndRotation(INode *pNode, bool parent, int ticks,
					Point3 &transbone, Quat &rotbone,
					Point3 &transabs, Quat &rotabs)
{
  bool success = true;
  Matrix3 tm1;
  Matrix3 tm2;
  Matrix3 absmatrix;
  Matrix3 bonematrix;

  INode *pParentNode = pNode->GetParentNode();
  
  tm1 = pNode->GetNodeTM(ticks);
  if (tm1.Parity()) success = false;
  tm1.NoScale();

  if ((!parent)||(pParentNode==0)) tm2.IdentityMatrix();
  else {
    tm2 = pParentNode->GetNodeTM(ticks);
    tm2.NoScale();
  }
  
  absmatrix = tm1 * Inverse(tm2);
  bonematrix = Inverse(tm1);
  
  transabs = absmatrix.GetTrans();
  rotabs = absmatrix;
  transbone = bonematrix.GetTrans();
  rotbone = bonematrix;
  return success;
}

