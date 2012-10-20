static char sqla_program_id[292] = 
{
 172,0,65,69,65,78,65,73,103,66,112,107,82,82,75,99,48,49,49,49,
 49,32,50,32,32,32,32,32,32,32,32,32,8,0,84,73,71,69,82,32,
 32,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,8,0,84,95,68,66,83,51,48,54,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {' ',' ',' ',' '}};


static const short sqlIsLiteral   = SQL_IS_LITERAL;
static const short sqlIsInputHvar = SQL_IS_INPUT_HVAR;


#line 1 "t_dbs306_hdi_flag_sel.sqc"
#include <string.h> 
#include "dbs/dbs_basopr.h" 
#include "dbs/hdi_flag_sel.h"


/*
EXEC SQL INCLUDE sqlca;
*/

/* SQL Communication Area - SQLCA - structures and constants */
#include "sqlca.h"
struct sqlca sqlca;


#line 5 "t_dbs306_hdi_flag_sel.sqc"


/*
EXEC SQL WHENEVER SQLERROR CONTINUE;
*/

#line 6 "t_dbs306_hdi_flag_sel.sqc"
 

/*
EXEC SQL WHENEVER SQLWARNING CONTINUE;
*/

#line 7 "t_dbs306_hdi_flag_sel.sqc"
 

/*
EXEC SQL WHENEVER NOT FOUND CONTINUE;
*/

#line 8 "t_dbs306_hdi_flag_sel.sqc"



/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 10 "t_dbs306_hdi_flag_sel.sqc"

struct dbsTempHdiFlagSel_t
{
    char        caInDataObject[30+1];    short       indcaInDataObject;
    sqlint64    lInDataId;    short       indlInDataId;
    char        caOutDataFlag[200+1];    short       indcaOutDataFlag;
}htTempHdiFlagSel;


/*
EXEC SQL END DECLARE SECTION;
*/

#line 18 "t_dbs306_hdi_flag_sel.sqc"



void dbsHdiFlagSelInit(T_HdiFlagSel *ptHdiFlagSel)
{
    memset(ptHdiFlagSel, 0x00, sizeof(*ptHdiFlagSel));
}


int dbsHdiFlagSel(int iFuncCode, T_HdiFlagSel *ptHdiFlagSel)
{
    switch (iFuncCode)
    {
      case KR_DBSELECT:
          memcpy(&htTempHdiFlagSel, ptHdiFlagSel, sizeof(htTempHdiFlagSel));
          
/*
EXEC SQL SELECT  A.DATA_FLAG   INTO 
                     :htTempHdiFlagSel.caOutDataFlag :htTempHdiFlagSel.indcaOutDataFlag
                   FROM  KR_TBL_DYNDATA_FLAG A  WHERE  A.DATA_OBJECT =   :htTempHdiFlagSel.caInDataObject    AND A.DATA_ID =   :htTempHdiFlagSel.lInDataId   ;
*/

{
#line 35 "t_dbs306_hdi_flag_sel.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 35 "t_dbs306_hdi_flag_sel.sqc"
  sqlaaloc(2,2,1,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 31;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqldata = (void*)htTempHdiFlagSel.caInDataObject;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[1].sqltype = 492; sql_setdlist[1].sqllen = 8;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[1].sqldata = (void*)&htTempHdiFlagSel.lInDataId;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 35 "t_dbs306_hdi_flag_sel.sqc"
  sqlaaloc(3,1,2,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqltype = 461; sql_setdlist[0].sqllen = 201;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqldata = (void*)htTempHdiFlagSel.caOutDataFlag;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sql_setdlist[0].sqlind = &htTempHdiFlagSel.indcaOutDataFlag;
#line 35 "t_dbs306_hdi_flag_sel.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 35 "t_dbs306_hdi_flag_sel.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 35 "t_dbs306_hdi_flag_sel.sqc"
  sqlastop(0L);
}

#line 35 "t_dbs306_hdi_flag_sel.sqc"

          if ((sqlca.sqlcode != KR_DBOK) && (sqlca.sqlcode != KR_DBNOTFOUND))
          {
              return sqlca.sqlcode;
          }
          memcpy( ptHdiFlagSel, &htTempHdiFlagSel, sizeof(htTempHdiFlagSel));
          return sqlca.sqlcode;

      default:
          return -1;
    }
}