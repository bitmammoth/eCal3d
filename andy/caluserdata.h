
#ifndef CALUSERDATA_H
#define CALUSERDATA_H

#define MR_MAXTEXTURES 4
#define MR_MAXSOUNDS   2

#define MR_SOUNDMODE_SILENT   0
#define MR_SOUNDMODE_NORMAL   1
#define MR_SOUNDMODE_TERRAIN  2

class  RenderMode;
class  GscriptTex;
class  CalCoreModel;
class  CalCoreAnimation;
struct varray;

struct CalTerrainCode
{
  char    *TerrainType;
  int      TerrainCode;
};

#undef CalCoreModelUserData
class CAL3D_API CalCoreModelUserData
{
 public:
  char           *Graphic;                 // The name of the associated resource.
  bool            Loaded;                  // True if model has been loaded from resource.
  double          LastAccess;              // Last time the model was used.
  float           ApproxRadius;            // bounding box data.
  float           ApproxHeight;            // bounding box data.
  float           FlatRadius;              // Radius around which ground must be flattened.
  CalTerrainCode *FlatEffect;              // Terrain type that appears in the flat radius.
  GscriptTex     *TexD[MR_MAXTEXTURES];    // Diffuse for Cal3D object.
  GscriptTex     *TexB[MR_MAXTEXTURES];    // Bumpmap for Cal3D object.
};

#undef CalCoreAnimationUserData
class CAL3D_API CalCoreAnimationUserData
{
 public:
  char         *Graphic;                  // The name of the associated resource.
  bool          Loaded;                   // True if the animation has been loaded from resource.
  double        WalkSpeed;                // Moving Speed, if moving, as a function of terrain speed.
  CalCoreModel *Basis;                    // Model that contains the animation.
  double        LastAccess;               // Last time the CalCoreAnimation was accessed.
  int           SoundMode[MR_MAXSOUNDS];  //
  double        SoundTime[MR_MAXSOUNDS];  //
  char         *SoundName[MR_MAXSOUNDS];  //
};

#undef CalCoreSubmeshUserData
class CAL3D_API CalCoreSubmeshUserData
{
 public:
  CalCoreModel *Model;
  char          Label[32];
  char          Submenu[32];
  int           Color;
  CalCoreModel *TexSrc;
  int           TexSelect;
  RenderMode   *Mode;
  int           RangeLo;
  int           RangeHi;
};

#undef CalSubmeshUserData
class CAL3D_API CalSubmeshUserData
{
 public:
  bool           OverrideColor;
  bool           OverrideMode;
  bool           OverrideRangeLo;
  bool           OverrideRangeHi;
  bool           OverrideTexSelect;

  int            Color;
  RenderMode    *Mode;
  int            RangeLo;
  int            RangeHi;
  int            TexSelect;
  
  CalCoreModel  *TexSrc;
  
  int            RenderBits;
  varray        *VArray;
  int            IOffset; // Offset of submesh in VArray's index array.
};

#endif
