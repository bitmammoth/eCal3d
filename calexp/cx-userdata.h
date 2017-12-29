
class INode;

#undef CalCoreSubmeshUserData
class CAL3D_API CalCoreSubmeshUserData
{
 public:
  int BumpChannel;
};

#undef CalCoreBoneUserData
class CAL3D_API CalCoreBoneUserData
{
 public:
  INode *MaxNode;
};

#undef CalCoreVertexUserData
class CAL3D_API CalCoreVertexUserData
{
 public:
  float tanx,tany,tanz;
  bool reverseWinding;
};

