#include "ertu_thread.h"
#include "serial.h"
#include "ertu_subtask.h"
#include "Subfunc.h"
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include "Global_Para_Ex.h"
#include <sys/time.h>

#include <globel_Ex.h>
#include "receive_data.h"

INT8U CID1 = 0x00;
INT8U CID2 = 0x00;

//#define  TEST_DC_SEND_NUM    //���ڲ���: ��ӡ����ֱ����صĴ���

#ifdef TestDug

extern INT32U   RS232_3WriteNum;
extern INT32U   RS232_3ReadNum;
extern INT32U   RS232_4WriteNum;
extern INT32U   RS232_4ReadNum;

#endif

#ifdef   TEST_DC_SEND_NUM
unsigned int DC_SEND_NUM_0x20 = 0;
unsigned int DC_SEND_NUM_0x28 = 0;
#endif 

extern VQUEUE_ID	g_global_quequ[ERTU_QUEUE_NUM_MAX]; 
extern ERTU_device	g_ertu_device;					   

static INT8U  gDCTX_PSMXB_No[3] = {0};
static INT8U  gDCTX_PSMXB_No_W[3] = {0};

//��psmx-bͨѶ�жϴ������ձ��8���ټ�4��.
#define CMDRPT_XB     0x04

//�Ż�Ӳ�ڵ㱨������źŵ��˲�������2��1�θĳ�1��3�Σ���
//���ϸ澯�˲�ʱ��
#define NUM_ERROR_FILTER  0x03   
#if 1 //�Ż���ϵͳ�ܹ������Ӳ�ڵ㣬�����ܼ�ع��϶���/�������ͳ����ʾ.
INT16U NumOfGPIOAll_Error_On = 0x00;   //�ܹ��϶�������
INT16U NumOfGPIOAll_OK_Off = 0x00;     //�ܹ��ϸ������
#endif
//������
/*********************************************************************
 *************Function name: Serial_ReadOperation*********************
 *************Input: iFd��������������������������********************
 *************       pcRs_cfg                    *********************
 *************       ReadLen :the max number     *********************
 *************Return:Data len received           *********************
 *********************************************************************/
INT16U Serial_ReadOperation(int iFd,RS_CFG *pcRs_cfg,INT16U ReadLen)
{
    INT8U	UartBuf[ERTU_MAX_FRAME_SIZE];
    INT16U	UartBufCnt = 0;
    int     iRsize;                     
    struct  timeval cTv;                
    fd_set  cReadFd;                  
    int     j;
    INT8U 	port = 0;
    port = pcRs_cfg->byPort;

    if(port == 2)   //��Ӧ��̨ͨѶ
    {
        cTv.tv_sec = 0;
        cTv.tv_usec = 50000; 			//50ms
    }
    else
    {
        cTv.tv_sec = 1;
        cTv.tv_usec = 0; 			//1s
    }
    FD_ZERO(&cReadFd);
    FD_SET(iFd, &cReadFd);
    UartBufCnt = 0;
    memset((void *)UartBuf, 0, ERTU_MAX_FRAME_SIZE);  //2000

    for(j = 0;j<ReadLen;j++)
    {
        //select: ���ļ����������������ݵ���ʱ���ں�(I/O)����״̬�޸��ļ�����������������һ������0����    
        if ( (iRsize = select(iFd + 1, &cReadFd, NULL, NULL, &cTv)) > 0 )
        {
            iRsize = read(iFd, UartBuf + UartBufCnt, sizeof(UartBuf) - UartBufCnt);

            //debug_printf(0,"addr==%x ,,Uart1Buf[1] == %x ,,  sizeof(UartBuf) - UartBufCnt ==%d  ,, iRsize== %d\n",
            //UartBuf[0],UartBuf[1],sizeof(UartBuf) - UartBufCnt,iRsize);

            if (iRsize > 0)
            {			
                UartBufCnt += iRsize;
            }
        }
    }

#if 0  //��������,��ֹ����ִ��memcpy()ʱ���
    if (UartBufCnt > 255)
    {
        UartBufCnt = 255;
    }
#else  //�Ż����ڽ����������ƴ���
    if (port == 4)  //�˴������⣬��Ҫ���ǵ�����ģ����ͨѶ(�ᳬ��255)
    {
        if (UartBufCnt > 2000)
        {
            UartBufCnt = 2000;
        }    
    }
    else 
    {
        if (UartBufCnt > 255)
        {
            UartBufCnt = 255;
        }    
    }
#endif 

    if(UartBufCnt > 0 && port == 5)           //δʹ��
    {
        memcpy(Uart2Buf,UartBuf,UartBufCnt);  //Uart2Buf���������!!!!!!!!!!!!!!!!!
    }
    else if(port == 4 && UartBufCnt > 0)      //������ups��ͨ�š���Ե����ʱ
    {
        memcpy(Uart1Buf,UartBuf,UartBufCnt);
    }
    else if(port == 3 && UartBufCnt > 0)      //ֱ��
    {
        memcpy(Uart4Buf,UartBuf,UartBufCnt);  //Uart4Buf���������!!!!!!!!!!!!!!!!!
    }
    else if(port == 2 && UartBufCnt > 0)      //��λ��(��̨)
    {
        memcpy(Uart3Buf,UartBuf,UartBufCnt);  //Uart3Buf���������!!!!!!!!!!!!!!!!!
    }
    return UartBufCnt;
}

//void Ertu_Start_Time::run()
//ʱ������߳�
void ERTU_START_TIME(ERTU_WATCHDOG 	*pcWatchDog)
{	
    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            g_ertu_device.dev = (ERTU_DEV_CFG *)pcWatchDog->pcDevice;

            ERTU_TIME(&g_ertu_device,
                    &pcWatchDog->iWatch_Dog, 
                    pcWatchDog->iWatch_Time,
                    &pcWatchDog->bExit,
                    &pcWatchDog->iTaskRunType);
        }
        else
        {
            gError = 56;
        }
    }
}

//ʱ���̣߳�500msΪ��λ�����Ը�����Ҫ����
void ERTU_TIME(ERTU_device *pcDevice, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType)
{		
    INT32U ii;
    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        ii =0;
        while(1)
        {
            //Get_Now_Time2();
            usleep(1000);
            ii ++;
            if(ii >= 500)
                break;
        }
        HalfSecondCnt1 ++;
    }
}
/**********************************************************************************
 *������:	ERTU_start_interact(void Ertu_Start_Interact::run())
 *��������:	������ͨ�š�ups���ݷ����߳�
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void ERTU_start_interact(ERTU_WATCHDOG 	*pcWatchDog)
{	
    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            g_ertu_device.dev = (ERTU_DEV_CFG *)pcWatchDog->pcDevice;

            ERTU_Send_AC_T_UPS(&g_ertu_device,
                    &pcWatchDog->iWatch_Dog, 
                    pcWatchDog->iWatch_Time,
                    &pcWatchDog->bExit,
                    &pcWatchDog->iTaskRunType);
        }
        else
        {
            gError = 14;
        }
    }
}

/* case 1: 0x40 ----- ups
 * case 2: 0x7E ----- if (LN_MK_communication == 2) ���� //ģ��ģ��������1��(CID1=1): 7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D   
 * case 3: 0x30 ----- if (LN_MK_communication != 2) ͨ�ŵ�Դ1
 * case 4: 0x31 ----- if (LN_MK_communication != 2) ͨ�ŵ�Դ2
 * case 5: 0x7E ----- if (LN_MK_communication == 2) ���� //ģ��ģ��������2��(CID1=2): 7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
 * case 6: bgCfg_Ats1Addr -- 0x03   ATS1
 * case 7: bgCfg_Ats2Addr -- 0x03   ATS2
 * case 8: bgCfg_Ats1Addr -- 0x01   ATS1
 * case 9: bgCfg_Ats2Addr -- 0x01   ATS2
 * case 10: 0x10 ----- 0x0B/0x0C(5���) �������� 
 * case 11: 0x11 ----- ��������
 * case 12: 0x80 ----- ��Ե???
 * case 13: 0x7E ----- if (LN_MK_communication == 2) ���� //ģ�鿪����״̬1��(CID1=5): 7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
 * case 14: 0x7E ----- if (LN_MK_communication == 2) ���� //ģ�鿪����״̬2��(CID1=6): 7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
 * case 15: 0x7E ----- if (LN_MK_communication == 2) ���� //�澯��Ϣ1��(CID1=7):       7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
 * case 16: 0x7E ----- if (LN_MK_communication == 2) ���� //�澯��Ϣ2��(CID1=8):       7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
 * case 17: 0x55 ----- B���ʱ 
 * case 18: 0x7E ----- if (LN_MK_communication == 2) ���� //ֱ��ģ���������Ϣ1��(CID1=3): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
 * case 19: 0x7E ----- if (LN_MK_communication == 2) ���� //ֱ��ģ���������Ϣ2��(CID1=4): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D) 
 * case 20~22: bgCfg_Ats1Addr -- 0x05   ATS1 
 * case 23~25: bgCfg_Ats2Addr -- 0x05   ATS2  
 * */
