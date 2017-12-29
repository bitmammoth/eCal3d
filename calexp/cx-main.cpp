
#include "max.h"
#include "iparamb2.h"
#include "cx-resource.h"
#include "cx-export.h"

#define MAXITEMS 100
HINSTANCE hInstance;
int controlsInit = FALSE;
static BOOL creating = FALSE;

#define CALEXPMODEL_CLASS_ID 33849
#define CALEXPANIM_CLASS_ID 33850
#define UPDATE_FILENAME 1
#define UPDATE_NUMBERS  2
#define UPDATE_ITEMLIST 4
#define UPDATE_BUMPLIST 8
#define UPDATE_BUTTONS 16
#define UPDATE_ALL     31

TCHAR *GetString(int id)
{
  static TCHAR buf[256];
  
  if (hInstance)
    return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DLL Initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
  hInstance = hinstDLL;
  
  if ( !controlsInit ) {
    controlsInit = TRUE;
    InitCustomControls(hInstance);
    InitCommonControls();
  }
  
  return(TRUE);
}

__declspec( dllexport ) const TCHAR *LibDescription()
{
  return GetString(IDS_LIBDESCRIPTION);
}

__declspec( dllexport ) int LibNumberClasses()
{
  return 2;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
  extern ClassDesc *GetCalexpDescModel(void);
  extern ClassDesc *GetCalexpDescAnim(void);
  switch(i) {
  case 0: return GetCalexpDescModel();
  case 1: return GetCalexpDescAnim();
  default: return 0;
  }
}

__declspec( dllexport ) ULONG LibVersion()
{
  return VERSION_3DSMAX;
}

__declspec( dllexport ) ULONG CanAutoDefer()
{
  return 1;
}


class CalexpObject: public HelperObject 
{
  friend class CalexpObjCreateCallBack;
  friend BOOL CALLBACK CalexpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
  friend void resetProtParams();
public:

  // Class vars
  static Mesh mesh;           // This plugin generates no geometry, this mesh is not passed on to 3D Studio.
  static short meshBuilt;
  static HWND hCalexpParams;
  static IObjParam *iObjParams;
  char   ErrorMsg[512];

  // Configuration parameters.
  char    conf_mode;
  char   *conf_filename;
  bool    conf_progmesh;
  bool    conf_springsys;
  int     conf_maxbones;
  double  conf_minweight;
  int     conf_animstart;
  int     conf_animstop;
  double  conf_animrate;
  char   *conf_itemlist[1024];
  int     conf_itemcount;
  int     conf_bumpid[1024];
  int     conf_bumpmap[1024];
  int     conf_bumpcount;
  
  //  inherited virtual methods for Reference-management
  RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
  void BuildMesh();
  void UpdateFromUI(int parts);
  void UpdateUI(int parts);
  void GetMat(TimeValue t, INode* inod, ViewExp *vpt, Matrix3& mat);

  CalexpObject(char mode);
  ~CalexpObject();

  //  inherited virtual methods:

  // From BaseObject
  int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
  // void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
  int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
  CreateMouseCallBack* GetCreateMouseCallBack();
  void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
  void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
  TCHAR *GetObjectName() { return GetString(IDS_DB_CALEXP); }

  // From Object
  ObjectState Eval(TimeValue time);
  void InitNodeName(TSTR& s) { s = GetString(IDS_DB_CALEXP); }
  Interval ObjectValidity(TimeValue time);
  void Invalidate();
  int DoOwnSelectHilite() { return 1; }

  // From GeomObject
  int IntersectRay(TimeValue t, Ray& r, float& at) { return 0; }
  void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
  void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
  void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

  // Animatable methods
  void DeleteThis() { delete this; }
  Class_ID ClassID()
  {
    if (conf_mode=='M') return Class_ID(CALEXPMODEL_CLASS_ID,0);
    if (conf_mode=='A') return Class_ID(CALEXPANIM_CLASS_ID,0);
    assert(0); return Class_ID(0,0);
  }
  void GetClassName(TSTR& s)
  {
    if (conf_mode=='M') s = TSTR(GetString(IDS_DB_CALMODEL));
    if (conf_mode=='A') s = TSTR(GetString(IDS_DB_CALANIM));
  }
  TSTR SubAnimName(int i)
  {
    if (conf_mode=='M') return TSTR(GetString(IDS_DB_CALMODEL));
    if (conf_mode=='A') return TSTR(GetString(IDS_DB_CALANIM));
    assert(0); return TSTR("");
  }
  
