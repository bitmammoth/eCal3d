
////////////////////////////////////////////////////////////////////////////
//
// To export a MAX Model or Animation, simply call CalExportModel or
// CalExportAnimation, passing in the configuration parameters.  If the
// exporter returns an error message, display it.  These functions present
// no dialog boxes.
//
////////////////////////////////////////////////////////////////////////////

char *CalExportModel(IObjParam *iobjparams,
		     char   *conf_filename,
		     bool    conf_progmesh,
		     bool    conf_springsys,
		     int    *conf_bumpid,
		     int    *conf_bumpmap,
		     int     conf_bumpcount,
		     int     conf_maxbones,
		     double  conf_minweight,
		     char  **conf_itemlist,
		     int     conf_itemcount);

char *CalExportAnimation(IObjParam *iobjparams,
			 char   *conf_filename,
			 int     conf_animstart,
			 int     conf_animstop,
			 double  conf_animrate,
			 char  **conf_itemlist,
			 int     conf_itemcount);

