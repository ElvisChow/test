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

//#define  TEST_DC_SEND_NUM    //用于测试: 打印发给直流监控的次数

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

//跟psmx-b通讯中断次数由普遍的8次再加4次.
#define CMDRPT_XB     0x04

//优化硬节点报警输出信号的滤波处理（从2秒1次改成1秒3次）。
//故障告警滤波时间
#define NUM_ERROR_FILTER  0x03   
#if 1 //优化了系统总故障输出硬节点，增加总监控故障动作/复归次数统计显示.
INT16U NumOfGPIOAll_Error_On = 0x00;   //总故障动作次数
INT16U NumOfGPIOAll_OK_Off = 0x00;     //总故障复归次数
#endif
//读串口
/*********************************************************************
 *************Function name: Serial_ReadOperation*********************
 *************Input: iFd　　　　　　　　　　　　　********************
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

    if(port == 2)   //对应后台通讯
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
        //select: 读文件描述符集中有数据到来时，内核(I/O)根据状态修改文件描述符集，并返回一个大于0的数    
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

#if 0  //新增限制,防止后面执行memcpy()时溢出
    if (UartBufCnt > 255)
    {
        UartBufCnt = 255;
    }
#else  //优化串口接收数据限制处理
    if (port == 4)  //此串口特殊，主要考虑到雷能模块监控通讯(会超过255)
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

    if(UartBufCnt > 0 && port == 5)           //未使用
    {
        memcpy(Uart2Buf,UartBuf,UartBufCnt);  //Uart2Buf有溢出风险!!!!!!!!!!!!!!!!!
    }
    else if(port == 4 && UartBufCnt > 0)      //交流、ups、通信、绝缘、对时
    {
        memcpy(Uart1Buf,UartBuf,UartBufCnt);
    }
    else if(port == 3 && UartBufCnt > 0)      //直流
    {
        memcpy(Uart4Buf,UartBuf,UartBufCnt);  //Uart4Buf有溢出风险!!!!!!!!!!!!!!!!!
    }
    else if(port == 2 && UartBufCnt > 0)      //上位机(后台)
    {
        memcpy(Uart3Buf,UartBuf,UartBufCnt);  //Uart3Buf有溢出风险!!!!!!!!!!!!!!!!!
    }
    return UartBufCnt;
}

//void Ertu_Start_Time::run()
//时间相关线程
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

//时间线程，500ms为单位，可以根据需要更改
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
 *函数名:	ERTU_start_interact(void Ertu_Start_Interact::run())
 *函数功能:	交流、通信、ups数据发送线程
 *函数参数:	
 *函数返回值:
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
 * case 2: 0x7E ----- if (LN_MK_communication == 2) 雷能 //模块模拟量数据1段(CID1=1): 7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D   
 * case 3: 0x30 ----- if (LN_MK_communication != 2) 通信电源1
 * case 4: 0x31 ----- if (LN_MK_communication != 2) 通信电源2
 * case 5: 0x7E ----- if (LN_MK_communication == 2) 雷能 //模块模拟量数据2段(CID1=2): 7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
 * case 6: bgCfg_Ats1Addr -- 0x03   ATS1
 * case 7: bgCfg_Ats2Addr -- 0x03   ATS2
 * case 8: bgCfg_Ats1Addr -- 0x01   ATS1
 * case 9: bgCfg_Ats2Addr -- 0x01   ATS2
 * case 10: 0x10 ----- 0x0B/0x0C(5电操) 交流数据 
 * case 11: 0x11 ----- 交流馈线
 * case 12: 0x80 ----- 绝缘???
 * case 13: 0x7E ----- if (LN_MK_communication == 2) 雷能 //模块开关量状态1段(CID1=5): 7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
 * case 14: 0x7E ----- if (LN_MK_communication == 2) 雷能 //模块开关量状态2段(CID1=6): 7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
 * case 15: 0x7E ----- if (LN_MK_communication == 2) 雷能 //告警信息1段(CID1=7):       7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
 * case 16: 0x7E ----- if (LN_MK_communication == 2) 雷能 //告警信息2段(CID1=8):       7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
 * case 17: 0x55 ----- B码对时 
 * case 18: 0x7E ----- if (LN_MK_communication == 2) 雷能 //直流模拟量电池信息1段(CID1=3): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
 * case 19: 0x7E ----- if (LN_MK_communication == 2) 雷能 //直流模拟量电池信息2段(CID1=4): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D) 
 * case 20~22: bgCfg_Ats1Addr -- 0x05   ATS1 
 * case 23~25: bgCfg_Ats2Addr -- 0x05   ATS2  
 * */
