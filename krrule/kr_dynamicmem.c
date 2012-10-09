#include "kr_dynamicmem.h"
#include "kr_rule.h"
#include "kr_aid.h"
#include "kr_sdi.h"
#include "kr_ddi.h"
#include "kr_hdi.h"

T_KRDynamicMem *kr_dynamicmem_init(T_KRShareMem *ptShm)
{
    short nSecId = ptShm->nSecId;
    
    T_KRDynamicMem *ptDyn = kr_malloc(sizeof(T_KRDynamicMem));
    if (ptDyn == NULL) {
        KR_LOG(KR_LOGERROR, "kr_malloc ptDyn Failed!");
        return NULL;
    }
    
    ptDyn->sditable = kr_sdi_table_construct(&ptShm->stShmSDI[nSecId]);
    if (ptDyn->sditable == NULL) {
	    KR_LOG(KR_LOGERROR, "kr_sdi_table_construct Failed!");
        return NULL;
	}
	
	ptDyn->dditable = kr_ddi_table_construct(&ptShm->stShmDDI[nSecId]);
    if (ptDyn->dditable == NULL) {
	    KR_LOG(KR_LOGERROR, "kr_ddi_table_construct Failed!");
        return NULL;
	}

	ptDyn->hditable = kr_hdi_table_construct(&ptShm->stShmHDI[nSecId]);
    if (ptDyn->hditable == NULL) {
	    KR_LOG(KR_LOGERROR, "kr_hdi_table_construct Failed!");
        return NULL;
	}

	ptDyn->rulegroup = kr_rule_group_construct(&ptShm->stShmRule[nSecId]);
    if (ptDyn->rulegroup == NULL) {
	    KR_LOG(KR_LOGERROR, "kr_rule_group_construct Failed!");
        return NULL;
	}
	
	return ptDyn;
}


void kr_dynamicmem_fini(T_KRDynamicMem *ptDyn)
{
    if (ptDyn) {
        kr_sdi_table_destruct(ptDyn->sditable);
        kr_ddi_table_destruct(ptDyn->dditable);
        kr_hdi_table_destruct(ptDyn->hditable);
        kr_rule_group_destruct(ptDyn->rulegroup);
        kr_free(ptDyn);
    }
}


int kr_dynamicmem_check(T_KRDynamicMem *ptDyn, T_KRShareMem *ptShm)
{  
    short nShmSec = ptShm->nSecId;
    T_KRShmSDI *ptShmSDI = &ptShm->stShmSDI[nShmSec];
    T_KRShmDDI *ptShmDDI = &ptShm->stShmDDI[nShmSec];
    T_KRShmHDI *ptShmHDI = &ptShm->stShmHDI[nShmSec];
    T_KRShmRule *ptShmRule = &ptShm->stShmRule[nShmSec];
    
    /*check sdi table*/
    if (ptDyn->sditable->tConstructTime != ptShmSDI->tLastLoadTime) {
        T_KRSDITable *ptSDITable = kr_sdi_table_construct(ptShmSDI);
        if (ptSDITable != NULL) {
            kr_sdi_table_destruct(ptDyn->sditable);
            ptDyn->sditable = ptSDITable;
        } else {
            KR_LOG(KR_LOGERROR, "reload sdi table error!");
            return -1;
        }
    }
    
    /*check ddi table*/
    if (ptDyn->dditable->tConstructTime != ptShmDDI->tLastLoadTime) {
        T_KRDDITable *ptDDITable = kr_ddi_table_construct(ptShmDDI);
        if (ptDDITable != NULL) {
            kr_ddi_table_destruct(ptDyn->dditable);
            ptDyn->dditable = ptDDITable;
        } else {
            KR_LOG(KR_LOGERROR, "reload ddi table error!");
            return -1;
        }
    }
    
    /*check hdi table*/
    if (ptDyn->hditable->tConstructTime != ptShmHDI->tLastLoadTime) {
        T_KRHDITable *ptHDITable = kr_hdi_table_construct(ptShmHDI);
        if (ptHDITable != NULL) {
            kr_hdi_table_destruct(ptDyn->hditable);
            ptDyn->hditable = ptHDITable;
        } else {
            KR_LOG(KR_LOGERROR, "reload hdi table error!");
            return -1;
        }
    }
    
    /*check rule group*/
    if (ptDyn->rulegroup->tConstructTime != ptShmRule->tLastLoadTime) {
        T_KRRuleGroup *ptRuleGroup = kr_rule_group_construct(ptShmRule);
        if (ptRuleGroup != NULL) {
            kr_rule_group_destruct(ptDyn->rulegroup);
            ptDyn->rulegroup = ptRuleGroup;
        } else {
            KR_LOG(KR_LOGERROR, "reload rule group error!");
            return -1;
        }
    }
    
    return 0;
}