  // From ref
  RefTargetHandle Clone(RemapDir& remap = NoRemap());

  // IO
  IOResult Save(ISave *isave);
  IOResult Load(ILoad *iload);

  // Buttons in the rollout panel.
  void PressExportButton(void);
  void PressBrowseButton(void);
  void PressDelItemButton(void);
  void PressDelBumpButton(void);
};				



class CalexpModelClassDesc : public ClassDesc 
{
public:
  int 			IsPublic() {  return 1;  }
  void *		Create(BOOL loading = FALSE) { return new CalexpObject('M'); }
  const TCHAR *	        ClassName() { return GetString(IDS_DB_CALMODEL); }
  SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
  Class_ID 		ClassID() { return Class_ID(CALEXPMODEL_CLASS_ID,0); }
  const TCHAR* 	        Category() { return _T("Cal3D Exporters"); }
  int 			BeginCreate(Interface *i) { creating = TRUE; return ClassDesc::BeginCreate(i); }
  int 			EndCreate(Interface *i)	{ creating = FALSE; return ClassDesc::EndCreate(i); }
  void			ResetClassParams(BOOL fileReset) { }
};

class CalexpAnimClassDesc : public ClassDesc 
{
public:
  int 			IsPublic() {  return 1;  }
  void *		Create(BOOL loading = FALSE) { return new CalexpObject('A'); }
  const TCHAR *	        ClassName() { return GetString(IDS_DB_CALANIM); }
  SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
  Class_ID 		ClassID() { return Class_ID(CALEXPANIM_CLASS_ID,0); }
  const TCHAR* 	        Category() { return _T("Cal3D Exporters"); }
  int 			BeginCreate(Interface *i) { creating = TRUE; return ClassDesc::BeginCreate(i); }
  int 			EndCreate(Interface *i)	{ creating = FALSE; return ClassDesc::EndCreate(i); }
  void			ResetClassParams(BOOL fileReset) { }
};

static CalexpModelClassDesc CalexpDescModel;
static CalexpAnimClassDesc  CalexpDescAnim;

ClassDesc* GetCalexpDescModel() { return &CalexpDescModel; }
ClassDesc* GetCalexpDescAnim() { return &CalexpDescAnim; }

// class variable for measuring tape class.
Mesh CalexpObject::mesh;
short CalexpObject::meshBuilt=0;
HWND CalexpObject::hCalexpParams = NULL;
IObjParam *CalexpObject::iObjParams;

