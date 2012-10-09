#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "krutils/kr_utils.h"
#include "kr_shm.h"


#define SHM_PERM  0600


/** 
 *  @brief ��թ��⹲���ڴ�Ĵ���
 *  @details ���������ļ����������ڴ�
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1��ʧ��
 */
T_KRShareMem *kr_shm_create(int shmkey)
{
    int iShmId;
    key_t tShmKey = (key_t)shmkey;
    T_KRShareMem *ptShmBuf = NULL;
    
    iShmId = shmget(tShmKey, sizeof(T_KRShareMem), SHM_PERM|IPC_EXCL|IPC_CREAT);
    if (iShmId == -1) {
        KR_LOG(KR_LOGERROR, "shmget ptShmBuf failed[%s]!", strerror(errno));
        return NULL;
    }
    
    ptShmBuf = (T_KRShareMem *)shmat(iShmId, 0, 0);
    if (((long)ptShmBuf) == -1L) {
        KR_LOG(KR_LOGERROR, "shmat ptShmBuf failed[%s]!", strerror(errno));
        return NULL;
    }
    
    ptShmBuf->nSecId = 0;

    return ptShmBuf;
}


/** 
 *  @brief ��թ��⹲���ڴ������
 *  @details ���������ļ�app.ini�е�IPCKEY���ӹ����ڴ��
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1��ʧ��
 */
T_KRShareMem *kr_shm_attach(int shmkey)
{
    int iShmId;
    key_t tShmKey = (key_t )shmkey;
    T_KRShareMem *ptShmBuf = NULL;

    if ((iShmId = shmget(tShmKey, sizeof(T_KRShareMem), SHM_PERM)) == -1) {
        KR_LOG(KR_LOGERROR, "shmget ptShmBuf failed[%s]!", strerror(errno));
        return NULL;
    }
    
    ptShmBuf = (T_KRShareMem *)shmat(iShmId, 0, 0);
    if (((long)ptShmBuf) == -1L) {
        KR_LOG(KR_LOGERROR, "shmat ptShmBuf failed[%s]!", strerror(errno));
        return NULL;
    }
    
    return ptShmBuf;
}

/** 
 *  @brief ��թ��⹲���ڴ�ĶϿ�
 *  @details �Ͽ������ڴ��
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1��ʧ��
 */
void kr_shm_detach(T_KRShareMem *ptShmBuf)
{
    shmdt((void*)ptShmBuf);
    ptShmBuf = NULL;
}


/** 
 *  @brief ��թ��⹲���ڴ��ɾ��
 *  @details ���������ļ�app.ini�е�IPCKEYɾ�������ڴ��
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1��ʧ��
 */
int kr_shm_destroy(int shmkey)
{
    int     iShmId;
    key_t   tShmKey = (key_t)shmkey;

    iShmId = shmget(tShmKey, sizeof(T_KRShareMem), SHM_PERM| IPC_CREAT);
    if (iShmId == -1) {
        KR_LOG(KR_LOGERROR, "shmget ptShmBuf failed[%s]!", strerror(errno));
        return -1;
    }
    
    if (shmctl(iShmId, IPC_RMID, 0) < 0) {
        KR_LOG(KR_LOGERROR, "shmat ptShmBuf failed[%s]!", strerror(errno));
        return -1;
    }
    
    return 0;
}


/** 
 *  @brief ��թ��⹲���ڴ��л�
 *  @details �л���ǰ�����ڴ��������
 *  @return ��
 */
short kr_shm_switch(T_KRShareMem *ptShmBuf)
{
    short nSecId = -1;
    
    nSecId = (ptShmBuf->nSecId+1)%N_MAX_SEC;
    ptShmBuf->nSecId = nSecId;
    
    return nSecId;
}


/** 
 *  @brief ���ع����ڴ���������
 *  @param[in]  ��
 *  @param[out] ��
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1: ʧ��
 */
short kr_shm_load(T_KRShareMem *ptShmBuf)
{
    short       nSecId = -1;
    short       nCurrSecId;
    short       nBackSecId;
    
    if(ptShmBuf == NULL) {
        KR_LOG(KR_LOGERROR, "ptShmBuf is NULL!\n");
        return -1;
    }
     
    nCurrSecId = ptShmBuf->nSecId;
    nBackSecId = (ptShmBuf->nSecId+1)%N_MAX_SEC;
            
    /// �����ڴ汸�鵼�뾲̬������
    memset(&ptShmBuf->stShmSDI[nBackSecId], 0, sizeof(T_KRShmSDI));
    if (LoadShmSDI(&ptShmBuf->stShmSDI[nBackSecId]) != 0) {
        KR_LOG(KR_LOGERROR, "LoadShmSDI failed!");
        return -1;
    }
    
    /// �����ڴ汸�鵼�붯̬ͳ����
    memset(&ptShmBuf->stShmDDI[nBackSecId], 0, sizeof(T_KRShmDDI));
    if (LoadShmDDI(&ptShmBuf->stShmDDI[nBackSecId]) != 0) {
        KR_LOG(KR_LOGERROR, "LoadShmDDI failed!");
        return -1;
    }

    /// �����ڴ汸�鵼����ʷͳ����
    memset(&ptShmBuf->stShmHDI[nBackSecId], 0, sizeof(T_KRShmHDI));
    if (LoadShmHDI(&ptShmBuf->stShmHDI[nBackSecId]) != 0) {
        KR_LOG(KR_LOGERROR, "LoadShmHDI failed!");
        return -1;
    }
    
    /// �����ڴ汸�鵼�������Ϣ
    memset(&ptShmBuf->stShmRule[nBackSecId], 0, sizeof(T_KRShmRule));
    if (LoadShmRule(&ptShmBuf->stShmRule[nBackSecId]) != 0) {
        KR_LOG(KR_LOGERROR, "LoadShmRule failed!");
        return -1;
    }

    /// ���ü���ʱ��
    ptShmBuf->tLastLoadTime = time(NULL);

    /// �����ڴ��������л�
    nSecId = kr_shm_switch(ptShmBuf);
    
    return nSecId;
}


/** 
 *  @brief ���������ڴ���������
 *  @param[in]  ��
 *  @param[out] ��
 *  @return �����������
 *  @retval   0: �ɹ�
 *  @retval  -1: ʧ��
 */
void kr_shm_dump(T_KRShareMem *ptShmBuf, FILE *fp)
{
    short nCurrSecId = ptShmBuf->nSecId;
    short nBackSecId = (ptShmBuf->nSecId+1)%N_MAX_SEC;

    fprintf(fp, "Dumping Share-Memory:Size[%d], Active[%d], Standby[%d]...", \
            sizeof(*ptShmBuf), nCurrSecId, nBackSecId);
    
    fprintf(fp, "\n******Dump Active******\n");
    DumpShmSDI(&ptShmBuf->stShmSDI[nCurrSecId], fp);
    DumpShmDDI(&ptShmBuf->stShmDDI[nCurrSecId], fp);
    DumpShmRule(&ptShmBuf->stShmRule[nCurrSecId], fp);

    fprintf(fp, "\n****Dump Standby******\n");
    DumpShmSDI(&ptShmBuf->stShmSDI[nBackSecId], fp);
    DumpShmDDI(&ptShmBuf->stShmDDI[nBackSecId], fp);
    DumpShmRule(&ptShmBuf->stShmRule[nBackSecId], fp);
    
    fflush(fp);
}