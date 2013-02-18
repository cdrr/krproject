#include "kr_shm_rule.h"
#include "krutils/kr_utils.h"
#include "dbs/dbs_basopr.h"
#include "dbs/dbs/rule_cur.h"

int LoadShmRule(T_DbsEnv *dbsenv, T_KRShmRule *ptShmRule, long lGroupId)
{
    int nRet = 0;
    int iResult = 0;
    int iCnt = 0;    
    T_RuleCur stRuleCur = {0};
    T_KRShmRuleDef *ptShmRuleDef = &ptShmRule->stShmRuleDef[0];
    
    stRuleCur.lInGroupId = lGroupId;
    iResult = dbsRuleCur(dbsenv, KR_DBCUROPEN, &stRuleCur);
    if (iResult != KR_DBOK) {
        KR_LOG(KR_LOGERROR, "dbsRuleCur Open Error!");
        return -1;
    }
    
    while(1)
    {
        iResult=dbsRuleCur(dbsenv, KR_DBCURFETCH, &stRuleCur);
        if (iResult != KR_DBNOTFOUND && iResult != KR_DBOK) {
            KR_LOG(KR_LOGERROR, "dbsRuleCur Fetch Error!");
            nRet = -1;
            break;
        } else if (iResult == KR_DBNOTFOUND) {
            KR_LOG(KR_LOGDEBUG, "Load [%d] Rules Totally!", iCnt);
            break;
        }
        
        if (iCnt >= N_MAX_RULE_NUM) {
            KR_LOG(KR_LOGERROR, "Only [%d] Rules Allowed!", N_MAX_RULE_NUM);
            nRet = -1;
            break;
        }
        
        memset(ptShmRuleDef, 0x00, sizeof(T_KRShmRuleDef));
        ptShmRuleDef->lRuleId = stRuleCur.lOutRuleId;
        strncpy(ptShmRuleDef->caRuleName, \
                kr_string_rtrim(stRuleCur.caOutRuleName), \
                sizeof(ptShmRuleDef->caRuleName));
        strncpy(ptShmRuleDef->caRuleDesc, \
                kr_string_rtrim(stRuleCur.caOutRuleDesc), \
                sizeof(ptShmRuleDef->caRuleDesc));
        strncpy(ptShmRuleDef->caRuleString, \
                kr_string_rtrim(stRuleCur.caOutRuleString), \
                sizeof(ptShmRuleDef->caRuleString));
        strncpy(ptShmRuleDef->caRuleType, \
                kr_string_rtrim(stRuleCur.caOutRuleType), \
                sizeof(ptShmRuleDef->caRuleType));        
        strncpy(ptShmRuleDef->caRuleFunc, \
                kr_string_rtrim(stRuleCur.caOutRuleFunc), \
                sizeof(ptShmRuleDef->caRuleFunc));        
        ptShmRuleDef->lRuleWeight = stRuleCur.lOutRuleWeight;
        ptShmRuleDef->dRiskAdvcThresh = stRuleCur.dOutRiskAdvcThresh;
        ptShmRuleDef->dRiskNotifThresh = stRuleCur.dOutRiskNotifThresh;
        ptShmRuleDef->dRiskAlertThresh = stRuleCur.dOutRiskAlertThresh;
        ptShmRuleDef->dRiskWarnThresh = stRuleCur.dOutRiskWarnThresh;
        strncpy(ptShmRuleDef->caThreshIsConfig, \
                kr_string_rtrim(stRuleCur.caOutThreshIsConfig),
                sizeof(ptShmRuleDef->caThreshIsConfig));
        
        ptShmRule->lRuleDefCnt++;
        ptShmRuleDef++;
        iCnt++;
    }
    ptShmRule->tLastLoadTime = time(NULL);

    iResult = dbsRuleCur(dbsenv, KR_DBCURCLOSE, &stRuleCur);
    if (iResult != KR_DBOK) {
        KR_LOG(KR_LOGERROR, "dbsRuleCur Close Error!");
        return -1;
    }
    
    return nRet;
}


int DumpShmRule(T_KRShmRule *ptShmRule, FILE *fp)
{
    long l;
    T_KRShmRuleDef *ptShmRuleDef = &ptShmRule->stShmRuleDef[0];
    char            caTimeString[80];
    struct tm *ptmNow = localtime(&ptShmRule->tLastLoadTime);
    strftime(caTimeString, sizeof(caTimeString), "%c", ptmNow);
    
    fprintf(fp, "Dumping Rule...\n");
    fprintf(fp, "Last Load Time[%s]\n", caTimeString);
    for (l=0; l<ptShmRule->lRuleDefCnt; l++) {
        fprintf(fp, "  lRuleId=[%ld], caRuleName=[%s], caRuleDesc=[%s], ", 
                ptShmRuleDef->lRuleId, ptShmRuleDef->caRuleName, ptShmRuleDef->caRuleDesc);
        fprintf(fp, "  caRuleString=[%s], caRuleType=[%s], caRuleFunc=[%s], ",
                ptShmRuleDef->caRuleString, ptShmRuleDef->caRuleType, ptShmRuleDef->caRuleFunc);
        fprintf(fp, "  lRuleWeight=[%ld] \n caThreshIsConfig=[%s], ", 
                ptShmRuleDef->lRuleWeight, ptShmRuleDef->caThreshIsConfig);
        fprintf(fp, "  dRiskAdvcThresh=[%lf], dRiskNotifThresh=[%lf], dRiskAlertThresh=[%lf], dRiskWarnThresh=[%lf] \n", 
                ptShmRuleDef->dRiskAdvcThresh, ptShmRuleDef->dRiskNotifThresh, 
                ptShmRuleDef->dRiskAlertThresh, ptShmRuleDef->dRiskWarnThresh);
        ptShmRuleDef++;
    }
    
    return 0;
}