static void SetICustEditT(HWND wnd, char *text)
{
  ICustEdit *edit = GetICustEdit(wnd);
  edit->Enable();
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

static void SetICustEditI(HWND wnd, int n)
{
  char text[80];
  sprintf(text, "%d", n);
  ICustEdit *edit = GetICustEdit(wnd);
  edit->Enable();
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

static void SetICustEditF(HWND wnd, double n)
{
  char text[80];
  sprintf(text, "%3.2f", n);
  ICustEdit *edit = GetICustEdit(wnd);
  edit->Enable();
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

static char *GetICustEditT(HWND wnd)
{
  static char buffer[256];
  ICustEdit *edit = GetICustEdit(wnd);
  edit->Enable();
  edit->GetText(buffer,256);
  ReleaseICustEdit(edit);
  return buffer;
}

static void SetupICustButton(HWND button, BOOL check)
{
  ICustButton *btn = GetICustButton(button);
  btn->SetType(CBT_CHECK);
  btn->SetHighlightColor(GREEN_WASH);
  btn->SetCheck(check);
  ReleaseICustButton(btn);
}

// Handles the work of actually picking the target.
class PickItem : public PickModeCallback, PickNodeCallback 
{
public:
  CalexpObject *ph;
  int doingPick;
  CommandMode *cm;
  HWND hDlg;
  PickItem () {}
  
  BOOL HitTest (IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);
  BOOL Pick (IObjParam *ip, ViewExp *vpt);
  void EnterMode(IObjParam *ip);
  void ExitMode(IObjParam *ip);
  
  // Allow right-clicking out of mode.
  BOOL RightClick (IObjParam *ip,ViewExp *vpt) { return TRUE; }
  
  // Is supposed to return a PickNodeCallback-derived class: we qualify!
  PickNodeCallback *GetFilter() {return this;}
  
  // PickNodeCallback methods:
  BOOL Filter(INode *node) { return node==NULL ? FALSE : TRUE; }
};

BOOL PickItem::HitTest (IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags) 
{	
  INode *node = ip->PickNode(hWnd, m, this);
  return Filter (node);
}

BOOL PickItem::Pick (IObjParam *ip, ViewExp *vpt) 
{
  INode *node = vpt->GetClosestHit();
  assert(node);
  char trunc[256];
  char *name = node->GetName();
  strncpy(trunc,name,255);
  HWND items = GetDlgItem(hDlg, IDC_ITEMLIST);
  int sel = SendMessage(items, LB_FINDSTRINGEXACT, -1, (LPARAM)trunc);
  if (sel == LB_ERR) {
    int count = SendMessage(items, LB_GETCOUNT, 0, 0);
    if (count < MAXITEMS) {
      SendMessage(items, LB_ADDSTRING, 0, (LPARAM)trunc);
      ph->UpdateFromUI(UPDATE_ITEMLIST);
    }
  }
  return TRUE;
}

void PickItem::EnterMode(IObjParam *ip) 
{
  doingPick = 1;
  ph->UpdateUI(UPDATE_BUTTONS);
}

void PickItem::ExitMode(IObjParam *ip) 
{
  doingPick = 0;
  ph->UpdateUI(UPDATE_BUTTONS);
}

static PickItem pickCB;

BOOL CALLBACK BumpMapDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message ) {
  case WM_COMMAND:			
    switch( LOWORD(wParam) ) {
    case IDOK: {
      int mtlid = atoi(GetICustEditT(GetDlgItem(hDlg,IDC_BUMPMAPMTL))) - 1;
      char *chan = GetICustEditT(GetDlgItem(hDlg,IDC_BUMPMAPCHAN));
      int chanid = ((chan[0]=='n')||(chan[0]=='N')) ? 65535 : atoi(chan)-1;
      if ((mtlid<0)||(mtlid>65535)) { EndDialog(hDlg, -1); return FALSE; }
      if ((chanid<0)||(chanid>65535)) { EndDialog(hDlg, -1); return FALSE; }
      EndDialog(hDlg,(mtlid << 16) | chanid);
      return FALSE;
    }
    case IDCANCEL: {
      EndDialog(hDlg, -1);
      return FALSE;
    }
    default: return FALSE;
    }
  default: return FALSE;
  }
}

BOOL CALLBACK CalexpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  CalexpObject *ph = (CalexpObject *)GetWindowLong( hDlg, GWL_USERDATA );	
  if ( !ph && message != WM_INITDIALOG ) return FALSE;
  
  switch ( message ) {
  case WM_INITDIALOG:
    ph = (CalexpObject *)lParam;
    SetWindowLong( hDlg, GWL_USERDATA, (LONG)ph );
    SetDlgFont( hDlg, ph->iObjParams->GetAppHFont() );
    CalexpObject::hCalexpParams = hDlg;
    return FALSE;			
    
  case WM_DESTROY:
    return FALSE;
    
  case WM_MOUSEACTIVATE:
    ph->iObjParams->RealizeParamPanel();
    return FALSE;
    
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MOUSEMOVE:
    ph->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
    return FALSE;
    
  case WM_CUSTEDIT_ENTER:
    ph->UpdateFromUI(UPDATE_FILENAME | UPDATE_NUMBERS);
    ph->UpdateUI(UPDATE_FILENAME | UPDATE_NUMBERS);
    return FALSE;
    
  case WM_COMMAND:			
    switch( LOWORD(wParam) ) {

    case IDC_EXPORTNOW: ph->PressExportButton(); break;
    case IDC_BROWSE:    ph->PressBrowseButton(); break;
    
    case IDC_ITEMDEL:   ph->PressDelItemButton(); break;
    case IDC_BUMPDEL:   ph->PressDelBumpButton(); break;
      
    case IDC_ITEMADD:
      if (pickCB.doingPick) {
	// Cancel the pick.
	ph->iObjParams->SetCommandMode(pickCB.cm);
      } else {
	// Initiate a pick.
	pickCB.ph = ph;
	pickCB.hDlg = hDlg;
	pickCB.cm = ph->iObjParams->GetCommandMode();
	ph->iObjParams->SetPickMode(&pickCB);
	ph->iObjParams->RedrawViews (ph->iObjParams->GetTime());
      }
      break;
    
    case IDC_BUMPADD: {
      int retval = DialogBox(hInstance, MAKEINTRESOURCE(IDD_BUMPMAP), hDlg, BumpMapDialogProc);
      if ((retval != -1)&&(ph->conf_bumpcount < 1024)) {
	ph->conf_bumpid[ph->conf_bumpcount] = ((retval >> 16) & 0xFFFF);
	ph->conf_bumpmap[ph->conf_bumpcount] = (retval & 0xFFFF);
	ph->conf_bumpcount++;
	ph->UpdateUI(UPDATE_BUMPLIST);
      }
      break;
    }

    case IDC_PROGMESH:
    case IDC_SPRINGSYS:
      ph->UpdateFromUI(UPDATE_NUMBERS);
      ph->UpdateUI(UPDATE_NUMBERS);
      break;
    }
    return FALSE;
    
  default:
    return FALSE;
  }
}



void CalexpObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
  iObjParams = ip;
  if ( !hCalexpParams ) {
    if (conf_mode == 'M') {
      hCalexpParams = ip->AddRollupPage(hInstance, 
					MAKEINTRESOURCE(IDD_CALMODEL),
					CalexpParamDialogProc, 
					GetString(IDS_RB_PARAMETERS), 
					(LPARAM)this );
    } else if (conf_mode == 'A') {
      hCalexpParams = ip->AddRollupPage(hInstance, 
					MAKEINTRESOURCE(IDD_CALANIM),
					CalexpParamDialogProc, 
					GetString(IDS_RB_PARAMETERS), 
					(LPARAM)this );
    } else assert(0);
    ip->RegisterDlgWnd(hCalexpParams);
  } else {
    SetWindowLong( hCalexpParams, GWL_USERDATA, (LONG)this );
  }
  UpdateUI(UPDATE_ALL);
}

void CalexpObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
  if ( flags&END_EDIT_REMOVEUI ) {		
    ip->UnRegisterDlgWnd(hCalexpParams);
    ip->DeleteRollupPage(hCalexpParams);
    hCalexpParams = NULL;				
  } else {
    SetWindowLong( hCalexpParams, GWL_USERDATA, 0 );
  }
  iObjParams = NULL;
}


void CalexpObject::BuildMesh()
{
  int i;
  if(meshBuilt) return;
  
  mesh.setNumVerts(8);
  mesh.setNumFaces(12);
  mesh.setSmoothFlags(0);
  mesh.setNumTVerts (0);
  mesh.setNumTVFaces (0);
  
  mesh.setVert (0,  5.0, -5.0,  5.0);
  mesh.setVert (1,  5.0,  5.0,  5.0);
  mesh.setVert (2, -5.0,  5.0,  5.0);
  mesh.setVert (3, -5.0, -5.0,  5.0);
  mesh.setVert (4,  5.0, -5.0, -5.0);
  mesh.setVert (5,  5.0,  5.0, -5.0);
  mesh.setVert (6, -5.0,  5.0, -5.0);
  mesh.setVert (7, -5.0, -5.0, -5.0);
  
  for (i=0; i<12; i++) mesh.faces[i].setEdgeVisFlags(1, 1, 0);
  for (i=0; i<12; i++) mesh.faces[i].setSmGroup(0);
  mesh.faces[ 0].setVerts(0,1,2);
  mesh.faces[ 1].setVerts(2,3,0);
  mesh.faces[ 2].setVerts(4,7,6);
  mesh.faces[ 3].setVerts(6,5,4);
  mesh.faces[ 4].setVerts(5,1,0);
  mesh.faces[ 5].setVerts(0,4,5);
  mesh.faces[ 6].setVerts(7,3,2);
  mesh.faces[ 7].setVerts(2,6,7);
  mesh.faces[ 8].setVerts(4,0,3);
  mesh.faces[ 9].setVerts(3,7,4);
  mesh.faces[10].setVerts(2,1,5);
  mesh.faces[11].setVerts(5,6,2);
  
  mesh.InvalidateGeomCache();
  mesh.BuildStripsAndEdges();
  
  meshBuilt = 1;
}