//������ͨ�š�ups���ݷ���
void ERTU_Send_AC_T_UPS(ERTU_device *pcDevice, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType)
{		

    INT8U  CmdAddr = 1;   //8λ�޷�������
    INT8U  CmdAddrBk;
    INT16U CrcValue;
    Cmd_Res_Frame cCmdFrm;
    INT8U  cFrmLen;
    INT8U  CmdCnt;
    INT8U  Len;
    INT8U  cRs232Num = 0;
    INT8U  i=0;

    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        CmdCnt = 0;
        cFrmLen = sizeof(cCmdFrm);
        if(gCmdAddrEnable == 1)  //�Ƿ������ȷ���
        {		
            CmdAddrBk = CmdAddr;
            CmdAddr = gCmdAddr;  //ң��ʱ����gCmdAddr����
        }
        cRs232Num = 5;   //������Ϊ5������Ϊ3

        switch(CmdAddr)
        {
            case 1:
                if(Sys_set_Ups_Num > 0)
                {
                    cCmdFrm.addr = 0x40;
                    cCmdFrm.len = 0x04;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.ups_num & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.switch_monitor_num & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.state_monitor_num & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.work_mode;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else
                {
                    gUPSCmd_No = 0;
                    gUPSCmd_No_W = 0;
                }
                break;
            case 3:
                if (LN_MK_communication != 2)
                {
                    if(Sys_set_Comm_Num >= 1)
                    {
                        cCmdFrm.addr = 0x30;
                        cCmdFrm.len = 0x0B;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].shuntfactor >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].shuntfactor;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_GY_value >>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_GY_value;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_QY_value >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_QY_value & 0x00FF;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                    else
                    {
                        gComPWCmd_No = 0;
                        gComPWCmd_No_W = 0;   //��û��ͨ�ŵ�Դʱ��2�����ϱ�־��Ϊ0�������ù��ϱ�����
                    }
                }
                break;
            case 4:
                if (LN_MK_communication != 2)
                {
                    if(Sys_set_Comm_Num > 1)    //�ڶ���ͨ�ŵ�Դ
                    {
                        cCmdFrm.addr = 0x31;
                        cCmdFrm.len = 0x0B;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].shuntfactor >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].shuntfactor;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_GY_value >>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_GY_value;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_QY_value >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_QY_value & 0x00FF;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                    else
                    {
                        gComPW1Cmd_No = 0;
                        gComPW1Cmd_No_W = 0;
                    }
                }
                break;

            case 6:
                //if (Sys_set_AC_cfg>0)  //�ų�����������潻��ѡ����ʱ��1#ATSͨ�Ź��ϱ���������
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 1 && Sys_cfg_info.ac_set.control_mode == 1)  //ATS(��̩)
                    //ATS��1�����Ʒ�ʽ�������ý������õģ�0ʲô�����ǣ�1ATS,2���
                {
                    cCmdFrm.addr = bgCfg_Ats1Addr;    //��ATSͨ�ţ��߱�׼MoDBusЭ��
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x00;    //ȡң������0x0000~0x0013
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x14;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else if (Sys_cfg_info.ac_set.ATSE_num >= 1 
                        && Sys_cfg_info.ac_set.control_mode == 3)  //ATS(����) //�����ͺ���ATSͨѶ֧��.
                {
                    cCmdFrm.addr = bgCfg_Ats1Addr;    //��ATSͨ�ţ��߱�׼MoDBusЭ��
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x03;    //ȡң������(1000~1075)
                    cCmdFrm.data[CmdCnt++] = 0xE8;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x4C;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else
                {
                    gATSCmd_No = 0;
                    gATSCmd_No_W = 0;
                }
                //}
                break;
            case 7:
                //if (Sys_set_AC_cfg>0)  //�ų�����������潻��ѡ����ʱ��2#ATSͨ�Ź��ϱ���������
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 2 && Sys_cfg_info.ac_set.control_mode == 1) //ATS(��̩)
                {
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x00;  //ȡң������0x0000~0x0013
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x14;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else if (Sys_cfg_info.ac_set.ATSE_num >= 2 
                        && Sys_cfg_info.ac_set.control_mode == 3)  //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                {
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x03;  //ȡң������(1000~1075)
                    cCmdFrm.data[CmdCnt++] = 0xE8;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x4C;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else
                {
                    gATS1Cmd_No = 0;
                    gATS1Cmd_No_W = 0;
                }
                //}
                break;
            case 8:
                //if (Sys_set_AC_cfg>0)  //�ų�����������潻��ѡ����ʱ��1#ATSͨ�Ź��ϱ���������
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 1)  //�����ͺ���ATSͨѶ֧��.
                {
                    if (Sys_cfg_info.ac_set.control_mode == 1)  //ATS(��̩)
                    {
                        cCmdFrm.addr = bgCfg_Ats1Addr;  //�ӻ���ַ
                        cCmdFrm.len  = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //ȡң������0x0000~0x0020
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x20;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                    else if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)
                    {
                        cCmdFrm.addr = bgCfg_Ats1Addr;  //�ӻ���ַ
                        cCmdFrm.len  = 0x03;
                        cCmdFrm.data[CmdCnt++] = 0x01;  //ȡң������(500~509)
                        cCmdFrm.data[CmdCnt++] = 0xF4;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x0A;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                }
                //}
                break;
            case 9:
                //if (Sys_set_AC_cfg>0)  //�ų�����������潻��ѡ����ʱ��2#ATSͨ�Ź��ϱ���������
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 2)  //�����ͺ���ATSͨѶ֧��.
                {
                    if (Sys_cfg_info.ac_set.control_mode == 1) //ATS(��̩)
                    {
                        cCmdFrm.addr = bgCfg_Ats2Addr;
                        cCmdFrm.len  = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //ȡң������0x0000~0x0020
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x20;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                    else if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)
                    {
                        cCmdFrm.addr = bgCfg_Ats2Addr;
                        cCmdFrm.len  = 0x03;
                        cCmdFrm.data[CmdCnt++] = 0x01;  //ȡң������(500~509)
                        cCmdFrm.data[CmdCnt++] = 0xF4;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x0A;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                }
                //}
                break;
            case 10:
                if(Sys_set_AC_cfg>0)
                {
                    if (Sys_cfg_info.ac_set.diancao_num <= 4)
                    {
                        cCmdFrm.addr = 0x10;
                        cCmdFrm.len  = 11;   //0x0B
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.duan_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ATSE_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.diancao_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ac_sampling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //���������и���
#if 0
                        for(i=0;i<4;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //�Զ� diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // �ֶ���բ
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // �ֶ���բ
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;
                            }
                        }
#else                   //�޸��ͺ�̨֮���3����ٵ�ң��ָ��: ��̨ң�ص���߼�����
                        //���1,2
                        for (i = 0; i < 2; i++)
                        {
                            if (sys_ctl_info.diancao_mode_ctl[i] == 0x02)   //�·�����Ϊ�ֶ���բ
                            {
                                if ((Ac_info.ac_in_data[i].SW_state == 0x00) 
                                        && (diancao_cmd_num[i] > 0x00)) //��ǰ���״̬Ϊ��բ, ����δ�ﵽ��෢�ʹ���
                                {
                                    cCmdFrm.data[CmdCnt++] = 0x01;  //�ֶ���բ
                                }
                                else  //������ɺ�ת���Զ�����  
                                {
                                    cCmdFrm.data[CmdCnt++] = 0xFF;           //�Զ����� 
                                    sys_ctl_info.diancao_mode_ctl[i] = 0x00; //�Զ�����
                                    diancao_cmd_num[i] = 0x00;
                                }
                            }
                            else if (sys_ctl_info.diancao_mode_ctl[i] == 0x01) //�·�����Ϊ�ֶ���բ     
                            {
                                if ((Ac_info.ac_in_data[i].SW_state == 0x01)
                                        && (diancao_cmd_num[i] > 0x00)) //��ǰ���״̬Ϊ��բ, ����δ�ﵽ��෢�ʹ���  
                                {
                                    cCmdFrm.data[CmdCnt++] = 0x00;  //�ֶ���բ 
                                }
                                else   //������ɺ�ת���Զ����� 
                                {
                                    cCmdFrm.data[CmdCnt++] = 0xFF;           //�Զ����� 
                                    sys_ctl_info.diancao_mode_ctl[i] = 0x00; //�Զ�����
                                    diancao_cmd_num[i] = 0x00;        
                                }
                            }
                            else  //Ĭ��Ϊ�Զ�����(��������ز����ܼ�ػ��̨����)
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;  //�Զ�����
                            }
                        }

                        //���3(ĸ��)
                        if (sys_ctl_info.diancao_mode_ctl[2] == 0x02) //�·�����Ϊ�ֶ���բ
                        {
                            if ((Ac_info.ML_state == 0x00)
                                    && (diancao_cmd_num[2] > 0x00)) //��ǰ���״̬Ϊ��բ, ����δ�ﵽ��෢�ʹ���  
                            {
                                cCmdFrm.data[CmdCnt++] = 0x01;  //�ֶ���բ
                            }
                            else  //������ɺ�ת���Զ�����  
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;           //�Զ����� 
                                sys_ctl_info.diancao_mode_ctl[2] = 0x00; //�Զ����� 
                                diancao_cmd_num[2] = 0x00;                              
                            }
                        }
                        else if (sys_ctl_info.diancao_mode_ctl[2] == 0x01) //�·�����Ϊ�ֶ���բ              
                        {
                            if ((Ac_info.ML_state == 0x01)
                                    && (diancao_cmd_num[2] > 0x00))  //��ǰ���״̬Ϊ��բ, ����δ�ﵽ��෢�ʹ��� 
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;  //�ֶ���բ
                            }
                            else  //������ɺ�ת���Զ����� 
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;           //�Զ����� 
                                sys_ctl_info.diancao_mode_ctl[2] = 0x00; //�Զ�����
                                diancao_cmd_num[2] = 0x00;                                 
                            }                            
                        }
                        else  //Ĭ��Ϊ�Զ�����(��������ز����ܼ�ػ��̨����)
                        {
                            cCmdFrm.data[CmdCnt++] = 0xFF;  //�Զ�����
                        }

                        //���4Ĭ��Ϊ�Զ�����  
                        cCmdFrm.data[CmdCnt++] = 0xFF;  
#endif 
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                    else if(Sys_cfg_info.ac_set.diancao_num == 5)  //5��٣�������10��һ֡  //����5���ʱ����֡0x10λ���ƶ�.
                    {
                        cCmdFrm.addr = 0x10;
                        cCmdFrm.len  = 0x0C;  //12
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.duan_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ATSE_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.diancao_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ac_sampling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //���������и���
                        for(i=0;i<5;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //�Զ� diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // �ֶ���բ
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // �ֶ���բ
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;
                            }
                        }
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }                    
                }
                break;
            case 11:
                if(Sys_cfg_info.ac_set.state_monitor_num > 0)  //0-20��������   һ���ֽ�8λ
                {
                    cCmdFrm.addr = 0x11;
                    cCmdFrm.len  = 0x00;
                    Len = 0;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
#if 0   //�ϲ���case 10   //����5���ʱ����֡0x10λ���ƶ�.
            case 20:
                if(Sys_set_AC_cfg>0)  
                {
                    if(Sys_cfg_info.ac_set.diancao_num == 5)  //5��٣�������10��һ֡
                    {
                        cCmdFrm.addr = 0x10;
                        cCmdFrm.len  = 0x0C;  //12
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.duan_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ATSE_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.diancao_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ac_sampling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //���������и���
                        for(i=0;i<5;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //�Զ� diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // �ֶ���բ
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // �ֶ���բ
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;
                            }
                        }
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 3;
                    }
                }
                break;
#endif 
            case 12:
                if(Sys_cfg_info.sys_set.insulate_mode == 0x00)    //����
                {
                    cCmdFrm.addr = 0x80;
                    cCmdFrm.len = 0x07;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].polling_num;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].switch_monitor_num;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].state_monitor_num;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].dropout_voltage_limit>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].dropout_voltage_limit;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].GND_R_limit>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.insulate_set[0].GND_R_limit;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                else
                {
                    gIsuCmd_No = 0;
                    gIsuCmd_No_W = 0;
                }
                break;
            case 2:    //ģ��ģ��������1��(CID1=1): 7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num >= 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x42;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                    else {
                        gComPWCmd_No = 0;
                        gComPWCmd_No_W = 0;
                    }
                }
                break;
            case 5:    //ģ��ģ��������2��(CID1=2): 7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num > 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x42;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }else {
                        gComPW1Cmd_No = 0;
                        gComPW1Cmd_No_W = 0;	
                    }
                }
                break;		
            case 13:    //ģ�鿪����״̬1��(CID1=5): 7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num >= 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x33;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x42;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;
            case 14:   //ģ�鿪����״̬2��(CID1=6): 7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num > 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x33;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x41;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;
            case 15:    //�澯��Ϣ1��(CID1=7): 7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num >= 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x41;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;	
            case 16:    //�澯��Ϣ2��(CID1=8): 7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num > 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x41;
                        cCmdFrm.data[CmdCnt++] = 0x45;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;	
            case 18:    //ֱ��ģ���������Ϣ1��(CID1=3): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num >= 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x45;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x33;
                        cCmdFrm.data[CmdCnt++] = 0x39;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;	
            case 19:    //ֱ��ģ���������Ϣ2��(CID1=4): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D)
                if (LN_MK_communication == 2)
                {
                    if (Sys_set_Comm_Num > 1)
                    {
                        cCmdFrm.addr = 0x7E;
                        cCmdFrm.len = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x34;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x45;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x32;
                        cCmdFrm.data[CmdCnt++] = 0x30;
                        cCmdFrm.data[CmdCnt++] = 0x31;
                        cCmdFrm.data[CmdCnt++] = 0x46;
                        cCmdFrm.data[CmdCnt++] = 0x44;
                        cCmdFrm.data[CmdCnt++] = 0x33;
                        cCmdFrm.data[CmdCnt++] = 0x38;
                        cCmdFrm.data[CmdCnt++] = 0x0D;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len+2;
                        cRs232Num = 3;	
                    }
                }
                break;		
            case 17:
                if(HalfSecondCnt1 >=10)    //B���ʱ
                {
                    cCmdFrm.addr = 0x55;
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    cCmdFrm.data[CmdCnt++] = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x08;
                    cCmdFrm.data[CmdCnt++] = 0x1F;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
                /*  ATSE control */
            case 20:   //�����ͺ���ATSͨѶ֧��.
                if(Sys_set_AC_cfg>0){
                    cCmdFrm.addr = bgCfg_Ats1Addr;
                    cCmdFrm.len  = 0x05;

                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x3A;    //15004: �Զ�/�ֶ�	0���ֶ� 1���Զ�
                        cCmdFrm.data[CmdCnt++] = 0x9C;                    
                    }
                    else //ATS(��̩)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0x0004: �Զ�/�ֶ�	0���ֶ� 1���Զ�
                        cCmdFrm.data[CmdCnt++] = 0x04;                    
                    }

                    if(sys_ctl_info.ATSE_mode_ctl[0] != 0x00)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;
                    }else{
                        cCmdFrm.data[CmdCnt++] = 0xFF;
                    }                
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            case 21:
                if(sys_ctl_info.ATSE_mode_ctl[0]  != 0x00){
                    cCmdFrm.addr = bgCfg_Ats1Addr;
                    cCmdFrm.len = 0x05;            //������05 ��ǿ�ƿ�����
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A; //1/3: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }
                    }         
                    else  //ATS(��̩)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00; //1/3: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01;
                        }                    
                    }                           
                    cCmdFrm.data[CmdCnt++] = 0xFF;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            case 22:
                if(sys_ctl_info.ATSE_mode_ctl[0]  != 0x00){
                    cCmdFrm.addr = bgCfg_Ats1Addr;
                    cCmdFrm.len = 0x05;
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;    //0/2: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        } 
                    }   
                    else //ATS(��̩) 
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0/2: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03;
                        }                                        
                    }                 
                    cCmdFrm.data[CmdCnt++] = 0xFF;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            case 23:
                if(Sys_set_AC_cfg>0){
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len  = 0x05;
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x3A;    //15004: �Զ�/�ֶ�	0���ֶ� 1���Զ�
                        cCmdFrm.data[CmdCnt++] = 0x9C;                    
                    }
                    else //ATS(��̩)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0x0004: �Զ�/�ֶ�	0���ֶ� 1���Զ�
                        cCmdFrm.data[CmdCnt++] = 0x04;                    
                    }
                    if(sys_ctl_info.ATSE_mode_ctl[1]  != 0x00)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;
                    }else{
                        cCmdFrm.data[CmdCnt++] = 0xFF;
                    }                
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            case 24:
                if(sys_ctl_info.ATSE_mode_ctl[1]  != 0x00){
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len = 0x05;
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;   //1/3: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }   
                    }   
                    else //ATS(��̩) 
                    { 
                        cCmdFrm.data[CmdCnt++] = 0x00;   //1/3: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01;
                        }                   
                    }           
                    cCmdFrm.data[CmdCnt++] = 0xFF;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            case 25:
                if(sys_ctl_info.ATSE_mode_ctl[1]  != 0x00){
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len = 0x05;
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(����)  //�����ͺ���ATSͨѶ֧��.
                    {                  
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;   //0/2: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }
                    }  
                    else  //ATS(��̩)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;   //0/2: ��բ
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03;
                        }
                    }            
                    cCmdFrm.data[CmdCnt++] = 0xFF;
                    cCmdFrm.data[CmdCnt++] = 0x00;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 3;
                }
                break;
            default:
                break;
        }

        if(cRs232Num == 3 && gCmdAddrEnable == 1)  //gCmdAddrEnable: ���ò���ʱ�ĵ�ַ���ȷ�  //�������(���ò���ʱ)
        {
            if(gEnQueue3)  //˵���ѷ���1�����ݣ������ٷ���һ�����ݰ������Ͷ���
            {
                cFrmLen = sizeof(cCmdFrm);

                //���������(����)
                if(VQ_Enqueue(g_global_quequ[ERTU_QUEUE_ERTUTORS232],
                            (INT8U*)&cCmdFrm,cFrmLen,1000) == TRUE)  
                {
                    gError = 0;
                }
                else
                {
                    gError = 15;
                }

                gCmdAddr++;
                if(sys_ctl_info.ATSE2_ctl_flag == 1)  //ң��ATS2(20~25)
                {
                    if(CmdAddr == 25)
                    {
                        gCmdAddrEnable = 0;
                        sys_ctl_info.ATSE2_ctl_flag = 0;
                    }
                    if(gCmdAddr > 25)
                        gCmdAddr = 20;
                }
                else  //ң��ATS1(20~22)
                {
                    if(CmdAddr == 22)
                    {
                        gCmdAddrEnable = 0;
                    }
                    if(gCmdAddr > 22)
                        gCmdAddr = 20;
                }
                gEnQueue3 = 0;
            }
        }
        else if(cRs232Num == 3 && gCmdAddrEnable == 0) //�������(�����ò���ʱ)
        {
            if(gEnQueue3)
            {
                cFrmLen = sizeof(cCmdFrm);

                //�Ѵ����͵����ݷ�������Ͷ�����(����)
                if(VQ_Enqueue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_Auto],
                            (INT8U*)&cCmdFrm,cFrmLen,1000) == TRUE)
                {
                    gError = 0;
                }
                else
                {
                    gError = 16;
                }
                gCmdAddrEnable = 0;
                CmdAddr++;
                gEnQueue3 = 0;
            }
        }
        else
        {
            if(cRs232Num == 5)
            {
                gCmdAddrEnable = 0;
                gCmdAddr = 20;
            }
            CmdAddr ++;
        }
        //if(CmdAddr > 20)
        if(CmdAddr > 19)   //ԭcase20�ϲ���case10
        {
            CmdAddr = 1;
        }
    }
}
/**********************************************************************************
 *������:	ERTU_start_interact1(Ertu_Start_Interact1::run())
 *��������:	ֱ�����ݷ����߳�
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void ERTU_start_interact1(ERTU_WATCHDOG 	*pcWatchDog)
{	
    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            g_ertu_device.dev = (ERTU_DEV_CFG *)pcWatchDog->pcDevice;

            ERTU_Send_DC(&g_ertu_device,
                    &pcWatchDog->iWatch_Dog, 
                    pcWatchDog->iWatch_Time,
                    &pcWatchDog->bExit,
                    &pcWatchDog->iTaskRunType);
        }
        else
        {
            gError = 14;
        }
    }
}

//ֱ�����ݷ��ʹ���
void ERTU_Send_DC(ERTU_device *pcDevice, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType)
{		

    INT8U CmdAddr = 1;
    INT8U CmdAddrBk;
    INT16U CrcValue;
    Cmd_Res_Frame cCmdFrm;
    INT8U cFrmLen;
    INT8U  CmdCnt;
    INT8U  Len;
    INT8U  cRs232Num;
    INT8U DataIn[8];
    INT8U DataByte;
    INT8U DC_duan_NUM = 0;
    INT8U i=0;
    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        CmdCnt = 0;
        cFrmLen = sizeof(cCmdFrm);
        if(gCmdAddrEnable1 == 1)
        {
            CmdAddrBk = CmdAddr;
            CmdAddr = gCmdAddr1;
        }
        cRs232Num = 5;
        switch(CmdAddr)
        {
            case 1:      
                if (DC_duan_NUM == 0){
                    cCmdFrm.addr = 0x20;

#if 0
                    cCmdFrm.len = 0x19;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                    for(i=0;i<8;i++)
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //���ģ�鴦���ԭ8������12��.
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8��ģ��  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8������ģ��

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //���4��ģ��ػ�
                    }
#endif 

                    cCmdFrm.data[CmdCnt++] = sys_ctl_info.battery_mode_ctl[DC_duan_NUM];
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V ;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger >>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10);
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule;

                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;
                }else if(DC_duan_NUM == 1){
                    cCmdFrm.addr = 0x28;

#if 0
                    cCmdFrm.len = 0x19;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                    for(i=0;i<8;i++)
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //���ģ�鴦���ԭ8������12��. 
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8��ģ��  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8������ģ��

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //���4��ģ��ػ�
                    }
#endif 

                    cCmdFrm.data[CmdCnt++] = sys_ctl_info.battery_mode_ctl[DC_duan_NUM];
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V ;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger >>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10);
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule;

                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;	
                }else if (DC_duan_NUM == 2){
                    cCmdFrm.addr = 0x25;

#if 0
                    cCmdFrm.len = 0x19;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                    for(i=0;i<8;i++)
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //���ģ�鴦���ԭ8������12��.
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8��ģ��  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8������ģ��

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //��Ϊѡ��1Ҫ��������Э���п���Ϊ0  ~�෴
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //���4��ģ��ػ�
                    }
#endif 

                    cCmdFrm.data[CmdCnt++] = sys_ctl_info.battery_mode_ctl[DC_duan_NUM];
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_V ;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_V;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].temperature_compensation;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = 0;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].BatteryCharge_I);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger >>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].FloatCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_trigger;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_timer*10);
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10) >> 8;
                    cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_delay*10);
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].EqualCharge_regule;

                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;	
                }		
                break;
            case 2:
                if (DC_duan_NUM == 0){
                    cCmdFrm.addr = 0x21;
                    cCmdFrm.len = 0x15;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcPowerSupply_mode;
                    /* if(Sys_cfg_info.sys_set.insulate_mode == 0)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x00;
                       }
                       else
                       {
                       if(Sys_set_DC_duan == 1)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       else
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       }*/
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.insulate_mode;
                    /*if(Sys_cfg_info.sys_set.battery_mode == 0)
                      {
                      cCmdFrm.data[CmdCnt++] = 0x00;
                      }
                      else
                      {
                      cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                      }*/
                    cCmdFrm.data[CmdCnt++] = Sys_set_battery_group_num;

