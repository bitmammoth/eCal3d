
#include "cal3d.h"
#include "max.h"
#include "cx-extramax.h"

static char ErrorMsg[1024];

/////////////////////////////////////////////////////////////////////////////////////////////
//
// A Null View Class.  When Max wants to convert an object to a mesh,
// it needs us to pass in a "view" object.  This one will do.
//
/////////////////////////////////////////////////////////////////////////////////////////////

class NullView : public View
{
public:
  Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
  NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// AddFace - adds a face to a CalCoreSubmesh
//
/////////////////////////////////////////////////////////////////////////////////////////////

static void AddFace(CalCoreSubmesh *ccsubmesh, int vtx0, int vtx1, int vtx2)
{
  int faceCount = ccsubmesh->getFaceCount();
  CalCoreSubmesh::Face theFace;
  theFace.vertexId[0] = vtx0;
  theFace.vertexId[1] = vtx1;
  theFace.vertexId[2] = vtx2;
  ccsubmesh->resize(-1, -1, faceCount+1, -1);
  ccsubmesh->setFace(faceCount, theFace);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// GetCoreBoneIdByINode - scans a CalCoreSkeleton for the bone with the specified INode.
//
/////////////////////////////////////////////////////////////////////////////////////////////

static int GetCoreBoneIdByINode(INode *node, CalCoreModel *skel)
{
  for (int boneId = 0; boneId < skel->getCoreBoneCount(); boneId++)
    if (skel->getCoreBone(boneId)->MaxNode == node)
      return boneId;
  
  return (-1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// GetCoreSubmeshWithMaterial - scans a CalCoreMesh for a submesh with the given material ID.
//
/////////////////////////////////////////////////////////////////////////////////////////////

static CalCoreSubmesh *GetCoreSubmeshWithMaterial(CalCoreModel *model, int materialId)
{
  for (int i=0; i<model->getCoreSubmeshCount(); i++) {
    CalCoreSubmesh *sub = model->getCoreSubmesh(i);
    if (sub->getCoreMaterialThreadId() == materialId) return sub;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// FindOrAddVertex
//    - Obtains info on specified MAX Vertex: position, normal, tex coords, influences, etc.
//    - Checks if a matching vertex exists in the CalCoreSubmesh.
//    - If not, adds the vertex to the CalCoreSubmesh.
//    - Returns the index of the vertex in the CalCoreSubmesh.
//
/////////////////////////////////////////////////////////////////////////////////////////////

static int FindOrAddVertex(INode *node, Mesh *mesh, Modifier *physique, Modifier *skin, 
			   Matrix3 &m_tm, Matrix3 &m_tmit, bool reversewinding, int mapchannels,
			   int conf_maxbones, float conf_minweight,
			   int faceId, int faceVertexId, CalCoreSubmesh *ccsubmesh, CalCoreModel *ccskel)
{
  int influenceId, channelId, calVertexId, boneId, boneId2;

  // The following variables contain all the data about the vertex.
  // We will initialize these variables, then we will scan for a matching vertex.
  Point3 vertex;
  int    nx,ny,nz;
  float  weight;
  float  texcoordu[100];
  float  texcoordv[100];

  // get vertex id
  int vertexId = mesh->faces[faceId].v[faceVertexId];

  // get the absolute vertex position
  vertex = mesh->getVert(vertexId) * m_tm;

  // get the absolute vertex normal and quantize
  Point3 normal = ExtraMax_GetVertexNormal(mesh, faceId, faceVertexId) * m_tmit;
  normal = normal.Normalize();
  nx = (int)(normal.x * 127.5);
  ny = (int)(normal.y * 127.5);
  nz = (int)(normal.z * 127.5);
  if (nx > 127.0) nx = 127.0; if (nx < -127.0) nx = -127.0;
  if (ny > 127.0) ny = 127.0; if (ny < -127.0) ny = -127.0;
  if (nz > 127.0) nz = 127.0; if (nz < -127.0) nz = -127.0;

  // get the vertex weight
  float *pVertexWeights;
  pVertexWeights = mesh->getVertexWeights();
  if (pVertexWeights != 0) {
    weight = pVertexWeights[vertexId];
    if(weight < 0.0005f) weight = 0.0f;
  } else weight=0.0;

  // get the texture coordinates
  for (channelId=0; channelId<mapchannels; channelId++) {
    Point3 coord;
    coord = ExtraMax_GetTexCoordDirect(mesh, faceId, faceVertexId, channelId);
    texcoordu[channelId] = coord.x;
    texcoordv[channelId] = coord.y;
  }
  
  // Search for a matching vertex.  If we find one, return it.
  float EPSILON = 0.000001f;
  std::vector<CalCoreSubmesh::Vertex> &vvtx = ccsubmesh->getVectorVertex();
  std::vector<CalCoreSubmesh::LodControl> &vlod = ccsubmesh->getVectorLodControl();
  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> > &vtex = ccsubmesh->getVectorVectorTextureCoordinate();
  std::vector<CalCoreSubmesh::PhysicalProperty> &vphys = ccsubmesh->getVectorPhysicalProperty();
  for (calVertexId = 0; calVertexId < vvtx.size(); calVertexId++) {
    if (vvtx[calVertexId].reverseWinding != reversewinding) goto mismatch;
    if (fabs(vvtx[calVertexId].position[0] - vertex.x) >= EPSILON) goto mismatch;
    if (fabs(vvtx[calVertexId].position[1] - vertex.y) >= EPSILON) goto mismatch;
    if (fabs(vvtx[calVertexId].position[2] - vertex.z) >= EPSILON) goto mismatch;
    if (vvtx[calVertexId].nx != nx) goto mismatch;
    if (vvtx[calVertexId].ny != ny) goto mismatch;
    if (vvtx[calVertexId].nz != nz) goto mismatch;
    for (channelId=0; channelId<mapchannels; channelId++) {
      if (fabs(vtex[channelId][calVertexId].u - texcoordu[channelId]) >= EPSILON) goto mismatch;
      if (fabs(vtex[channelId][calVertexId].v - texcoordv[channelId]) >= EPSILON) goto mismatch;
    }
    // In the original code, the physical properties were not compared. Why?
    //   if (fabs(vphys[calVertexId].weight - weight) >= EPSILON) goto mismatch;
    // In the original code, the bone influences were not compared. Why?
    //    if (vvtx[calVertexId].vectorInfluence.size() != bonecount) goto mismatch;
    //    int influenceId;
    //    for(influenceId = 0; influenceId < vectorInfluence.size(); influenceId++) {
    //      if (vvtx[calVertexId].vectorInfluence[influenceId].boneId != boneindex[influenceId]) goto mismatch;
    //      if (vvtx[calVertexId].vectorInfluence[influenceId].weight != boneweight[influenceId]) goto mismatch;
    //    }
    return calVertexId;
  mismatch:;
  }

  // Variables to store the bone data.
  INode *bonenode[100];
  int    boneindex[100];
  float  boneweight[100];
  //int    bonequant[100];
  int    bonecount;
  //int    bonequanta;

  // Get the bone influence data.
  if (physique)  ExtraMax_GetPhysiqueInfluences(physique, node, vertexId, bonenode, boneweight, &bonecount);
  else if (skin) ExtraMax_GetSkinInfluences(skin, node, vertexId, bonenode, boneweight, &bonecount);
  else bonecount = 0;
  for (boneId=0; boneId<bonecount; boneId++) {
    boneindex[boneId] = GetCoreBoneIdByINode(bonenode[boneId], ccskel);
    if (boneindex[boneId] < 0) {
      sprintf(ErrorMsg,"Mesh %s is tied to bone %s, but bone %s isn't exported.", 
	      node->GetName(), bonenode[boneId]->GetName(), bonenode[boneId]->GetName());
      return -1;
    }
  }
  if (bonecount > 1) {
	  int x = 1;
  }
  
  // Sort the bone influences.
  for (boneId=0; boneId<bonecount; boneId++) {
    for (boneId2=boneId+1; boneId2<bonecount; boneId2++) {
      float w = boneweight[boneId];
      float w2 = boneweight[boneId2];
      if (w2 > w) {
	int i = boneindex[boneId];
	int i2 = boneindex[boneId2];
	boneweight[boneId] = w2;
	boneweight[boneId2] = w;
	boneindex[boneId] = i2;
	boneindex[boneId2] = i;
      }
    }
  }
  
  // Discard influences under the threshold.
  while ((bonecount > 0)&&(boneweight[bonecount-1] < conf_minweight))
    bonecount--;
  if (bonecount > conf_maxbones) bonecount=conf_maxbones;
  
  // Renormalize the influences.
  float totInf = 0;
  for (boneId = 0; boneId < bonecount; boneId++)
    totInf += boneweight[boneId];
  for (boneId = 0; boneId < bonecount; boneId++)
    boneweight[boneId] /= totInf;
  
  // Quantize the influences.
  //  int quantavail = 8;
  //  bonequanta = quantavail;
  //  for (boneId = 0; boneId < bonecount; boneId++) {
  //    int q = (int)(boneweight[boneId] * quantavail);
  //    bonequant[boneId] = q;
  //    bonequanta -= q;
  //  }
  //  while (bonequanta) {
  //    int best = -1;
  //    float diff = 0.0;
  //    for (boneId=0; boneId<bonecount; boneId++) {
  //      float bdiff = boneweight[boneId] - bonequant[boneId]*(1.0/quantavail);
  //      if (bdiff > diff) { diff=bdiff; best=boneId; }
  //    }
  //    bonequant[best]++;
  //    bonequanta--;
  //  }
  //  while (bonecount && (bonequant[bonecount-1]==0))
  //    bonecount--;
  //  for (boneId=0; boneId<bonecount; boneId++)
  //    boneweight[boneId] = bonequant[boneId] * (1.0/quantavail);
  
  // Push the influences onto the influence vector.
  std::vector<CalCoreSubmesh::Influence>& vectorInfluence = ccsubmesh->getVectorInfluence();
  int firstInfluence = vectorInfluence.size();
  vectorInfluence.resize(firstInfluence + bonecount);
  for (influenceId=0; influenceId<bonecount; influenceId++) {
    vectorInfluence[firstInfluence + influenceId].boneId = boneindex[influenceId];
    vectorInfluence[firstInfluence + influenceId].weight = boneweight[influenceId];
  }
  
  // Create the vertex.
  int vertexCount = ccsubmesh->getVertexCount();
  ccsubmesh->resize(vertexCount+1, -1, -1, -1);

  vvtx[vertexCount].reverseWinding = reversewinding;
  vvtx[vertexCount].position.x = vertex.x;
  vvtx[vertexCount].position.y = vertex.y;
  vvtx[vertexCount].position.z = vertex.z;
  vvtx[vertexCount].nx = nx;
  vvtx[vertexCount].ny = ny;
  vvtx[vertexCount].nz = nz;
  vlod[vertexCount].collapseId = -1;
  vlod[vertexCount].faceCollapseCount = 0;
  vvtx[vertexCount].influenceCount = bonecount;
  vphys[vertexCount].weight = weight;
  for (channelId=0; channelId<mapchannels; channelId++) {
    vtex[channelId][vertexCount].u = texcoordu[channelId];
    vtex[channelId][vertexCount].v = texcoordv[channelId];
    if (ccsubmesh->tangentsEnabled(channelId)) {
      vvtx[vertexCount].tanx = 0.0;
      vvtx[vertexCount].tany = 0.0;
      vvtx[vertexCount].tanz = 0.0;
    }
  }

  return vertexCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// UpdateTangentVector
//
/////////////////////////////////////////////////////////////////////////////////////////////

void UpdateTangentVector(CalCoreSubmesh *ccsubmesh, int v0, int v1, int v2, int channel)
{
  std::vector<CalCoreSubmesh::Vertex> &vvtx = ccsubmesh->getVectorVertex();
  std::vector<CalCoreSubmesh::TextureCoordinate> &vtex = ccsubmesh->getVectorTextureCoordinate(channel);
  
  // Step 1. Compute the approximate tangent vector.
  double du1 = vtex[v1].u - vtex[v0].u;
  double dv1 = vtex[v1].v - vtex[v0].v;
  double du2 = vtex[v2].u - vtex[v0].u;
  double dv2 = vtex[v2].v - vtex[v0].v;
  
  double prod1 = (du1*dv2-dv1*du2);
  double prod2 = (du2*dv1-dv2*du1);
  if ((fabs(prod1) < 0.000001)||(fabs(prod2) < 0.000001)) return;
  
  double x = dv2/prod1;
  double y = dv1/prod2;

  CalVector vec1 = vvtx[v1].position - vvtx[v0].position;
  CalVector vec2 = vvtx[v2].position - vvtx[v0].position;
  CalVector tangent = (vec1 * x) + (vec2 * y);
  
  // Step 2. Orthonormalize the tangent.
  CalVector normal(vvtx[v0].nx, vvtx[v0].ny, vvtx[v0].nz);
  normal *= (1.0f / 127.0f);
  double component = (tangent * normal);
  tangent -= (normal * component);
  tangent.normalize();
  
  // Step 3: Add the estimated tangent to the overall estimate for the vertex.
  vvtx[v0].tanx += tangent.x;
  vvtx[v0].tany += tangent.y;
  vvtx[v0].tanz += tangent.z;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// BuildMesh
//
//  - Writes a single Cal3D mesh file combining all the faces and vertices
//    of the specified MAX meshes.
//
/////////////////////////////////////////////////////////////////////////////////////////////

bool BuildMesh(CalCoreModel *model,
	       IObjParam *iObjParams, int t,
	       int *conf_bumpid, int *conf_bumpmap, int conf_bumpcount, 
	       int conf_maxbones, float conf_minweight,
	       int meshcount, INode **meshnodes, int mapchannels)
{
  int meshId, faceId, submeshId, materialId, faceVertexId;
  Mesh     *meshes[1000];
  Modifier *physiques[1000];
  Modifier *skins[1000];
  BOOL      delmesh[1000];

  // Step 2. Initialize the Node Table.
  
  NullView nullview;
  for (meshId=0; meshId<meshcount; meshId++) {
    INode *node = meshnodes[meshId];
    ObjectState os = node->EvalWorldState(t);
    Mesh *mesh = ((TriObject *)os.obj)->GetRenderMesh(t, node, nullview, delmesh[meshId]);
    mesh->checkNormals(TRUE);
    meshes[meshId] = mesh;
    physiques[meshId] = ExtraMax_FindPhysiqueModifier(node);
    skins[meshId]     = ExtraMax_FindSkinModifier(node);
  }

  // Step 3. Loop over all faces, add all vertices and faces to appropriate core submesh.

  for (meshId=0; meshId<meshcount; meshId++) {
    INode *node = meshnodes[meshId];
    Mesh *mesh = meshes[meshId];
    int numfaces = mesh->getNumFaces();
    Matrix3 m_tm = node->GetObjectTM(t);
    Matrix3 m_tmit = Inverse(ExtraMax_Transpose(m_tm));
    for (faceId=0; faceId<numfaces; faceId++) {
      if (conf_bumpcount==0) materialId = 0;
      else materialId = mesh->getFaceMtlIndex(faceId);
      // Create the core submesh.
      CalCoreSubmesh *ccsubmesh = GetCoreSubmeshWithMaterial(model, materialId);
      if (ccsubmesh == 0) {
	// Determine if material should be bump-mapped.
	int bumpchan = -1;
	bool found = false;
	for (int t=0; t<conf_bumpcount; t++) {
	  if (materialId == conf_bumpid[t]) {
	    found = true;
	    if (conf_bumpmap[t] < 65535) bumpchan = conf_bumpmap[t];
	  }
	}
	if ((conf_bumpcount > 0) && (!found)) {
	  sprintf(ErrorMsg,"You used Material ID %d, but didn't mention it in the Material ID list.", materialId+1);
	  return false;
	}
	if (bumpchan >= mapchannels) bumpchan = (-1);
	ccsubmesh = model->getCoreSubmesh(model->addCoreSubmesh());
	ccsubmesh->setCoreMaterialThreadId(materialId);
	ccsubmesh->setLodCount(0);
	ccsubmesh->BumpChannel = bumpchan;
	ccsubmesh->resize(0,mapchannels,0,1);
	if (bumpchan >= 0) ccsubmesh->enableTangents(bumpchan, true);
      }
      
      // Determine if MAX got the polygon windings backwards.
      Point3 v0 = mesh->getVert(mesh->faces[faceId].v[0]) * m_tm;
      Point3 v1 = mesh->getVert(mesh->faces[faceId].v[1]) * m_tm;
      Point3 v2 = mesh->getVert(mesh->faces[faceId].v[2]) * m_tm;
      Point3 fnorm = ((v1 - v0) ^ (v2 - v0)).Normalize();
      Point3 mfnorm = (mesh->getFaceNormal(faceId) * m_tmit).Normalize();
      bool reversepolywinding = ((fnorm % mfnorm) < 0.0) ? true : false;
      
      // When bumpmapping, we need to know if poly is backwards on the bumpmap.
      bool reversetexwinding = false;
      if (ccsubmesh->BumpChannel >= 0) {
	Point3 wind1 = ExtraMax_GetTexCoordDirect(mesh, faceId, 0, ccsubmesh->BumpChannel);
	Point3 wind2 = ExtraMax_GetTexCoordDirect(mesh, faceId, 1, ccsubmesh->BumpChannel);
	Point3 wind3 = ExtraMax_GetTexCoordDirect(mesh, faceId, 2, ccsubmesh->BumpChannel);
	Point3 vec1, vec2;
	if (reversepolywinding)
	  { vec1 = wind2-wind1; vec2=wind3-wind1; }
	else
	  { vec1 = wind3-wind1; vec2=wind2-wind1; }
	if (vec1.x * vec2.y - vec2.x * vec1.y < 0.0)
	  reversetexwinding = true;
      }
      
      // Fetch the vertices.
      int vtx[3];
      for (faceVertexId=0; faceVertexId<3; faceVertexId++) {
	vtx[faceVertexId] = FindOrAddVertex(node, mesh, physiques[meshId], skins[meshId], m_tm, m_tmit, reversetexwinding,
					    mapchannels, conf_maxbones, conf_minweight, 
					    faceId, faceVertexId, ccsubmesh, model);
	if (vtx[faceVertexId] < 0) return false;
      }
      
      // If MAX got the polygon winding backwards, fix it.
      if (reversepolywinding) {	int t = vtx[0]; vtx[0] = vtx[2]; vtx[2] = t; }
      
      // If bumpmapping, update the tangent vector for each vertex.
      if (ccsubmesh->BumpChannel >= 0) {
	UpdateTangentVector(ccsubmesh, vtx[0], vtx[1], vtx[2], ccsubmesh->BumpChannel);
	UpdateTangentVector(ccsubmesh, vtx[1], vtx[2], vtx[0], ccsubmesh->BumpChannel);
	UpdateTangentVector(ccsubmesh, vtx[2], vtx[0], vtx[1], ccsubmesh->BumpChannel);
      }
      
      // Finally, add the face to the core submesh.
      AddFace(ccsubmesh,vtx[0],vtx[1],vtx[2]);
    }
  }
  
  // Step 5. Normalize tangents. Compute binormals. Throw out springs.
  
  for (submeshId=0; submeshId<model->getCoreSubmeshCount(); submeshId++) {
    // Normalize tangents, compute binormals.
    CalCoreSubmesh *ccsubmesh = model->getCoreSubmesh(submeshId);
    if (ccsubmesh->BumpChannel >= 0) {
      std::vector<CalCoreSubmesh::Vertex> &vvtx = ccsubmesh->getVectorVertex();
      std::vector<CalCoreSubmesh::TangentSpace> &vtan = ccsubmesh->getVectorTangentSpace(ccsubmesh->BumpChannel);
      for (int vertexId = 0; vertexId < ccsubmesh->getVertexCount(); vertexId++) {
	float tanx = vvtx[vertexId].tanx;
	float tany = vvtx[vertexId].tany;
	float tanz = vvtx[vertexId].tanz;
	float scale = 127.5 / sqrt(tanx * tanx + tany * tany + tanz * tanz);
	char tx = tanx * scale;
	char ty = tany * scale;
	char tz = tanz * scale;
	if (tx < -127) tx = -127; if (tx > 127) tx = 127;
	if (ty < -127) ty = -127; if (ty > 127) ty = 127;
	if (tz < -127) tz = -127; if (tz > 127) tz = 127;
	vtan[vertexId].tx = tx;
	vtan[vertexId].ty = ty;
	vtan[vertexId].tz = tz;
	vtan[vertexId].crossFactor = (vvtx[vertexId].reverseWinding) ? -1 : 1;
      }
    }
    
    // Discard springs.
    ccsubmesh->resize(-1, -1, -1, 0);
  }
  
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// BuildSkeleton
//
//   - Builds a CalCoreSkeleton object, given an array of MAX bones.  This routine
//     inserts not only the specified bones, but also any child bones.
//
/////////////////////////////////////////////////////////////////////////////////////////////

static bool BuildSkeleton(CalCoreModel *skel, int parentId, int t, INode **bones, int bonecount)
{
  bool success = true;
  ExtraMax_AlphaSortNodes(bones, bonecount);
  for (int boneid=0; boneid<bonecount; boneid++) {
    INode *bone = bones[boneid];
    
    int boneId = skel->addCoreBone(bone->GetName());
    CalCoreBone *pCoreBone = skel->getCoreBone(boneId);
    assert(pCoreBone != 0);
    pCoreBone->MaxNode = bone;
    pCoreBone->setParentId(parentId);
    
    // get the translation and the rotation of the bone candidate
    Point3 transbone; Quat rotbone;
    Point3 transabs; Quat rotabs;
    bool ok = ExtraMax_GetTranslationAndRotation(bone, (parentId >= 0), t, transbone, rotbone, transabs, rotabs);
    if (!ok) {
      sprintf(ErrorMsg, "The bone %s has been scaled or mirrored.\nCal3D can handle scaled meshes, but not scaled bones.", bone->GetName());
      success = false;
    }
    
    // Convert MAX vectors/quaternions into Cal3D vectors/quaternions
    CalVector caltransbone(transbone[0], transbone[1], transbone[2]);
    CalVector caltransabs(transabs[0], transabs[1], transabs[2]);
    CalQuaternion calrotbone(rotbone[0], rotbone[1], rotbone[2], rotbone[3]);
    CalQuaternion calrotabs(rotabs[0], rotabs[1], rotabs[2], rotabs[3]);
    
    // set the translation and rotation
    float length = caltransabs.length();
    if (length < 0.001) length = 0.001f;
    pCoreBone->setLength(length);
    pCoreBone->setTranslation(caltransabs);
    pCoreBone->setRotation(calrotabs);
    pCoreBone->setTranslationBoneSpace(caltransbone);
    pCoreBone->setRotationBoneSpace(calrotbone);
    
    // adjust child list of parent bone
    if (parentId != -1) {
      // get parent core bone
      CalCoreBone *pParentCoreBone;
      pParentCoreBone = skel->getCoreBone(parentId);
      assert(pParentCoreBone != 0);
      // add this core bone to the child list of the parent bone
      pParentCoreBone->addChildId(boneId);
    }

    // Insert the children into the hierarchy.
    INode *childbone[1000];
    int childbones=0;
    for (int childid=0; childid<bone->NumberOfChildren(); childid++) {
      INode *child = bone->GetChildNode(childid);
      if (ExtraMax_IsBone(child)) childbone[childbones++] = child;
    }
    if (childbones > 0) {
      bool ok = BuildSkeleton(skel, boneId, t, childbone, childbones);    
      if (!ok) success=false;
    }
  }
  return success;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// BuildAnimation - takes a core skeleton, and generates an animation
// that animates each bone in the skeleton.
//
// Gotta wait on the rebased animation stuff.
//
/////////////////////////////////////////////////////////////////////////////////////////////

bool BuildAnimation(IObjParam *iObjParams, CalCoreModel *skel, CalCoreAnimation *anim,
 		    int conf_animstart, int conf_animstop, float conf_animrate)
{
  int currentBoneId;
  
  // Convert frames to ticks, and never go back.
  
  conf_animstart *= GetTicksPerFrame();
  conf_animstop *= GetTicksPerFrame();
  
  // Clip animation interval to animation window.

  int true_animstart = iObjParams->GetAnimRange().Start();
  int true_animstop = iObjParams->GetAnimRange().End();
  if (conf_animstart < true_animstart) conf_animstart = true_animstart;
  if (conf_animstop > true_animstop) conf_animstop = true_animstop;
  
  // Make sure there's actually something to animate.

  if (conf_animstart >= conf_animstop) {
    sprintf(ErrorMsg, "Animation ends before it starts.\nStart tick=%d   End tick=%d\n", conf_animstart, conf_animstop);
    return false;
  }
  
  // Compute the animation's duration.

  int duration = conf_animstop - conf_animstart;
  float duration_sec = ((float)duration) / ((float)TIME_TICKSPERSEC);
  anim->setDuration(duration_sec);
  
  // decide how many samples to take.
  
  int steps = ((duration_sec * conf_animrate) + 0.5);
  if (steps < 1) steps = 1;
  
  // Create all the tracks, one for each bone.
  
  for (currentBoneId = 0; currentBoneId < skel->getCoreBoneCount(); currentBoneId++) {
    
    CalCoreBone *currentBone = skel->getCoreBone(currentBoneId);
    CalCoreTrack *pCoreTrack = new CalCoreTrack();
    pCoreTrack->create();
    pCoreTrack->setCoreBoneName(currentBone->getName());
    anim->addCoreTrack(pCoreTrack);
    
//    Control* cpos = currentBone->MaxNode->GetTMController()->GetPositionController();
 //   Control* crot = currentBone->MaxNode->GetTMController()->GetRotationController();
 //   Control* cscl = currentBone->MaxNode->GetTMController()->GetScaleController();
 //   int nkpos = cpos->NumKeys();
 //   int nkrot = crot->NumKeys();
 //   int nkscl = cscl->NumKeys();
    
    // Load the keyframes into the track.
    
    for (int i=0; i<=steps; i++) {
      float portion = ((float)i) / ((float)steps);
      int ticks = conf_animstart + ((int)(portion * duration + 0.5));
      float frametime = portion * duration_sec;
      if (i==steps) { ticks = conf_animstop; frametime = duration_sec + 0.0001; }
      
      // allocate new core keyframe instance
      CalCoreKeyframe *pCoreKeyframe = new CalCoreKeyframe();
      assert(pCoreKeyframe != 0);
      assert(pCoreKeyframe->create());
      pCoreKeyframe->setTime(frametime);
      
      // get the translation and the rotation of the bone at the specified time.
      Point3 transabs, transbone;
      Quat rotabs, rotbone;
      bool ok = ExtraMax_GetTranslationAndRotation(currentBone->MaxNode, (currentBone->getParentId()>=0), ticks,
						   transbone, rotbone, transabs, rotabs);
      if (!ok) {
	sprintf(ErrorMsg, "The bone %s has been scaled or mirrored.\nCal3D can handle scaled meshes, but not scaled bones.", currentBone->MaxNode->GetName());
	return false;
      }
      
      // Convert MAX vectors/quaternions into Cal3D vectors/quaternions
      CalVector caltransbone(transbone[0], transbone[1], transbone[2]);
      CalVector caltransabs(transabs[0], transabs[1], transabs[2]);
      CalQuaternion calrotbone(rotbone[0], rotbone[1], rotbone[2], rotbone[3]);
      CalQuaternion calrotabs(rotabs[0], rotabs[1], rotabs[2], rotabs[3]);
      
      // set the orientation and rotation
      float bonelen = currentBone->getLength();
      float translen = caltransabs.length();
      pCoreKeyframe->setOrientation(caltransabs / bonelen);
      pCoreKeyframe->setRotation(calrotabs);
      
      // add this core keyframe to the core track
      pCoreTrack->addCoreKeyframe(pCoreKeyframe);
    }
  }
  return true;
}

bool BonesNonExclusive(INode **rootbones, int rootcount)
{
  for (int rootid=0; rootid<rootcount; rootid++) {
    INode *node = rootbones[rootid];
    node = node->GetParentNode();
    while (!node->IsRootNode()) {
      for (int i=0; i<rootcount; i++) {
	if (rootbones[i] == node) {
	  char name1[256];
	  char name2[256];
	  strncpy(name1, rootbones[rootid]->GetName(), 255);
	  strncpy(name2, rootbones[i]->GetName(), 255);
	  sprintf(ErrorMsg, "Bone \"%s\" is a child of \"%s\"", name1, name2);
	  return true;
	}
      }
      node = node->GetParentNode();
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// The Cal3D Exporter.
//
/////////////////////////////////////////////////////////////////////////////////////////////

char *CalExportModel(IObjParam *iObjParams,
		     char   *conf_filename,
		     bool    conf_progmesh,
		     bool    conf_springsys,
		     int    *conf_bumpid,
		     int    *conf_bumpmap,
		     int     conf_bumpcount,
		     int     conf_maxbones,
		     double  conf_minweight,
		     char  **conf_itemlist,
		     int     conf_itemcount)
{
  INode *rootbones[1000];
  INode *meshes[1000];
  int rootcount = 0;
  int meshcount = 0;
  int itemid;
  bool ok;
  
  // Step 1. Check the skeleton filename, see if it's valid.
  int filenamelen = strlen(conf_filename);
  if ((filenamelen < 5)||(stricmp(conf_filename+filenamelen-4,".cdf"))) {
    sprintf(ErrorMsg, "Model file must have CDF extension.\n");
    return ErrorMsg;
  }
  
  // Step 1. Generate the rootbone list and the mesh list.
  
  for (itemid=0; itemid<conf_itemcount; itemid++) {
    INode *node = iObjParams->GetINodeByName(conf_itemlist[itemid]);
    if (node == 0) {
      sprintf(ErrorMsg, "Object %s no longer exists, remove it from export list.", conf_itemlist[itemid]);
      return ErrorMsg;
    }
    if (ExtraMax_IsBone(node)) { rootbones[rootcount++] = node; continue; }
    if (ExtraMax_IsMesh(node)) { meshes[meshcount++] = node; continue; }
    sprintf(ErrorMsg, "Node %s isn't a bone or a mesh, can't export it.", conf_itemlist[itemid]);
    return ErrorMsg;
  }
  // Is there any real reason there has to be a bone? Yes, Cal3D barfs if none.
  if (rootcount==0) {
    sprintf(ErrorMsg, "No bones selected in the item list.");
    return ErrorMsg;
  }
  
  // Step 2. Make sure none of the root bones inherit from each other.

  if (BonesNonExclusive(rootbones, rootcount)) return ErrorMsg;
  
  // Step 3. Build the Cal3D skeleton from the MAX bones.

  int t = iObjParams->GetTime();
  CalCoreModel coreModel;
  if(!coreModel.create("Model Titles not Implemented")) {
    sprintf(ErrorMsg, "Could not create the Cal3D model object.");
    return ErrorMsg;
  }
  ok = BuildSkeleton(&coreModel, -1, t, rootbones, rootcount);
  if (!ok) { coreModel.destroy(); return ErrorMsg; }
  
  // Step 5. Build the Cal3D mesh from the MAX meshes.

  ok = BuildMesh(&coreModel, 
		      iObjParams, t, conf_bumpid, conf_bumpmap, conf_bumpcount,
		      conf_maxbones, conf_minweight,
		      meshcount, meshes, 1);
  if (!ok) { coreModel.destroy(); return ErrorMsg; }
  
  // Step 4. Export the Cal3D model to the file.

  CalSaver saver;
  if(!saver.saveCoreModel(conf_filename, &coreModel)) {
    sprintf(ErrorMsg, "Cal3D saver could not write Model file %s", conf_filename);
    coreModel.destroy();
    return ErrorMsg;
  }
  
  // Step 6. Clean up everything.
  
  coreModel.destroy();
  return 0;
}


char *CalExportAnimation(IObjParam *iObjParams,
			 char   *conf_filename,
			 int     conf_animstart,
			 int     conf_animstop,
			 double  conf_animrate,
			 char  **conf_itemlist,
			 int     conf_itemcount)
{
  INode *rootbones[1000];
  int rootcount = 0;
  int itemid;
  bool ok;
  
  // Step 1. Check the animation filename, see if it's valid.
  
  int animfilelen = strlen(conf_filename);
  if ((animfilelen < 5)||(stricmp(conf_filename+animfilelen-4,".caf"))) {
    sprintf(ErrorMsg, "Animation file must have CAF extension.\n");
    return ErrorMsg;
  }
  
  // Step 2. Generate the rootbone list and the mesh list.
  
  for (itemid=0; itemid<conf_itemcount; itemid++) {
    INode *node = iObjParams->GetINodeByName(conf_itemlist[itemid]);
    if (node == 0) {
      sprintf(ErrorMsg, "Object %s no longer exists, remove it from export list.", conf_itemlist[itemid]);
      return ErrorMsg;
    }
    if (ExtraMax_IsBone(node)) { rootbones[rootcount++] = node; continue; }
    sprintf(ErrorMsg, "Node %s isn't a bone, can't export it.", conf_itemlist[itemid]);
    return ErrorMsg;
  }
  // Is there any real reason there has to be a bone? Yes, Cal3D barfs if none.
  if (rootcount==0) {
    sprintf(ErrorMsg, "No bones selected in the item list.");
    return ErrorMsg;
  }
  
  // Step 3. Make sure none of the root bones inherit from each other.
  
  if (BonesNonExclusive(rootbones, rootcount)) return ErrorMsg;
  
  // Step 4. Build the Cal3D skeleton from the bones.
  
  int t = iObjParams->GetTime();
  CalCoreModel coreModel;
  if(!coreModel.create("untitled")) {
    sprintf(ErrorMsg, "Could not create the Cal3D model object.");
    return ErrorMsg;
  }
  ok = BuildSkeleton(&coreModel, -1, t, rootbones, rootcount);
  if (!ok) { coreModel.destroy(); return ErrorMsg; }
  
  // Step 5. Build the animation tracks.
  
  CalCoreAnimation coreAnimation;
  if(!coreAnimation.create("")) {
    sprintf(ErrorMsg, "Could not create the Cal3D animation object.");
    coreModel.destroy();
    return ErrorMsg;
  }
  if (!BuildAnimation(iObjParams, &coreModel, &coreAnimation, conf_animstart, conf_animstop, conf_animrate)) {
    coreModel.destroy();
    return ErrorMsg;
  }
  
  // Step 6. Export the animation.

  CalSaver saver;
  if(!saver.saveCoreAnimation(conf_filename, &coreAnimation)) {
    sprintf(ErrorMsg, "Cal3D Saver could not write animation file %s", conf_filename);
    coreAnimation.destroy();
    coreModel.destroy();
    return ErrorMsg;
  }
  
  // Step 7. Clean up everything.
  
  coreAnimation.destroy();
  coreModel.destroy();
  return 0;
}