void CalexpObject::PressExportButton(void)
{
  if ( hCalexpParams &&	GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    UpdateUI(UPDATE_BUTTONS);
    char *errmsg = 0;
    if (conf_mode == 'M')
      errmsg = CalExportModel(iObjParams,
			      conf_filename,
			      conf_progmesh,
			      conf_springsys,
			      conf_bumpid,
			      conf_bumpmap,
			      conf_bumpcount,
			      conf_maxbones,
			      conf_minweight,
			      conf_itemlist,
			      conf_itemcount);
    if (conf_mode == 'A')
      errmsg = CalExportAnimation(iObjParams,
				  conf_filename,
				  conf_animstart,
				  conf_animstop,
				  conf_animrate,
				  conf_itemlist,
				  conf_itemcount);
    if (errmsg) MessageBox(NULL, errmsg, "Export Failure", MB_OK);
    else MessageBox(NULL, "Export Complete", "Export Complete", MB_OK);
  }
}

void CalexpObject::PressBrowseButton(void)
{
  if ( hCalexpParams &&	GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    static TCHAR fname[256];
    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(ofn));
    fname[0] = 0;
    
    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = NULL; // hCalexpParams;
    if (conf_mode == 'M') ofn.lpstrFilter = "Cal3D Skeleton Files\0*.cdf\0";
    if (conf_mode == 'A') ofn.lpstrFilter = "Cal3D Animation Files\0*.caf\0";
    ofn.lpstrFile       = fname;
    ofn.nMaxFile        = 256;    
    //ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
    ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    if (conf_mode == 'M') ofn.lpstrDefExt = _T("cdf");
    if (conf_mode == 'A') ofn.lpstrDefExt = _T("caf");
    ofn.lpstrTitle      = _T("Locate Skeleton File");
    
    if (GetSaveFileName(&ofn)) {
      free(conf_filename);
      conf_filename = strdup(fname);
    }
    UpdateUI(UPDATE_BUTTONS | UPDATE_FILENAME);
  }
}

void CalexpObject::PressDelItemButton(void)
{
  if ( hCalexpParams && GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    HWND items = GetDlgItem(hCalexpParams, IDC_ITEMLIST);
    int sel = SendMessage(items, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR) SendMessage(items, LB_DELETESTRING, sel, 0);
    UpdateFromUI(UPDATE_ITEMLIST);
    UpdateUI(UPDATE_BUTTONS);
  }
}

void CalexpObject::PressDelBumpButton(void)
{
  if ( hCalexpParams && GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    HWND items = GetDlgItem(hCalexpParams, IDC_BUMPLIST);
    int sel = SendMessage(items, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR) SendMessage(items, LB_DELETESTRING, sel, 0);
    UpdateFromUI(UPDATE_BUMPLIST);
    UpdateUI(UPDATE_BUTTONS);
  }
}

void CalexpObject::UpdateFromUI(int parts)
{
  int i;
  if ( hCalexpParams &&	GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    if (parts & UPDATE_FILENAME) {
      free(conf_filename);
      conf_filename = strdup(GetICustEditT(GetDlgItem(hCalexpParams,IDC_FILENAME)));
    }
    if (parts & UPDATE_NUMBERS) {
      if (conf_mode == 'M') {
	conf_progmesh   = (SendMessage(GetDlgItem(hCalexpParams, IDC_PROGMESH), BM_GETCHECK, 0, 0)==BST_CHECKED)?true:false;
	conf_springsys  = (SendMessage(GetDlgItem(hCalexpParams, IDC_SPRINGSYS), BM_GETCHECK, 0, 0)==BST_CHECKED)?true:false;
	conf_maxbones = atoi(GetICustEditT(GetDlgItem(hCalexpParams,IDC_MAXBONES)));
	conf_minweight = atof(GetICustEditT(GetDlgItem(hCalexpParams,IDC_MINWEIGHT)));
      }
      if (conf_mode == 'A') {
	conf_animstart = atoi(GetICustEditT(GetDlgItem(hCalexpParams,IDC_ANIMSTART)));
	conf_animstop = atoi(GetICustEditT(GetDlgItem(hCalexpParams,IDC_ANIMSTOP)));
	conf_animrate = atof(GetICustEditT(GetDlgItem(hCalexpParams,IDC_ANIMRATE)));
      }
    }
    if (parts & UPDATE_ITEMLIST) {
      HWND items = GetDlgItem(hCalexpParams,IDC_ITEMLIST);
      for (i=0; i<conf_itemcount; i++) free(conf_itemlist[i]);
      conf_itemcount = SendMessage(items,LB_GETCOUNT,0,0);
      for (i=0; i<conf_itemcount; i++) {
	char buf[256];
	SendMessage(items,LB_GETTEXT,i,(LPARAM)buf);
	conf_itemlist[i] = strdup(buf);
      }
    }
    if (parts & UPDATE_BUMPLIST) {
      HWND items = GetDlgItem(hCalexpParams,IDC_BUMPLIST);
      conf_bumpcount = SendMessage(items,LB_GETCOUNT,0,0);
      for (i=0; i<conf_bumpcount; i++) {
	char buf[256],tmp1[256],tmp2[256],tmp3[256]; int id, mat;
	SendMessage(items,LB_GETTEXT,i,(LPARAM)buf);
	sscanf(buf,"%s%s%d%s%d",tmp1,tmp2,&id,tmp3,&mat);
	conf_bumpid[i] = id - 1;
	conf_bumpmap[i] = stricmp(tmp3,"no") ? (mat-1) : 65535;
      }
    }
  }
}

