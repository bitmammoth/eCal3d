///////////////////////////////////////////////////////////////////////////////////////////
//
// This file is included as the body of the following functions:
//
// CalPhysique::calculateVertices
// CalPhysique::calculateNormals
// CalPhysique::calculateTangentSpaces
// CalPhysique::calculateVN
// CalPhysique::calculateVNT
//
// Each of these functions does basically the same thing: calculate the
// transformed vertices, normals, and tangents.  Some of the functions omit
// certain components.
//
///////////////////////////////////////////////////////////////////////////////////////////

// get bone vectors
CalMatrix *arrayTransformMatrix = &(m_pModel->m_vectorTransformMatrix[0]);
CalVector *arrayTransformVector = &(m_pModel->m_vectorTransformVector[0]);

// get vertex vector of the core submesh
CalCoreSubmesh::Vertex *arrayVertex = &(m_pCoreSubmesh->getVectorVertex()[0]);

#if CALCULATE_TANGENTS
// get tangent space vector of the core submesh.
if (!m_pCoreSubmesh->tangentsEnabled(textureCoordinateId))
{
  CalError::setLastError(CalError::INVALID_TANGENT_SPACE, __FILE__, __LINE__, "");
  return 0;
}
CalCoreSubmesh::TangentSpace *arrayTangentSpace = &(m_pCoreSubmesh->getVectorTangentSpace(textureCoordinateId)[0]);
#endif

// get influence vector of the core submesh
CalCoreSubmesh::Influence *arrayInfluence = &(m_pCoreSubmesh->getVectorInfluence()[0]);

// get physical property vector of the core submesh
CalCoreSubmesh::PhysicalProperty *arrayPhysicalProperty = &(m_pCoreSubmesh->getVectorPhysicalProperty()[0]);