#if 0
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.charge_mode;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].charge_mode;
#endif 

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.battery_mode;

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;	
                }else if (DC_duan_NUM == 1){
                    cCmdFrm.addr = 0x29;
                    cCmdFrm.len = 0x15;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcPowerSupply_mode;
                    /* if(Sys_cfg_info.sys_set.insulate_mode == 0)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x00;
                       }
                       else
                       {
                       if(Sys_set_DC_duan == 1)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       else
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       }*/
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.insulate_mode;
                    /*if(Sys_cfg_info.sys_set.battery_mode == 0)
                      {
                      cCmdFrm.data[CmdCnt++] = 0x00;
                      }
                      else
                      {
                      cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                      }*/
                    cCmdFrm.data[CmdCnt++] = Sys_set_battery_group_num;

#if 0
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.charge_mode;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].charge_mode;
#endif 

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.battery_mode;

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;	
                }else if (DC_duan_NUM == 2){
                    cCmdFrm.addr = 0x26;
                    cCmdFrm.len = 0x15;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcPowerSupply_mode;
                    /* if(Sys_cfg_info.sys_set.insulate_mode == 0)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x00;
                       }
                       else
                       {
                       if(Sys_set_DC_duan == 1)
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       else
                       {
                       cCmdFrm.data[CmdCnt++] = 0x01;
                       }
                       }*/

#if 0
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.insulate_mode;
#else      //��3·ֱ���޾�Ե��� 
                    cCmdFrm.data[CmdCnt++] = 0x00;  //��Ե����(0x00��1-----��������)
#endif 

                    /*if(Sys_cfg_info.sys_set.battery_mode == 0)
                      {
                      cCmdFrm.data[CmdCnt++] = 0x00;
                      }
                      else
                      {
                      cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                      }*/
                    cCmdFrm.data[CmdCnt++] = Sys_set_battery_group_num;
#if 0
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.charge_mode;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].charge_mode;
#endif 

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.sys_set.battery_mode;

                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].AcInput_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].charger_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_GY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY >> 8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].control_busbar_QY;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].ChargerSplitter_factor;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor>>8;
                    cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].BatterySplitter_factor;
                    Len = CmdCnt;
                    CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                    cCmdFrm.CrcChk = CrcValue;
                    cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                    cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                    Len = CmdCnt;
                    cCmdFrm.CFrmSize = Len +2;
                    cRs232Num = 4;	
                }
                break;
            case 3:
                if (DC_duan_NUM == 0){
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //��Ե������־
                    {
                        cCmdFrm.addr = 0x22;
                        cCmdFrm.len = 0x05;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].insulate_polling_num;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10)>> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10);
                        //                cCmdFrm.data[CmdCnt++] = ((bSysCfgSet_IsuInd_NO_Yes &0x01)<<2)|(Sys_set_DC_duan);
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }	
                }else if (DC_duan_NUM == 1){
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //��Ե������־
                    {
                        cCmdFrm.addr = 0x2A;
                        cCmdFrm.len = 0x05;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].insulate_polling_num;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10)>> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10);
                        //                cCmdFrm.data[CmdCnt++] = ((bSysCfgSet_IsuInd_NO_Yes &0x01)<<2)|(Sys_set_DC_duan);
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }	
                }
#if 0  //����			
                else if (DC_duan_NUM == 2){				
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //��Ե������־
                    {
                        cCmdFrm.addr = 0x27;
                        cCmdFrm.len = 0x05;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].insulate_polling_num;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_GND_R_limit*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10)>> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.dc_set[0].Insulate_DropoutVoltage_limit*10);
                        //                cCmdFrm.data[CmdCnt++] = ((bSysCfgSet_IsuInd_NO_Yes &0x01)<<2)|(Sys_set_DC_duan);
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }				
                }
#endif 			

                break;
            case 4:
                if (DC_duan_NUM == 0){
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)
                    {
                        cCmdFrm.addr = 0x23;
                        cCmdFrm.len = 0x02;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].switch_monitor_num;
                        if(Sys_cfg_info.sys_set.insulate_mode==0x03)
                            cCmdFrm.data[CmdCnt++] = 0;
                        else{
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;
                        }
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }	
                }else if (DC_duan_NUM == 1){
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)
                    {
                        cCmdFrm.addr = 0x2B;
                        cCmdFrm.len = 0x02;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].switch_monitor_num;
                        if(Sys_cfg_info.sys_set.insulate_mode==0x03)
                            cCmdFrm.data[CmdCnt++] = 0;
                        else{
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;
                        }
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }	
                }else if (DC_duan_NUM == 2){
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)
                    {
                        cCmdFrm.addr = 0x2D;
                        cCmdFrm.len = 0x02;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].switch_monitor_num;
                        if(Sys_cfg_info.sys_set.insulate_mode==0x03)
                            cCmdFrm.data[CmdCnt++] = 0;
                        else{
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;
                        }
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }
                }

                break;
            case 5:
                if (DC_duan_NUM == 0){
                    if(Fg_SysSet_BatteryCheckMode != 0)    //��ض�����־
                    {
                        cCmdFrm.addr = 0x24;
                        cCmdFrm.len = 0x0C;

#if 0
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit;
#endif 

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                        gPwrCmd_No = 0;
                        gPwrCmd_No_W = 0;
                    }
                    else  //�����ĵ��Ѳ��: PSMX-B 
                    {
                        cCmdFrm.addr = 0xB0;
                        cCmdFrm.len = 0x03;

                        cCmdFrm.data[CmdCnt++] = 0x00;  //Star Addr
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //Length
                        cCmdFrm.data[CmdCnt++] = 0x6E;

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }    
                }else if (DC_duan_NUM == 1 && Sys_set_battery_group_num >= 0x02){
                    if(Fg_SysSet_BatteryCheckMode != 0)    //��ض�����־
                    {
                        cCmdFrm.addr = 0x2C;
                        cCmdFrm.len = 0x0C;
#if 0
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit;
#endif 

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                        gPwrCmd_No = 0;
                        gPwrCmd_No_W = 0;
                    }
                    else  //�����ĵ��Ѳ��: PSMX-B 
                    {
                        cCmdFrm.addr = 0xB1;
                        cCmdFrm.len = 0x03;

                        cCmdFrm.data[CmdCnt++] = 0x00;  //Star Addr
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //Length
                        cCmdFrm.data[CmdCnt++] = 0x6E;

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }    
                }
                else if (DC_duan_NUM == 2 && Sys_set_battery_group_num >= 0x03)
                {
                    if(Fg_SysSet_BatteryCheckMode != 0)    //��ض�����־
                    {
                        cCmdFrm.addr = 0x2E;
                        cCmdFrm.len = 0x0C;

#if 0
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set.single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set.voltage_lowerLimit;
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].battery_amount;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_upperLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10) >> 8;
                        cCmdFrm.data[CmdCnt++] = (Sys_cfg_info.battery_set[DC_duan_NUM].single_lowerLimit_v*10);
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num >> 8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].polling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].single_capacity;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit>>8;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.battery_set[DC_duan_NUM].voltage_lowerLimit;
#endif 
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                        gPwrCmd_No = 0;
                        gPwrCmd_No_W = 0;
                    }
                    else  //�����ĵ��Ѳ��: PSMX-B 
                    {
                        cCmdFrm.addr = 0xB2;
                        cCmdFrm.len = 0x03;

                        cCmdFrm.data[CmdCnt++] = 0x00;  //Star Addr
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //Length
                        cCmdFrm.data[CmdCnt++] = 0x6E;

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }    
                }
                break; 
            case 6:
                if (DC_duan_NUM == 0){
#if 0
                    if(Special_35KV_flag == 2)
#else  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                        if ((Special_35KV_flag == 0x02)   //��û��UPS���ʱ 
                                || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02))    
#endif 
                        {
                            cCmdFrm.addr = 0x26;
                            cCmdFrm.len = 0x04;
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.ups_num & 0x00FF;          //UPS��Դ����
                            cCmdFrm.data[CmdCnt++] = 0;
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.state_monitor_num & 0x00FF;//UPS״̬�и���
                            cCmdFrm.data[CmdCnt++] = 0;
                            Len = CmdCnt;
                            CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                            cCmdFrm.CrcChk = CrcValue;
                            cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                            cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                            Len = CmdCnt;
                            cCmdFrm.CFrmSize = Len +2;
                            cRs232Num = 4;
                        }
                }

                break;
            case 7:
                if (DC_duan_NUM == 0){
                    if(LN_MK_communication == 2){  //����ģ��
#if 0
                        if(Special_35KV_flag == 2) 
#else  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                            if ((Special_35KV_flag == 0x02)   //��û��ͨ�ż��ʱ 
                                    || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))    