void CalexpObject::UpdateUI(int parts)
{
  if ( hCalexpParams &&	GetWindowLong(hCalexpParams,GWL_USERDATA)==(LONG)this ) {
    if (parts & UPDATE_BUTTONS) {
      SetupICustButton(GetDlgItem(hCalexpParams, IDC_EXPORTNOW), FALSE);
      SetupICustButton(GetDlgItem(hCalexpParams, IDC_BROWSE), FALSE);
      SetupICustButton(GetDlgItem(hCalexpParams, IDC_ITEMDEL), FALSE);
      SetupICustButton(GetDlgItem(hCalexpParams, IDC_ITEMADD), pickCB.doingPick ? TRUE:FALSE);
    }
    if (parts & UPDATE_FILENAME)
      SetICustEditT(GetDlgItem(hCalexpParams,IDC_FILENAME), conf_filename);
    if (parts & UPDATE_NUMBERS) {
      if (conf_mode == 'M') {
	SendMessage(GetDlgItem(hCalexpParams, IDC_PROGMESH), BM_SETCHECK, conf_progmesh, 0);
	SendMessage(GetDlgItem(hCalexpParams, IDC_SPRINGSYS), BM_SETCHECK, conf_springsys, 0);
	SetICustEditI(GetDlgItem(hCalexpParams,IDC_MAXBONES), conf_maxbones);
	SetICustEditF(GetDlgItem(hCalexpParams,IDC_MINWEIGHT), conf_minweight);
      }
      if (conf_mode == 'A') {
	SetICustEditI(GetDlgItem(hCalexpParams,IDC_ANIMSTART), conf_animstart);
	SetICustEditI(GetDlgItem(hCalexpParams,IDC_ANIMSTOP), conf_animstop);
	SetICustEditF(GetDlgItem(hCalexpParams,IDC_ANIMRATE), conf_animrate);
      }
    }
    if (parts & UPDATE_ITEMLIST) {
      HWND items = GetDlgItem(hCalexpParams, IDC_ITEMLIST);
      SendMessage(items, LB_RESETCONTENT, 0, 0);
      for (int i=0; i<conf_itemcount; i++)
	SendMessage(items, LB_ADDSTRING, 0, (LPARAM)(conf_itemlist[i]));
    }
    if (parts & UPDATE_BUMPLIST) {
      HWND items = GetDlgItem(hCalexpParams, IDC_BUMPLIST);
      SendMessage(items, LB_RESETCONTENT, 0, 0);
      for (int i=0; i<conf_bumpcount; i++) {
	char buf[256];
	if (conf_bumpmap[i] == 65535) {
	  sprintf(buf,"Mtl ID %d no bump", conf_bumpid[i]+1);
	} else {
	  sprintf(buf,"Mtl ID %d chan %d", conf_bumpid[i] + 1, conf_bumpmap[i] + 1);
	}
	SendMessage(items, LB_ADDSTRING, 0, (LPARAM)buf);
      }
    }
  }
}

CalexpObject::CalexpObject(char mode) : HelperObject() 
{
  conf_mode = mode;
  if      (mode == 'M') conf_filename = strdup("c:\\calexp.cdf");
  else if (mode == 'A') conf_filename = strdup("c:\\calexp.caf");
  else assert(0);
  conf_progmesh = false;
  conf_springsys = false;
  conf_maxbones = 99;
  conf_minweight = 0.01;
  conf_animstart = 0;
  conf_animstop = 999999;
  conf_animrate = 30.0;
  conf_itemcount = 0;
  conf_bumpcount = 0;
  
  BuildMesh();
}

