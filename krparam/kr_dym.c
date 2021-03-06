#include "kr_dym.h"

T_KRDynamicMem * kr_dynamicmem_init(T_KRShareMem *ptShm, 
        T_KRModule *ptDataModule, T_KRModule *ptRuleModule)
{
    short nSecId = ptShm->nSecId;
    
    T_KRDynamicMem *ptDyn = kr_malloc(sizeof(T_KRDynamicMem));
    if (ptDyn == NULL) {
        KR_LOG(KR_LOGERROR, "kr_malloc ptDyn Failed!");
        return NULL;
    }
    
    ptDyn->sditable = \
        kr_sdi_table_construct(&ptShm->stShmSDI[nSecId], ptDataModule);
    if (ptDyn->sditable == NULL) {
        KR_LOG(KR_LOGERROR, "kr_sdi_table_construct Failed!");
        return NULL;
    }
    
    ptDyn->dditable = \
        kr_ddi_table_construct(&ptShm->stShmDDI[nSecId], ptDataModule);
    if (ptDyn->dditable == NULL) {
        KR_LOG(KR_LOGERROR, "kr_ddi_table_construct Failed!");
        return NULL;
    }

    ptDyn->hditable = \
        kr_hdi_table_construct(&ptShm->stShmHDI[nSecId], ptDataModule);
    if (ptDyn->hditable == NULL) {
        KR_LOG(KR_LOGERROR, "kr_hdi_table_construct Failed!");
        return NULL;
    }

    ptDyn->grouplist = \
        kr_group_list_construct(&ptShm->stShmGroup[nSecId], ptRuleModule);
    if (ptDyn->grouplist == NULL) {
        KR_LOG(KR_LOGERROR, "kr_group_list_construct Failed!");
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
        kr_group_list_destruct(ptDyn->grouplist);
        kr_free(ptDyn);
    }
}


int kr_dynamicmem_check(T_KRDynamicMem *ptDyn, T_KRShareMem *ptShm, 
        T_KRModule *ptDataModule, T_KRModule *ptRuleModule)
{  
    short nShmSec = ptShm->nSecId;
    T_KRShmSDI *ptShmSDI = &ptShm->stShmSDI[nShmSec];
    T_KRShmDDI *ptShmDDI = &ptShm->stShmDDI[nShmSec];
    T_KRShmHDI *ptShmHDI = &ptShm->stShmHDI[nShmSec];
    T_KRShmGroup *ptShmGroup = &ptShm->stShmGroup[nShmSec];
    
    /*check sdi table*/
    if (ptDyn->sditable->tConstructTime != ptShmSDI->tLastLoadTime) {
        T_KRSDITable *ptSDITable = \
            kr_sdi_table_construct(ptShmSDI, ptDataModule);
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
        T_KRDDITable *ptDDITable = \
            kr_ddi_table_construct(ptShmDDI, ptDataModule);
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
        T_KRHDITable *ptHDITable = \
            kr_hdi_table_construct(ptShmHDI, ptDataModule);
        if (ptHDITable != NULL) {
            kr_hdi_table_destruct(ptDyn->hditable);
            ptDyn->hditable = ptHDITable;
        } else {
            KR_LOG(KR_LOGERROR, "reload hdi table error!");
            return -1;
        }
    }
    
    /*check group list*/
    if (ptDyn->grouplist->tConstructTime != ptShmGroup->tLastLoadTime) {
        T_KRGroupList *ptGroupList = \
            kr_group_list_construct(ptShmGroup, ptRuleModule);
        if (ptGroupList != NULL) {
            kr_group_list_destruct(ptDyn->grouplist);
            ptDyn->grouplist = ptGroupList;
        } else {
            KR_LOG(KR_LOGERROR, "reload group list error!");
            return -1;
        }
    }
    
    return 0;
}