#endif   
                            {
                                cCmdFrm.addr = 0x25;
                                cCmdFrm.len = 0x0B;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].state_monitor_num;  //ͨ�ŵ�Դ״̬�и���
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                Len = CmdCnt;
                                CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                                cCmdFrm.CrcChk = CrcValue;
                                cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                                cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                                Len = CmdCnt;
                                cCmdFrm.CFrmSize = Len +2;
                                cRs232Num = 4;
                            }		
                    }else {  //������ģ��
#if 0 
                        if(Special_35KV_flag == 2)
#else  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                            if ((Special_35KV_flag == 0x02)   //��û��ͨ�ż��ʱ 
                                    || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))    
#endif   
                            {
                                cCmdFrm.addr = 0x25;
                                cCmdFrm.len = 0x0B;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_num;        //ͨ�ŵ�Դģ�����
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].state_monitor_num; //ͨ�ŵ�Դ״̬�и���
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                Len = CmdCnt;
                                CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                                cCmdFrm.CrcChk = CrcValue;
                                cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                                cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                                Len = CmdCnt;
                                cCmdFrm.CFrmSize = Len +2;
                                cRs232Num = 4;
                            }	
                    }
                }
                break;

            case 8:
                if (DC_duan_NUM == 0){
#if 0
                    if(Sys_cfg_info.battery_set.CYH_communi == 1)
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        if(Sys_cfg_info.battery_set[DC_duan_NUM].CYH_communi == 1)
#endif 
                        {
                            cCmdFrm.addr = 0x7E;
                            cCmdFrm.len = 0x60;
                            cCmdFrm.data[CmdCnt++] = 0x70;
                            cCmdFrm.data[CmdCnt++] = 0x00;
                            Len = CmdCnt;
                            CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                            cCmdFrm.CrcChk = CrcValue;
                            cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                            cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                            Len = CmdCnt;
                            cCmdFrm.CFrmSize = Len +2;
                            cRs232Num = 4;
                        }
                }
                break;

            case 9:
                if (DC_duan_NUM == 0){
#if 0
                    if(Sys_cfg_info.battery_set.CYH_communi == 1)
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        if(Sys_cfg_info.battery_set[DC_duan_NUM].CYH_communi == 1)
#endif 
                        {
                            cCmdFrm.addr = 0x7E;
                            cCmdFrm.len = 0x61;
                            cCmdFrm.data[CmdCnt++] = 0x70;
                            cCmdFrm.data[CmdCnt++] = 0x00;
                            Len = CmdCnt;
                            CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                            cCmdFrm.CrcChk = CrcValue;
                            cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                            cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                            Len = CmdCnt;
                            cCmdFrm.CFrmSize = Len +2;
                            cRs232Num = 4;
                        }
                }
                break;

            case 10:
                if (DC_duan_NUM == 0){
#if 0
                    if(Sys_cfg_info.battery_set.CYH_communi == 1)
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        if(Sys_cfg_info.battery_set[DC_duan_NUM].CYH_communi == 1)
#endif 
                        {
                            cCmdFrm.addr = 0x7E;
                            cCmdFrm.len = 0x62;
                            cCmdFrm.data[CmdCnt++] = 0x70;
                            cCmdFrm.data[CmdCnt++] = 0x00;
                            Len = CmdCnt;
                            CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                            cCmdFrm.CrcChk = CrcValue;
                            cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                            cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                            Len = CmdCnt;
                            cCmdFrm.CFrmSize = Len +2;
                            cRs232Num = 4;
                        }
                }
                break;

            case 11:
                if (DC_duan_NUM == 0){
#if 0
                    if(Sys_cfg_info.battery_set.CYH_communi == 1)
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                        if(Sys_cfg_info.battery_set[DC_duan_NUM].CYH_communi == 1)
#endif 
                        {
                            cCmdFrm.addr = 0x7E;
                            cCmdFrm.len = 0x63;
                            cCmdFrm.data[CmdCnt++] = 0x70;
                            cCmdFrm.data[CmdCnt++] = 0x00;
                            Len = CmdCnt;
                            CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                            cCmdFrm.CrcChk = CrcValue;
                            cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                            cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                            Len = CmdCnt;
                            cCmdFrm.CFrmSize = Len +2;
                            cRs232Num = 4;
                        }
                }
                break;

            case 12:
                if (DC_duan_NUM == 0){
                    if ((Sys_cfg_info.sys_set.sampling_box_num != 0)
                            || (Special_35KV_flag == 2)  //���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3�����
#if 1  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
                            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02)
#endif 
                       )  
                    {
                        cCmdFrm.addr = 0x50;
                        cCmdFrm.len = 0x20;
                        cCmdFrm.data[CmdCnt++] = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;   //Byte5
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //��׼��Ԫ��ģ�������״̬������������
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte10						
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v >> 8;  //ATDģ���ѹ*10   ���ֽ�
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v;       //ATDģ���ѹ*10   ���ֽ�
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte15
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte20
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte25
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte30
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }
                }else if (DC_duan_NUM == 1){
                    if ((Sys_cfg_info.sys_set.sampling_box1_num != 0)
                            || (Special_35KV_flag == 2)//���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3�����
#if 1  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
                            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02)
#endif                           
                       )  
                    {
                        cCmdFrm.addr = 0x51;
                        cCmdFrm.len = 0x20;
                        cCmdFrm.data[CmdCnt++] = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;    //Byte5
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //��׼��Ԫ��ģ�������״̬������������
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;		  //Byte10				
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v >> 8;  //ATDģ���ѹ*10   ���ֽ�
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v;       //ATDģ���ѹ*10   ���ֽ�
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte15
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte20
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte25
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte30
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }
                }else if (DC_duan_NUM == 2){
                    if ((Sys_cfg_info.sys_set.sampling_box1_num == 20)
                            || (Special_35KV_flag == 2)  //���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3�����
#if 1  //������������ͨ�š�UPS��Դ��ص�ѡ����.
                            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
                            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02)
#endif   
                       ) 
                    {   //ֱ��3��Ԥ���������ⲻȻ��
                        cCmdFrm.addr = 0x52;
                        cCmdFrm.len = 0x20;
                        cCmdFrm.data[CmdCnt++] = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0x00;    //Byte5
                        cCmdFrm.data[CmdCnt++] = 0x00;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //��׼��Ԫ��ģ�������״̬������������
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;		  //Byte10			
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v >> 8;  //ATDģ���ѹ*10   ���ֽ�(�˴�ͬ�ڶ���ͨ�ŵ�Դ)
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v;       //ATDģ���ѹ*10   ���ֽ�
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte15
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte20
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte25
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;       //Byte30
                        cCmdFrm.data[CmdCnt++] = 0;
                        cCmdFrm.data[CmdCnt++] = 0;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }
                }
                break;

            case 13:  //��PSMX-B�ĵ������
                if (Fg_SysSet_BatteryCheckMode == 0)    //�����ĵ��Ѳ��: PSMX-B 
                {
                    if ((DC_duan_NUM == 0x00)
                            || (DC_duan_NUM == 0x01 && Sys_set_battery_group_num >= 0x02) 
                            || (DC_duan_NUM == 0x02 && Sys_set_battery_group_num >= 0x03))
                    {
                        cCmdFrm.addr = 0xB0 + DC_duan_NUM;                   
                        cCmdFrm.len = 0x03;

                        cCmdFrm.data[CmdCnt++] = 0x00;  //Star Addr
                        cCmdFrm.data[CmdCnt++] = 0x6E;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //Length
                        cCmdFrm.data[CmdCnt++] = 0x6D;
                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }           
                }
                break;

            case 14:  //��PSMX-B�ĵ��ң��(������ͨѶ�쳣�����賬��)
                if (Fg_SysSet_BatteryCheckMode == 0)    //�����ĵ��Ѳ��: PSMX-B 
                {
                    if ((DC_duan_NUM == 0x00)
                            || (DC_duan_NUM == 0x01 && Sys_set_battery_group_num >= 0x02) 
                            || (DC_duan_NUM == 0x02 && Sys_set_battery_group_num >= 0x03))
                    {
                        cCmdFrm.addr = 0xB0 + DC_duan_NUM;  
                        cCmdFrm.len = 0x04;
                        cCmdFrm.data[CmdCnt++] = 0x03;  //Star Addr
                        cCmdFrm.data[CmdCnt++] = 0x07;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //Length
                        cCmdFrm.data[CmdCnt++] = 0x08;

                        Len = CmdCnt;
                        CrcValue = Load_crc(Len+2,(INT8U *)&cCmdFrm);
                        cCmdFrm.CrcChk = CrcValue;
                        cCmdFrm.data[CmdCnt++] = CrcValue & 0x00FF;
                        cCmdFrm.data[CmdCnt++] = (CrcValue >> 8) & 0x00FF;
                        Len = CmdCnt;
                        cCmdFrm.CFrmSize = Len +2;
                        cRs232Num = 4;
                    }                   
                }
                break;
        }

#if 1   //��������桱-��ϵͳ���á�-��ֱ��ϵͳ����������ѡ��ޡ�.
        if (Sys_set_DC_duan > 0)  //������һ��ֱ��
        { 
#endif 
            if(cRs232Num == 4 && gCmdAddrEnable1 == 1)  //���ò���ʱ
            {
                while(gEnQueue4)
                {
                    cFrmLen = sizeof(cCmdFrm);

                    //���������(ֱ��)
                    if(VQ_Enqueue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4],
                                (INT8U*)&cCmdFrm,cFrmLen,1000) == TRUE)
                    {
                        gError = 0;
                        gCmdAddrEnable11 = 1;
                    }
                    else
                    {
                        gError = 17;
                    }
                    gEnQueue4 = 0;
                    gCmdAddrEnable1 = 0;
                    CmdAddr = CmdAddrBk;

                    if(DC_duan_NUM >= (Sys_set_DC_duan-1))
                    {
                        DC_duan_NUM = 0;
                        CmdAddr++;
                    }
                    else
                    {
                        DC_duan_NUM++;
                    }
                }
            }
            else if(cRs232Num == 4 && gCmdAddrEnable1 == 0) //�������(�����ò���ʱ)
            {
                while(gEnQueue4)
                {
                    cFrmLen = sizeof(cCmdFrm);

                    //���������(ֱ��)
                    if(VQ_Enqueue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4_Auto],
                                (INT8U*)&cCmdFrm,cFrmLen,1000) == TRUE)
                    {
                        gError = 0;
                    }
                    else
                    {
                        gError = 18;
                    }
                    gCmdAddrEnable1 = 0;

                    if(DC_duan_NUM >= (Sys_set_DC_duan-1))
                    {
                        DC_duan_NUM = 0;
                        CmdAddr++;
                    }
                    else
                    {
                        DC_duan_NUM++;
                    }
                    gEnQueue4 = 0;
                }
            }
            else
            {
                if(DC_duan_NUM >= (Sys_set_DC_duan-1))
                {
                    DC_duan_NUM = 0;
                    CmdAddr++;
                }
                else
                {
                    DC_duan_NUM++;
                }
            }
#if 1   //��������桱-��ϵͳ���á�-��ֱ��ϵͳ����������ѡ��ޡ�.
        }
        else   //Sys_set_DC_duan = 0
        {
            CmdAddr = 0x00;  //����ֱ��ͨ����ȫ���ⷢ����
            cRs232Num = 0x05;
            ERR_MainDC[0] = 0x00;
            ERR_MainDC[1] = 0x00;
            ERR_MainDC[2] = 0x00;
            gDCCmd_No = 0;          //�յĴ���֡������0
            gDCCmd_No_W = 0;        //дʧ�ܵļ�����0
            gDC1Cmd_No = 0;
            gDC1Cmd_No_W = 0;
            gDC2Cmd_No = 0;
            gDC2Cmd_No_W = 0;                    
        }