CalexpObject::~CalexpObject()
{
  free(conf_filename);
  for (int i=0; i<conf_itemcount; i++)
    free(conf_itemlist[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The creation callback - sets the initial position of the helper in the scene.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

class CalexpObjCreateCallBack: public CreateMouseCallBack 
{
  CalexpObject *ph;
public:
  int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
  void SetObj(CalexpObject *obj) { ph = obj; }
};

int CalexpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{	
  if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
    switch(point) {
    case 0:
      mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
      break;
    case 1:
      mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
      if (msg==MOUSE_POINT) return 0;
      break;			
    }
  } else if (msg == MOUSE_ABORT) {		
    return CREATE_ABORT;
  }
  return 1;
}

static CalexpObjCreateCallBack calexpCreateCB;

CreateMouseCallBack* CalexpObject::GetCreateMouseCallBack() 
{
  calexpCreateCB.SetObj(this);
  return &calexpCreateCB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

void CalexpObject::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm) 
{
  tm = inode->GetObjectTM(t);
  tm.NoScale();
  float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
  tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void CalexpObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
  box = mesh.getBoundingBox(tm);
}

void CalexpObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) 
{
  Matrix3 m = inode->GetObjectTM(t);
  Point3 pt;
  Point3 q[4];
  float scaleFactor = vpt->GetVPWorldWidth(m.GetTrans())/360.0f;
  box = mesh.getBoundingBox();
  box.Scale(scaleFactor);
}

void CalexpObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
  int i, nv; Matrix3 tm; Point3 pt;
  GetMat(t,inode,vpt,tm);
  nv = mesh.getNumVerts();
  box.Init();
  for (i=0; i<nv; i++) 
    box += tm*mesh.getVert(i);
}

int CalexpObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
{
  HitRegion hitRegion;
  DWORD	savedLimits;
  Matrix3 m;
  GraphicsWindow *gw = vpt->getGW();	
  Material *mtl = gw->getMaterial();
  MakeHitRegion(hitRegion,type,crossing,4,p);	
  gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
  GetMat(t,inode,vpt,m);
  gw->setTransform(m);
  gw->clearHitCode();
  if (mesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT )) 
    return TRUE;
  return FALSE;
}


int CalexpObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
  Matrix3 m;
  GraphicsWindow *gw = vpt->getGW();
  Material *mtl = gw->getMaterial();
  
  GetMat(t,inode,vpt,m);
  gw->setTransform(m);
  DWORD rlim = gw->getRndLimits();
  gw->setRndLimits(GW_WIREFRAME|GW_BACKCULL);
  if (inode->Selected()) 
    gw->setColor( LINE_COLOR, GetSelColor());
  else if(!inode->IsFrozen())
    gw->setColor( LINE_COLOR, GetUIColor(COLOR_TAPE_OBJ));
  mesh.render( gw, mtl, NULL, COMP_ALL);
  return(0);
}

RefResult CalexpObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message ) 
{
  UpdateUI(UPDATE_ALL);
  return REF_SUCCEED;
}

ObjectState CalexpObject::Eval(TimeValue time)
{
  return ObjectState(this);
}

void CalexpObject::Invalidate()
{
}

Interval CalexpObject::ObjectValidity(TimeValue t) 
{
  Interval ivalid;
  ivalid.SetInfinite();
  return ivalid;	
}

RefTargetHandle CalexpObject::Clone(RemapDir& remap) 
{
  CalexpObject* newob = new CalexpObject(conf_mode);
  return(newob);
}

// DO NOT USE THESE NUMBERS:
#define CHUNK_OBSOLETE0   0x1001
#define CHUNK_OBSOLETE1   0x1004
#define CHUNK_OBSOLETE2   0x100c
#define CHUNK_OBSOLETE3   0x100d
#define CHUNK_OBSOLETE4   0x100e

#define CHUNK_FILENAME    0x1000
#define CHUNK_PROGMESH    0x1002
#define CHUNK_SPRINGSYS   0x1003
#define CHUNK_MAXBONES    0x1005
#define CHUNK_MINWEIGHT   0x1006
#define CHUNK_ANIMSTART   0x1007
#define CHUNK_ANIMSTOP    0x1008
#define CHUNK_ANIMDISP    0x1009
#define CHUNK_ANIMRATE    0x100a
#define CHUNK_LISTITEM    0x100b
#define CHUNK_BUMPITEM    0x100f

