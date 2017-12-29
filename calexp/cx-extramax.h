
////////////////////////////////////////////////////////////////////////////////////
//
// ExtraMax
//
// This file simply contains an assortment of functions designed to
// make access to MAX data structures easier.
//
////////////////////////////////////////////////////////////////////////////////////

Matrix3   ExtraMax_Transpose(Matrix3& matrix);
bool      ExtraMax_IsBone(INode *pNode);
bool      ExtraMax_IsMesh(INode *pNode);
Modifier *ExtraMax_FindPhysiqueModifier(INode *pINode);
Modifier *ExtraMax_FindSkinModifier(INode *pINode);
void      ExtraMax_AlphaSortNodes(INode **bones, int bonecount);
Point3    ExtraMax_GetVertexNormal(Mesh *mesh, int faceId, int faceVertexId);
Point3    ExtraMax_GetTexCoordDirect(Mesh *mesh, int faceId, int faceVertexId, int channel);
void      ExtraMax_GetPhysiqueInfluences(Modifier *physique, INode *node, int vertexId,
					 INode **bones, float *weights, int *nbones);
void      ExtraMax_GetSkinInfluences(Modifier *skin, INode *node, int vertexId,
				     INode **bones, float *weights, int *nbones);
bool      ExtraMax_GetTranslationAndRotation(INode *pNode, bool parent, int ticks,
					     Point3 &transbone, Quat &rotbone,
					     Point3 &transabs, Quat &rotabs);