#endif 

        //��ַ: 1~12
        if(Fg_SysSet_BatteryCheckMode == 0)    //�����ĵ��Ѳ��: PSMX-B 
        {
            if(CmdAddr > 14)
            {
                CmdAddr = 1;
            }          
        }
        else 
        {
            if(CmdAddr > 12)
            {
                CmdAddr = 1;
            }                
        }
    }
}
/**********************************************************************************
 *������:	ERTU_start_sio(void Ertu_Start_Sio::run())
 *��������:	������ͨ�š�ups���ݽ����߳�
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void ERTU_start_sio(ERTU_WATCHDOG 	*pcWatchDog)
{
    //pthread_detach(pthread_self());
    RS_CFG *pcRs_cfg;
    int iFd;			

    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            pcRs_cfg = (RS_CFG *)pcWatchDog->pcDevice;
            if ((iFd = openport(pcRs_cfg->byPort)) != -1) //�򿪶˿�
            {
                sio_fd = iFd;
                if (setport(iFd,                          //���ö˿�
                            pcRs_cfg->dwBaud_rate, 
                            pcRs_cfg->byByte_bit,
                            pcRs_cfg->byStop_bit,
                            pcRs_cfg->byparity) != -1)
                {
                    ERTU_Rec_AC_T_UPS(iFd,                //��������
                            &pcWatchDog->iWatch_Dog, 
                            pcWatchDog->iWatch_Time,
                            &pcWatchDog->bExit,
                            &pcWatchDog->iTaskRunType,
                            pcRs_cfg);
                }
                else
                {
                    gError = 19;
                }
            }
            else
            {
                gError = 20;
            }
        }
    }
}

//������ͨ�š�ups���ݡ���ʱ���������ݽ���
void ERTU_Rec_AC_T_UPS(int iFd, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType,
        RS_CFG *pcRs_cfg)

{
    Cmd_Res_Frame cCmdFrame;  //���ڷ��ͻ�����
    Cmd_Res_Frame cResFrame;  //���ڽ��ջ�����
    INT16U i = 0;
    INT16U m = 0;
    INT16U Num = 0x0000;
    INT16U cFrmLen;
    INT16U CrcValue;
    INT16U sum_check;
    INT16U data_bulk;
    INT8U  bCrc_Flag;
    INT16U  data_length;
    INT8U showBuf[255];
    INT16U j;
    INT8U  cPort;
    struct tm nowtime;
    time_t timep;
    struct timeval tv;
    gSetConfigEnable1 = 0;
    //int ij = 0;

#if 1  //�Ż�Ӳ�ڵ㱨������źŵ��˲�������2��1�θĳ�1��3�Σ���
    static INT8U gpio_all = 0;   //����: �Ż���ϵͳ�ܹ������Ӳ�ڵ㣬�����ܼ�ع��϶���/�������ͳ����ʾ.
    static INT8U gpio_ac = 0;
    static INT8U gpio_dc = 0;
    static INT8U gpio_comm = 0;
    static INT8U gpio_ups = 0;
    static INT8U NumOfLightAC_No = 0x00;
    static INT8U NumOfLightDC_No = 0x00;
    static INT8U NumOfLightComm_No = 0x00;
    static INT8U NumOfLightUPS_No = 0x00;
    static INT8U NumOfLight_No = 0x00;
    static INT8U NumOfLightAC_Yes = 0x00;
    static INT8U NumOfLightDC_Yes = 0x00;
    static INT8U NumOfLightComm_Yes = 0x00;
    static INT8U NumOfLightUPS_Yes = 0x00;
    static INT8U NumOfLight_Yes = 0x00;
#endif 
    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        usleep(10000);  //10ms
        bCrc_Flag = 0;
        cPort = pcRs_cfg->byPort;
        cFrmLen = sizeof(cCmdFrame);
        memset(&showBuf,0,255);

        //�ӷ������ݶ�����ȡ������������
        //������(������ͨ�š�ups����)
        if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232], 
                    (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))     //(����: ���ò���ʱ)
        {
            gSetConfigEnable1 = 0;
            if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_Auto], 
                        (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT)) //(����: �����ò���ʱ)
            {
                usleep(1);   //�����г�ʱʧ��
                continue;
            }
        }
        else
        {
            gSetConfigEnable1 = 1;  //�����гɹ�
        }
        clearport(iFd);
        gEnQueue3 = 1;

        //�ѴӶ���ȡ�����������ⷢ��
        //��������д�뺯��        
        if (writeport(iFd, (char *)&cCmdFrame, cCmdFrame.CFrmSize) == TRUE)  //�������ݳɹ�ʱ
        {
            if (cCmdFrame.addr == 0x7E)  //��������
            {
                if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x01;  //ģ��ģ��������1��(����):     7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x02;  //ģ��ģ��������2��(����):     7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x32 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x03;  //ֱ��ģ���������Ϣ1��(����): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x32 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x04;  //ֱ��ģ���������Ϣ2��(����): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x33){
                    CID1 = 0x05;  //ģ�鿪����״̬1��(����):     7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x33){
                    CID1 = 0x06;  //ģ�鿪����״̬2��(����):     7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x34){
                    CID1 = 0x07;  //�澯��Ϣ1��(����):           7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x34){
                    CID1 = 0x08;  //�澯��Ϣ2��(����):           7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
                }else{
                    CID1 = 0x00;
                }
                //debug_printf(0,",,	CID4 == %4x,,	CID1 == %4x,,	CID2 == %4x\n",CID4,CID1,CID2);
                //debug_printf(0,"send:--	CID1 == %4x----------\n",CID1);
            }

#ifdef TestDug
            if(RS232_3WriteNum <= 0xF800 && RS232_3ReadNum <= 0xF800)
            {
                RS232_3WriteNum ++;
            }
#endif

            //-----------------------------------------------------
            //��ʼ�������� 
            for(j = 0;j < 255;j++)
            {
                Uart1Buf[j] = 0x00;
            }

            memset(&cResFrame,0,sizeof(cResFrame));
            Uart1BufCnt = 0;
            cFrmLen = sizeof(cResFrame);
            //printf("rve data1 rly\n");

            //������ͨ�š�ups���ݽ��� 
            Uart1BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,cFrmLen);  
            //debug_printf(0,",,Uart1BufCnt  == %4d\n ",Uart1BufCnt);

            //--------------------------------------------------------
            //���յ����ݵĴ���
            if(Uart1BufCnt > 0)   //Uart1BufCnt�յ�֡�ĳ���
            {
                //printf("rve data 1\n");
                //set_gpio_output(GPIO_AC,0);
                //usleep(1000*500);
                //set_gpio_output(GPIO_AC,1);

                if (Uart1Buf[0] == 0x7E)   //��ʼ�ж����ܻص������ǲ�����ȷ
                {
                    //�������ݴ���    
                    for (m=0;m<Uart1BufCnt;m++)
                    {
                        if (Uart1Buf[m] == 0x0D)
                        {
                            if (m == Uart1BufCnt-1)
                            {
                                for (i=1;i<Uart1BufCnt-5;i++)
                                {
                                    Num = Num + Uart1Buf[i];
                                }
                                cResFrame.CrcChk = ~(Num) + 1;
                                sum_check = (AscToHex(Uart1Buf[Uart1BufCnt-5])<<12) | (AscToHex(Uart1Buf[Uart1BufCnt-4])<<8) | (AscToHex(Uart1Buf[Uart1BufCnt-3])<<4) | (AscToHex(Uart1Buf[Uart1BufCnt-2]));
                                data_bulk = (AscToHex(Uart1Buf[10])<<8) | (AscToHex(Uart1Buf[11])<<4) | (AscToHex(Uart1Buf[12]));

                                //debug_printf(0,",,cResFrame.CrcChk  == %4x,,data_bulk  == %4d\n ",cResFrame.CrcChk,data_bulk);
                                //for (i=0;i<Uart1BufCnt;i++){
                                //debug_printf(0,",,  Uart1Buf[%d] == %4x\n",i,Uart1Buf[i]);
                                //}
                                debug_printf(0,",,cResFrame.CrcChk  == %4x,,sum_check  == %4x,,data_bulk  == %4d,,Uart1BufCnt-18  == %4d\n ",cResFrame.CrcChk,sum_check,data_bulk,Uart1BufCnt-18);
                                //debug_printf(0,",,Uart1Buf[0]  == %4x,,Uart1Buf[Uart1BufCnt-1]  == %4x\n ",Uart1Buf[0],Uart1Buf[Uart1BufCnt-1]);

                                //����������У��ͨ�� 
                                if (cResFrame.CrcChk == sum_check && data_bulk == Uart1BufCnt-18 )
                                {	
                                    if (CID1 == 0x01 && Uart1Buf[4] == 0x31 ){
                                        data_length = 30 + 78 * Sys_cfg_info.comm_set[0].module_num;
                                        CID2 = 0x31;
                                    }else if (CID1 == 0x02 && Uart1Buf[4] == 0x32 ){
                                        data_length = 30 + 78 * Sys_cfg_info.comm_set[1].module_num;
                                        CID2 = 0x31;
                                    }else if (CID1 ==  0x03 && Uart1Buf[4] == 0x31 ){
                                        data_length = 98;
                                        CID2 = 0x31;
                                    }else if (CID1 ==  0x04 && Uart1Buf[4] == 0x32 ){
                                        data_length = 98;
                                        CID2 = 0x31;
                                    }else if (CID1 ==  0x05 && Uart1Buf[4] == 0x31 ){
                                        data_length = 22 + 10 * Sys_cfg_info.comm_set[0].module_num;	
                                        CID2 = 0x33;
                                    }else if (CID1 ==  0x06 && Uart1Buf[4] == 0x32 ){
                                        data_length = 22 + 10 * Sys_cfg_info.comm_set[1].module_num;	
                                        CID2 = 0x33;
                                    }else if (CID1 ==  0x07 && Uart1Buf[4] == 0x31 ){
                                        data_length = 22 + 40 * Sys_cfg_info.comm_set[0].module_num;
                                        CID2 = 0x34;
                                    }else if (CID1 ==  0x08 && Uart1Buf[4] == 0x32 ){
                                        data_length = 22 + 40 * Sys_cfg_info.comm_set[1].module_num;
                                        CID2 = 0x34;
                                    }
                                    if (Uart1BufCnt == data_length)
                                    {
                                        bCrc_Flag = 1;   //��������У��ͨ��
                                        data_length = 0;
                                    }
                                    //debug_printf(0,", bCrc_Flag = %d,,CID1 == %4d,,CID2 == %4d\n ",bCrc_Flag,CID1,CID2);
                                    //debug_printf(0,",,data_length == %4d,,Uart1BufCnt == %4d,,Uart1Buf[4] == %4x\n ",data_length,Uart1BufCnt,Uart1Buf[4]);
                                    CID1 = 0x00;
                                }
                                else 
                                {
                                    bCrc_Flag = 0;	
                                }
                                Num = 0x0000;
                            }
                            else
                            {
                                bCrc_Flag = 0;	
                            }
                        }
                        else
                        {
                            bCrc_Flag = 0;	
                        }
                    }
                }
                else 
                {
                    if(Uart1BufCnt == 11 && Uart1Buf[0] == 0x55    //��ʱ
                            && Uart1Buf[1] == 0x03 && Uart1Buf[2] == 0x06)
                    {
                        nowtime.tm_hour = Uart1Buf[3];
                        nowtime.tm_min = Uart1Buf[4];
                        nowtime.tm_sec = Uart1Buf[5];
                        nowtime.tm_mday = Uart1Buf[6];
                        nowtime.tm_mon = Uart1Buf[7]-1;           //�£������ʾ�����Ǵ�0��ʼ�ģ�B���ʱ�����Ǵ�1��ʼ�ģ����Լ�1����ز�����ȷ��ʾʱ��
                        nowtime.tm_year = Uart1Buf[8]+2000-1900;  //�꣬B���ʱ��������17����18�Ǽ�ȥ2000�ģ������ʾʱ����1900�꿪ʼ�ģ�����17��2000��1900�������ʾʱ���Լ��ټ���1900��������ʾ
                        nowtime.tm_isdst = -1;
                        timep = mktime(&nowtime);
                        tv.tv_sec = timep;
                        //tv.tv_sec = tv.tv_sec -1;
                        //tv.tv_usec = (bgCfg_UPS*1000);
                        if(settimeofday(&tv,(struct timezone*)0)<0)
                        {
                            printf("Set time of nowtime is error\n");
                        }
                        system ("hwclock -w");
                        HalfSecondCnt1 = 0;
                    }
                    else  //RS485-3���ݽ��մ���(�������ܺͶ�ʱ)
                    {
                        cResFrame.CFrmSize = Uart1BufCnt;
                        cResFrame.addr = Uart1Buf[0];
                        cResFrame.len = Uart1Buf[1];
                        for(j = 2; j< Uart1BufCnt; j++)
                        {
                            cResFrame.data[j-2] = Uart1Buf[j];    //���ݵ�һλ������Э��ĵ�3λ��0�ǵ�һλ��2�ǵ���λ
                        }
#ifdef Dug
                        if(cCmdFrame.addr == 0x10 )
                        {
                            mem_show1((U_BYTE *)showBuf,(INT8U *)&Uart1Buf,Uart1BufCnt,"RS232 receive the data2222:");
                            debug_printf(0,"%2s\n",showBuf);
                            //     debug_printf(0,"Uart1Buf[50] ========= %d\n",Uart1Buf[50]);
                        }
#endif
                        CrcValue = Load_crc(Uart1BufCnt-2,(INT8U *)&Uart1Buf);   //����Load_crc()���������������ݵ�CRC
                        cResFrame.CrcChk = (Uart1Buf[Uart1BufCnt-2])|((Uart1Buf[Uart1BufCnt-1]<<8)& 0xFF00);  //���ݴ���������CRC
#ifdef Dug
                        debug_printf(0,"addr==%x,, CrcValue == %4x ,,  cResFrame.CrcChk == %4x\n",Uart1Buf[0],CrcValue,cResFrame.CrcChk);
#endif
                        if(cResFrame.CrcChk == CrcValue)   //�Ƚ�CRC�Ƿ�һ�£�һ��˵��������������������ȷ��
                        {
                            bCrc_Flag = 1;  //RS485-3���ݽ��մ���(�������ܺͶ�ʱ)У��ͨ��
                        }
                        else
                        {
                            bCrc_Flag = 0;
                        }
                    }
                }
            }

            if (bCrc_Flag == 1)   //���ݼ���ɹ�
            {
                if(cResFrame.addr==0x30)
                {
                    gComPWCmd_No = 0;       //�յĴ���֡������0
                    gComPWCmd_No_W = 0;     //дʧ�ܵļ�����0
                    ERR_MainComPw= 0;       //������0
                }
                else if(cResFrame.addr==0x31)
                {
                    gComPW1Cmd_No = 0;
                    gComPW1Cmd_No_W = 0;
                    ERR_MainComPw1= 0;
                }
                else if(cResFrame.addr==0x40)
                {
                    gUPSCmd_No = 0;
                    gUPSCmd_No_W = 0;
                    ERR_MainUPS= 0;
                }
                else if(cResFrame.addr==0x80)
                {
                    gIsuCmd_No = 0;
                    gIsuCmd_No_W = 0;
                    ERR_MainIsu= 0;
                }
                else if(cResFrame.addr==0x81)
                {
                    gIsu1Cmd_No = 0;
                    gIsu1Cmd_No_W = 0;
                    ERR_MainIsu1= 0;
                }
                else if(cResFrame.addr==0x82)
                {
                    gIsu2Cmd_No[0] = 0;
                    gIsu2Cmd_No_W[0] = 0;
                    ERR_MainIsu2[0]= 0;
                }
                else if(cResFrame.addr==0x83)
                {
                    gIsu2Cmd_No[1] = 0;
                    gIsu2Cmd_No_W[1] = 0;
                    ERR_MainIsu2[1]= 0;
                }
                else if(cResFrame.addr==0x84)
                {
                    gIsu2Cmd_No[2] = 0;
                    gIsu2Cmd_No_W[2] = 0;
                    ERR_MainIsu2[2]= 0;
                }
                else if(cResFrame.addr==0x85)
                {
                    gIsu2Cmd_No[3] = 0;
                    gIsu2Cmd_No_W[3] = 0;
                    ERR_MainIsu2[3]= 0;
                }
                else if(cResFrame.addr==bgCfg_Ats1Addr)   //1#ATS����
                {
                    gATSCmd_No = 0;        //�յĴ���֡������0
                    gATSCmd_No_W = 0;      //дʧ�ܵļ�����0
                    ERR_MainATS= 0;        //������0
                }
                else if(cResFrame.addr == bgCfg_Ats2Addr)  //2#ATS����
                {
                    gATS1Cmd_No = 0;
                    gATS1Cmd_No_W = 0;
                    ERR_MainATS1= 0;
                }
                else if(cResFrame.addr == 0x10)  //����
                {
                    gACCmd_No = 0;
                    gACCmd_No_W = 0;
                    ERR_MainAC = 0;
                }else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x31)
                {
                    gComPWCmd_No = 0;       //�յĴ���֡������0
                    gComPWCmd_No_W = 0;     //дʧ�ܵļ�����0
                    ERR_MainComPw= 0;       //������0
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x32)
                {
                    gComPW1Cmd_No = 0;
                    gComPW1Cmd_No_W = 0;
                    ERR_MainComPw1= 0;
                }

                //--------------------------------------------------------
                /*******��������������͸���Ӧ����Ҫ��ʾ�Ĳ���***********/
                if (CID2 ==  0x31){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x31); //������������-01 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }else if (CID2 ==  0x33){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x33); //������������-02 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }else if (CID2 ==  0x34){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x34); //������������-03 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }
                else
                {
                    receive_data(Uart1Buf[0],Uart1Buf[1],0);    //���ս�����ͨ�š�ups����
                }   
            }
            else  //���ݼ���ʧ��
            {
                if(cCmdFrame.addr==0x30 && gComPWCmd_No <= CMDRPT)       //8��
                {
                    gComPWCmd_No ++;
                }
                else if(cCmdFrame.addr==0x31 && gComPW1Cmd_No <= CMDRPT)
                {
                    gComPW1Cmd_No ++;
                }
                else if(cCmdFrame.addr==0x40 && gUPSCmd_No <= CMDRPT)
                {
                    gUPSCmd_No ++;
                }
                else if(cCmdFrame.addr==0x80 && gIsuCmd_No <= CMDRPT)
                {
                    gIsuCmd_No ++;
                }
                else if(cCmdFrame.addr==0x81 && gIsu1Cmd_No <= CMDRPT)
                {
                    gIsu1Cmd_No ++;
                }
                else if(cCmdFrame.addr==0x82 && gIsu2Cmd_No[0] <= CMDRPT)
                {
                    gIsu2Cmd_No[0] ++;
                }
                else if(cCmdFrame.addr==0x83 && gIsu2Cmd_No[1] <= CMDRPT)
                {
                    gIsu2Cmd_No[1] ++;
                }
                else if(cCmdFrame.addr==0x84 && gIsu2Cmd_No[2] <= CMDRPT)
                {
                    gIsu2Cmd_No[2] ++;
                }
                else if(cCmdFrame.addr==0x85 && gIsu2Cmd_No[3] <= CMDRPT)
                {
                    gIsu2Cmd_No[3] ++;
                }
                else if(cCmdFrame.addr==bgCfg_Ats1Addr && gATSCmd_No <= CMDRPT)
                {
                    gATSCmd_No ++;  //�صı����д�1#ATS�����־λ��1
                }
                else if(cCmdFrame.addr == bgCfg_Ats2Addr && gATS1Cmd_No <= CMDRPT)
                {
                    gATS1Cmd_No ++;   //�صı����д�2#ATS�����־λ��1
                }
                else if(cCmdFrame.addr == 0x10 && gACCmd_No <= CMDRPT)
                {
                    gACCmd_No ++;     //��������ص�Ԫ����
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x31 && gComPWCmd_No <= CMDRPT)  //8��
                {
                    gComPWCmd_No ++;
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x32 && gComPW1Cmd_No <= CMDRPT)
                {
                    gComPW1Cmd_No ++;
                }
                if (Uart1BufCnt == 0){
                    if(cCmdFrame.data[1]==0x30  && cCmdFrame.data[2]==0x31 && gComPWCmd_No <= CMDRPT)       //8��
                    {
                        gComPWCmd_No ++;
                    }else if(cCmdFrame.data[1]==0x30  && cCmdFrame.data[2]==0x32 && gComPW1Cmd_No <= CMDRPT)
                    {
                        gComPW1Cmd_No ++;
                    }
                }
                gError = 21;
            }
        }
        else  //���ⷢ������ʧ��ʱ
        {
            if(cCmdFrame.addr==0x30 && gComPWCmd_No_W <= CMDRPTW)
            {
                gComPWCmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x31 && gComPW1Cmd_No_W <= CMDRPTW)
            {
                gComPW1Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x40 && gUPSCmd_No_W<= CMDRPTW)
            {
                gUPSCmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x80 && gIsuCmd_No_W<= CMDRPTW)
            {
                gIsuCmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x81 && gIsu1Cmd_No_W<= CMDRPTW)
            {
                gIsu1Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x82 && gIsu2Cmd_No_W[0]<= CMDRPTW)
            {
                gIsu2Cmd_No_W[0] ++;
            }
            else if(cCmdFrame.addr==0x83 && gIsu2Cmd_No_W[1]<= CMDRPTW)
            {
                gIsu2Cmd_No_W[1] ++;
            }
            else if(cCmdFrame.addr==0x84 && gIsu2Cmd_No_W[2]<= CMDRPTW)
            {
                gIsu2Cmd_No_W[2] ++;
            }
            else if(cCmdFrame.addr==0x85 && gIsu2Cmd_No_W[3]<= CMDRPTW)
            {
                gIsu2Cmd_No_W[3] ++;
            }
            else if(cCmdFrame.addr==bgCfg_Ats1Addr && gATSCmd_No_W<= CMDRPTW)
            {
                gATSCmd_No_W ++;   //��ʧ�ܣ�1#ATS��ͨ�Ź��ϼ�1
            }
            else if(cCmdFrame.addr == bgCfg_Ats2Addr && gATS1Cmd_No_W<= CMDRPTW)
            {
                gATS1Cmd_No_W++;
            }
            else if(cCmdFrame.addr == 0x10 && gACCmd_No_W <= CMDRPTW)
            {
                gACCmd_No_W ++;
            }
            else if(cCmdFrame.data[1]==0x30  && cCmdFrame.data[2]==0x31 && gComPWCmd_No_W <= CMDRPTW)   
            {
                gComPWCmd_No_W ++;
                //debug_printf(0,",,gComPWCmd_No_W  == %4x\n ",gComPWCmd_No_W);
            }
            else if(cCmdFrame.data[1]==0x30  && cCmdFrame.data[2]==0x32 && gComPW1Cmd_No_W <= CMDRPTW)   
            {
                gComPW1Cmd_No_W ++;
                //debug_printf(0,",,gComPW1Cmd_No_W  == %4x\n ",gComPW1Cmd_No_W);
            }
            gError = 21;
        }

        //����ͨѶ�Ƿ��ж�
        if(gACCmd_No > CMDRPT || gACCmd_No_W > CMDRPTW)
        {
            ERR_MainAC = 1;
        }
        else
        {
            ERR_MainAC = 0;
        }
        if(gUPSCmd_No> CMDRPT ||gUPSCmd_No_W> CMDRPTW) 
        {
#if 0
            if(Special_35KV_flag != 2)
                ERR_MainUPS= 1; 
#else       //������������ͨ�š�UPS��Դ��ص�ѡ����.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoUpsMon_WithCommMon != 2))
            {
                //����UPS���ʱ
                ERR_MainUPS= 1; 
            }           
#endif 
        }
        else 
        {
            ERR_MainUPS= 0;
        }
        if(gComPWCmd_No> CMDRPT||gComPWCmd_No_W> CMDRPTW)
        {
#if 0
            if(Special_35KV_flag != 2)
                ERR_MainComPw= 1; 
#else       //������������ͨ�š�UPS��Դ��ص�ѡ����.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoCommMon_WithUpsMon != 2))
            {
                //����ͨ�ż��ʱ
                ERR_MainComPw= 1;
            }           
#endif 
        }
        else 
        {
            ERR_MainComPw= 0;
        }
        if(gComPW1Cmd_No> CMDRPT||gComPW1Cmd_No_W> CMDRPTW)
        {
#if 0
            if(Special_35KV_flag != 2)
                ERR_MainComPw1= 1; 
#else       //������������ͨ�š�UPS��Դ��ص�ѡ����.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoCommMon_WithUpsMon != 2))
            {
                //����ͨ�ż��ʱ
                ERR_MainComPw1= 1;
            }          
#endif 
        }
        else 
        {
            ERR_MainComPw1= 0;
        }
        /*if(gComPWCmd_No> CMDRPT||gComPWCmd_No_W> CMDRPTW)
          {
          ERR_MainComPw= 1; 
          }
          else 
          {
          ERR_MainComPw= 0;
          }
          if(gComPW1Cmd_No> CMDRPT||gComPW1Cmd_No_W> CMDRPTW)
          {
          ERR_MainComPw1= 1; 
          }
          else 
          {
          ERR_MainComPw1= 0;
          }*/
        if(gIsuCmd_No> CMDRPT || gIsuCmd_No_W > CMDRPTW) 
        {
            ERR_MainIsu= 1;
        }
        else
        {
            ERR_MainIsu= 0;
        }
        if(gIsu1Cmd_No> CMDRPT || gIsu1Cmd_No_W > CMDRPTW)
        {
            ERR_MainIsu1= 1;
        }
        else 
        {
            ERR_MainIsu1= 0;
        }
        if(gIsu2Cmd_No[0]> CMDRPT || gIsu2Cmd_No_W[0] > CMDRPTW)
        {
            ERR_MainIsu2[0]= 1;   
        }
        else 
        {
            ERR_MainIsu2[0]= 0;
        }
        if(gIsu2Cmd_No[1]> CMDRPT || gIsu2Cmd_No_W[1] > CMDRPTW)
        {
            ERR_MainIsu2[1]= 1;   
        }
        else 
        {
            ERR_MainIsu2[1]= 0;
        }
        if(gIsu2Cmd_No[2]> CMDRPT || gIsu2Cmd_No_W[2] > CMDRPTW)
        {
            ERR_MainIsu2[2]= 1;   
        }
        else 
        {
            ERR_MainIsu2[2]= 0;
        }
        if(gIsu2Cmd_No[3]> CMDRPT || gIsu2Cmd_No_W[3] > CMDRPTW)
        {
            ERR_MainIsu2[3]= 1;   
        }
        else 
        {
            ERR_MainIsu2[3]= 0;
        }
        if(gATSCmd_No> CMDRPT || gATSCmd_No_W> CMDRPTW)     
        {
            ERR_MainATS= 1;
        }
        else
        {
            ERR_MainATS= 0;
        }
        if(gATS1Cmd_No> CMDRPT || gATS1Cmd_No_W> CMDRPTW) 
        {
            ERR_MainATS1= 1;
        }
        else 
        {
            ERR_MainATS1= 0;
        }
        if((gCmdDoFin == 0) && (gSetConfigEnable1 == 1))
        {
            gCmdDoFin = 1;
            gCmdRight = bCrc_Flag;
            gCmdAddrEnable11 = 0;
            gSetConfigEnable1 = 0;
        }

