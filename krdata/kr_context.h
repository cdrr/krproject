#ifndef __KR_CONTEXT_H__
#define __KR_CONTEXT_H__

#include "krutils/kr_utils.h"
#include "dbs/dbs_basopr.h"
#include "krparam/kr_param.h"
#include "krdb/kr_db.h"
#include "kr_hdi_cache.h"


/* static environment, set while init */
typedef struct _kr_context_env_t
{
    void             *data;        /* engine startup extra data */
    T_KRModule       *krdbModule;  /* module for krdb */
    T_KRModule       *dataModule;  /* module for data */
    T_KRModule       *ruleModule;  /* module for rule */
    T_DbsEnv         *ptDbs;       /* db connection */
    T_KRShareMem     *ptShm;       /* share memory, read only in thread */
    T_KRDB           *ptKRDB;      /* krdb, read only in thread */
    T_KRCache        *ptHDICache;  /* hdi cache, need lock while mutli thread */
}T_KRContextEnv;

typedef struct _kr_context_arg_t
{
    T_KRRecord       *ptCurrRec;   /* pointer to current record */
    void             *pExtra;      /* pointer to user's argument data */
    KRFreeFunc       ExtraFreeFunc;/* free function of user's data */
}T_KRContextArg;

typedef struct _kr_context_t
{
    T_KRContextEnv   *ptEnv;      /* pointer to environment */
    T_DbsEnv         *ptDbs;      /* db connection, one per thread */
    T_KRDynamicMem   *ptDym;      /* pointer to dynamic memory */
    T_KRContextArg   *ptArg;      /* pointer to arguments */
    
    T_KRRecord       *ptCurrRec;  /* the same as ptArg->ptCurrRec */
    E_KRType          eKeyType;
    void             *pKeyValue;
    T_KRList         *ptRecList;
    T_KRRecord       *ptRecord;
    char             *pcaResp;    /* response pointer */
}T_KRContext;

typedef enum {
    KR_RETRIEVE_DATASRC     = '0',  /*retrieve from datasrc*/
    KR_RETRIEVE_SELFDEF     = '1'   /*retrieve by self-define function*/
}E_KRRetrieveType;

static inline time_t kr_get_transtime(T_KRRecord *ptRecord)
{
    time_t tTransTime;
    tTransTime = (time_t )*(long *)kr_get_field_value(\
                 ptRecord, KR_FIELDNO_TRANSTIME);
    return tTransTime;
}


T_KRContext *kr_context_init(T_KRContextEnv *ptEnv);
int kr_context_set(T_KRContext *ptContext, T_KRContextArg *ptContextArg);
void kr_context_fini(T_KRContext *ptContext);



#endif /* __KR_CONTEXT_H__ */