//交流、通信、ups数据发送
void ERTU_Send_AC_T_UPS(ERTU_device *pcDevice, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType)
{		

    INT8U  CmdAddr = 1;   //8位无符号整数
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
        if(gCmdAddrEnable == 1)  //是否需优先发送
        {		
            CmdAddrBk = CmdAddr;
            CmdAddr = gCmdAddr;  //遥控时，由gCmdAddr决定
        }
        cRs232Num = 5;   //不发送为5，发送为3

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
                        gComPWCmd_No_W = 0;   //当没有通信电源时，2个故障标志都为0，不会让故障报出来
                    }
                }
                break;
            case 4:
                if (LN_MK_communication != 2)
                {
                    if(Sys_set_Comm_Num > 1)    //第二组通信电源
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
                //if (Sys_set_AC_cfg>0)  //放出来当特殊界面交流选择无时，1#ATS通信故障报不出来了
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 1 && Sys_cfg_info.ac_set.control_mode == 1)  //ATS(旭泰)
                    //ATS是1，控制方式交流设置界面设置的，0什么都不是，1ATS,2电操
                {
                    cCmdFrm.addr = bgCfg_Ats1Addr;    //跟ATS通信，走标准MoDBus协议
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x00;    //取遥测数据0x0000~0x0013
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
                        && Sys_cfg_info.ac_set.control_mode == 3)  //ATS(韩光) //新增和韩光ATS通讯支持.
                {
                    cCmdFrm.addr = bgCfg_Ats1Addr;    //跟ATS通信，走标准MoDBus协议
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x03;    //取遥测数据(1000~1075)
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
                //if (Sys_set_AC_cfg>0)  //放出来当特殊界面交流选择无时，2#ATS通信故障报不出来了
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 2 && Sys_cfg_info.ac_set.control_mode == 1) //ATS(旭泰)
                {
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x00;  //取遥测数据0x0000~0x0013
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
                        && Sys_cfg_info.ac_set.control_mode == 3)  //ATS(韩光)  //新增和韩光ATS通讯支持.
                {
                    cCmdFrm.addr = bgCfg_Ats2Addr;
                    cCmdFrm.len  = 0x03;
                    cCmdFrm.data[CmdCnt++] = 0x03;  //取遥测数据(1000~1075)
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
                //if (Sys_set_AC_cfg>0)  //放出来当特殊界面交流选择无时，1#ATS通信故障报不出来了
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 1)  //新增和韩光ATS通讯支持.
                {
                    if (Sys_cfg_info.ac_set.control_mode == 1)  //ATS(旭泰)
                    {
                        cCmdFrm.addr = bgCfg_Ats1Addr;  //子机地址
                        cCmdFrm.len  = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //取遥信数据0x0000~0x0020
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
                    else if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)
                    {
                        cCmdFrm.addr = bgCfg_Ats1Addr;  //子机地址
                        cCmdFrm.len  = 0x03;
                        cCmdFrm.data[CmdCnt++] = 0x01;  //取遥信数据(500~509)
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
                //if (Sys_set_AC_cfg>0)  //放出来当特殊界面交流选择无时，2#ATS通信故障报不出来了
                //{
                if(Sys_cfg_info.ac_set.ATSE_num >= 2)  //新增和韩光ATS通讯支持.
                {
                    if (Sys_cfg_info.ac_set.control_mode == 1) //ATS(旭泰)
                    {
                        cCmdFrm.addr = bgCfg_Ats2Addr;
                        cCmdFrm.len  = 0x01;
                        cCmdFrm.data[CmdCnt++] = 0x00;  //取遥信数据0x0000~0x0020
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
                    else if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)
                    {
                        cCmdFrm.addr = bgCfg_Ats2Addr;
                        cCmdFrm.len  = 0x03;
                        cCmdFrm.data[CmdCnt++] = 0x01;  //取遥信数据(500~509)
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
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //电流采样盒个数
#if 0
                        for(i=0;i<4;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //自动 diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // 手动合闸
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // 手动分闸
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;
                            }
                        }
#else                   //修复和后台之间的3个电操的遥控指令: 后台遥控电操逻辑调整
                        //电操1,2
                        for (i = 0; i < 2; i++)
                        {
                            if (sys_ctl_info.diancao_mode_ctl[i] == 0x02)   //下发命令为手动合闸
                            {
                                if ((Ac_info.ac_in_data[i].SW_state == 0x00) 
                                        && (diancao_cmd_num[i] > 0x00)) //当前电操状态为分闸, 并且未达到最多发送次数
                                {
                                    cCmdFrm.data[CmdCnt++] = 0x01;  //手动合闸
                                }
                                else  //动作完成后转成自动控制  
                                {
                                    cCmdFrm.data[CmdCnt++] = 0xFF;           //自动控制 
                                    sys_ctl_info.diancao_mode_ctl[i] = 0x00; //自动控制
                                    diancao_cmd_num[i] = 0x00;
                                }
                            }
                            else if (sys_ctl_info.diancao_mode_ctl[i] == 0x01) //下发命令为手动分闸     
                            {
                                if ((Ac_info.ac_in_data[i].SW_state == 0x01)
                                        && (diancao_cmd_num[i] > 0x00)) //当前电操状态为合闸, 并且未达到最多发送次数  
                                {
                                    cCmdFrm.data[CmdCnt++] = 0x00;  //手动分闸 
                                }
                                else   //动作完成后转成自动控制 
                                {
                                    cCmdFrm.data[CmdCnt++] = 0xFF;           //自动控制 
                                    sys_ctl_info.diancao_mode_ctl[i] = 0x00; //自动控制
                                    diancao_cmd_num[i] = 0x00;        
                                }
                            }
                            else  //默认为自动控制(即交流监控不受总监控或后台控制)
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;  //自动控制
                            }
                        }

                        //电操3(母联)
                        if (sys_ctl_info.diancao_mode_ctl[2] == 0x02) //下发命令为手动合闸
                        {
                            if ((Ac_info.ML_state == 0x00)
                                    && (diancao_cmd_num[2] > 0x00)) //当前电操状态为分闸, 并且未达到最多发送次数  
                            {
                                cCmdFrm.data[CmdCnt++] = 0x01;  //手动合闸
                            }
                            else  //动作完成后转成自动控制  
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;           //自动控制 
                                sys_ctl_info.diancao_mode_ctl[2] = 0x00; //自动控制 
                                diancao_cmd_num[2] = 0x00;                              
                            }
                        }
                        else if (sys_ctl_info.diancao_mode_ctl[2] == 0x01) //下发命令为手动分闸              
                        {
                            if ((Ac_info.ML_state == 0x01)
                                    && (diancao_cmd_num[2] > 0x00))  //当前电操状态为合闸, 并且未达到最多发送次数 
                            {
                                cCmdFrm.data[CmdCnt++] = 0x00;  //手动分闸
                            }
                            else  //动作完成后转成自动控制 
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;           //自动控制 
                                sys_ctl_info.diancao_mode_ctl[2] = 0x00; //自动控制
                                diancao_cmd_num[2] = 0x00;                                 
                            }                            
                        }
                        else  //默认为自动控制(即交流监控不受总监控或后台控制)
                        {
                            cCmdFrm.data[CmdCnt++] = 0xFF;  //自动控制
                        }

                        //电操4默认为自动控制  
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
                    else if(Sys_cfg_info.ac_set.diancao_num == 5)  //5电操，发交流10那一帧  //交流5电操时发送帧0x10位置移动.
                    {
                        cCmdFrm.addr = 0x10;
                        cCmdFrm.len  = 0x0C;  //12
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.duan_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ATSE_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.diancao_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ac_sampling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //电流采样盒个数
                        for(i=0;i<5;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //自动 diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // 手动合闸
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // 手动分闸
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
                if(Sys_cfg_info.ac_set.state_monitor_num > 0)  //0-20个采量盒   一个字节8位
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
#if 0   //合并到case 10   //交流5电操时发送帧0x10位置移动.
            case 20:
                if(Sys_set_AC_cfg>0)  
                {
                    if(Sys_cfg_info.ac_set.diancao_num == 5)  //5电操，发交流10那一帧
                    {
                        cCmdFrm.addr = 0x10;
                        cCmdFrm.len  = 0x0C;  //12
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.duan_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ATSE_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.diancao_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.ac_sampling_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.switch_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.state_monitor_num;
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ac_set.current_sampling_num;   //电流采样盒个数
                        for(i=0;i<5;i++){
                            if(sys_ctl_info.diancao_mode_ctl[i] == 0)    //自动 diancao_mode_ctl
                            {
                                cCmdFrm.data[CmdCnt++] = 0xFF;       

                            }
                            else if(sys_ctl_info.diancao_mode_ctl[i] == 2)
                            {  // 手动合闸
                                cCmdFrm.data[CmdCnt++] = 0x01;
                            }else                               // 手动分闸
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
                if(Sys_cfg_info.sys_set.insulate_mode == 0x00)    //独立
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
            case 2:    //模块模拟量数据1段(CID1=1): 7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D
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
            case 5:    //模块模拟量数据2段(CID1=2): 7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
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
            case 13:    //模块开关量状态1段(CID1=5): 7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
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
            case 14:   //模块开关量状态2段(CID1=6): 7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
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
            case 15:    //告警信息1段(CID1=7): 7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
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
            case 16:    //告警信息2段(CID1=8): 7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
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
            case 18:    //直流模拟量电池信息1段(CID1=3): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
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
            case 19:    //直流模拟量电池信息2段(CID1=4): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D)
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
                if(HalfSecondCnt1 >=10)    //B码对时
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
            case 20:   //新增和韩光ATS通讯支持.
                if(Sys_set_AC_cfg>0){
                    cCmdFrm.addr = bgCfg_Ats1Addr;
                    cCmdFrm.len  = 0x05;

                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x3A;    //15004: 自动/手动	0：手动 1：自动
                        cCmdFrm.data[CmdCnt++] = 0x9C;                    
                    }
                    else //ATS(旭泰)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0x0004: 自动/手动	0：手动 1：自动
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
                    cCmdFrm.len = 0x05;            //功能码05 ，强制开关量
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)  //新增和韩光ATS通讯支持.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A; //1/3: 分闸
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }
                    }         
                    else  //ATS(旭泰)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00; //1/3: 分闸
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
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)  //新增和韩光ATS通讯支持.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;    //0/2: 合闸
                        if(sys_ctl_info.ATSE_mode_ctl[0] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[0] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        } 
                    }   
                    else //ATS(旭泰) 
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0/2: 合闸
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
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)  //新增和韩光ATS通讯支持.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x3A;    //15004: 自动/手动	0：手动 1：自动
                        cCmdFrm.data[CmdCnt++] = 0x9C;                    
                    }
                    else //ATS(旭泰)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;    //0x0004: 自动/手动	0：手动 1：自动
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
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)  //新增和韩光ATS通讯支持.
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;   //1/3: 分闸
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x01 + 0x98;
                        }   
                    }   
                    else //ATS(旭泰) 
                    { 
                        cCmdFrm.data[CmdCnt++] = 0x00;   //1/3: 分闸
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
                    if (Sys_cfg_info.ac_set.control_mode == 3) //ATS(韩光)  //新增和韩光ATS通讯支持.
                    {                  
                        cCmdFrm.data[CmdCnt++] = 0x00 + 0x3A;   //0/2: 合闸
                        if(sys_ctl_info.ATSE_mode_ctl[1] == 0x01){
                            cCmdFrm.data[CmdCnt++] = 0x00 + 0x98;
                        }else if(sys_ctl_info.ATSE_mode_ctl[1] == 0x02){
                            cCmdFrm.data[CmdCnt++] = 0x02 + 0x98;
                        }else{
                            cCmdFrm.data[CmdCnt++] = 0x03 + 0x98;
                        }
                    }  
                    else  //ATS(旭泰)
                    {
                        cCmdFrm.data[CmdCnt++] = 0x00;   //0/2: 合闸
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

        if(cRs232Num == 3 && gCmdAddrEnable == 1)  //gCmdAddrEnable: 设置参数时的地址优先发  //特殊情况(设置参数时)
        {
            if(gEnQueue3)  //说明已发送1个数据，可以再放下一个数据包到发送队列
            {
                cFrmLen = sizeof(cCmdFrm);

                //数据入队列(交流)
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
                if(sys_ctl_info.ATSE2_ctl_flag == 1)  //遥控ATS2(20~25)
                {
                    if(CmdAddr == 25)
                    {
                        gCmdAddrEnable = 0;
                        sys_ctl_info.ATSE2_ctl_flag = 0;
                    }
                    if(gCmdAddr > 25)
                        gCmdAddr = 20;
                }
                else  //遥控ATS1(20~22)
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
        else if(cRs232Num == 3 && gCmdAddrEnable == 0) //常规情况(不设置参数时)
        {
            if(gEnQueue3)
            {
                cFrmLen = sizeof(cCmdFrm);

                //把待发送的数据放入待发送队列中(交流)
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
        if(CmdAddr > 19)   //原case20合并到case10
        {
            CmdAddr = 1;
        }
    }
}
/**********************************************************************************
 *函数名:	ERTU_start_interact1(Ertu_Start_Interact1::run())
 *函数功能:	直流数据发送线程
 *函数参数:	
 *函数返回值:
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

//直流数据发送处理
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
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //充电模块处理从原8个扩到12个.
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8个模块  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8个以上模块

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //最后4个模块关机
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
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //充电模块处理从原8个扩到12个. 
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8个模块  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8个以上模块

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //最后4个模块关机
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
                        DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                    DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                    cCmdFrm.data[CmdCnt++] = DataByte;
#else  //充电模块处理从原8个扩到12个.
                    if (Sys_cfg_info.dc_set[DC_duan_NUM].module_num <= 8)
                    {
                        cCmdFrm.len = 0x19;  //8个模块  

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;
                    }
                    else
                    {
                        cCmdFrm.len = 0x1A;  //8个以上模块

                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[DC_duan_NUM].module_num; 				
                        for(i=0;i<8;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte;

                        for(i=0;i<4;i++)
                            DataIn[i] = (~sys_ctl_info.module_ctl[DC_duan_NUM][i + 8]) & 0x01;   //打钩为选中1要开机，而协议中开机为0  ~相反
                        DataByte = ChgData_8BytesToByte((INT8U *)DataIn);
                        cCmdFrm.data[CmdCnt++] = DataByte | 0xF0;  //最后4个模块关机
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
#else  //系统设置电池参数从统一设置改成分组设置. 
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
#else  //系统设置电池参数从统一设置改成分组设置. 
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
#else      //第3路直流无绝缘监控 
                    cCmdFrm.data[CmdCnt++] = 0x00;  //绝缘设置(0x00：1-----独立配置)
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
#else  //系统设置电池参数从统一设置改成分组设置. 
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
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //绝缘独立标志
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
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //绝缘独立标志
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
#if 0  //屏蔽			
                else if (DC_duan_NUM == 2){				
                    if(Sys_cfg_info.sys_set.insulate_mode != 0)			  //绝缘独立标志
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
                    if(Fg_SysSet_BatteryCheckMode != 0)    //电池独立标志
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
                    else  //独立的电池巡检: PSMX-B 
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
                    if(Fg_SysSet_BatteryCheckMode != 0)    //电池独立标志
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
                    else  //独立的电池巡检: PSMX-B 
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
                    if(Fg_SysSet_BatteryCheckMode != 0)    //电池独立标志
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
                    else  //独立的电池巡检: PSMX-B 
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
#else  //新增两种有无通信、UPS电源监控的选择项.
                        if ((Special_35KV_flag == 0x02)   //当没有UPS监控时 
                                || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02))    
#endif 
                        {
                            cCmdFrm.addr = 0x26;
                            cCmdFrm.len = 0x04;
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.ups_num & 0x00FF;          //UPS电源个数
                            cCmdFrm.data[CmdCnt++] = 0;
                            cCmdFrm.data[CmdCnt++] = Sys_cfg_info.ups_set.state_monitor_num & 0x00FF;//UPS状态盒个数
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
                    if(LN_MK_communication == 2){  //雷能模块
#if 0
                        if(Special_35KV_flag == 2) 
#else  //新增两种有无通信、UPS电源监控的选择项.
                            if ((Special_35KV_flag == 0x02)   //当没有通信监控时 
                                    || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))    
#endif   
                            {
                                cCmdFrm.addr = 0x25;
                                cCmdFrm.len = 0x0B;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].state_monitor_num;  //通信电源状态盒个数
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
                    }else {  //非雷能模块
#if 0 
                        if(Special_35KV_flag == 2)
#else  //新增两种有无通信、UPS电源监控的选择项.
                            if ((Special_35KV_flag == 0x02)   //当没有通信监控时 
                                    || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))    
#endif   
                            {
                                cCmdFrm.addr = 0x25;
                                cCmdFrm.len = 0x0B;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_num;        //通信电源模块个数
                                cCmdFrm.data[CmdCnt++] = 0;
                                cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].state_monitor_num; //通信电源状态盒个数
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
#else  //系统设置电池参数从统一设置改成分组设置.  
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
#else   //系统设置电池参数从统一设置改成分组设置.  
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
                            || (Special_35KV_flag == 2)  //特殊工程，通信电源和逆变电源挂在PSM-3监控下
#if 1  //新增两种有无通信、UPS电源监控的选择项.
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
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //标准单元的模块个数用状态量个数来传递
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;      //Byte10						
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v >> 8;  //ATD模块电压*10   高字节
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[0].module_output_v;       //ATD模块电压*10   低字节
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
                            || (Special_35KV_flag == 2)//特殊工程，通信电源和逆变电源挂在PSM-3监控下
#if 1  //新增两种有无通信、UPS电源监控的选择项.
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
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //标准单元的模块个数用状态量个数来传递
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;		  //Byte10				
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v >> 8;  //ATD模块电压*10   高字节
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v;       //ATD模块电压*10   低字节
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
                            || (Special_35KV_flag == 2)  //特殊工程，通信电源和逆变电源挂在PSM-3监控下
#if 1  //新增两种有无通信、UPS电源监控的选择项.
                            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
                            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02)
#endif   
                       ) 
                    {   //直流3段预留现在特意不然进
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
                        //cCmdFrm.data[CmdCnt++] = Sys_cfg_info.dc_set[0].state_monitor_num;   //标准单元的模块个数用状态量个数来传递
                        cCmdFrm.data[CmdCnt++] = 0;	
                        cCmdFrm.data[CmdCnt++] = 0;		  //Byte10			
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v >> 8;  //ATD模块电压*10   高字节(此处同第二组通信电源)
                        cCmdFrm.data[CmdCnt++] = Sys_cfg_info.comm_set[1].module_output_v;       //ATD模块电压*10   低字节
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

            case 13:  //读PSMX-B的电池内阻
                if (Fg_SysSet_BatteryCheckMode == 0)    //独立的电池巡检: PSMX-B 
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

            case 14:  //读PSMX-B的电池遥信(采样盒通讯异常、内阻超限)
                if (Fg_SysSet_BatteryCheckMode == 0)    //独立的电池巡检: PSMX-B 
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

#if 1   //“特殊界面”-“系统配置”-“直流系统”增加下拉选项“无”.
        if (Sys_set_DC_duan > 0)  //至少有一段直流
        { 
#endif 
            if(cRs232Num == 4 && gCmdAddrEnable1 == 1)  //设置参数时
            {
                while(gEnQueue4)
                {
                    cFrmLen = sizeof(cCmdFrm);

                    //数据入队列(直流)
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
            else if(cRs232Num == 4 && gCmdAddrEnable1 == 0) //常规情况(不设置参数时)
            {
                while(gEnQueue4)
                {
                    cFrmLen = sizeof(cCmdFrm);

                    //数据入队列(直流)
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
#if 1   //“特殊界面”-“系统配置”-“直流系统”增加下拉选项“无”.
        }
        else   //Sys_set_DC_duan = 0
        {
            CmdAddr = 0x00;  //禁掉直流通道的全部外发数据
            cRs232Num = 0x05;
            ERR_MainDC[0] = 0x00;
            ERR_MainDC[1] = 0x00;
            ERR_MainDC[2] = 0x00;
            gDCCmd_No = 0;          //收的错误帧计数清0
            gDCCmd_No_W = 0;        //写失败的计数清0
            gDC1Cmd_No = 0;
            gDC1Cmd_No_W = 0;
            gDC2Cmd_No = 0;
            gDC2Cmd_No_W = 0;                    
        }
#endif 

        //地址: 1~12
        if(Fg_SysSet_BatteryCheckMode == 0)    //独立的电池巡检: PSMX-B 
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
 *函数名:	ERTU_start_sio(void Ertu_Start_Sio::run())
 *函数功能:	交流、通信、ups数据接收线程
 *函数参数:	
 *函数返回值:
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
            if ((iFd = openport(pcRs_cfg->byPort)) != -1) //打开端口
            {
                sio_fd = iFd;
                if (setport(iFd,                          //设置端口
                            pcRs_cfg->dwBaud_rate, 
                            pcRs_cfg->byByte_bit,
                            pcRs_cfg->byStop_bit,
                            pcRs_cfg->byparity) != -1)
                {
                    ERTU_Rec_AC_T_UPS(iFd,                //接收数据
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

//交流、通信、ups数据、对时、雷能数据接收
void ERTU_Rec_AC_T_UPS(int iFd, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType,
        RS_CFG *pcRs_cfg)

{
    Cmd_Res_Frame cCmdFrame;  //串口发送缓冲区
    Cmd_Res_Frame cResFrame;  //串口接收缓冲区
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

#if 1  //优化硬节点报警输出信号的滤波处理（从2秒1次改成1秒3次）。
    static INT8U gpio_all = 0;   //新增: 优化了系统总故障输出硬节点，增加总监控故障动作/复归次数统计显示.
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

        //从发送数据队列中取出待发送数据
        //出队列(交流、通信、ups数据)
        if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232], 
                    (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))     //(特殊: 设置参数时)
        {
            gSetConfigEnable1 = 0;
            if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_Auto], 
                        (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT)) //(常规: 不设置参数时)
            {
                usleep(1);   //出队列超时失败
                continue;
            }
        }
        else
        {
            gSetConfigEnable1 = 1;  //出队列成功
        }
        clearport(iFd);
        gEnQueue3 = 1;

        //把从队列取出的数据向外发送
        //串口数据写入函数        
        if (writeport(iFd, (char *)&cCmdFrame, cCmdFrame.CFrmSize) == TRUE)  //发送数据成功时
        {
            if (cCmdFrame.addr == 0x7E)  //雷能数据
            {
                if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x01;  //模块模拟量数据1段(发送):     7E 32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x02;  //模块模拟量数据2段(发送):     7E 32 31 30 32 34 31 34 31 30 30 30 30 46 44 42 31 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x32 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x03;  //直流模拟量电池信息1段(发送): 7E 32 31 30 31 34 32 34 31 45 30 30 32 30 31 46 44 33 39 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x32 && cCmdFrame.data[6] ==  0x31){
                    CID1 = 0x04;  //直流模拟量电池信息2段(发送): 7E 32 31 30 32 34 32 34 31 45 30 30 32 30 31 46 44 33 38 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x33){
                    CID1 = 0x05;  //模块开关量状态1段(发送):     7E 32 31 30 31 34 31 34 33 30 30 30 30 46 44 42 30 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x33){
                    CID1 = 0x06;  //模块开关量状态2段(发送):     7E 32 31 30 32 34 31 34 33 30 30 30 30 46 44 41 46 0D
                }else if (cCmdFrame.data[2] ==  0x31 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x34){
                    CID1 = 0x07;  //告警信息1段(发送):           7E 32 31 30 31 34 31 34 34 30 30 30 30 46 44 41 46 0D
                }else if (cCmdFrame.data[2] ==  0x32 && cCmdFrame.data[4] ==  0x31 && cCmdFrame.data[6] ==  0x34){
                    CID1 = 0x08;  //告警信息2段(发送):           7E 32 31 30 32 34 31 34 34 30 30 30 30 46 44 41 45 0D
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
            //开始接收数据 
            for(j = 0;j < 255;j++)
            {
                Uart1Buf[j] = 0x00;
            }

            memset(&cResFrame,0,sizeof(cResFrame));
            Uart1BufCnt = 0;
            cFrmLen = sizeof(cResFrame);
            //printf("rve data1 rly\n");

            //交流、通信、ups数据接收 
            Uart1BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,cFrmLen);  
            //debug_printf(0,",,Uart1BufCnt  == %4d\n ",Uart1BufCnt);

            //--------------------------------------------------------
            //接收到数据的处理
            if(Uart1BufCnt > 0)   //Uart1BufCnt收到帧的长度
            {
                //printf("rve data 1\n");
                //set_gpio_output(GPIO_AC,0);
                //usleep(1000*500);
                //set_gpio_output(GPIO_AC,1);

                if (Uart1Buf[0] == 0x7E)   //开始判断雷能回的数据是不是正确
                {
                    //雷能数据处理    
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

                                //若雷能数据校验通过 
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
                                        bCrc_Flag = 1;   //雷能数据校验通过
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
                    if(Uart1BufCnt == 11 && Uart1Buf[0] == 0x55    //对时
                            && Uart1Buf[1] == 0x03 && Uart1Buf[2] == 0x06)
                    {
                        nowtime.tm_hour = Uart1Buf[3];
                        nowtime.tm_min = Uart1Buf[4];
                        nowtime.tm_sec = Uart1Buf[5];
                        nowtime.tm_mday = Uart1Buf[6];
                        nowtime.tm_mon = Uart1Buf[7]-1;           //月，监控显示的月是从0开始的，B码对时的月是从1开始的，所以减1，监控才能正确显示时间
                        nowtime.tm_year = Uart1Buf[8]+2000-1900;  //年，B码对时给的年是17或者18是减去2000的，监控显示时间是1900年开始的，所以17加2000减1900，监控显示时间自己再加上1900后，再来显示
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
                    else  //RS485-3数据接收处理(除了雷能和对时)
                    {
                        cResFrame.CFrmSize = Uart1BufCnt;
                        cResFrame.addr = Uart1Buf[0];
                        cResFrame.len = Uart1Buf[1];
                        for(j = 2; j< Uart1BufCnt; j++)
                        {
                            cResFrame.data[j-2] = Uart1Buf[j];    //数据第一位，就是协议的第3位，0是第一位，2是第三位
                        }
#ifdef Dug
                        if(cCmdFrame.addr == 0x10 )
                        {
                            mem_show1((U_BYTE *)showBuf,(INT8U *)&Uart1Buf,Uart1BufCnt,"RS232 receive the data2222:");
                            debug_printf(0,"%2s\n",showBuf);
                            //     debug_printf(0,"Uart1Buf[50] ========= %d\n",Uart1Buf[50]);
                        }
#endif
                        CrcValue = Load_crc(Uart1BufCnt-2,(INT8U *)&Uart1Buf);   //调用Load_crc()函数，计数出数据的CRC
                        cResFrame.CrcChk = (Uart1Buf[Uart1BufCnt-2])|((Uart1Buf[Uart1BufCnt-1]<<8)& 0xFF00);  //数据传递下来的CRC
#ifdef Dug
                        debug_printf(0,"addr==%x,, CrcValue == %4x ,,  cResFrame.CrcChk == %4x\n",Uart1Buf[0],CrcValue,cResFrame.CrcChk);
#endif
                        if(cResFrame.CrcChk == CrcValue)   //比较CRC是否一致，一次说明传递下来的数据是正确的
                        {
                            bCrc_Flag = 1;  //RS485-3数据接收处理(除了雷能和对时)校验通过
                        }
                        else
                        {
                            bCrc_Flag = 0;
                        }
                    }
                }
            }

            if (bCrc_Flag == 1)   //数据检验成功
            {
                if(cResFrame.addr==0x30)
                {
                    gComPWCmd_No = 0;       //收的错误帧计数清0
                    gComPWCmd_No_W = 0;     //写失败的计数清0
                    ERR_MainComPw= 0;       //故障清0
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
                else if(cResFrame.addr==bgCfg_Ats1Addr)   //1#ATS故障
                {
                    gATSCmd_No = 0;        //收的错误帧计数清0
                    gATSCmd_No_W = 0;      //写失败的计数清0
                    ERR_MainATS= 0;        //故障清0
                }
                else if(cResFrame.addr == bgCfg_Ats2Addr)  //2#ATS故障
                {
                    gATS1Cmd_No = 0;
                    gATS1Cmd_No_W = 0;
                    ERR_MainATS1= 0;
                }
                else if(cResFrame.addr == 0x10)  //交流
                {
                    gACCmd_No = 0;
                    gACCmd_No_W = 0;
                    ERR_MainAC = 0;
                }else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x31)
                {
                    gComPWCmd_No = 0;       //收的错误帧计数清0
                    gComPWCmd_No_W = 0;     //写失败的计数清0
                    ERR_MainComPw= 0;       //故障清0
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x32)
                {
                    gComPW1Cmd_No = 0;
                    gComPW1Cmd_No_W = 0;
                    ERR_MainComPw1= 0;
                }

                //--------------------------------------------------------
                /*******根据命令把数据送给相应的需要显示的参数***********/
                if (CID2 ==  0x31){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x31); //接收雷能数据-01 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }else if (CID2 ==  0x33){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x33); //接收雷能数据-02 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }else if (CID2 ==  0x34){
                    receive_data(Uart1Buf[4],Uart1Buf[6],0x34); //接收雷能数据-03 
                    CID2 =  0x00;
                    bCrc_Flag =0;
                }
                else
                {
                    receive_data(Uart1Buf[0],Uart1Buf[1],0);    //接收交流、通信、ups数据
                }   
            }
            else  //数据检验失败
            {
                if(cCmdFrame.addr==0x30 && gComPWCmd_No <= CMDRPT)       //8次
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
                    gATSCmd_No ++;  //回的报文有错，1#ATS报错标志位加1
                }
                else if(cCmdFrame.addr == bgCfg_Ats2Addr && gATS1Cmd_No <= CMDRPT)
                {
                    gATS1Cmd_No ++;   //回的报文有错，2#ATS报错标志位加1
                }
                else if(cCmdFrame.addr == 0x10 && gACCmd_No <= CMDRPT)
                {
                    gACCmd_No ++;     //交流屏监控单元数据
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x31 && gComPWCmd_No <= CMDRPT)  //8次
                {
                    gComPWCmd_No ++;
                }
                else if(Uart1Buf[3]==0x30  && Uart1Buf[4]==0x32 && gComPW1Cmd_No <= CMDRPT)
                {
                    gComPW1Cmd_No ++;
                }
                if (Uart1BufCnt == 0){
                    if(cCmdFrame.data[1]==0x30  && cCmdFrame.data[2]==0x31 && gComPWCmd_No <= CMDRPT)       //8次
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
        else  //往外发送数据失败时
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
                gATSCmd_No_W ++;   //发失败，1#ATS报通信故障加1
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

        //处理通讯是否中断
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
#else       //新增两种有无通信、UPS电源监控的选择项.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoUpsMon_WithCommMon != 2))
            {
                //当有UPS监控时
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
#else       //新增两种有无通信、UPS电源监控的选择项.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoCommMon_WithUpsMon != 2))
            {
                //当有通信监控时
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
#else       //新增两种有无通信、UPS电源监控的选择项.
            if ((Special_35KV_flag != 2) 
                    && (Special_35KV_flag_NoCommMon_WithUpsMon != 2))
            {
                //当有通信监控时
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

#if 1   //优化硬节点报警输出信号的滤波处理（从2秒1次改成1秒3次）。
        //增加3次滤波处理 
        if (Light_AC)
        {
            NumOfLightAC_No = 0x00;
            if (NumOfLightAC_Yes < NUM_ERROR_FILTER)
            {
                NumOfLightAC_Yes ++;
            }
            else  //告警动作 
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
            else  //告警复归
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
            else  //告警动作 
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
            else  //告警复归
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
            else  //告警动作 
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
            else  //告警复归
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
            else  //告警动作 
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
            else  //告警复归
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
            else  //告警动作 
            {
#if 0
                if (J4_GPIO_ALL != 2)
                    set_gpio_output(GPIO_ALL,1);
                else
                    set_gpio_output(GPIO_Zong,1);  //“J4用作总故障”起作用
#else    //优化了系统总故障输出硬节点，增加总监控故障动作/复归次数统计显示.
                if (!gpio_all)
                {
                    gpio_all = 0x01;
                    if (J4_GPIO_ALL != 2)
                        set_gpio_output(GPIO_ALL,1);
                    else
                        set_gpio_output(GPIO_Zong,1);  //“J4用作总故障”起作用
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
            else  //告警复归
            {
#if 0
                if (J4_GPIO_ALL != 2)
                    set_gpio_output(GPIO_ALL,0);
                else
                    set_gpio_output(GPIO_Zong,0);
#else //优化了系统总故障输出硬节点，增加总监控故障动作/复归次数统计显示.
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
 *函数名:	ERTU_start_sio1(void Ertu_Start_Sio1::run())
 *函数功能:	直流数据接收线程
 *函数参数:	
 *函数返回值:
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

//直流数据接收
void ERTU_Rec_DC(int iFd, 
        int *piWatchdog, 
        int iWatchtime, 
        U_BOOL *pbExit, 
        int *piTaskRunType,
        RS_CFG *pcRs_cfg)
{

    Cmd_Res_Frame cCmdFrame;  //串口发送缓冲区
    Cmd_Res_Frame cResFrame;  //串口接收缓冲区
    INT8U  cFrmLen;
    INT16U CrcValue;
    INT8U  bCrc_Flag;
    INT8U showBuf[2048];
    INT16U j;
    INT8U  cPort;
#if 0  //优化硬节点报警输出信号的滤波处理（从2秒1次改成1秒3次）。
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

#if 1   //正常程序
        sleep(1); //为了保证通信成功率，执行完命令后等500ms；
#else   //用于测试报文：每5秒发出1次报文；
        sleep(5);         
#endif 

        //usleep(1000);
        bCrc_Flag = 0;
        cPort = pcRs_cfg->byPort;
        cFrmLen = sizeof(cCmdFrame);
        memset(&showBuf,0,255);

        //出队列(直流)
        if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4], //(特殊: 设置参数时)
                    (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))   
        {
            if (!VQ_Dequeue(g_global_quequ[ERTU_QUEUE_ERTUTORS232_4_Auto], 
                        (U_BYTE*)&cCmdFrame, &cFrmLen, NO_WAIT))  //(常规: 不设置参数时)
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
        if (writeport(iFd, (char *)&cCmdFrame, cCmdFrame.CFrmSize) == TRUE)    //写数据失败   writeport串口数据写入函数
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
            Uart4BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,cFrmLen); //直流
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
                CrcValue = Load_crc(Uart4BufCnt-2,(INT8U *)&Uart4Buf);       //计算CRC
                cResFrame.CrcChk = (Uart4Buf[Uart4BufCnt-2])|((Uart4Buf[Uart4BufCnt-1]<<8)& 0xFF00);   //拿到CRC，帧上面最后一个字节，左移8位，再与上1111 1111 0000 000，再或上最后第二个字节
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
                    gDCCmd_No = 0;          //收的错误帧计数清0
                    gDCCmd_No_W = 0;        //写失败的计数清0
                    ERR_MainDC[0] = 0;      //故障清0
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

                if (Fg_SysSet_BatteryCheckMode == 0)  //独立的电池巡检: PSMX-B 
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

                /**************根据命令把数据送给相应的需要显示的参数******************/
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
                if ((Fg_SysSet_BatteryCheckMode == 0)  //独立的电池巡检: PSMX-B 
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

                if (Fg_SysSet_BatteryCheckMode == 0)  //独立的电池巡检: PSMX-B 
                {
                    for (j = 0; j < 3; j ++)
                    {
                        //跟psmx-b通讯中断次数由普遍的8次再加4次.
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

            if (Fg_SysSet_BatteryCheckMode == 0)  //独立的电池巡检: PSMX-B 
            {
                for (j = 0; j < 3; j ++)
                {
                    //跟psmx-b通讯中断次数由普遍的8次再加4次.
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

        if (Fg_SysSet_BatteryCheckMode == 0)  //独立的电池巡检: PSMX-B 
        {
            for (j = 0; j < 3; j ++)
            {
                //跟psmx-b通讯中断次数由普遍的8次再加4次
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

#if 0   //优化硬节点报警输出信号的滤波处理（从2秒1次改成1秒3次）。
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
                set_gpio_output(GPIO_Zong,1);  //“J4用作总故障”起作用
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
 *函数名:	ERTU_IEC61850_RS232(void Ertu_Start_Rs232::run())
 *函数功能:	后台数据通信线程
 *函数参数:	
 *函数返回值:
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
            if ( (iFd = openport(pcRs_cfg->byPort)) != -1 )  //打开端口
            {
                server_fd = iFd;
                if (setport(iFd,                             //设置端口
                            pcRs_cfg->dwBaud_rate, 
                            pcRs_cfg->byByte_bit,
                            pcRs_cfg->byStop_bit,
                            pcRs_cfg->byparity) != -1)
                {
                    IEC61850_RS232(iFd,                      //接收数据
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

//后台数据通信
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
    //INT8U showBuf[2048];  //屏蔽无用代码

    while (*pbExit == FALSE)
    {
        *piWatchdog = iWatchtime;
        bCrc_Flag = 0;
        memset(&Uart3Buf,0,255);
        Uart3BufCnt = 0;

        //后台数据通信 
        Uart3BufCnt = Serial_ReadOperation(iFd,pcRs_cfg,255);   
        if(Uart3BufCnt > 0)
        {
            cSlvAddr = Uart3Buf[0];
            cFunc = Uart3Buf[1];
            cStaAddr = (Uart3Buf[2]<<8)|Uart3Buf[3];
            cLen = (Uart3Buf[4]<<8)|Uart3Buf[5];
            CrcValue = Load_crc(Uart3BufCnt-2,(INT8U *)&Uart3Buf);   //算出CRC
            getCRC   = (Uart3Buf[Uart3BufCnt-2])|((Uart3Buf[Uart3BufCnt-1]<<8)& 0xFF00);  //拿到CRC
            if(getCRC == CrcValue)   //比较CRC
            {
                bCrc_Flag = 1;  //后台数据校验通过
            }
            //如果CRC正确，但是不符合命令格式，就是发什么回什么
            if (bCrc_Flag == 1)
            {
                MoveData_RS232ToServer();
                SendData_RS232ToSever(cSlvAddr,cFunc,cStaAddr,cLen);
#if 1  //多发两个无效的字符,解决发不出CRC校验值的问题
                if (Uart3BufCnt <= (255 - 2))
                {
                    Uart3BufCnt += 2;
                }                 
#endif 
                writeport(iFd, (char *)Uart3Buf, Uart3BufCnt);
            }
            else   //如果收到的数据，CRC都不正确，回特定数据
            {
                Uart3Buf[0] = 0x55;
                Uart3Buf[1] = 0xAA;
                Uart3Buf[2] = 0xAA;
                Uart3Buf[3] = 0x55;
                Uart3Buf[4] = 0x8F;
                Uart3Buf[5] = 0x57;
                Uart3BufCnt = 6;
#if 1  //多发两个无效的字符,解决发不出CRC校验值的问题
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