#if 1   //�Ż�Ӳ�ڵ㱨������źŵ��˲�������2��1�θĳ�1��3�Σ���
        //����3���˲����� 
        if (Light_AC)
        {
            NumOfLightAC_No = 0x00;
            if (NumOfLightAC_Yes < NUM_ERROR_FILTER)
            {
                NumOfLightAC_Yes ++;
            }
            else  //�澯���� 
            {
                if (!gpio_ac)
                {
                    gpio_ac = 0x01;
                    set_gpio_output(GPIO_AC,0); 
                }                               
            }
        }
        else
        {
            NumOfLightAC_Yes = 0x00;
            if (NumOfLightAC_No < NUM_ERROR_FILTER)
            {
                NumOfLightAC_No ++;
            }
            else  //�澯����
            {
                if (gpio_ac)
                {
                    gpio_ac = 0x00;
                    set_gpio_output(GPIO_AC,1); 
                }                                   
            }
        }

        if (Light_DC)
        {
            NumOfLightDC_No = 0x00;
            if (NumOfLightDC_Yes < NUM_ERROR_FILTER)
            {
                NumOfLightDC_Yes ++;
            }
            else  //�澯���� 
            {
                if (!gpio_dc)
                {
                    gpio_dc = 0x01;
                    set_gpio_output(GPIO_DC,0); 
                }                               
            }
        }
        else
        {
            NumOfLightDC_Yes = 0x00;
            if (NumOfLightDC_No < NUM_ERROR_FILTER)
            {
                NumOfLightDC_No ++;
            }
            else  //�澯����
            {
                if (gpio_dc)
                {
                    gpio_dc = 0x00;
                    set_gpio_output(GPIO_DC,1); 
                }                                   
            }
        }

        if (Light_Comm)
        {
            NumOfLightComm_No = 0x00;
            if (NumOfLightComm_Yes < NUM_ERROR_FILTER)
            {
                NumOfLightComm_Yes ++;
            }
            else  //�澯���� 
            {
                if (!gpio_comm)
                {
                    gpio_comm = 0x01;
                    set_gpio_output(GPIO_COMM,0); 
                }                               
            }
        }
        else
        {
            NumOfLightComm_Yes = 0x00;
            if (NumOfLightComm_No < NUM_ERROR_FILTER)
            {
                NumOfLightComm_No ++;
            }
            else  //�澯����
            {
                if (gpio_comm)
                {
                    gpio_comm = 0x00;
                    set_gpio_output(GPIO_COMM,1); 
                }                                   
            }
        }

        if (Light_UPS)
        {
            NumOfLightUPS_No = 0x00;
            if (NumOfLightUPS_Yes < NUM_ERROR_FILTER)
            {
                NumOfLightUPS_Yes ++;
            }
            else  //�澯���� 
            {
                if (!gpio_ups)
                {
                    gpio_ups = 0x01;
                    set_gpio_output(GPIO_UPS,0); 
                }                               
            }
        }
        else
        {
            NumOfLightUPS_Yes = 0x00;
            if (NumOfLightUPS_No < NUM_ERROR_FILTER)
            {
                NumOfLightUPS_No ++;
            }
            else  //�澯����
            {
                if (gpio_ups)
                {
                    gpio_ups = 0x00;
                    set_gpio_output(GPIO_UPS,1); 
                }                                   
            }
        }

        if (Light_AC || Light_DC || Light_Comm || Light_UPS || Light_Main)
        {
            NumOfLight_No = 0x00;
            if (NumOfLight_Yes < NUM_ERROR_FILTER)
            {
                NumOfLight_Yes ++;
            }
            else  //�澯���� 
            {
#if 0
                if (J4_GPIO_ALL != 2)
                    set_gpio_output(GPIO_ALL,1);
                else
                    set_gpio_output(GPIO_Zong,1);  //��J4�����ܹ��ϡ�������
#else    //�Ż���ϵͳ�ܹ������Ӳ�ڵ㣬�����ܼ�ع��϶���/�������ͳ����ʾ.
                if (!gpio_all)
                {
                    gpio_all = 0x01;
                    if (J4_GPIO_ALL != 2)
                        set_gpio_output(GPIO_ALL,1);
                    else
                        set_gpio_output(GPIO_Zong,1);  //��J4�����ܹ��ϡ�������
                    if (NumOfGPIOAll_Error_On < 1000)
                        NumOfGPIOAll_Error_On ++;
                    //printf("GPIO_ALL--Error---On.NumOfGPIOAll_Error_On: %d.\n",NumOfGPIOAll_Error_On);  
                }
#endif 
                if (Sys_cfg_info.sys_set.sound)
                    Beep_ctl(0,1000*1000);
            }
        }
        else
        {
            NumOfLight_Yes = 0x00;
            if (NumOfLight_No < NUM_ERROR_FILTER)
            {
                NumOfLight_No ++;
            }
            else  //�澯����
            {
#if 0
                if (J4_GPIO_ALL != 2)
                    set_gpio_output(GPIO_ALL,0);
                else
                    set_gpio_output(GPIO_Zong,0);
#else //�Ż���ϵͳ�ܹ������Ӳ�ڵ㣬�����ܼ�ع��϶���/�������ͳ����ʾ.
                if (gpio_all)
                {
                    gpio_all = 0x00;

                    if (J4_GPIO_ALL != 2)
                        set_gpio_output(GPIO_ALL,0);
                    else
                        set_gpio_output(GPIO_Zong,0);

                    if (NumOfGPIOAll_OK_Off < 1000)
                        NumOfGPIOAll_OK_Off ++;
                    //printf("GPIO_ALL--OK---Off.NumOfGPIOAll_OK_Off: %d.\n",NumOfGPIOAll_OK_Off); 
                }
#endif               
            }
        }