void ChunkSaveString(ISave *isave, int chunkid, char *value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  int bytes = strlen(value) + 1;
  assert(bytes <= 256);
  isave->Write(&bytes, sizeof(int), &nb);
  isave->Write(value, bytes, &nb);
  isave->EndChunk();
}

void ChunkSaveInt(ISave *isave, int chunkid, int value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  isave->Write(&value, sizeof(int), &nb);
  isave->EndChunk();
}

void ChunkSaveFloat(ISave *isave, int chunkid, float value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  isave->Write(&value, sizeof(float), &nb);
  isave->EndChunk();
}

char *ChunkLoadString(ILoad *iload)
{
  ULONG nb;
  char buffer[1024];
  int bytes;
  IOResult res;
  res = iload->Read(&bytes, sizeof(int), &nb);
  assert(res == IO_OK);
  res = iload->Read(buffer, bytes, &nb);
  assert(res == IO_OK);
  return strdup(buffer);
}

int ChunkLoadInt(ILoad *iload)
{
  ULONG nb;
  int value;
  IOResult res;
  res = iload->Read(&value, sizeof(int), &nb);
  assert(res == IO_OK);
  return value;
}

float ChunkLoadFloat(ILoad *iload)
{
  ULONG nb;
  float value;
  IOResult res;
  res = iload->Read(&value, sizeof(float), &nb);
  assert(res == IO_OK);
  return value;
}

IOResult CalexpObject::Save(ISave *isave)
{
  ChunkSaveString(isave, CHUNK_FILENAME,    conf_filename);
  ChunkSaveInt(isave,    CHUNK_PROGMESH,    (int)conf_progmesh);
  ChunkSaveInt(isave,    CHUNK_SPRINGSYS,   (int)conf_springsys);
  ChunkSaveInt(isave,    CHUNK_MAXBONES,    conf_maxbones);
  ChunkSaveFloat(isave,  CHUNK_MINWEIGHT,   conf_minweight);
  ChunkSaveInt(isave,    CHUNK_ANIMSTART,   conf_animstart);
  ChunkSaveInt(isave,    CHUNK_ANIMSTOP,    conf_animstop);
  ChunkSaveFloat(isave,  CHUNK_ANIMRATE,    conf_animrate);
  for (int i=0; i<conf_itemcount; i++)
    ChunkSaveString(isave, CHUNK_LISTITEM, conf_itemlist[i]);
  for (i=0; i<conf_bumpcount; i++)
    ChunkSaveInt(isave, CHUNK_BUMPITEM, (conf_bumpid[i] << 16) | (conf_bumpmap[i]));
  return IO_OK;
}

IOResult CalexpObject::Load(ILoad *iload)
{
  for (int i=0; i<conf_itemcount; i++)
    free(conf_itemlist[i]);
  conf_itemcount = 0;

  while (1) {
    IOResult res = iload->OpenChunk();
    if (res != IO_OK) break;
    switch(iload->CurChunkID()) {
    case CHUNK_FILENAME:    free(conf_filename); conf_filename = ChunkLoadString(iload); break;
    case CHUNK_PROGMESH:    conf_progmesh  = ChunkLoadInt(iload) ? true:false; break;
    case CHUNK_SPRINGSYS:   conf_springsys = ChunkLoadInt(iload) ? true:false; break;
    case CHUNK_MAXBONES:    conf_maxbones  = ChunkLoadInt(iload); break;
    case CHUNK_MINWEIGHT:   conf_minweight = ChunkLoadFloat(iload); break;
    case CHUNK_ANIMSTART:   conf_animstart = ChunkLoadInt(iload); break;
    case CHUNK_ANIMSTOP:    conf_animstop  = ChunkLoadInt(iload); break;
    case CHUNK_ANIMDISP:    ChunkLoadInt(iload); break; // Ignore this chunk
    case CHUNK_ANIMRATE:    conf_animrate  = ChunkLoadFloat(iload); break;
    case CHUNK_LISTITEM:    conf_itemlist[conf_itemcount++] = ChunkLoadString(iload); break;
    case CHUNK_BUMPITEM:  {
      int val = ChunkLoadInt(iload);
      conf_bumpid[conf_bumpcount] = ((val>>16) & 0xFFFF);
      conf_bumpmap[conf_bumpcount] = (val & 0xFFFF);
      conf_bumpcount++;
    }
    }
    iload->CloseChunk();
  }
  return IO_OK;
}