// calculate all submesh vertices
int vertexId;
int nextInfluence = 0;
for(vertexId = 0; vertexId < m_vertexCount; vertexId++)
{
  // skip vertices that are controlled by the spring subsystem.
  if(m_pCoreSubmesh->m_vectorSpring.size() > 0)
  {
    CalCoreSubmesh::PhysicalProperty& physicalProperty = arrayPhysicalProperty[vertexId];
    if(physicalProperty.weight > 0.0f) continue;
  }
  
  // get the vertex
  CalCoreSubmesh::Vertex &vertex = arrayVertex[vertexId];
  
  // Fetch the not-yet-transformed position.
  #if CALCULATE_VERTICES
  float vx = vertex.position.x;
  float vy = vertex.position.y;
  float vz = vertex.position.z;
  #endif

  // Fetch the not-yet-transformed normal.
  // You know, I think this scaling by (1 / 127.0) may not be needed, if renormalization is turned on.
  #if CALCULATE_NORMALS
  float nx = vertex.nx * (1.0f / 127.0f);
  float ny = vertex.ny * (1.0f / 127.0f);
  float nz = vertex.nz * (1.0f / 127.0f);
  #endif

  // Fetch the not-yet-transformed tangent.
  // You know, I think this scaling by (1 / 127.0) may not be needed, if renormalization is turned on.
  #if CALCULATE_TANGENTS
  CalCoreSubmesh::TangentSpace &tanspace = arrayTangentSpace[vertexId];
  float tx = tanspace.tx * (1.0f / 127.0f);
  float ty = tanspace.ty * (1.0f / 127.0f);
  float tz = tanspace.tz * (1.0f / 127.0f);
  float crossFactor = tanspace.crossFactor;
  #endif
  
  if (vertex.influenceCount == 1)
  {
    // Get data straight out of the bone, no blending involved.
    int boneId = arrayInfluence[nextInfluence].boneId;
    const CalMatrix &r = arrayTransformMatrix[boneId];
    nextInfluence += vertex.influenceCount;
    
    // Apply the bone transform to the position.
    #if CALCULATE_VERTICES
    const CalVector &t = arrayTransformVector[boneId];
    pVertexBuffer[0] = t.x+r.dxdx*vx+r.dxdy*vy+r.dxdz*vz;
    pVertexBuffer[1] = t.y+r.dydx*vx+r.dydy*vy+r.dydz*vz;
    pVertexBuffer[2] = t.z+r.dzdx*vx+r.dzdy*vy+r.dzdz*vz;
    pVertexBuffer += 3;
    #endif
    
    // Apply the bone transform to the normal.
    #if CALCULATE_NORMALS
    pNormalBuffer[0] = r.dxdx*nx+r.dxdy*ny+r.dxdz*nz;
    pNormalBuffer[1] = r.dydx*nx+r.dydy*ny+r.dydz*nz;
    pNormalBuffer[2] = r.dzdx*nx+r.dzdy*ny+r.dzdz*nz;
    pNormalBuffer += 3;
    #endif

    // Apply the bone transform to the tangent.
    #if CALCULATE_TANGENTS
    pTangentBuffer[0] = r.dxdx*tx+r.dxdy*ty+r.dxdz*tz;
    pTangentBuffer[1] = r.dydx*tx+r.dydy*ty+r.dydz*tz;
    pTangentBuffer[2] = r.dzdx*tx+r.dzdy*ty+r.dzdz*tz;
    pTangentBuffer[3] = crossFactor;
    pTangentBuffer += 4;
    #endif
  }
  else
  {
    if (vertex.influenceCount == 0) {
      // Apply the bone transform to the position.
      #if CALCULATE_VERTICES
      pVertexBuffer[0] = vx;
      pVertexBuffer[1] = vy;
      pVertexBuffer[2] = vz;
      pVertexBuffer += 3;
      #endif
    
      // Apply the bone transform to the normal.
      #if CALCULATE_NORMALS
      pNormalBuffer[0] = nx;
      pNormalBuffer[1] = ny;
      pNormalBuffer[2] = nz;
      pNormalBuffer += 3;
      #endif

      // Apply the bone transform to the tangent.
      #if CALCULATE_TANGENTS
      pTangentBuffer[0] = tx;
      pTangentBuffer[1] = ty;
      pTangentBuffer[2] = tz;
      pTangentBuffer[3] = crossFactor;
      pTangentBuffer += 4;
      #endif
    }
    else
    {
      // Apply the first influence to the blended rotation.
      int boneId = arrayInfluence[nextInfluence].boneId;
      float weight = arrayInfluence[nextInfluence].weight;
      CalMatrix r(weight, arrayTransformMatrix[boneId]);
      
      // Apply the first influence to the blended translation.
      #if CALCULATE_VERTICES
      const CalVector &t = arrayTransformVector[boneId];
      float x = t.x*weight;
      float y = t.y*weight;
      float z = t.z*weight;
      #endif
      
      // Add in all other influences to the blended rotation and translation.
      int influenceId;
      for(influenceId = 1; influenceId < vertex.influenceCount; influenceId++)
      {
	int boneId = arrayInfluence[nextInfluence + influenceId].boneId;
	float weight = arrayInfluence[nextInfluence + influenceId].weight;
	r.blend(weight, arrayTransformMatrix[boneId]);
        #if CALCULATE_VERTICES
	const CalVector &t = arrayTransformVector[boneId];
	x += t.x*weight;
	y += t.y*weight;
	z += t.z*weight;
        #endif
      }
      nextInfluence += vertex.influenceCount;
      
      // Apply the blended rotation and blended translation to the position.
      #if CALCULATE_VERTICES
      pVertexBuffer[0] = x+r.dxdx*vx+r.dxdy*vy+r.dxdz*vz;
      pVertexBuffer[1] = y+r.dydx*vx+r.dydy*vy+r.dydz*vz;
      pVertexBuffer[2] = z+r.dzdx*vx+r.dzdy*vy+r.dzdz*vz;
      pVertexBuffer += 3;
      #endif
    
      // Apply the blended rotation to the normal.
      #if CALCULATE_NORMALS
      float postnx = r.dxdx*nx+r.dxdy*ny+r.dxdz*nz;
      float postny = r.dydx*nx+r.dydy*ny+r.dydz*nz;
      float postnz = r.dzdx*nx+r.dzdy*ny+r.dzdz*nz;
      float nscale = 1.0f / sqrt(postnx * postnx + postny * postny + postnz * postnz);
      pNormalBuffer[0] = postnx * nscale;
      pNormalBuffer[1] = postny * nscale;
      pNormalBuffer[2] = postnz * nscale;
      pNormalBuffer += 3;
      #endif
      
      // Apply the blended rotation to the tangent.
      #if CALCULATE_TANGENTS
      float posttx = r.dxdx*tx+r.dxdy*ty+r.dxdz*tz;
      float postty = r.dydx*tx+r.dydy*ty+r.dydz*tz;
      float posttz = r.dzdx*tx+r.dzdy*ty+r.dzdz*tz;
      float tscale = 1.0f / sqrt(posttx * posttx + postty * postty + posttz * posttz);
      pTangentBuffer[0] = posttx * tscale;
      pTangentBuffer[1] = postty * tscale;
      pTangentBuffer[2] = posttz * tscale;
      pTangentBuffer[3] = crossFactor;
      pTangentBuffer += 4;
      #endif
    }
  }
}

return m_vertexCount;