#endif 
    }
    close(iFd);
}
/**********************************************************************************
 *������:	ERTU_start_sio1(void Ertu_Start_Sio1::run())
 *��������:	ֱ�����ݽ����߳�
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void ERTU_start_sio1(ERTU_WATCHDOG 	*pcWatchDog)
{
    //pthread_detach(pthread_self());
    RS_CFG *pcRs_cfg;
    int iFd;			

    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            pcRs_cfg = (RS_CFG *)pcWatchDog->pcDevice;
            if ( (iFd = openport(pcRs_cfg->byPort)) != -1 )
            {
                sio_fd_1 = iFd;
                if (setport(iFd,
                            pcRs_cfg->dwBaud_rate, 
                            pcRs_cfg->byByte_bit,
                            pcRs_cfg->byStop_bit,
                            pcRs_cfg->byparity) != -1)
                {
                    ERTU_Rec_DC(iFd, 
                            &pcWatchDog->iWatch_Dog, 
                            pcWatchDog->iWatch_Time,
                            &pcWatchDog->bExit,
                            &pcWatchDog->iTaskRunType,
                            pcRs_cfg);
                }
                else
                {
                    gError = 22;
                }
            }
            else
            {
                gError = 23;
            }
        }
    }
}

//ֱ�����ݽ���
void ERTU_Rec_DC(int iFd, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType,
        RS_CFG *pcRs_cfg)
{

    Cmd_Res_Frame cCmdFrame;  //���ڷ��ͻ�����
    Cmd_Res_Frame cResFrame;  //���ڽ��ջ�����
    INT8U  cFrmLen;
    INT16U CrcValue;
    INT8U  bCrc_Flag;
    INT8U showBuf[2048];
    INT16U j;
    INT8U  cPort;
#if 0  //�Ż�Ӳ�ڵ㱨������źŵ��˲�������2��1�θĳ�1��3�Σ���
    static INT8U gpio_ac = 0;
    static INT8U gpio_dc = 0;
    static INT8U gpio_comm = 0;
    static INT8U gpio_ups = 0;
#endif 

    //int fd,len;
    //INT8U ChkSum = 0;
    //INT8U str[CFGLEN];  
    //int ij = 0;

    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;  

#if 1   //��������
        sleep(1); //Ϊ�˱�֤ͨ�ųɹ��ʣ�ִ����������500ms��
#else   //���ڲ��Ա��ģ�ÿ5�뷢��1�α��ģ�
        sleep(5);         
#endif 

        //usleep(1000);
        bCrc_Flag = 0;
        cPort = pcRs_cfg->byPort;
        cFrmLen = sizeof(cCmdFrame);
        memset(&showBuf,0,255);

        //������(ֱ��)
        if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4], //(����: ���ò���ʱ)
                    (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))   
        {
            if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4_Auto], 
                        (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))  //(����: �����ò���ʱ)
            {
                usleep(1);
                continue;
            }
        }
        else
        {
            ;
        }
        clearport(iFd);
        gEnQueue4 = 1;
#ifdef Dug
        if(cCmdFrame.addr == 0x20)
        {
            mem_show1((U_BYTE *)showBuf,(INT8U *)&cCmdFrame,cCmdFrame.CFrmSize,"Send the command44:");
            debug_printf(0,"%2s\n",showBuf);
        }
#endif
        if (writeport(iFd, (char *)&cCmdFrame, cCmdFrame.CFrmSize) == TRUE)    //д����ʧ��   writeport��������д�뺯��
        {
#ifdef   TEST_DC_SEND_NUM    //just for test      
            if (cCmdFrame.addr == 0x20)
            {
                if (DC_SEND_NUM_0x20 < 0xFFFF)
                {
                    DC_SEND_NUM_0x20 ++;
                }
                printf("DC_SEND_NUM_0x20: %d, cCmdFrm.addr: %d.\n", DC_SEND_NUM_0x20, cCmdFrame.addr);
                printf("gDCCmd_No: %d, gDCCmd_No_W: %d, ERR_MainDC[0]: %d.\n",
                        gDCCmd_No, gDCCmd_No_W, ERR_MainDC[0]);    
            }

            if (cCmdFrame.addr == 0x28)
            {
                if (DC_SEND_NUM_0x28 < 0xFFFF)
                {
                    DC_SEND_NUM_0x28 ++;
                }
                printf("DC_SEND_NUM_0x28: %d, cCmdFrm.addr: %d.\n", DC_SEND_NUM_0x28, cCmdFrame.addr);
                printf("gDC1Cmd_No: %d, gDC1Cmd_No_W: %d, ERR_MainDC[1]: %d.\n",
                        gDC1Cmd_No, gDC1Cmd_No_W, ERR_MainDC[1]);    
            }
#endif 
#ifdef TestDug
            if(RS232_4WriteNum <= 0xF800 && RS232_4ReadNum <= 0xF800)
            {
                RS232_4WriteNum ++;
            }
#endif
            for(j = 0;j < 255;j++)
            {
                Uart4Buf[j] = 0x00;
            }
            memset(&cResFrame,0,sizeof(cResFrame));
            Uart4BufCnt = 0;
            cFrmLen = sizeof(cResFrame);
            Uart4BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,cFrmLen); //ֱ��
            if(Uart4BufCnt > 0)
            {
                cResFrame.CFrmSize = Uart4BufCnt;
                cResFrame.addr = Uart4Buf[0];
                cResFrame.len = Uart4Buf[1];
                //debug_printf(0,"cResFrame.CFrmSize == %4d ,,cResFrame.addr == %4x ,,  cResFrame.len == %4x\n",Uart4BufCnt,cResFrame.addr,cResFrame.len);
                for(j = 2; j< Uart4BufCnt;j++)
                {
                    cResFrame.data[j-2] = Uart4Buf[j];
                }
#ifdef Dug
                mem_show1((U_BYTE *)showBuf,(INT8U *)&Uart4Buf,Uart4BufCnt,"RS232 receive the data44444444:");
                debug_printf(0,"%2s\n",showBuf);
#endif
                CrcValue = Load_crc(Uart4BufCnt-2,(INT8U *)&Uart4Buf);       //����CRC
                cResFrame.CrcChk = (Uart4Buf[Uart4BufCnt-2])|((Uart4Buf[Uart4BufCnt-1]<<8)& 0xFF00);   //�õ�CRC��֡�������һ���ֽڣ�����8λ��������1111 1111 0000 000���ٻ������ڶ����ֽ�
#ifdef Dug
                debug_printf(0,"CrcValue4 == %4x ,,  CrcChk4 == %4x\n",CrcValue,cResFrame.CrcChk);
#endif
                if(cResFrame.CrcChk == CrcValue)
                {
                    bCrc_Flag = 1;
                }
                else
                {
                    bCrc_Flag = 0;
                }
            }
            if (bCrc_Flag == 1)
            {
                if(cResFrame.addr==0x20)
                {
                    gDCCmd_No = 0;          //�յĴ���֡������0
                    gDCCmd_No_W = 0;        //дʧ�ܵļ�����0
                    ERR_MainDC[0] = 0;      //������0
                }
                else if(cResFrame.addr==0x28)
                {
                    gDC1Cmd_No = 0;
                    gDC1Cmd_No_W = 0;
                    ERR_MainDC[1] = 0;
                }
                //else if(cResFrame.addr==0x25 && cResFrame.len ==0x39 )
                else if(cResFrame.addr==0x25 && 
                        (cResFrame.len ==0x39 || cResFrame.len ==0x72))
                {
                    gDC2Cmd_No = 0;
                    gDC2Cmd_No_W = 0;
                    ERR_MainDC[2] = 0;
                }
                else if(cResFrame.addr == 0x01)
                {
                    gPwrCmd_No = 0;
                    gPwrCmd_No_W = 0;
                    ERR_MainPWR = 0;
                }
                else if(cResFrame.addr == 0x91)
                {
                    gPwr1Cmd_No = 0;
                    gPwr1Cmd_No_W = 0;
                    ERR_MainPWR1 = 0;
                }
                else if(cResFrame.addr==0x81 && cResFrame.len==0x60 )
                {
                    gDCTX1Cmd_No = 0;
                    gDCTX1Cmd_No_W = 0;
                    ERR_MainDCTX[0] = 0;
                    //debug_printf(0,"ERR_MainDCTX[0] == %4d \n",ERR_MainDCTX[0]);

                }
                else if(cResFrame.addr==0x81 && cResFrame.len==0x61)
                {
                    gDCTX2Cmd_No = 0;
                    gDCTX2Cmd_No_W = 0;
                    ERR_MainDCTX[1] = 0;
                    //debug_printf(0,"ERR_MainDCTX[1] == %4d \n",ERR_MainDCTX[1]);

                }
                else if(cResFrame.addr==0x81 && cResFrame.len==0x62)
                {
                    gDCTX3Cmd_No = 0;
                    gDCTX3Cmd_No_W = 0;
                    ERR_MainDCTX[2] = 0;
                }
                else if(cResFrame.addr==0x81 && cResFrame.len==0x63)
                {
                    gDCTX4Cmd_No = 0;
                    gDCTX4Cmd_No_W = 0;
                    ERR_MainDCTX[3] = 0;
                }

                if (Fg_SysSet_BatteryCheckMode == 0)  //�����ĵ��Ѳ��: PSMX-B 
                {
                    for (j = 0; j < 3; j++)
                    {
                        if (cResFrame.addr == (0xB0 + j) && cResFrame.len == 0x03)
                        {
                            gDCTX_PSMXB_No[j] = 0x00;
                            gDCTX_PSMXB_No_W[j] = 0x00;
                            ERR_MainDC_PSMX_B_Comm[j] = 0x00;
                        }                
                    }                       
                }

                /**************��������������͸���Ӧ����Ҫ��ʾ�Ĳ���******************/
#ifdef TestDug
                if(RS232_4WriteNum <= 0xF800 && RS232_4ReadNum <= 0xF800)
                {
                    RS232_4ReadNum++;
                }
#endif
                //MoveData_RS2324ToGlobalData(Uart4Buf[0],Uart4Buf[1]);

#if 0
                receive_data(Uart4Buf[0],Uart4Buf[1],0);
#else 
                if ((Fg_SysSet_BatteryCheckMode == 0)  //�����ĵ��Ѳ��: PSMX-B 
                        && ((Uart4Buf[0] == 0xB0) 
                            || (Uart4Buf[0] == 0xB1)
                            || (Uart4Buf[0] == 0xB2)))
                {
                    receive_data(Uart4Buf[0],Uart4Buf[2],Uart4Buf[1]);    
                }
                else 
                {
                    receive_data(Uart4Buf[0],Uart4Buf[1],0);    
                }                
#endif 
            }
            else
            {
                //                while(gThread4 == 0) {;}
                if(cCmdFrame.addr==0x20 && gDCCmd_No <= CMDRPT)
                {
                    gDCCmd_No ++;
                }
                else if(cCmdFrame.addr==0x28 && gDC1Cmd_No <= CMDRPT)
                {
                    gDC1Cmd_No ++;
                }
                else if(cCmdFrame.addr==0x25 && cCmdFrame.len==0x19 && gDC2Cmd_No <= CMDRPT)
                    //else if(cCmdFrame.addr==0x25 && gDC2Cmd_No <= CMDRPT)
                {
                    gDC2Cmd_No ++;
                }
                else if(cCmdFrame.addr==0x01 && gPwrCmd_No <= CMDRPT)
                {
                    gPwrCmd_No ++;
                }
                else if(cCmdFrame.addr==0x91 && gPwr1Cmd_No <= CMDRPT)
                {
                    gPwr1Cmd_No ++;
                } 
                else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x60 && gDCTX1Cmd_No <= CMDRPT)
                {
                    gDCTX1Cmd_No ++;
                    //debug_printf(0,"gDCTX1Cmd_No == %4d\n",gDCTX1Cmd_No);
                }
                else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x61 && gDCTX2Cmd_No <= CMDRPT)
                {
                    gDCTX2Cmd_No ++;

                }
                else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x62 && gDCTX3Cmd_No <= CMDRPT)
                {
                    gDCTX3Cmd_No ++;

                }
                else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x63 && gDCTX4Cmd_No <= CMDRPT)
                {
                    gDCTX4Cmd_No ++;
                }

                if (Fg_SysSet_BatteryCheckMode == 0)  //�����ĵ��Ѳ��: PSMX-B 
                {
                    for (j = 0; j < 3; j ++)
                    {
                        //��psmx-bͨѶ�жϴ������ձ��8���ټ�4��.
                        if (cCmdFrame.addr == (0xB0 + j) 
                                && gDCTX_PSMXB_No[j] <= (CMDRPT + CMDRPT_XB))
                        {
                            gDCTX_PSMXB_No[j] ++;
                        }                
                    }                       
                }

                gError = 21;
            }
        }
        else
        {
            if(cCmdFrame.addr==0x20 && gDCCmd_No_W <= CMDRPTW)
            {
                gDCCmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x28 && gDC1Cmd_No_W<= CMDRPTW)
            {
                gDC1Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x25 &&  cCmdFrame.len==0x19 && gDC2Cmd_No_W<= CMDRPTW)
                //else if(cCmdFrame.addr==0x25 && gDC2Cmd_No_W<= CMDRPTW)
            {
                gDC2Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x01 && gPwrCmd_No_W<= CMDRPTW)
            {
                gPwrCmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x91 && gPwr1Cmd_No_W<= CMDRPTW)
            {
                gPwr1Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x60 && gDCTX1Cmd_No_W <= CMDRPTW)
            {
                gDCTX1Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x61 && gDCTX2Cmd_No_W<= CMDRPTW)
            {
                gDCTX2Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x62 && gDCTX3Cmd_No_W<= CMDRPTW)
            {
                gDCTX3Cmd_No_W ++;
            }
            else if(cCmdFrame.addr==0x7E && cCmdFrame.len==0x63 && gDCTX4Cmd_No_W<= CMDRPTW)
            {
                gDCTX4Cmd_No_W ++;
            }

            if (Fg_SysSet_BatteryCheckMode == 0)  //�����ĵ��Ѳ��: PSMX-B 
            {
                for (j = 0; j < 3; j ++)
                {
                    //��psmx-bͨѶ�жϴ������ձ��8���ټ�4��.
                    if (cCmdFrame.addr == (0xB0 + j) 
                            && gDCTX_PSMXB_No_W[j] <= (CMDRPT + CMDRPT_XB))
                    {
                        gDCTX_PSMXB_No_W[j] ++;
                    }                
                }                       
            }

            gError = 21;
        }
        if(gDCCmd_No > CMDRPT || gDCCmd_No_W > CMDRPTW)
        {
            ERR_MainDC[0] = 1;
        }
        else 
        {
            ERR_MainDC[0] = 0;
        }
        if(gDC1Cmd_No > CMDRPT || gDC1Cmd_No_W > CMDRPTW)      
        {
            ERR_MainDC[1] = 1; 
        }
        else 
        {
            ERR_MainDC[1] = 0;
        }

        if(gDC2Cmd_No > CMDRPT || gDC2Cmd_No_W > CMDRPTW)      
        {
            ERR_MainDC[2] = 1; 
        }
        else 
        {
            ERR_MainDC[2] = 0;
        }

        if(gPwrCmd_No > CMDRPT || gPwrCmd_No_W > CMDRPTW)
        {
            ERR_MainPWR= 1;  
        }
        else 
        {
            ERR_MainPWR= 0;
        }
        if(gPwr1Cmd_No > CMDRPT || gPwr1Cmd_No_W > CMDRPTW)
        {
            ERR_MainPWR1= 1;  
        }
        else 
        {
            ERR_MainPWR1= 0;
        }
        if(gDCTX1Cmd_No > CMDRPT || gDCTX1Cmd_No_W > CMDRPTW)
        {
            ERR_MainDCTX[0] = 1;
        }
        else 
        {
            ERR_MainDCTX[0] = 0;
        }
        if(gDCTX2Cmd_No > CMDRPT || gDCTX2Cmd_No_W > CMDRPTW)      
        {
            ERR_MainDCTX[1] = 1; 
        }
        else 
        {
            ERR_MainDCTX[1] = 0;
        }
        if(gDCTX3Cmd_No > CMDRPT || gDCTX3Cmd_No_W > CMDRPTW)      
        {
            ERR_MainDCTX[2] = 1; 
        }
        else 
        {
            ERR_MainDCTX[2] = 0;
        }
        if(gDCTX4Cmd_No > CMDRPT || gDCTX4Cmd_No_W > CMDRPTW)
        {
            ERR_MainDCTX[3] = 1;   
        }
        else 
        {
            ERR_MainDCTX[3] = 0; 
        }

        if (Fg_SysSet_BatteryCheckMode == 0)  //�����ĵ��Ѳ��: PSMX-B 
        {
            for (j = 0; j < 3; j ++)
            {
                //��psmx-bͨѶ�жϴ������ձ��8���ټ�4��
                if (gDCTX_PSMXB_No[j] > (CMDRPT + CMDRPT_XB) 
                        || gDCTX_PSMXB_No_W[j] > (CMDRPTW + CMDRPT_XB))
                {
                    ERR_MainDC_PSMX_B_Comm[j] = 1;   
                }
                else 
                {
                    ERR_MainDC_PSMX_B_Comm[j] = 0; 
                }             
            }                       
        }

#if 0   //�Ż�Ӳ�ڵ㱨������źŵ��˲�������2��1�θĳ�1��3�Σ���
        if(Light_AC){
            if(!gpio_ac)
                set_gpio_output(GPIO_AC,0);
        }else{
            if(gpio_ac)
                set_gpio_output(GPIO_AC,1);
        }
        if(Light_DC){
            if(!gpio_dc)
                set_gpio_output(GPIO_DC,0); 
        }else{
            if(gpio_dc)
                set_gpio_output(GPIO_DC,1);
        }
        if(Light_Comm){
            if(!gpio_comm)
                set_gpio_output(GPIO_COMM,0);
        }else{
            if(gpio_comm)
                set_gpio_output(GPIO_COMM,1);
        }
        if(Light_UPS){
            if(!gpio_ups)
                set_gpio_output(GPIO_UPS,0);
        }else{
            if(gpio_ups)
                set_gpio_output(GPIO_UPS,1);
        }
        if(Light_AC || Light_DC || Light_Comm || Light_UPS || Light_Main){
            if(J4_GPIO_ALL != 2)
                set_gpio_output(GPIO_ALL,1);
            else
                set_gpio_output(GPIO_Zong,1);  //��J4�����ܹ��ϡ�������
            if(Sys_cfg_info.sys_set.sound)
                Beep_ctl(0,1000*1000);
        }else{
            if(J4_GPIO_ALL != 2)
                set_gpio_output(GPIO_ALL,0);
            else
                set_gpio_output(GPIO_Zong,0);
        }
        gpio_ac = (Light_AC>0)?1:0;
        gpio_dc = (Light_DC>0)?1:0;
        gpio_comm = (Light_Comm>0)?1:0;
        gpio_ups = (Light_UPS>0)?1:0;
#endif
    }
    close(iFd);
}



/**********************************************************************************
 *������:	ERTU_IEC61850_RS232(void Ertu_Start_Rs232::run())
 *��������:	��̨����ͨ���߳�
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void ERTU_IEC61850_RS232(ERTU_WATCHDOG * pcWatchDog)
{
    //pthread_detach(pthread_self());
    RS_CFG *pcRs_cfg;
    int iFd;			

    if (pcWatchDog != NULL)
    {
        if ((pcWatchDog->bValid == TRUE) && (pcWatchDog->pcDevice != NULL))
        {
            pcRs_cfg = (RS_CFG *)pcWatchDog->pcDevice;
            if ( (iFd = openport(pcRs_cfg->byPort)) != -1 )  //�򿪶˿�
            {
                server_fd = iFd;
                if (setport(iFd,                             //���ö˿�
                            pcRs_cfg->dwBaud_rate, 
                            pcRs_cfg->byByte_bit,
                            pcRs_cfg->byStop_bit,
                            pcRs_cfg->byparity) != -1)
                {
                    IEC61850_RS232(iFd,                      //��������
                            &pcWatchDog->iWatch_Dog, 
                            pcWatchDog->iWatch_Time,
                            &pcWatchDog->bExit,
                            &pcWatchDog->iTaskRunType,
                            pcRs_cfg);
                }
                else
                {
                    gError = 25;
                }
            }
            else
            {

                gError = 26;
            }
        }
    }
}

//��̨����ͨ��
void IEC61850_RS232(int iFd, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType,
        RS_CFG *pcRs_cfg)
{
    INT16U CrcValue;
    INT16U getCRC;
    INT8U  bCrc_Flag;
    INT16U cStaAddr;
    INT16U cLen;
    INT8U  cSlvAddr;
    INT8U  cFunc;
    //INT8U showBuf[2048];  //�������ô���

    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        bCrc_Flag = 0;
        memset(&Uart3Buf,0,255);
        Uart3BufCnt = 0;

        //��̨����ͨ�� 
        Uart3BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,255);   
        if(Uart3BufCnt > 0)
        {
            cSlvAddr = Uart3Buf[0];
            cFunc = Uart3Buf[1];
            cStaAddr = (Uart3Buf[2]<<8)|Uart3Buf[3];
            cLen = (Uart3Buf[4]<<8)|Uart3Buf[5];
            CrcValue = Load_crc(Uart3BufCnt-2,(INT8U *)&Uart3Buf);   //���CRC
            getCRC   = (Uart3Buf[Uart3BufCnt-2])|((Uart3Buf[Uart3BufCnt-1]<<8)& 0xFF00);  //�õ�CRC
            if(getCRC == CrcValue)   //�Ƚ�CRC
            {
                bCrc_Flag = 1;  //��̨����У��ͨ��
            }
            //���CRC��ȷ�����ǲ����������ʽ�����Ƿ�ʲô��ʲô
            if (bCrc_Flag == 1)
            {
                MoveData_RS232ToServer();
                SendData_RS232ToSever(cSlvAddr,cFunc,cStaAddr,cLen);
#if 1  //�෢������Ч���ַ�,���������CRCУ��ֵ������
                if (Uart3BufCnt <= (255 - 2))
                {
                    Uart3BufCnt += 2;
                }                 
#endif 
                writeport(iFd, (char *)Uart3Buf, Uart3BufCnt);
            }
            else   //����յ������ݣ�CRC������ȷ�����ض�����
            {
                Uart3Buf[0] = 0x55;
                Uart3Buf[1] = 0xAA;
                Uart3Buf[2] = 0xAA;
                Uart3Buf[3] = 0x55;
                Uart3Buf[4] = 0x8F;
                Uart3Buf[5] = 0x57;
                Uart3BufCnt = 6;
#if 1  //�෢������Ч���ַ�,���������CRCУ��ֵ������
                if (Uart3BufCnt <= (255 - 2))
                {
                    Uart3BufCnt += 2;
                }                 
#endif 
                writeport(iFd, (char *)Uart3Buf, Uart3BufCnt);
            }
        }
    }
    close(iFd);
}
