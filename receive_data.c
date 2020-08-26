#include "receive_data.h"
#include "global_define.h"
#include "globel_Ex.h"
#include "Global_Para_Ex.h"
#include "Subfunc.h"
#include "version.h"

#if 0
#define MAXCOUNT_GY_QY  3                //过欠压去抖次数
#else   //修正电池单体过欠压处理,滤波次数从3次增至8次. 
#define MAXCOUNT_GY_QY  8                //过欠压去抖次数
#endif 
#define UPS_MODE_MAINS_SUPPLY   (0x01)   //UPS市电模式 
#define UPS_MODE_BAT_SUPPLY     (0x02)   //UPS电池模式
#define UPS_MODE_BY_PASS        (0x04)   //UPS旁路模式
#define UPS_MODE_OVER_HAUL      (0x08)   //UPS检修模式  
//#define  UPS_OUTPUT_CUR          (36)  //UPS额定输出电流36A
#define UPS_OUTPUT_CUR          (Sys_cfg_info.ups_set.Capacity * 0.8 / 220.0) //UPS额定输出电流

static char ERR_DC_BatterySingle_GY_CNT[3][127] = {{0}};   // 过压计数
static char ERR_DC_BatterySingle_GYHF_CNT[3][127] = {{0}}; // 过压恢复计数
static char ERR_DC_BatterySingle_QY_CNT[3][127] = {{0}};   // 欠压计数
static char ERR_DC_BatterySingle_QYHF_CNT[3][127] = {{0}}; // 欠压恢复计数

extern INT8U JudgeBitInByte(INT8U ByteData,INT8U BitPos);

//#define  TEST_DC_REC_NUM    //用于测试: 打印直流监控返回数据的次数
#ifdef TEST_DC_REC_NUM
static unsigned int DC_REC_NUM_0x20 = 0;
static unsigned int DC_REC_NUM_0x28 = 0;
#endif 

/**********************************************************************************
 *函数名:	 receive_data
 *函数功能:  处理串口收到的信息,按照设备的地址、帧长度和功能码，分别调用下面的函数转存数据
 *函数参数:	
 *函数返回值:
 ***********************************************************************************/
void receive_data(char Addr,char Len,char mark)
{
#if 1   //“特殊界面”-“系统配置”-“直流系统”增加下拉选项“无”.  --转移位置
    //接收交流监控数据 
    if(Addr == 0x10 && Len == 0xC4){       //取交流屏监控单元数据--01
        receive_AC_data();            
    }else if(Addr == 0x11 && Len == 0x80){ //取馈线开关状态数据
        receive_AC_FeederLine_data();
    }else if(Addr == 0x10 && Len == 0xC6){ //取交流屏监控单元数据--02
        receive_AC_dc_data();
    } 

    //接收雷能通信电源监控数据 
    if(Addr == 0x31 && Len == 0x31 && mark == 0x31){
        receive_LNMKComm_data(0);   //模块模拟量数据1段
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x31){
        receive_LNMKComm_data(1);   //模块模拟量数据2段
    }else if(Addr == 0x31 && Len == 0x31 && mark == 0x33){
        receive_LNMKComm_status(0); //模块开关量状态1段
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x33){
        receive_LNMKComm_status(1); //模块开关量状态2段
    }else if(Addr == 0x31 && Len == 0x31 && mark == 0x34){
        receive_LNMKComm_alarm(0);  //告警信息1段
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x34){
        receive_LNMKComm_alarm(1);  //告警信息2段
    }else if(Addr == 0x31 && Len == 0x32 && mark == 0x31){
        receive_LNDCComm_data(0);   //直流模拟量电池信息1段
    }else if(Addr == 0x32 && Len == 0x32 && mark == 0x31){
        receive_LNDCComm_data(1);   //直流模拟量电池信息2段
    }


    if(Addr == 0x30 && Len == 0x8F){  //取第1组通信电源监控单元数据
        receive_Comm_data(0);
    }else if(Addr == 0x31 && Len == 0x8F){  //取第2组通信电源监控单元数据
        receive_Comm_data(1);
    }else if(Addr == 0x40 && Len == 0x42){  //取UPS监控单元数据
        receive_UPS_data();
    }
#endif

    unsigned int group;
    for(group = 0;group < Sys_set_DC_duan; group ++)  //Sys_set_DC_duan 1,2,3
    {  
#if 0   //“特殊界面”-“系统配置”-“直流系统”增加下拉选项“无”.  --转移位置
        //接收交流监控数据 
        if(Addr == 0x10 && Len == 0xC4){       //取交流屏监控单元数据--01
            receive_AC_data();            
        }else if(Addr == 0x11 && Len == 0x80){ //取馈线开关状态数据
            receive_AC_FeederLine_data();
        }else if(Addr == 0x10 && Len == 0xC6){ //取交流屏监控单元数据--02
            receive_AC_dc_data();
        } 

        //接收雷能通信电源监控数据 
        if(Addr == 0x31 && Len == 0x31 && mark == 0x31){
            receive_LNMKComm_data(0);   //模块模拟量数据1段
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x31){
            receive_LNMKComm_data(1);   //模块模拟量数据2段
        }else if(Addr == 0x31 && Len == 0x31 && mark == 0x33){
            receive_LNMKComm_status(0); //模块开关量状态1段
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x33){
            receive_LNMKComm_status(1); //模块开关量状态2段
        }else if(Addr == 0x31 && Len == 0x31 && mark == 0x34){
            receive_LNMKComm_alarm(0);  //告警信息1段
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x34){
            receive_LNMKComm_alarm(1);  //告警信息2段
        }else if(Addr == 0x31 && Len == 0x32 && mark == 0x31){
            receive_LNDCComm_data(0);   //直流模拟量电池信息1段
        }else if(Addr == 0x32 && Len == 0x32 && mark == 0x31){
            receive_LNDCComm_data(1);   //直流模拟量电池信息2段
        } 
#endif

        //接收电池采样盒数据(通信电源电池信息)
        if (Addr == 0x81 && Len == 0x60){
            receive_TX_Battery_data(0);
        }else if (Addr == 0x81 && Len == 0x61){
            receive_TX_Battery_data(1);
        }else if (Addr == 0x81 && Len == 0x62){
            receive_TX_Battery_data(2);
        }else if (Addr == 0x81 && Len == 0x63){
            receive_TX_Battery_data(3);
        }

        //接收直流监控系统数据
        if (group == 0)
        {
            if(Addr == 0x20 && Len == 0x39){
#ifdef TEST_DC_REC_NUM  //just for test
                if (DC_REC_NUM_0x20 < 0xFFFF)
                {
                    DC_REC_NUM_0x20 ++;
                }
                printf("DC_REC_NUM_0x20(0x39): %d.\n", DC_REC_NUM_0x20);
#endif 
                receive_DC_module_data(0);        //取充电模块数据
            }else if(Addr == 0x20 && Len == 0x72){
#ifdef TEST_DC_REC_NUM  //just for test
                if (DC_REC_NUM_0x20 < 0xFFFF)
                {
                    DC_REC_NUM_0x20 ++;
                }
                printf("DC_REC_NUM_0x20(0x72): %d.\n", DC_REC_NUM_0x20);
#endif           

                receive_DC_module_data_NUM12(0);  //取充电模块数据(12个充电模块)
            }else if(Addr == 0x21 && Len == 0x21){
                receive_DC_monitor_data(0);       //取直流母线数据
            }else if(Addr == 0x22){
                receive_DC_JY_data(0);            //取绝缘参数
            }else if(Addr == 0x23 && Len == 0x84){
                receive_DC_Switch_data(0);        //取馈线开关数据
            }else if(Addr == 0x24 && Len == 0xDA 
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //无独立的电池巡检: 数据从PSM-3读取
                receive_DC_Battery_data(0);       //取电池相关数据: 温度、电压 
            }else if(Addr == 0x50 && Len == 0x80){
                receive_DC_FenGui_data(0);        //馈线开关相关数据
            }else if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
            {
                if (Addr == 0xB0 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(0); //取电池相关数据: 电池电压 
                }
                else if (Addr == 0xB0 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(0); //取电池相关数据: 电池内阻
                }
                else if (Addr == 0xB0 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(0);  //取电池相关数据: 电池遥信量
                }                
            }
        }
        else if (group == 1)
        {
            if(Addr == 0x28 && Len == 0x39){
#ifdef TEST_DC_REC_NUM //just for test
                if (DC_REC_NUM_0x28 < 0xFFFF)
                {
                    DC_REC_NUM_0x28 ++;
                }
                printf("DC_REC_NUM_0x28(0x39): %d.\n", DC_REC_NUM_0x28);
#endif 

                receive_DC_module_data(1);
            }else if(Addr == 0x28 && Len == 0x72){
#ifdef TEST_DC_REC_NUM  //just for test
                if (DC_REC_NUM_0x28 < 0xFFFF)
                {
                    DC_REC_NUM_0x28 ++;
                }
                printf("DC_REC_NUM_0x28(0x72): %d.\n", DC_REC_NUM_0x28);
#endif 
                receive_DC_module_data_NUM12(1);  //取充电模块数据(12个充电模块)
            }else if(Addr == 0x29 && Len == 0x21){
                receive_DC_monitor_data(1);
            }else if(Addr == 0x2A){
                receive_DC_JY_data(1);
            }else if(Addr == 0x2B && Len == 0x84){
                receive_DC_Switch_data(1);
            }else if(Addr == 0x2C && Len == 0xDA
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //无独立的电池巡检: 数据从PSM-3读取
                receive_DC_Battery_data(1);
            }else if(Addr == 0x51 && Len == 0x80){
                receive_DC_FenGui_data(1);
            }else if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
            {
                if (Addr == 0xB1 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(1); //取电池相关数据: 电池电压 
                }
                else if (Addr == 0xB1 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(1); //取电池相关数据: 电池内阻
                }
                else if (Addr == 0xB1 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(1);  //取电池相关数据: 电池遥信量
                }                
            }
        }
        else if (group == 2)
        {
            if(Addr == 0x25 && Len == 0x39){
                receive_DC_module_data(2);
            }else if(Addr == 0x25 && Len == 0x72){
                receive_DC_module_data_NUM12(2);  //取充电模块数据(12个充电模块)
            }else if(Addr == 0x26 && Len == 0x21){
                receive_DC_monitor_data(2);
            }else if(Addr == 0x27){
                receive_DC_JY_data(2);
            }else if(Addr == 0x2D && Len == 0x84){
                receive_DC_Switch_data(2);
            }else if(Addr == 0x2E && Len == 0xDA
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //无独立的电池巡检: 数据从PSM-3读取
                receive_DC_Battery_data(2);
            }else if(Addr == 0x52 && Len == 0x80){
                receive_DC_FenGui_data(2);
            }else if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
            {
                if (Addr == 0xB2 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(2); //取电池相关数据: 电池电压 
                }
                else if (Addr == 0xB2 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(2); //取电池相关数据: 电池内阻
                }
                else if (Addr == 0xB2 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(2);  //取电池相关数据: 电池遥信量
                }                
            }
        }

#if 0   //“特殊界面”-“系统配置”-“直流系统”增加下拉选项“无”.  --转移位置
        if(Addr == 0x50 + group * 5 && Len == 0x01){
            //receive_DC_FD_data(group);		//分电
        }else if(Addr == 0x30 && Len == 0x8F){  //取第1组通信电源监控单元数据
            receive_Comm_data(0);
        }else if(Addr == 0x31 && Len == 0x8F){  //取第2组通信电源监控单元数据
            receive_Comm_data(1);
        }else if(Addr == 0x40 && Len == 0x42){  //取UPS监控单元数据
            receive_UPS_data();
        }else if(Addr == 0x80 && Len == 0x01){
            ;
        }else {;}
#endif 
    }

#if 0
    //特殊工程，通信电源和逆变电源挂在PSM-3监控下。通信电源地址 0x25
    //特殊工程，通信电源和逆变电源挂在PSM-3监控下。逆变电源地址 0x26
    if(Addr == 0x25 && Special_35KV_flag == 2){       
        receive_Comm_data_35K();
    }else if(Addr == 0x26 && Special_35KV_flag == 2){ 
        receive_UPS_data_35K();
    }else{;}
#else //新增两种有无通信、UPS电源监控的选择项. 
    //特殊工程，通信电源和逆变电源挂在PSM-3监控下。通信电源地址 0x25、逆变电源地址 0x26
    if(Addr == 0x25 && ((Special_35KV_flag == 2) 
                || (Special_35KV_flag_NoCommMon_WithUpsMon == 2)))
    {   //当无通信电源监控时    
        receive_Comm_data_35K();
    }
    else if(Addr == 0x26 && ((Special_35KV_flag == 2)
                || (Special_35KV_flag_NoUpsMon_WithCommMon == 2)))
    {   //当无UPS监控时 
        receive_UPS_data_35K();
    }    
#endif 

    //取ATSE数据
    if (Sys_cfg_info.ac_set.control_mode == 1){       //ATS(旭泰)
        if(Addr == ATSE_Addr_1 && Len == 0x01){       //ATSE_Addr_1 0x01
            receive_ATSE_state(0);
        }else if(Addr == ATSE_Addr_1 && Len == 0x03){
#if 0
            receive_ATSE_data(0);
#else    //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data(0);
            }
#endif 
        }else if(Addr == ATSE_Addr_2 && Len == 0x01){ //ATSE_Addr_2 0x02
            receive_ATSE_state(1);
        }else if(Addr == ATSE_Addr_2 && Len == 0x03){
#if 0
            receive_ATSE_data(1);
#else    //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data(1);
            }
#endif 
        }
    }
    else if (Sys_cfg_info.ac_set.control_mode == 3)   //ATS(韩光)  //新增和韩光ATS通讯支持.
    {
        if(Addr == ATSE_Addr_1 && Len == 0x03 && (Uart1Buf[2] == 0x14))
        {    
            receive_ATSE_state_HanKwang(0); //返回遥信数据(500~509)
        }
        else if(Addr == ATSE_Addr_1 && Len == 0x03 && (Uart1Buf[2] == 0x98))
        {
#if 0
            receive_ATSE_data_HanKwang(0);  //返回遥测数据(1000~1075)
#else    //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data_HanKwang(0);  //返回遥测数据(1000~1075)            
            }
#endif 
        }
        else if(Addr == ATSE_Addr_2 && Len == 0x03 && (Uart1Buf[2] == 0x14))
        {
            receive_ATSE_state_HanKwang(1);
        }
        else if(Addr == ATSE_Addr_2 && Len == 0x03 && (Uart1Buf[2] == 0x98))
        {
#if 0
            receive_ATSE_data_HanKwang(1);
#else   //定制功能：新增交流进线信息来源（有ATS）下拉框选择. 
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data_HanKwang(1);
            }
#endif 
        }    
    }
}
/**************************************************************************
 *函数名:	receive_ATSE_state(旭泰)
 *函数功能:	将接收的ATSE开关状态赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:		void
 *返回值:	void
 ***************************************************************************/
void receive_ATSE_state(unsigned char num)
{
    Ac_info.atse_stat[num].state = JudgeBitInByte(Uart1Buf[3],4); //自动／手动:	为1自动，为0手动
    if(JudgeBitInByte(Uart1Buf[3],0) == 1){       //1#开关状态: 为1合闸，为0分闸
        Ac_info.atse_stat[num].sw = 1;  //1路
    }else if(JudgeBitInByte(Uart1Buf[3],2) == 1){ //2#开关状态: 为1合闸，为0分闸
        Ac_info.atse_stat[num].sw = 2;  //2路
    }else{
        Ac_info.atse_stat[num].sw = 3;  //开闸
    }
}
/**************************************************************************
 *函数名:	receive_ATSE_data(旭泰)
 *函数功能:	将接收的ATSE开关数据赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:		void
 *返回值:	void
 ***************************************************************************/
void receive_ATSE_data(unsigned char num)
{
    unsigned int tmp_data = 0x00; //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    unsigned int buf_Num = 15; 
    int j=0;	
    num = num*2;
    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num].Voltage[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*10;		//一路进线电压ABC
        buf_Num +=2;
    }
    Ac_info.ac_in_data[num].Voltage[3] = (Ac_info.ac_in_data[num].Voltage[0]+Ac_info.ac_in_data[num].Voltage[1]+Ac_info.ac_in_data[num].Voltage[2])/3;  //电压合相

    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num+1].Voltage[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*10;		//二路进线电压ABC
        buf_Num +=2;
    }
    Ac_info.ac_in_data[num+1].Voltage[3] = (Ac_info.ac_in_data[num+1].Voltage[0]+Ac_info.ac_in_data[num+1].Voltage[1]+Ac_info.ac_in_data[num+1].Voltage[2])/3;  //电流合相

#if 0
    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num].Current[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//一路进线电流 A
        Ac_info.ac_in_data[num+1].Current[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//二路进线电流 A
        buf_Num +=2;
    }
#else //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS). 
    for(j = 0; j < 3; j++)
    {
        //进线电流
        tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;  
        buf_Num += 0x02;

        Ac_info.ac_in_data[num].Current[j] = 0x00;     //一路进线电流 A
        Ac_info.ac_in_data[num + 1].Current[j] = 0x00; //二路进线电流 A  
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].Current[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].Current[j] = tmp_data;
        } 
    }
#endif 

    for(j=0;j<4;j++){
        Ac_info.ac_in_data[num].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//一路进线频率
    }
    buf_Num +=2;
    for(j=0;j<4;j++){
        Ac_info.ac_in_data[num+1].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//二路进线频率
    }
    buf_Num +=2;

#if 0
    Ac_info.ac_in_data[num].ActivePower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//一路进线有功功率;
    Ac_info.ac_in_data[num+1].ActivePower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//二路进线有功功率;
    Ac_info.ac_in_data[num].ActivePower[0] = Ac_info.ac_in_data[num].ActivePower[1] = Ac_info.ac_in_data[num].ActivePower[2] = Ac_info.ac_in_data[num].ActivePower[3]/3;
    Ac_info.ac_in_data[num+1].ActivePower[0] = Ac_info.ac_in_data[num+1].ActivePower[1] = Ac_info.ac_in_data[num+1].ActivePower[2] = Ac_info.ac_in_data[num+1].ActivePower[3]/3;
    buf_Num +=2;

    Ac_info.ac_in_data[num].ApparentPower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//一路进线视在功率
    Ac_info.ac_in_data[num+1].ApparentPower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//二路进线视在功率
    Ac_info.ac_in_data[num].ApparentPower[0] = Ac_info.ac_in_data[num].ApparentPower[1] = Ac_info.ac_in_data[num].ApparentPower[2] = Ac_info.ac_in_data[num].ApparentPower[3]/3;
    Ac_info.ac_in_data[num+1].ApparentPower[0] = Ac_info.ac_in_data[num+1].ApparentPower[1] = Ac_info.ac_in_data[num+1].ApparentPower[2] = Ac_info.ac_in_data[num+1].ApparentPower[3]/3;
    buf_Num +=2;		

    Ac_info.ac_in_data[num].ReactivePower[3] = Ac_info.ac_in_data[num].ApparentPower[3] - Ac_info.ac_in_data[num].ActivePower[3];
    Ac_info.ac_in_data[num+1].ReactivePower[3] = Ac_info.ac_in_data[num+1].ApparentPower[3] - Ac_info.ac_in_data[num+1].ActivePower[3];
    Ac_info.ac_in_data[num].ReactivePower[0] = Ac_info.ac_in_data[num].ReactivePower[1] = Ac_info.ac_in_data[num].ReactivePower[2] = Ac_info.ac_in_data[num].ReactivePower[3]/3;
    Ac_info.ac_in_data[num+1].ReactivePower[0] = Ac_info.ac_in_data[num+1].ReactivePower[1] = Ac_info.ac_in_data[num+1].ReactivePower[2] = Ac_info.ac_in_data[num+1].ReactivePower[3]/3;

    Ac_info.ac_in_data[num].PowerFactor[3]	= ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]);		//一路进线功率因数
    Ac_info.ac_in_data[num+1].PowerFactor[3]	= ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]);		//二路进线功率因数
#else  //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS). 
    //有功功率 -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;	
    buf_Num += 0x02;

    //合相
    Ac_info.ac_in_data[num].ActivePower[3] = 0x00;
    Ac_info.ac_in_data[num + 1].ActivePower[3] = 0x00;
    if (Ac_info.atse_stat[num / 2].sw == 0x01)
    {
        Ac_info.ac_in_data[num].ActivePower[3] = tmp_data;
    }
    else if (Ac_info.atse_stat[num / 2].sw == 0x02)
    {
        Ac_info.ac_in_data[num + 1].ActivePower[3] = tmp_data;
    }

    //ABC相 
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ActivePower[j] = Ac_info.ac_in_data[num].ActivePower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ActivePower[j] = Ac_info.ac_in_data[num + 1].ActivePower[3] / 3;    
    }    

    //视在功率 -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;	
    buf_Num += 0x02;

    //合相
    Ac_info.ac_in_data[num].ApparentPower[3] = 0x00;
    Ac_info.ac_in_data[num + 1].ApparentPower[3] = 0x00;
    if (Ac_info.atse_stat[num / 2].sw == 0x01)
    {
        Ac_info.ac_in_data[num].ApparentPower[3] = tmp_data;
    }
    else if (Ac_info.atse_stat[num / 2].sw == 0x02)
    {
        Ac_info.ac_in_data[num + 1].ApparentPower[3] = tmp_data;
    } 
    //ABC相
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ApparentPower[j] = Ac_info.ac_in_data[num].ApparentPower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ApparentPower[j] = Ac_info.ac_in_data[num + 1].ApparentPower[3] / 3;    
    }

    //无功功率 -----------------------------------------------------------
#if 0
    //合相
    Ac_info.ac_in_data[num].ReactivePower[3] = Ac_info.ac_in_data[num].ApparentPower[3] - Ac_info.ac_in_data[num].ActivePower[3];
    Ac_info.ac_in_data[num + 1].ReactivePower[3] = Ac_info.ac_in_data[num + 1].ApparentPower[3] - Ac_info.ac_in_data[num + 1].ActivePower[3];    
#else  //修复有功无功视在功率计算问题.
    Ac_info.ac_in_data[num].ReactivePower[3] = 
        sqrt(Ac_info.ac_in_data[num].ApparentPower[3] * Ac_info.ac_in_data[num].ApparentPower[3]  
                - Ac_info.ac_in_data[num].ActivePower[3] * Ac_info.ac_in_data[num].ActivePower[3]);
    Ac_info.ac_in_data[num + 1].ReactivePower[3] = 
        sqrt(Ac_info.ac_in_data[num + 1].ApparentPower[3] * Ac_info.ac_in_data[num + 1].ApparentPower[3] 
                - Ac_info.ac_in_data[num + 1].ActivePower[3] * Ac_info.ac_in_data[num + 1].ActivePower[3]); 
#endif 
    //ABC相
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ReactivePower[j] = Ac_info.ac_in_data[num].ReactivePower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ReactivePower[j] = Ac_info.ac_in_data[num + 1].ReactivePower[3] / 3;    
    }

    //功率因数 -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]);	
    buf_Num += 0x02;

    //合相
    Ac_info.ac_in_data[num].PowerFactor[3] = 0x00;
    Ac_info.ac_in_data[num + 1].PowerFactor[3] = 0x00;
    if (Ac_info.atse_stat[num / 2].sw == 0x01)
    {
        Ac_info.ac_in_data[num].PowerFactor[3] = tmp_data;
    }
    else if (Ac_info.atse_stat[num / 2].sw == 0x02)
    {
        Ac_info.ac_in_data[num + 1].PowerFactor[3] = tmp_data;
    } 
#endif 
}

/**************************************************************************
 *函数名:	receive_ATSE_state_HanKwang(韩光)(返回遥信数据500~509)
 *函数功能:	将接收的ATSE开关状态赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:		void
 *返回值:	void
 ***************************************************************************/
void receive_ATSE_state_HanKwang(unsigned char num)
{
    //500
    //自动／手动:	为1自动，为0手动
    Ac_info.atse_stat[num].state = JudgeBitInByte(Uart1Buf[3],0); 

    //507
    if (JudgeBitInByte(Uart1Buf[18],4) == 1) //1#开关状态: 为1合闸，为0分闸
    {           
        Ac_info.atse_stat[num].sw = 1;  //1路
    }
    else if (JudgeBitInByte(Uart1Buf[18],5) == 1) //2#开关状态: 为1合闸，为0分闸
    { 
        Ac_info.atse_stat[num].sw = 2;  //2路
    }
    else
    {
        Ac_info.atse_stat[num].sw = 3;  //开闸
    }
}
/**************************************************************************
 *函数名:	receive_ATSE_data_HanKwang(韩光)(返回遥测数据1000~1075)
 *函数功能:	将接收的ATSE开关数据赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:		void
 *返回值:	void
 ***************************************************************************/
void receive_ATSE_data_HanKwang(unsigned char num)
{
    unsigned int buf_Num = 3; 
    int i = 0;
    int j = 0;	
    num = num * 2;
    unsigned int tmp_data = 0x00; //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).

    //1000: 
    buf_Num += 0x06;

    //1003~5: 电压UA1 UB1 UC1
    for (j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num].Voltage[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10; //一路进线电压ABC
        buf_Num += 0x02;
    }
    Ac_info.ac_in_data[num].Voltage[3] = (Ac_info.ac_in_data[num].Voltage[0]
            + Ac_info.ac_in_data[num].Voltage[1] + Ac_info.ac_in_data[num].Voltage[2]) / 3;  //电压合相

    //1006~8
    buf_Num += 0x06;

    //1009: 频率1  待调试！！！！！
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //负值时作为0处理
            Ac_info.ac_in_data[num].Frequency[j] = 0x00; 
        }
        else //正值时正常处理
        {
            Ac_info.ac_in_data[num].Frequency[j] 
                = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) / 10; //一路进线频率
        }        
    }
    buf_Num += 0x02;

    //1010~1019
    buf_Num += 0x14;

    //1020:
    buf_Num += 0x06;

    //---------------------------------------------------
    //1023~1025: 电压UA2 UB2 UC2
    for (j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num + 1].Voltage[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10; //二路进线电压ABC
        buf_Num += 2;
    }
    Ac_info.ac_in_data[num + 1].Voltage[3] = (Ac_info.ac_in_data[num + 1].Voltage[0]
            + Ac_info.ac_in_data[num + 1].Voltage[1] + Ac_info.ac_in_data[num + 1].Voltage[2]) / 3;  //电流合相

    //1026~1028
    buf_Num += 0x06;

    //1029: 频率2   待调试！！！！！
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //负值时作为0处理
            Ac_info.ac_in_data[num + 1].Frequency[j] = 0x00; 
        }
        else //正值时正常处理
        {
            Ac_info.ac_in_data[num + 1].Frequency[j] 
                = ((Uart1Buf[buf_Num] << 8)| Uart1Buf[buf_Num + 1]) / 10; //二路进线频率
        }
    }
    buf_Num += 0x02;

    //---------------------------------------------------
    //1030~1039
    buf_Num += 0x14;

    //1040~1042
    //ABC相电流
#if 0
    for(j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num].Current[j] = ((Uart1Buf[buf_Num] << 8) 
                | Uart1Buf[buf_Num + 1]) * 10;	   //一路进线电流 A
        Ac_info.ac_in_data[num + 1].Current[j] = ((Uart1Buf[buf_Num] << 8)
                | Uart1Buf[buf_Num + 1]) * 10;     //二路进线电流 A
        buf_Num += 0x02;
    }
#else   //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    for(j = 0; j < 3; j++)
    {
        //进线电流 
        tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        buf_Num += 0x02;

        Ac_info.ac_in_data[num].Current[j] = 0x00;     //一路进线电流 A
        Ac_info.ac_in_data[num + 1].Current[j] = 0x00; //二路进线电流 A  
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].Current[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].Current[j] = tmp_data;
        } 
    }
#endif  

    //1043~1047
    buf_Num += 0x0A;

    //1048~1055: 有功功率: 16字节(4*4)  待调试！！！！！
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //负值时作为0处理
            Ac_info.ac_in_data[num].ActivePower[j] = 0x00;
        }
        else //正值时正常处理 
        {   //修复韩光ATS通讯的有功无功功率解析问题.
            Ac_info.ac_in_data[num].ActivePower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //一路进线有功功率
        }

        Ac_info.ac_in_data[num + 1].ActivePower[j] = Ac_info.ac_in_data[num].ActivePower[j];                   //二路进线有功功率
        buf_Num += 0x04;
    }
#else   //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    for (j = 0; j < 4; j++)
    {
        //有功功率 
        if (Uart1Buf[buf_Num] & 0x80) //负值时作为0处理
        {
            tmp_data = 0x00;   
        }
        else //正值时正常处理 
        {   
            tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        }        
        buf_Num += 0x04;

        Ac_info.ac_in_data[num].ActivePower[j] = 0x00;
        Ac_info.ac_in_data[num + 1].ActivePower[j] = 0x00;
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].ActivePower[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].ActivePower[j] = tmp_data;
        } 
    }
#endif      

    //1056~1063: 无功功率: 16字节(4*4)  待调试！！！！！
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //负值时作为0处理
            Ac_info.ac_in_data[num].ReactivePower[j] = 0x00;
        }
        else //正值时正常处理 
        {    //修复韩光ATS通讯的有功无功功率解析问题.
            Ac_info.ac_in_data[num].ReactivePower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //一路进线无功功率
        }

        Ac_info.ac_in_data[num + 1].ReactivePower[j] = Ac_info.ac_in_data[num].ReactivePower[j];               //二路进线无功功率
        buf_Num += 0x04;
    }
#else   //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    for (j = 0; j < 4; j++)
    {
        //无功功率
        if (Uart1Buf[buf_Num] & 0x80) //负值时作为0处理
        {
            tmp_data = 0x00;   
        }
        else //正值时正常处理 
        {   
            tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        }        
        buf_Num += 0x04;

        Ac_info.ac_in_data[num].ReactivePower[j] = 0x00;
        Ac_info.ac_in_data[num + 1].ReactivePower[j] = 0x00;
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].ReactivePower[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].ReactivePower[j] = tmp_data;
        } 
    }
#endif    

    //1064~1071: 视在功率: 16字节(4*4)   待调试！！！！！
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //负值时作为0处理
            Ac_info.ac_in_data[num].ApparentPower[j] = 0x00;
        }
        else //正值时正常处理 
        {    //修复韩光ATS通讯的有功无功功率解析问题.
            Ac_info.ac_in_data[num].ApparentPower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //一路进线视在功率
        }

        Ac_info.ac_in_data[num + 1].ApparentPower[j] = Ac_info.ac_in_data[num].ApparentPower[j];               //二路进线视在功率
        buf_Num += 0x04;
    }
#else   //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    for (j = 0; j < 4; j++)
    {
        //视在功率
        if (Uart1Buf[buf_Num] & 0x80) //负值时作为0处理
        {
            tmp_data = 0x00;   
        }
        else //正值时正常处理 
        {   
            tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        }        
        buf_Num += 0x04;

        Ac_info.ac_in_data[num].ApparentPower[j] = 0x00;
        Ac_info.ac_in_data[num + 1].ApparentPower[j] = 0x00;
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].ApparentPower[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].ApparentPower[j] = tmp_data;
        } 
    }
#endif   

    //1072~1075: 功率因数(传过来数据*100 * 10 / 1000处理显示)         
#if 0
    for (j = 0; j < 4; j++)
    {
        Ac_info.ac_in_data[num].PowerFactor[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;     //一路进线功率因数
        Ac_info.ac_in_data[num + 1].PowerFactor[j] = Ac_info.ac_in_data[num].PowerFactor[j];                  //二路进线功率因数
        buf_Num += 0x02;
    }
#else   //根据ATS合闸哪一路做电流、有功、无功、视在、功率因数处理(旭泰+韩光ATS).
    for (j = 0; j < 4; j++)
    {
        //功率因数
        tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        buf_Num += 0x02;

        Ac_info.ac_in_data[num].PowerFactor[j] = 0x00;
        Ac_info.ac_in_data[num + 1].PowerFactor[j] = 0x00;
        if (Ac_info.atse_stat[num / 2].sw == 0x01)
        {
            Ac_info.ac_in_data[num].PowerFactor[j] = tmp_data;
        }
        else if (Ac_info.atse_stat[num / 2].sw == 0x02)
        {
            Ac_info.ac_in_data[num + 1].PowerFactor[j] = tmp_data;
        }
    } 
#endif   
}

/**************************************************************************
 *函数名:			receive_AC_data   0X10(0xC4)
 *函数功能:		将接收的交流监控数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			void
 *返回值:			void
 ***************************************************************************/
void receive_AC_data(){
    unsigned int buf_Num = 2; 
    int i =0,j=0;

#if 1  //修复和后台之间的3个电操的遥控指令:后台遥控电操逻辑调整
    for (i = 0; i < 5; i ++)
    {
        if (diancao_cmd_num[i] > 0x00)
        {
            diancao_cmd_num[i] --;
        }
    }
#endif 
    if ((Sys_cfg_info.ac_set.control_mode != 1) 
            && (Sys_cfg_info.ac_set.control_mode != 3))  //新增和韩光ATS通讯支持.
    {    //工作方式，不是ATS工作方式时，进入下面的解析，方式是在交流设置界面设置的
        for(i=0;i<4;i++){
            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//一路进线电压
                buf_Num +=2;
            }
            for(j=0;j<3;j++)
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//频率(报文中只有一个频率数据，为了显示3个，把同一个值放到3个位置上，for是为了分位置)		
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//一路进线电流
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//总有功功率
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//有功功率
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//总无功功率
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//无功功率
                buf_Num +=2;
            }

#if 0
            Ac_info.ac_in_data[i].ApparentPower[3] = Ac_info.ac_in_data[i].ReactivePower[3] + Ac_info.ac_in_data[i].ActivePower[3];  //视在功率
#else       //修复有功无功视在功率计算问题.
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);  //视在功率                        
            }
#endif 

            Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//功率因数
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//功率因数
                buf_Num +=2;
            }
        }
    }
#if 1   //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
    else if (Sys_cfg_info.ac_set.source_of_ac_data != 0x00)   //有ATS时,不是从ATS读(从交流监控读)
    {
        for(i = 0; i < 2; i++)  //根据两个电表数据、两个ATS状态转换出四路进线数据
        {
            //一路进线电压
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3路电压=1路电压	    
                Ac_info.ac_in_data[i + 2].Voltage[j] = Ac_info.ac_in_data[i].Voltage[j];      
                buf_Num +=2;
            }

            //频率
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3路频率=1路频率
                Ac_info.ac_in_data[i + 2].Frequency[j] = Ac_info.ac_in_data[i].Frequency[j]; 
            }                
            buf_Num +=2;

            //-------------------------------------
            //先清零13、24路数据
            //进线电流
            for(j = 0; j < 3;j++)
            {
                Ac_info.ac_in_data[i].Current[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].Current[j] = 0x00;	   
            }

            //有功功率
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ActivePower[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].ActivePower[j] = 0x00;
            }

            //无功功率
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ReactivePower[j] = 0x00;
                Ac_info.ac_in_data[i + 2].ReactivePower[j] = 0x00;
            }

            //功率因数
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].PowerFactor[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].PowerFactor[j] = 0x00;
            }

            //-------------------------------------
            //根据ATS1、2开合闸状态处理4路电流数据
            if ((Ac_info.atse_stat[0].sw == (0x01 + i))         //ATS1--1路合闸 或 ATS1--2路合闸
                    && (Ac_info.atse_stat[1].sw == (0x01 + i))) //ATS2--1路合闸 或 ATS2--2路合闸
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].Current[j] = Ac_info.ac_in_data[i].Current[j];
                }

                //总有功功率
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ActivePower[3] = Ac_info.ac_in_data[i].ActivePower[3];

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = Ac_info.ac_in_data[i].ActivePower[j];
                }

                //总无功功率
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = Ac_info.ac_in_data[i].ReactivePower[3];

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = Ac_info.ac_in_data[i].ReactivePower[j];
                }

                //功率因数
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = Ac_info.ac_in_data[i].PowerFactor[3];
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = Ac_info.ac_in_data[i].PowerFactor[j];
                }
            }
            else if (Ac_info.atse_stat[0].sw == (0x01 + i)) //只有(ATS1--1路合闸 或 ATS1--2路合闸)
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总有功功率
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总无功功率
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //功率因数
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else if (Ac_info.atse_stat[1].sw == (0x01 + i)) //只有(ATS2--1路合闸 或 ATS2--2路合闸)
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总有功功率
                Ac_info.ac_in_data[i + 2].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总无功功率
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //功率因数
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else  //ATS1开闸 或 ATS2开闸
            {
                buf_Num += 30;
            }

            //---------------------------------------------
            //视在功率 
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);   

                Ac_info.ac_in_data[i + 2].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i + 2].ReactivePower[j] * Ac_info.ac_in_data[i + 2].ReactivePower[j] 
                            + Ac_info.ac_in_data[i + 2].ActivePower[j] * Ac_info.ac_in_data[i + 2].ActivePower[j]);  
            }
        }            
    }
#endif 

    buf_Num = 154;
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线电压
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线电流
        buf_Num +=2;
    }
    Ac_info.busbar[0].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线零序电流
    buf_Num +=2;

    for(i=0;i<3;i++){
        Ac_info.busbar[1].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[1].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
        buf_Num +=2;
    }
    Ac_info.busbar[1].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
    buf_Num +=2;

    Ac_info.ac_measure_I[0] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];					//测量电流
    buf_Num +=2;
    Ac_info.ac_measure_I[1] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;
    Ac_info.ac_measure_I[2] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;


    Ac_info.ac_in_data[0].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],0);		//进线开关电操状态		
    Ac_info.ac_in_data[1].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],1);
    if(Sys_cfg_info.ac_set.diancao_num == 3){       //电操3
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],2);
    }else if(Sys_cfg_info.ac_set.diancao_num == 5){ //电操5
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//进线开关电操状态		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],4);
    }else{ //其他情况: ATS控制或(电操不是3和5)
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//进线开关电操状态		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        ERR_AC_in_SW_trip[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);               //进线开关电操跳闸
    }

    ERR_AC_in_SW_trip[0] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_AC_in_SW_trip[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_in_SW_trip[2] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;

    ERR_AC_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);							//防雷器故障
    ERR_AC_SPD[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<6;i++)
        ERR_AC_in_V[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//进线电压异常
    ERR_AC_mu_V[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_mu_V[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);							//母线电压异常
    buf_Num++;

    ERR_AC_AcSample_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);				//交流采样通讯故障
    ERR_AC_AcSample_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_SW_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//交流开关量通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_SW_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_SW_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_SW_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);	
    for(i=2;i<8;i++)
        ERR_AC_St_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//交流状态量通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_St_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_St_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_St_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_CurrentSample_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);	//交流电流采样单元通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_CurrentSample_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    for(i=0;i<6;i++)
        ERR_AC_CurrentSample_comm[i+14] = JudgeBitInByte(Uart1Buf[buf_Num],i);
#if 0
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);					//电表或者ATSE通讯异常
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
#else  //修复总监控和后台不报电表1、2通讯故障的问题. 
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);					//电表或者ATSE通讯异常
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
#endif 

    buf_Num++;
    ERR_AC_Feeder_Duan_trip_1 = JudgeBitInByte(Uart1Buf[buf_Num],0); 			//一段馈线跳闸
    ERR_AC_Feeder_Duan_trip_2 = JudgeBitInByte(Uart1Buf[buf_Num],1);

    ERR_AC_SPD[2] = JudgeBitInByte(Uart1Buf[buf_Num],2);                 //3#防雷器故障
    ERR_AC_Meter_comm[2] = JudgeBitInByte(Uart1Buf[buf_Num],3);					//电表通讯异常
    ERR_AC_Meter_comm[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);	
}
/**************************************************************************
 *函数名:			receive_AC_dc_data   0X10(0xC6)
 *函数功能:		将接收的交流监控数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			void
 *返回值:			void
 ***************************************************************************/
void receive_AC_dc_data(){
    unsigned int buf_Num = 2; 
    int i =0,j=0;

    if ((Sys_cfg_info.ac_set.control_mode != 1)
            && (Sys_cfg_info.ac_set.control_mode != 3))  //新增和韩光ATS通讯支持.
    {    //工作方式，不是ATS工作方式时，进入下面的解析，方式是在交流设置界面设置的
        for(i=0;i<4;i++){
            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//一路进线电压
                buf_Num +=2;
            }
            for(j=0;j<3;j++)
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//频率(报文中只有一个频率数据，为了显示3个，把同一个值放到3个位置上，for是为了分位置)		
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//一路进线电流
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//总有功功率
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//有功功率
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//总无功功率
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//无功功率
                buf_Num +=2;
            }

#if 0
            Ac_info.ac_in_data[i].ApparentPower[3] = Ac_info.ac_in_data[i].ReactivePower[3] + Ac_info.ac_in_data[i].ActivePower[3];  //视在功率
#else       //修复有功无功视在功率计算问题.
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);  //视在功率                        
            }
#endif 

            Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//功率因数
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//功率因数
                buf_Num +=2;
            }
        }
    }
#if 1   //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
    else if (Sys_cfg_info.ac_set.source_of_ac_data != 0x00)   //有ATS时,不是从ATS读(从交流监控读)
    {
        for(i = 0; i < 2; i++)  //根据两个电表数据、两个ATS状态转换出四路进线数据
        {
            //一路进线电压
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3路电压=1路电压	    
                Ac_info.ac_in_data[i + 2].Voltage[j] = Ac_info.ac_in_data[i].Voltage[j];      
                buf_Num +=2;
            }

            //频率
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3路频率=1路频率
                Ac_info.ac_in_data[i + 2].Frequency[j] = Ac_info.ac_in_data[i].Frequency[j]; 
            }                
            buf_Num +=2;

            //-------------------------------------
            //先清零13、24路数据
            //进线电流
            for(j = 0; j < 3;j++)
            {
                Ac_info.ac_in_data[i].Current[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].Current[j] = 0x00;	   
            }

            //有功功率
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ActivePower[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].ActivePower[j] = 0x00;
            }

            //无功功率
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ReactivePower[j] = 0x00;
                Ac_info.ac_in_data[i + 2].ReactivePower[j] = 0x00;
            }

            //功率因数
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].PowerFactor[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].PowerFactor[j] = 0x00;
            }

            //-------------------------------------
            //根据ATS1、2开合闸状态处理4路电流数据
            if ((Ac_info.atse_stat[0].sw == (0x01 + i))         //ATS1--1路合闸 或 ATS1--2路合闸
                    && (Ac_info.atse_stat[1].sw == (0x01 + i))) //ATS2--1路合闸 或 ATS2--2路合闸
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].Current[j] = Ac_info.ac_in_data[i].Current[j];
                }

                //总有功功率
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ActivePower[3] = Ac_info.ac_in_data[i].ActivePower[3];

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = Ac_info.ac_in_data[i].ActivePower[j];
                }

                //总无功功率
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = Ac_info.ac_in_data[i].ReactivePower[3];

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = Ac_info.ac_in_data[i].ReactivePower[j];
                }

                //功率因数
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = Ac_info.ac_in_data[i].PowerFactor[3];
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = Ac_info.ac_in_data[i].PowerFactor[j];
                }
            }
            else if (Ac_info.atse_stat[0].sw == (0x01 + i)) //只有(ATS1--1路合闸 或 ATS1--2路合闸)
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总有功功率
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总无功功率
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //功率因数
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else if (Ac_info.atse_stat[1].sw == (0x01 + i)) //只有(ATS2--1路合闸 或 ATS2--2路合闸)
            {
                //进线电流
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总有功功率
                Ac_info.ac_in_data[i + 2].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //有功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //总无功功率
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //无功功率
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //功率因数
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //功率因数
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else  //ATS1开闸 或 ATS2开闸
            {
                buf_Num += 30;
            }

            //---------------------------------------------
            //视在功率 
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);   

                Ac_info.ac_in_data[i + 2].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i + 2].ReactivePower[j] * Ac_info.ac_in_data[i + 2].ReactivePower[j] 
                            + Ac_info.ac_in_data[i + 2].ActivePower[j] * Ac_info.ac_in_data[i + 2].ActivePower[j]);  
            }
        }            
    }
#endif 

    buf_Num = 154;
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线电压
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线电流
        buf_Num +=2;
    }
    Ac_info.busbar[0].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//一段母线零序电流
    buf_Num +=2;

    for(i=0;i<3;i++){
        Ac_info.busbar[1].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[1].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
        buf_Num +=2;
    }
    Ac_info.busbar[1].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//二段母线
    buf_Num +=2;

    Ac_info.ac_measure_I[0] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];					//测量电流
    buf_Num +=2;
    Ac_info.ac_measure_I[1] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;
    Ac_info.ac_measure_I[2] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;


    Ac_info.ac_in_data[0].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],0);		//进线开关电操状态		
    Ac_info.ac_in_data[1].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],1);
    if(Sys_cfg_info.ac_set.diancao_num == 3){       //电操3
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],2);
    }else if(Sys_cfg_info.ac_set.diancao_num == 5){ //电操5
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//进线开关电操状态		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],4);
    }else{  //其他情况: ATS控制或(电操不是3和5)
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//进线开关电操状态		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        ERR_AC_in_SW_trip[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);               //进线开关电操跳闸
    }

    ERR_AC_in_SW_trip[0] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_AC_in_SW_trip[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_in_SW_trip[2] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;

    ERR_AC_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);							//防雷器故障
    ERR_AC_SPD[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<6;i++)
        ERR_AC_in_V[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//进线电压异常
    ERR_AC_mu_V[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_mu_V[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);							//母线电压异常
    buf_Num++;

    ERR_AC_AcSample_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);				//交流采样通讯故障
    ERR_AC_AcSample_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_SW_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//交流开关量通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_SW_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_SW_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_SW_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);	
    for(i=2;i<8;i++)
        ERR_AC_St_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//交流状态量通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_St_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_St_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_St_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_CurrentSample_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);	//交流电流采样单元通讯故障
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_CurrentSample_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    for(i=0;i<6;i++)
        ERR_AC_CurrentSample_comm[i+14] = JudgeBitInByte(Uart1Buf[buf_Num],i);

#if 0
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);					//电表或者ATSE通讯异常
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
#else  //修复总监控和后台不报电表1、2通讯故障的问题. 
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);					//电表或者ATSE通讯异常
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
#endif 

    buf_Num++;
    ERR_AC_Feeder_Duan_trip_1 = JudgeBitInByte(Uart1Buf[buf_Num],0); 			//一段馈线跳闸
    ERR_AC_Feeder_Duan_trip_2 = JudgeBitInByte(Uart1Buf[buf_Num],1);

    ERR_AC_SPD[2] = JudgeBitInByte(Uart1Buf[buf_Num],2);                 //3#防雷器故障
    ERR_AC_Meter_comm[2] = JudgeBitInByte(Uart1Buf[buf_Num],3);					//电表通讯异常
    ERR_AC_Meter_comm[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);
    ERR_AC_BT_device =	JudgeBitInByte(Uart1Buf[buf_Num],5);       //备投装置故障
    Ac_info.BTDZ_device[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);    //1#备投装置备投动作
    Ac_info.BTDZ_device[1]= JudgeBitInByte(Uart1Buf[buf_Num],7);    //2#备投装置备投动作
    buf_Num++;
    ERR_AC_BT_device_he[0]  = JudgeBitInByte(Uart1Buf[buf_Num],0);    //1#电池合闸失败
    ERR_AC_BT_device_fen[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);    //1#电池分闸失败
    ERR_AC_BT_device_he[1]  = JudgeBitInByte(Uart1Buf[buf_Num],2);    //2#电池合闸失败
    ERR_AC_BT_device_fen[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);    //2#电池分闸失败
    ERR_AC_BT_device_he[2]  = JudgeBitInByte(Uart1Buf[buf_Num],4);    //3#电池合闸失败
    ERR_AC_BT_device_fen[2] = JudgeBitInByte(Uart1Buf[buf_Num],5);    //3#电池分闸失败
    ERR_AC_BT_device_he[3]  = JudgeBitInByte(Uart1Buf[buf_Num],6);    //4#电池合闸失败
    ERR_AC_BT_device_fen[3] = JudgeBitInByte(Uart1Buf[buf_Num],7);    //4#电池分闸失败
}


/**************************************************************************
 *函数名:			receive_AC_FeederLine_data   0X11
 *函数功能:		接收处理交流馈线状态和告警信号。
 *参数:			void
 *返回值:			void
 ***************************************************************************/
void receive_AC_FeederLine_data(){
    unsigned int buf_Num = 2;     //前面2个字节是地址跟长度
    int i = 0;
    int j = 0;
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            ERR_AC_Feeder_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);         //跳闸第一个字节
            ERR_AC_Feeder_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            ERR_AC_Feeder_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            ERR_AC_Feeder_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }

    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            Ac_info.feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);     //馈线状态第一个字节
            Ac_info.feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            Ac_info.feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            Ac_info.feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
}

/**************************************************************************
 *函数名:			receive_DC_module_data    0X20/0X28/0X25
 *函数功能:		将接收的直流监控充电模块数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			unsigned int group	直流组数
 *返回值:			void
 ***************************************************************************/
void receive_DC_module_data(unsigned int group)    //group 0,1,2
{
    unsigned int buf_Num = 2;
    int i = 0;
#if 0  //修复上报后台的电池均充信号处理.
    //battery_current_state[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
#endif 
    for(i = 0; i<8;i++){    //i表示模块数
        Dc_info[group].module[i].power_supply_mode = JudgeBitInByte(Uart4Buf[buf_Num],0);   //模块供电方式
        ERR_DC_Module[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],4);
        if(JudgeBitInByte(Uart4Buf[buf_Num],1) == 0){								//模块开机
            if(JudgeBitInByte(Uart4Buf[buf_Num],4) == 1){								//模块故障
                Dc_info[group].module[i].state = 2;  //界面上2是显示故障
            }else
                Dc_info[group].module[i].state = 1;   //界面上1是显示运行
        }else																			//模块关机
            Dc_info[group].module[i].state = 0;   //界面上0是关机
        buf_Num ++;
        Dc_info[group].module[i].current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出电流
        buf_Num += 2;
        Dc_info[group].module[i].voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出电压
        buf_Num += 2;
        Dc_info[group].module[i].temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出温度
        buf_Num += 2;
    }
    for(i=0;i<8;i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);		//模块通信故障
}

/**************************************************************************
 *函数名:	  receive_DC_module_data_NUM12    0X20/0X28/0X25
 *函数功能:	  将接收的直流监控充电模块数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:		  unsigned int group	直流组数
 *返回值:	  void
 ***************************************************************************/
void receive_DC_module_data_NUM12(unsigned int group)    //group 0,1,2
{
    unsigned int buf_Num = 2;
    int i = 0;
#if 0  //修复上报后台的电池均充信号处理.
    //battery_current_state[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
#endif 
    for(i = 0; i < 12; i++)  //i表示模块数
    {    
        Dc_info[group].module[i].power_supply_mode = JudgeBitInByte(Uart4Buf[buf_Num],0);   //模块供电方式
        ERR_DC_Module[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],4);
        if(JudgeBitInByte(Uart4Buf[buf_Num],1) == 0){								//模块开机
            if(JudgeBitInByte(Uart4Buf[buf_Num],4) == 1){								//模块故障
                Dc_info[group].module[i].state = 2;  //界面上2是显示故障
            }else
                Dc_info[group].module[i].state = 1;   //界面上1是显示运行
        }else																			//模块关机
            Dc_info[group].module[i].state = 0;   //界面上0是关机
        buf_Num ++;
        Dc_info[group].module[i].current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出电流
        buf_Num += 2;
        Dc_info[group].module[i].voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出电压
        buf_Num += 2;
        Dc_info[group].module[i].temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//输出温度
        buf_Num += 2;
    }

    //最后4个模块信息忽略
    buf_Num += (7 * 4);

    for(i=0;i<8;i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);		//模块通信故障
    buf_Num ++;

    //第8~11模块通信故障 
    for(i = 8; i < 12; i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],(i - 8));//模块通信故障
}

/**************************************************************************
 *函数名:			receive_DC_monitor_data  0X21/0X29/0X26
 *函数功能:		将接收的直流监控母线数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			unsigned int group	直流组数
 *返回值:			void
 ***************************************************************************/
void receive_DC_monitor_data(unsigned int group)
{
    static INT8U Num_battery_discharging[3] = {0};  //放电
    static INT8U Num_battery_charging[3] = {0};           

    unsigned int buf_Num = 2;
    int i = 0;				
    for(i = 0;i<2;i++){																	// 1路、2路
        Dc_info[group].input.voltage_A[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];			//A相输入电压
        buf_Num += 2;
        Dc_info[group].input.voltage_B[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];
        buf_Num += 2;
        Dc_info[group].input.voltage_C[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];
        buf_Num += 2;
    }
    Dc_info[group].input.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //输出电流
    buf_Num += 2;
    Dc_info[group].input.state = Uart4Buf[buf_Num];
    ERR_DC_Ac1_state[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],0); //=0 闭合  =1 断开
    ERR_DC_Ac1_state[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    buf_Num ++;		//BYTE15
    buf_Num ++;		//BYTE16

    Dc_info[group].busbar.battery_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//电池电压
    Dc_info[group].battery.voltage = Dc_info[group].busbar.battery_v;
    buf_Num += 2;
    Dc_info[group].busbar.switching_busbar_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//合母电压、充电机电压
    Dc_info[group].busbar.charger_v = Dc_info[group].busbar.switching_busbar_v;
    buf_Num += 2;
    Dc_info[group].busbar.control_busbar_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//控母电压
    buf_Num += 2;

#if 0
    Dc_info[group].busbar.battery_i = ((Uart4Buf[buf_Num]&0x7f)<<8)|Uart4Buf[buf_Num+1];		//电池电流
#else  //保留最高位(符号位) 
    Dc_info[group].busbar.battery_i = ((Uart4Buf[buf_Num])<<8)|Uart4Buf[buf_Num+1];		//电池电流
#endif 

    Dc_info[group].battery.current = Dc_info[group].busbar.battery_i;

#if 0  //转移位置: 放电标志改为提取直流监控上报的数据,如直流监控不上报时再自行判断.
    //新增电池放电标志位处理
    if ((Uart4Buf[buf_Num] & 0x80))  //放电
    {
        Num_battery_charging[group] = 0;            
        if (Num_battery_discharging[group] >= 3)
        {
            Dc_info[group].battery.FG_discharging = 1; //电池放电标志置1    
        }
        else 
        {
            Num_battery_discharging[group] ++;
        }
    }
    else 
    {
        Num_battery_discharging[group] = 0;
        if (Num_battery_charging[group] >= 3)
        {
            Dc_info[group].battery.FG_discharging = 0; //电池放电标志清0    
        }
        else 
        {
            Num_battery_charging[group] ++;
        }        
    }
#endif

    buf_Num += 2;
    Dc_info[group].busbar.charger_i = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//充电机电流
    buf_Num += 2;
    Dc_info[group].busbar.battery_t = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//温度
    buf_Num += 2;

#if 0  //新增控母电流值计算 -----放电标志改为提取直流监控上报的数据,如直流监控不上报时再自行判断.
    if (Dc_info[group].battery.FG_discharging)
    {
        //放电时，母线电流等于充电机电流+(电池电流的绝对值)
        Dc_info[group].busbar.control_busbar_i = Dc_info[group].busbar.charger_i + (Dc_info[group].busbar.battery_i & 0x7FFF); 
    }
    else 
    {
        //充电时,母线电流等于充电机电流-电池电流
        if (Dc_info[group].busbar.charger_i > Dc_info[group].busbar.battery_i)
        {
            Dc_info[group].busbar.control_busbar_i = 
                Dc_info[group].busbar.charger_i - Dc_info[group].busbar.battery_i; 
        }
        else 
        {
            Dc_info[group].busbar.control_busbar_i = 0x00;
        }             
    }
#endif 
    ERR_DC_DcSample_comm[group] = JudgeBitInByte(Uart4Buf[buf_Num],0);//BYTE 29  bit 0-5
    ERR_DC_AcSample_comm[group] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    ERR_DC_Charger_GY[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
    ERR_DC_Charger_QY[group] = JudgeBitInByte(Uart4Buf[buf_Num],3);
    ERR_DC_KM_GY[group] = JudgeBitInByte(Uart4Buf[buf_Num],4);
    ERR_DC_KM_QY[group] = JudgeBitInByte(Uart4Buf[buf_Num],5);
    buf_Num ++;
    ERR_DC_AcVoltage_GY[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],0);//BYTE 30  
    ERR_DC_AcVoltage_QY[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    ERR_DC_Ac_PowerCut[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],2);
    ERR_DC_Ac_PhaseLoss[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],3);
    ERR_DC_AcVoltage_GY[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],4);
    ERR_DC_AcVoltage_QY[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],5);
    ERR_DC_Ac_PowerCut[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],6);
    ERR_DC_Ac_PhaseLoss[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],7);
    buf_Num ++;
    ERR_DC_AcSPD[group] = JudgeBitInByte(Uart4Buf[buf_Num],0);          //BYTE 31
    for(i=0;i<6;i++)
        ERR_DC_AcSwitch[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i+1);
    buf_Num ++;
    ERR_DC_Battery_SW[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],0);  //BYTE 32
    ERR_DC_External[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],2);
    ERR_DC_BatteryFuse[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],4);
    //		ERR_DC_JY_detection[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],6);
    ERR_DC_Battery_SW[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    ERR_DC_External[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],3);
    ERR_DC_BatteryFuse[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],5);
    //		ERR_DC_JY_detection[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],7);
    buf_Num ++;
    ERR_DC_Battery_EQ_timeout[group] = JudgeBitInByte(Uart4Buf[buf_Num],0); //BYTE 33
    ERR_DC_Battery_QY[group] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    battery_current_state[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
    ERR_DC_BatteryPolling_comm[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],3);
    ERR_DC_BatteryPolling_comm[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],4);

#if 1  //新增电池组过压
    //Byte33-bit5: 电池组过压
    ERR_DC_Battery_GY[group] = JudgeBitInByte(Uart4Buf[buf_Num],5);
#endif

#if 1 //放电标志改为提取直流监控上报的数据,如直流监控不上报时再自行判断.
    /* -------------------------------------------------------- 
     * BIT6：0----充电                   1-----放电（新增）
     * BIT7: 0----充放电标志无效（BIT6） 1-----充放电标志有效（新增）                         
     * 备注：当充放电标志无效时，总监控自行根据电流正负来判断充放电状态，
     *       否则直接取直流监控上报的状态。
     * -------------------------------------------------------*/
    if (JudgeBitInByte(Uart4Buf[buf_Num],7))  //取直流监控上传的标志位
    {
        //新增电池组充放电标志位
        Dc_info[group].battery.FG_discharging = JudgeBitInByte(Uart4Buf[buf_Num],6);        
    }
    else //否则总监控自行根据电流的正负判断充放电标志
    {
        //新增电池放电标志位处理
        if ((Dc_info[group].busbar.battery_i & 0x8000))  //负电流，判为放电
        {
            Num_battery_charging[group] = 0;            
            if (Num_battery_discharging[group] >= 3)
            {
                Dc_info[group].battery.FG_discharging = 1; //电池放电标志置1    
            }
            else 
            {
                Num_battery_discharging[group] ++;
            }
        }
        else 
        {
            Num_battery_discharging[group] = 0;
            if (Num_battery_charging[group] >= 3)
            {
                Dc_info[group].battery.FG_discharging = 0; //电池放电标志清0    
            }
            else 
            {
                Num_battery_charging[group] ++;
            }        
        }    
    }
#endif 

#if 1  //新增控母电流值计算---------放电标志改为提取直流监控上报的数据,如直流监控不上报时再自行判断.
    if (Dc_info[group].battery.FG_discharging)
    {
        //放电时，母线电流等于充电机电流+(电池电流的绝对值)
        Dc_info[group].busbar.control_busbar_i = Dc_info[group].busbar.charger_i + (Dc_info[group].busbar.battery_i & 0x7FFF); 
    }
    else 
    {
        //充电时,母线电流等于充电机电流-电池电流
        if (Dc_info[group].busbar.charger_i > Dc_info[group].busbar.battery_i)
        {
            Dc_info[group].busbar.control_busbar_i = 
                Dc_info[group].busbar.charger_i - Dc_info[group].busbar.battery_i; 
        }
        else 
        {
            Dc_info[group].busbar.control_busbar_i = 0x00;
        }             
    }
#endif    
}
/**************************************************************************
 *函数名:			receive_DC_JY_data     0X22/0X2A/0X27
 *函数功能:		将接收的直流监控数据中的绝缘相关数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			unsigned int group	直流组数
 *返回值:			void
 ***************************************************************************/
void receive_DC_JY_data(unsigned int group)
{
    unsigned int buf_Num = 2;
    unsigned int i = 0;
    int j = 0;
    unsigned char JY_group=0,JY_lu=0,JY_ERR_Num=0;
    for(i=0;i<8;i++)
        ERR_DC_JY_Sample_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_DC_JY_Sample_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num],i);
    buf_Num++;
    Dc_info[group].insulate.control_bus_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//控母正对地
    buf_Num += 2;
    Dc_info[group].insulate.switching_bus_v1 = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //合母正对地
    buf_Num += 2;
    Dc_info[group].insulate.switching_bus_v2 = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//合母负对地
    buf_Num += 2;
    for(i=0;i<16;i++)  //原有16组
        for(j=0;j<32;j++){
            ERR_DC_SW_K_JY[group][i][j] = 0;    //故障清零
            ERR_DC_SW_H_JY[group][i][j] = 0;    //故障清零
        }

    for(i=0;i<24;i++)  //新扩的24组(17~40组)
        for(j=0;j<32;j++){
            ERR_DC_SW_K_JY_Add[group][i][j] = 0;    //故障清零
            ERR_DC_SW_H_JY_Add[group][i][j] = 0;    //故障清零
        }

#if 1   //新增
    if ((Uart4Buf[buf_Num] == 0x55) 
            && (Uart4Buf[buf_Num + 1] == 0xAA))
    {
        buf_Num += 2;

        //交流串入电压
        Dc_info[group].insulate.ACInVol = (Uart4Buf[buf_Num]<<8) | Uart4Buf[buf_Num+1];	
        buf_Num += 2;
        //正母线对地电阻
        Dc_info[group].insulate.PlusBusGroudRes = (Uart4Buf[buf_Num]<<8) | Uart4Buf[buf_Num+1];  
        buf_Num += 2;
        //负母线对地电阻 
        Dc_info[group].insulate.MinusBusGroudRes = (Uart4Buf[buf_Num]<<8) | Uart4Buf[buf_Num+1];
        buf_Num += 2;
    }
#endif 

    JY_ERR_Num = Uart4Buf[buf_Num];
    buf_Num++;


    if(JY_ERR_Num > 25)
        JY_ERR_Num=25;
    if(0 != JY_ERR_Num){
        //			ERR_DC_JY_VF[group] = 1;
        ERR_DC_JY_detection[group][0] = 1;  //<!--  一段绝缘故障(1号直流系统绝缘故障)-->
        Dc_info[group].ERR_DC_muxian = 1;   //1: 母线接地
        for(i=0;i<JY_ERR_Num;i++){
            if (Uart4Buf[buf_Num] <= 16)    //原有16组
            {
                JY_group = Uart4Buf[buf_Num];
                JY_lu = Uart4Buf[buf_Num+1];
                if(0x80 != (JY_lu&0x80)){
                    ERR_DC_SW_K_JY_R[group][JY_group-1][((JY_lu&0x7f)-1)] = (Uart4Buf[buf_Num+2]<<8)| Uart4Buf[buf_Num+3];
                    ERR_DC_SW_K_JY[group][JY_group-1][((JY_lu&0x7f)-1)] = 1;
                }else{
                    ERR_DC_SW_H_JY_R[group][JY_group-1][((JY_lu&0x7f)-1)] = (Uart4Buf[buf_Num+2]<<8)| Uart4Buf[buf_Num+3];
                    ERR_DC_SW_H_JY[group][JY_group-1][((JY_lu&0x7f)-1)] = 1;
                }
            }
            else if (Uart4Buf[buf_Num] <= 40) //新扩的24组(17~40组)
            {
                JY_group = Uart4Buf[buf_Num];
                JY_lu = Uart4Buf[buf_Num+1];

                if(0x80 != (JY_lu&0x80))
                {
                    ERR_DC_SW_K_JY_R_Add[group][JY_group-1 - 16][((JY_lu&0x7f)-1)] = (Uart4Buf[buf_Num+2]<<8)| Uart4Buf[buf_Num+3];
                    ERR_DC_SW_K_JY_Add[group][JY_group-1 - 16][((JY_lu&0x7f)-1)] = 1;
                }
                else
                {
                    ERR_DC_SW_H_JY_R_Add[group][JY_group-1 - 16][((JY_lu&0x7f)-1)] = (Uart4Buf[buf_Num+2]<<8)| Uart4Buf[buf_Num+3];
                    ERR_DC_SW_H_JY_Add[group][JY_group-1 - 16][((JY_lu&0x7f)-1)] = 1;
                }            
            }

            buf_Num += 4;
        }

    }
    else{
        //ERR_DC_JY_VF[group] = 0;
        ERR_DC_JY_detection[group][0] = 0;
        Dc_info[group].ERR_DC_muxian = 0;  //0: 正常
        for(i=0;i<16;i++)
            for(j=0;j<32;j++){
                ERR_DC_SW_K_JY[group][i][j] = 0;
                ERR_DC_SW_H_JY[group][i][j] = 0;
            }

        for(i=0;i<24;i++)  //新扩的24组(17~40组)
            for(j=0;j<32;j++){
                ERR_DC_SW_K_JY_Add[group][i][j] = 0;
                ERR_DC_SW_H_JY_Add[group][i][j] = 0;
            }
    }
}
/**************************************************************************
 *函数名:			receive_DC_Switch_data    0X23/0X2B/0X2D
 *函数功能:		将接收的直流监控数据中的馈线开关相关数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			unsigned int group	直流组数
 *返回值:			void
 ***************************************************************************/
void receive_DC_Switch_data(unsigned int group)
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j = 0;
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            ERR_DC_SW_trip[group][i][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      //开关量监控采样值（跳闸）（32路）
            ERR_DC_SW_trip[group][i][j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            ERR_DC_SW_trip[group][i][j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            ERR_DC_SW_trip[group][i][j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            Dc_info[group].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    //状态量监控采样值（合分）（32路）
            Dc_info[group].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Dc_info[group].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Dc_info[group].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<8;i++){
        ERR_DC_SW_Sample_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);    //开关量监控通信故障（16个监控）
        ERR_DC_SW_Sample_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<8;i++){
        ERR_DC_St_Sample_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);   //状态量监控通信故障（16个监控）
        ERR_DC_St_Sample_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
}
/**************************************************************************
 *函数名:			receive_DC_Battery_data   0X24/0X2C/0X2E
 *函数功能:		将接收的直流监控数据电池相关数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			unsigned int group	直流组数
 *返回值:			void
 ***************************************************************************/
void receive_DC_Battery_data(unsigned int group)
{
    unsigned int buf_Num = 2;
    unsigned int i = 0;
    int battery_flag = 1;      //作为标准位，表示电池电压

    if(Battery_12V_flag == 2)
    {
        battery_flag = 10;
    }
    else
    {
        battery_flag = 1;
    }

    //电池温度
    Dc_info[group].battery.temperature = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];  //X<<8,X的值左移8位或上下一字节
    buf_Num += 2;      

#if 0
    //电池电压最大最小初始值
    Dc_info[group].battery.single_max_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]; //假设第1节电池是最高电压，（冒泡一个个比过去）
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;
#else  //定制功能：修复12V时单体最小电压值比较显示的问题。 
    //电池电压最大最小初始值
    Dc_info[group].battery.single_max_v = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;
#endif 

#if 0
    //通过冒泡的方法比较出所有电池中的最大值跟最小值，再通过比较参数设置值，确定每一节电池是否欠压或者过压    
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
#else  //系统设置电池参数从统一设置改成分组设置. 
        for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
#endif 
        {
            //每一节电池电压
            Dc_info[group].battery.single_v[i] =( (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;   
            buf_Num += 2;

            //取单体电压最大小值
            if(Dc_info[group].battery.single_v[i] > Dc_info[group].battery.single_max_v)
            {
                Dc_info[group].battery.single_max_v = Dc_info[group].battery.single_v[i]; //取单体电压最大值
            }
            else if(Dc_info[group].battery.single_v[i] < Dc_info[group].battery.single_min_v)
            {
                Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_v[i]; //取单体电压最小值
            }

#if 0
            //处理过压和过压恢复
            if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v * 100) //过压
#else   //系统设置电池参数从统一设置改成分组设置. 
                //处理过压和过压恢复
                if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v * 100) //过压
#endif 
                {
                    ERR_DC_BatterySingle_GYHF_CNT[group][i] = 0;   //过压恢复计数清0                  
                    if (ERR_DC_BatterySingle_GY_CNT[group][i] < MAXCOUNT_GY_QY)      
                    {
                        ERR_DC_BatterySingle_GY_CNT[group][i] ++;  //过压计数加1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 1;  //判定为过压
                    }
                }
                else  //过压恢复 
                {
                    ERR_DC_BatterySingle_GY_CNT[group][i] = 0;  //过压计数清0                    
                    if (ERR_DC_BatterySingle_GYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_GYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 0;  //判定为过压恢复
                    }
                }

#if 0
            //处理欠压和欠压恢复
            if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100) //欠压
#else   //系统设置电池参数从统一设置改成分组设置. 
                //处理欠压和欠压恢复
                if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100) //欠压
#endif 
                {
                    ERR_DC_BatterySingle_QYHF_CNT[group][i] = 0;                    
                    if (ERR_DC_BatterySingle_QY_CNT[group][i] < MAXCOUNT_GY_QY)     
                    {
                        ERR_DC_BatterySingle_QY_CNT[group][i] ++; //欠压计数加1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 1;  //判定为欠压
                    }
                }
                else  //欠压恢复
                {
                    ERR_DC_BatterySingle_QY_CNT[group][i] = 0;                      
                    if (ERR_DC_BatterySingle_QYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_QYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 0;  //判定为欠压恢复
                    }
                }

#if 0   //屏蔽 //系统设置电池参数从统一设置改成分组设置. 
            if (i == 0)
            {
                printf ("BT Volt:%d,(%d,%d),(%d,%d),(%d,%d)\n",Dc_info[group].battery.single_v[i],
                        Sys_cfg_info.battery_set.single_upperLimit_v*100,
                        Sys_cfg_info.battery_set.single_lowerLimit_v*100,
                        ERR_DC_BatterySingle_GY_CNT[group][i],
                        ERR_DC_BatterySingle_GYHF_CNT[group][i],
                        ERR_DC_BatterySingle_QY_CNT[group][i],
                        ERR_DC_BatterySingle_QYHF_CNT[group][i]);
            }
#endif 
        }     //通过冒泡的方法比较出所有电池中的最大值跟最小值，再通过比较参数设置值，确定每一节电池是否欠压或者过压
}

/**
 * 将接收的PSMX-B发来的电池电压赋值给相关变量。(地址: 0XB0/0XB1/0XB2)
 * @param  group: 直流组数
 */
void receive_DC_Battery_data_PSMXB_Vol(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;
    int battery_flag = 1;      //作为标准位，表示电池电压

#if 0  //此处2V、12V电池一样处理
    if (Battery_12V_flag == 2)
    {
        battery_flag = 10;
    }
    else
    {
        battery_flag = 1;
    }
#endif 

#if 0
    //电池温度: 默认25.0度
    Dc_info[group].battery.temperature = 250;
#endif

    //被检测电池节数: 不以此处上传的电池节数为准，以界面设定为准
    //Sys_cfg_info.battery_set.battery_amount = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];
    //if (Sys_cfg_info.battery_set.battery_amount > 108)
    //{
    //    Sys_cfg_info.battery_set.battery_amount = 108;
    //}
    buf_Num += 2; 

    //电池电压: 不以此处上传的电压为准,以直流监控上传数据为准
    //Dc_info[group].busbar.battery_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];		
    //Dc_info[group].battery.voltage = Dc_info[group].busbar.battery_v;
    buf_Num += 2;

    //电池电压最大最小初始值
    Dc_info[group].battery.single_max_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]; //假设第1节电池是最高电压，（冒泡一个个比过去）
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;

#if 0
    //通过冒泡的方法比较出所有电池中的最大值跟最小值，再通过比较参数设置值，确定每一节电池是否欠压或者过压    
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
#else  //系统设置电池参数从统一设置改成分组设置.  
        //通过冒泡的方法比较出所有电池中的最大值跟最小值，再通过比较参数设置值，确定每一节电池是否欠压或者过压    
        for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
#endif 
        {
            //每一节电池电压
            Dc_info[group].battery.single_v[i] =( (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;   
            buf_Num += 2;

            //取单体电压最大小值
            if(Dc_info[group].battery.single_v[i] > Dc_info[group].battery.single_max_v)
            {
                Dc_info[group].battery.single_max_v = Dc_info[group].battery.single_v[i]; //取单体电压最大值
            }
            else if(Dc_info[group].battery.single_v[i] < Dc_info[group].battery.single_min_v)
            {
                Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_v[i]; //取单体电压最小值
            }

#if 0
            //处理过压和过压恢复
            if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v * 100) //过压
#else   //系统设置电池参数从统一设置改成分组设置.  
                //处理过压和过压恢复
                if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v * 100) //过压
#endif 
                {
                    ERR_DC_BatterySingle_GYHF_CNT[group][i] = 0;   //过压恢复计数清0                  
                    if (ERR_DC_BatterySingle_GY_CNT[group][i] < MAXCOUNT_GY_QY)      
                    {
                        ERR_DC_BatterySingle_GY_CNT[group][i] ++;  //过压计数加1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 1;  //判定为过压
                    }
                }
                else  //过压恢复 
                {
                    ERR_DC_BatterySingle_GY_CNT[group][i] = 0;  //过压计数清0                    
                    if (ERR_DC_BatterySingle_GYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_GYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 0;  //判定为过压恢复
                    }
                }

#if 0
            //处理欠压和欠压恢复
            if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100) //欠压
#else   //系统设置电池参数从统一设置改成分组设置.  
                //处理欠压和欠压恢复
                if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100) //欠压
#endif 
                {
                    ERR_DC_BatterySingle_QYHF_CNT[group][i] = 0;                    
                    if (ERR_DC_BatterySingle_QY_CNT[group][i] < MAXCOUNT_GY_QY)     
                    {
                        ERR_DC_BatterySingle_QY_CNT[group][i] ++; //欠压计数加1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 1;  //判定为欠压
                    }
                }
                else  //欠压恢复
                {
                    ERR_DC_BatterySingle_QY_CNT[group][i] = 0;                      
                    if (ERR_DC_BatterySingle_QYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_QYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 0;  //判定为欠压恢复
                    }
                }

#if 0   //屏蔽: //系统设置电池参数从统一设置改成分组设置. 
            if (i == 0)
            {
                printf ("BT Volt:%d,(%d,%d),(%d,%d),(%d,%d)\n",Dc_info[group].battery.single_v[i],
                        Sys_cfg_info.battery_set.single_upperLimit_v*100,
                        Sys_cfg_info.battery_set.single_lowerLimit_v*100,
                        ERR_DC_BatterySingle_GY_CNT[group][i],
                        ERR_DC_BatterySingle_GYHF_CNT[group][i],
                        ERR_DC_BatterySingle_QY_CNT[group][i],
                        ERR_DC_BatterySingle_QYHF_CNT[group][i]);
            }
#endif 
        }   
}

/**
 * 将接收的PSMX-B发来的电池内阻赋值给相关变量。(地址: 0XB0/0XB1/0XB2)
 * @param  group: 直流组数
 */
void receive_DC_Battery_data_PSMXB_Res(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;

#if 0
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
    {
        //每一节电池内阻
        Dc_info[group].battery.single_res[i] 
            = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]);   
        buf_Num += 2;
    }
#else  //系统设置电池参数从统一设置改成分组设置.  
    for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
    {
        //每一节电池内阻
        Dc_info[group].battery.single_res[i] 
            = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]);   
        buf_Num += 2;
    }
#endif 

    //电池温度
    buf_Num = 219;
    Dc_info[group].battery.temperature = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];  //X<<8,X的值左移8位或上下一字节
}

/**
 * 将接收的PSMX-B发来的电池遥信赋值给相关变量。(地址: 0XB0/0XB1/0XB2)
 * @param  group: 直流组数
 */
void receive_DC_Battery_data_PSMXB_ST(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;
    unsigned int j = 0;

#if 0
    //0x0307~0x030D: 1~108号电池内阻超限标示位(14 字节* 8 = 112) 
    for(i = 0;i < (Sys_cfg_info.battery_set.battery_amount / 8 + 1); i ++)
    {
        for (j = 0; j < 8; j ++)
        {
            if ((8 * i + j) < Sys_cfg_info.battery_set.battery_amount)
            {
                ERR_DC_BatterySingle_ResOver[group][8 * i + j] 
                    = JudgeBitInByte(Uart4Buf[buf_Num],j);               
            }  
        }

        buf_Num ++;
    }
#else  //系统设置电池参数从统一设置改成分组设置.  
    //0x0307~0x030D: 1~108号电池内阻超限标示位(14 字节* 8 = 112) 
    for(i = 0;i < (Sys_cfg_info.battery_set[group].battery_amount / 8 + 1); i ++)
    {
        for (j = 0; j < 8; j ++)
        {
            if ((8 * i + j) < Sys_cfg_info.battery_set[group].battery_amount)
            {
                ERR_DC_BatterySingle_ResOver[group][8 * i + j] 
                    = JudgeBitInByte(Uart4Buf[buf_Num],j);               
            }  
        }

        buf_Num ++;
    }
#endif 

    //0x030E(低字节Bit0~3): 1~4号采样盒通讯异常
    buf_Num = 18;
    for (i = 0; i < 4; i ++)
    {
        ERR_DC_BatterySamplingBox_CommError[group][i] 
            = JudgeBitInByte(Uart4Buf[buf_Num], i);
    }
}

/**************************************************************************
 *函数名:	receive_DC_FenGui_data   0X50/0X51/0X52
 *函数功能:	将接收的直流监控数据中的馈线开关相关数据赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:	    unsigned int group	直流组数
 *返回值:	void
 ***************************************************************************/
void receive_DC_FenGui_data(unsigned int group)
{
    unsigned int buf_Num = 10;
    int i = 0;
    int j = 0;

    //绝缘标准单元通信故障（合分）（30路） (8 Bytes)
    for(j=0;j<8;j++)
    {
        //1~32 
        ERR_DC_standard_cell_comm[group][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      
        ERR_DC_standard_cell_comm[group][j + 8] = JudgeBitInByte(Uart4Buf[buf_Num + 1],j);
        ERR_DC_standard_cell_comm[group][j + 16] = JudgeBitInByte(Uart4Buf[buf_Num + 2],j);
        ERR_DC_standard_cell_comm[group][j + 24] = JudgeBitInByte(Uart4Buf[buf_Num + 3],j);

#ifdef  _ANM03_DC_SWITCH_STATE_NUM_40       
        //扩到64组(33~64)
        ERR_DC_standard_cell_comm[group][j + 32] = JudgeBitInByte(Uart4Buf[buf_Num + 4],j);      
        ERR_DC_standard_cell_comm[group][j + 40] = JudgeBitInByte(Uart4Buf[buf_Num + 5],j);
        ERR_DC_standard_cell_comm[group][j + 48] = JudgeBitInByte(Uart4Buf[buf_Num + 6],j);
        ERR_DC_standard_cell_comm[group][j + 56] = JudgeBitInByte(Uart4Buf[buf_Num + 7],j);
#endif 
    }
    buf_Num += 8;  //8个字节是标准单元通信故障

    //分柜通信故障（10个监控） (4 Bytes)
    for(i=0;i<8;i++)
    {
        ERR_DC_FG_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);      
        ERR_DC_FG_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
    buf_Num += 4;  //2个字节是分柜通信故障，2个字节保留

#ifdef  _ANM03_DC_SWITCH_STATE_NUM_40 
    if (Uart4Buf[buf_Num - 1] == 40)  //新增40组处理
    {
        //状态量监控采样值（合分）（16路） (30 Bytes)
        for(i=0;i<24;i++){
            for(j=0;j<8;j++){
                Dc_FG_info[group].FGfeederLine[i].FGstate[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    
                Dc_FG_info[group].FGfeederLine[i].FGstate[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            }
            buf_Num += 2;
        }
        buf_Num += 2;  //2个字节保留

        //开关量监控采样值（跳闸）（16路） (28 Bytes)
        for(i=0;i<24;i++){
            for(j=0;j<8;j++){
                ERR_DC_FG_SW_trip[group][i][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      
                ERR_DC_FG_SW_trip[group][i][j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            }
            buf_Num += 2;    
        } 
    }
#else   
    //原30组处理
    //状态量监控采样值（合分）（16路） (30 Bytes)
    for(i=0;i<14;i++){
        for(j=0;j<8;j++){
            Dc_FG_info[group].FGfeederLine[i].FGstate[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    
            Dc_FG_info[group].FGfeederLine[i].FGstate[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
        }
        buf_Num += 2;
    }
    buf_Num += 2;  //2个字节保留

    //开关量监控采样值（跳闸）（16路） (28 Bytes)
    for(i=0;i<14;i++){
        for(j=0;j<8;j++){
            ERR_DC_FG_SW_trip[group][i][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      
            ERR_DC_FG_SW_trip[group][i][j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
        }
        buf_Num += 2;    
    }    

#endif

#if 0   //just for test 
    printf("addr(Uart4Buf[0]): %d, length(Uart4Buf[1]): %d, Uart4Buf[22]: %d, Uart4Buf[23]: %d.\n", 
            Uart4Buf[0], Uart4Buf[1], Uart4Buf[22], Uart4Buf[23]); 
#endif 
}

/*void receive_TX_Battery_data(unsigned int group)
  {
  unsigned int buf_Num = 4;
  unsigned int i = 0;

  for(i=0;i<12;i++)
  {
  Dc_info[group].battery.TXsingle_v[i] =((Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]);   //每一节电池电压
//debug_printf(0,"Dc_info[group].battery.TXsingle_v[i]== %4x\n",Dc_info[group].battery.TXsingle_v[i]);
buf_Num += 2;
}     

}*/
void receive_TX_Battery_data(unsigned int group)
{
    unsigned int buf_Num = 4;
    unsigned int i = 0;
    unsigned int j = 0;

    if (group == 0 || group==1){
        j = Sys_cfg_info.comm_set[0].battery_num/2;
    }else if (group == 2 || group==3){
        j = Sys_cfg_info.comm_set[1].battery_num/2;
    }

    Comm_DC_info[group].battery.TXsingle_max_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];//假设第1节电池是最高电压，（冒泡一个个比过去）
    Comm_DC_info[group].battery.TXsingle_min_v = Comm_DC_info[group].battery.TXsingle_max_v;
    for(i=0;i<j;i++){
        Comm_DC_info[group].battery.TXsingle_v[i] =( (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]);   //每一节电池电压
        buf_Num += 2;
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Comm_DC_info[group].battery.TXsingle_max_v){
            Comm_DC_info[group].battery.TXsingle_max_v = Comm_DC_info[group].battery.TXsingle_v[i];	//取单体电压最大值
        }else if(Comm_DC_info[group].battery.TXsingle_v[i] < Comm_DC_info[group].battery.TXsingle_min_v){
            Comm_DC_info[group].battery.TXsingle_min_v = Comm_DC_info[group].battery.TXsingle_v[i];	//取单体电压最小值
        }else{;}

#if 0
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v*100){
            ERR_comm_BatterySingle_GY[group][i] = 1;				  //电池电压大于过压设定值（X100），报过压告警
        }else {
            ERR_comm_BatterySingle_GY[group][i] = 0;
        }
        if(Comm_DC_info[group].battery.TXsingle_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100){
            ERR_comm_BatterySingle_QY[group][i] = 1;				  //电池电压小于欠压设定值，报欠压告警
        }else{
            ERR_comm_BatterySingle_QY[group][i] = 0;;
        }
#else   //系统设置电池参数从统一设置改成分组设置.  
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v*100){
            ERR_comm_BatterySingle_GY[group][i] = 1;				  //电池电压大于过压设定值（X100），报过压告警
        }else {
            ERR_comm_BatterySingle_GY[group][i] = 0;
        }
        if(Comm_DC_info[group].battery.TXsingle_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100){
            ERR_comm_BatterySingle_QY[group][i] = 1;				  //电池电压小于欠压设定值，报欠压告警
        }else{
            ERR_comm_BatterySingle_QY[group][i] = 0;;
        }
#endif 
    }	  //通过冒泡的方法比较出所有电池中的最大值跟最小值，再通过比较参数设置值，确定每一节电池是否欠压或者过压
}


/**************************************************************************
 *函数名:	receive_Comm_data   0X30  0X31
 *函数功能:	将接收的通信电源监控数据电池相关数据赋值给相关变量，包括显示用变量和故障标志变量。
 *参数:		unsigned int group通信电源屏号
 *返回值:	void
 ***************************************************************************/
void receive_Comm_data(unsigned int group)   //group 0,1
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;
    Comm_info[group].module.input_v = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Comm_info[group].module.output_v = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Comm_info[group].module.input_i = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Comm_info[group].module.output_i = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    for(i=0;i<16;i++){   //16个模块输出电压
        Comm_info[group].module.module_output_v[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];  //16个模块输出电压
        buf_Num += 2;
    }
    for(i=0;i<16;i++){   //16个模块输出电流
        Comm_info[group].module.module_output_i[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //16个模块输出电流
        buf_Num += 2;
    }
    for(i=0;i<16;i++){   //16个模块输出温度
        Comm_info[group].module.module_output_t[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //16个模块输出温度
        buf_Num += 2;
    }
    for(i=0;i<8;i++){    //一个字节8位，这里的i是8位的意思，一共5个字节
        ERR_Comm_SW_trip[group][0][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_SW_trip[group][0][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
        ERR_Comm_SW_trip[group][0][i+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],i);
        ERR_Comm_SW_specialty[group][i] = JudgeBitInByte(Uart1Buf[buf_Num+3],i);
        ERR_Comm_SW_specialty[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+4],i);
    }
    buf_Num += 5;
    for(i=1;i<4;i++){   //剩下3个开关监控值，i从1开始，这里的i是开关监控个数
        for(j=0;j<8;j++){
            ERR_Comm_SW_trip[group][i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
            ERR_Comm_SW_trip[group][i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            ERR_Comm_SW_trip[group][i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            ERR_Comm_SW_trip[group][i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<4;i++){   //i状态监控个数
        for(j=0;j<8;j++){    //j表示位，0-7,1个字节8位
            Comm_info[group].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
            Comm_info[group].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            Comm_info[group].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            Comm_info[group].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<8;i++){   //i表示一个字节8位，2个字节表示16个模块通讯故障
        ERR_Comm_Module_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_Module_comm[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<8;i++){    //i表示一个字节8位，2个字节表示16个模块故障
        ERR_Comm_Module[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_Module[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<4;i++){  //i表示位，前面4位表示开关监控通讯故障，后4位是状态监控通信故障
        ERR_Comm_SW_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_St_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i+4);
    }
    buf_Num ++;
    ERR_Comm_Feeder_sw[group] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_Comm_SPD[group]= JudgeBitInByte(Uart1Buf[buf_Num],1);
}
/**************************************************************************
 *函数名:			receive_LNMKComm_data   （雷能）
 *函数功能:		将接收雷能通信电源监控数据模块数据 --->模块模拟量数据1、2段
 *参数:			unsigned int group通信电源屏号
 *返回值:			void
 ***************************************************************************/
void receive_LNMKComm_data(unsigned int group)   //group 0,1
{
    unsigned int buf_Num =15;
    int i = 0;
    int j = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    float f = 0;
    unsigned char a[100];
    unsigned char b[100];
    for (i=0;i<4;i++)
    {
        b[i] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
        buf_Num += 2;
    }
    //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
    for (j= 0;j<4;j++)
    {
        a[j] = b[4-j-1];
    }
    //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
    //j = 0;   //j复位
    //x = atoh(a);    //字符变成数字
    z = 0;  //z复位
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);    //f是浮点型，memcpy是拷贝，把int型的x,拷贝成float型的f
    y = (int)((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].module.output_v = y;   //输出电压
    int MK_num = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
    //debug_printf(0,"MK_num==%d",MK_num);
    for(i=0;i< MK_num;i++)
    {   
        buf_Num += 2;
        for (x=0;x<4;x++)
        {
            b[x] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
            buf_Num += 2;
        }
        //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
        for (j= 0;j<4;j++)
        {
            a[j] = b[4-j-1];
        }
        //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
        //j = 0;   //j复位
        //x = atoh(a);    //字符变成数字
        z = 0;  //z复位
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_i[i] = y;   //16个模块输出电流(整流器电流)
        buf_Num += 2;
        for (x=0;x<4;x++)
        {
            b[x] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
            buf_Num += 2;
        }
        //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
        for (j= 0;j<4;j++)
        {
            a[j] = b[4-j-1];
        }
        //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
        //j = 0;   //j复位
        //x = atoh(a);    //字符变成数字
        z = 0;  //z复位
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_v[i] = y;  //16个模块输出电压(整流器的输出电压)
        buf_Num += 24;
        for (x=0;x<4;x++)
        {
            b[x] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
            buf_Num += 2;
        }
        //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
        for (j= 0;j<4;j++)
        {
            a[j] = b[4-j-1];
        }
        //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
        //j = 0;   //j复位
        //x = atoh(a);    //字符变成数字
        z = 0;  //z复位
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_t[i] = y;   //16个模块输出温度(整流器的DCDC 环境温度)
        buf_Num += 26;
    }	
}
/**************************************************************************
 *函数名:			receive_LNMKComm_status 
 *函数功能:		将接收雷能通信电源监控数据模块状态 --->模块开关量状态1、2段
 *参数:			unsigned int group通信电源屏号
 *返回值:			void
 ***************************************************************************/
void receive_LNMKComm_status(unsigned int group)   //group 0,1
{
    unsigned int buf_Num = 15;
    int i = 0;
    //int j =0;
    int MK_num = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
    for(i=0;i< MK_num;i++)
    {   
        buf_Num += 2;    //开机/关机
        Comm_info[group].module.module_statekg[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 2;    //限流/不限流
        Comm_info[group].module.module_statexl[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 2;    //均充/浮充
        Comm_info[group].module.module_statejfc[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 4;    //在位/不在位
        Comm_info[group].module.module_statezw[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
    }

}
/**************************************************************************
 *函数名:			receive_LNMKComm_alarm 
 *函数功能:		将接收雷能通信电源监控数据模块告警 --->告警信息1、2段
 *参数:			unsigned int group通信电源屏号
 *返回值:			void
 ***************************************************************************/
void receive_LNMKComm_alarm(unsigned int group)   //group 0,1
{
    unsigned int buf_Num = 15;
    int i = 0;
    //int j =0;
    int MK_num = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
    debug_printf(0,",, MK_num == %4x\n",MK_num );
    for(i=0;i< MK_num;i++)
    {   
        buf_Num += 2;
        ERR_LN_Comm_Module[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));           //模块故障
        buf_Num += 6;
        ERR_Comm_Module_output_GY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));    //输出过压关机故障
        buf_Num += 2;
        ERR_Comm_Module_output_QY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));    //输出欠压
        buf_Num += 2;
        ERR_Comm_Module_AC_import_GY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //输入过压
        buf_Num += 2;
        ERR_Comm_Module_AC_import_QY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //输入欠压
        buf_Num += 22;
        ERR_Comm_Module_bu_advection[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //不均流
        buf_Num += 4;
        ERR_LN_Comm_Module_comm[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));      //模块跟子监控通信故障
    }			
}
/**************************************************************************
 *函数名:			receive_LNDCComm_data   
 *函数功能:		将接收雷能通信电源监控数据电池相关数据--->直流模拟量电池信息1、2段
 *参数:			unsigned int group通信电源屏号
 *返回值:			void
 ***************************************************************************/
void receive_LNDCComm_data(unsigned int group)   //group 0,1
{
    unsigned int buf_Num = 15;
    int i = 0;
    int j = 0;
    int z = 0;
    int y = 0;
    float f;
    unsigned char a[100];
    unsigned char b[100];
    for (i=0;i<4;i++)
    {
        b[i] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
        buf_Num += 2;
    }
    //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
    for (j= 0;j<4;j++)
    {
        a[j] = b[4-j-1];
    }
    //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
    //j = 0;   //j复位
    //x = atoh(a);    //字符变成数字
    z = 0;  //z复位
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.dcoutput_v = y;   //直流输出电压
    //buf_Num += 8;
    for (i=0;i<4;i++)
    {
        b[i] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
        buf_Num += 2;
    }
    //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
    for (j= 0;j<4;j++)
    {
        a[j] = b[4-j-1];
    }
    //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
    //j = 0;   //j复位
    //x = atoh(a);    //字符变成数字
    z = 0;  //z复位
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.dcoutput_i = y;   //直流输出电流
    buf_Num += 2;
    for (i=0;i<4;i++)
    {
        b[i] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
        buf_Num += 2;
    }
    //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
    for (j= 0;j<4;j++)
    {
        a[j] = b[4-j-1];
    }
    //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
    //j = 0;   //j复位
    //x = atoh(a);    //字符变成数字
    z = 0;  //z复位
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.battery_output_i = y;   //电池电流
    buf_Num += 12;
    for (i=0;i<4;i++)
    {
        b[i] = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
        buf_Num += 2;
    }
    //debug_printf(0,"00000,, b[0] == %4x,, b[1] == %4x,,b[2] == %4x,, b[3] ==  %4x\n",b[0],b[1],b[2],b[3]);
    for (j= 0;j<4;j++)
    {
        a[j] = b[4-j-1];
    }
    //debug_printf(0,"00000,, a[0] == %4x,, a[1] == %4x,,a[2] == %4x,, a[3] ==  %4x\n",a[0],a[1],a[2],a[3]);
    //j = 0;   //j复位
    //x = atoh(a);    //字符变成数字
    z = 0;  //z复位
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.battery_output_t = y;   //电池温度		
}
/**************************************************************************
 *函数名:			receive_UPS_data   0x40
 *函数功能:		将接收的逆变电源监控数据电池相关数据赋值给相关变量，包括
 *					显示用变量和故障标志变量。
 *参数:			void
 *返回值:			void
 ***************************************************************************/
void receive_UPS_data()
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;

    //UPS_1	遥测
    Ups_info[0].input.AC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1交流输入电压
    buf_Num += 2;
    Ups_info[0].input.DC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1直流输入电压 
    buf_Num += 2;
    Ups_info[0].output.voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //UPS1输出电压
    buf_Num += 2;
    Ups_info[0].output.current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //输出电流负载百分比
    buf_Num += 2;
    Ups_info[0].output.frequency = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1输出频率
    buf_Num += 2;
    Ups_info[0].input.bypass_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1旁路输入电压
    buf_Num += 2;
    Ups_info[0].output.temperature = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];//UPS1温度
    buf_Num += 2;

    //UPS_2  遥测
    Ups_info[1].input.AC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].input.DC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].output.voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].output.current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].output.frequency = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].input.bypass_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;
    Ups_info[1].output.temperature = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num += 2;

    //UPS1~2状态(Byte29~30)
    ERR_UPS_Bypass[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
    ERR_UPS_Overload[0] = JudgeBitInByte(Uart1Buf[buf_Num],3);
    ERR_UPS[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);
    ERR_UPS_Bypass_output[0] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_UPS_DC[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_UPS_MainsSupply[0] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;
    ERR_UPS_Bypass[1] = JudgeBitInByte(Uart1Buf[buf_Num],2);
    ERR_UPS_Overload[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);
    ERR_UPS[1] = JudgeBitInByte(Uart1Buf[buf_Num],4);
    ERR_UPS_Bypass_output[1] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_UPS_DC[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_UPS_MainsSupply[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;

    //mkcom_error
    ERR_UPS_Comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);   //UPS1通信故障
    ERR_UPS_Comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);   //UPS2通信故障
    buf_Num++;

    //kgcom_error
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<8;i++){   //i表示位，前面8位表示开关监控通讯故障
            ERR_UPS_SW_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        }
        buf_Num++;
    }else {
        for(i=0;i<4;i++){   //i表示位，前面4位表示开关监控通讯故障，后4位是状态监控通信故障
            ERR_UPS_SW_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
            ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i+4); 
        }
        buf_Num++;
    }

    //开关位置告警(5字节: 前2字节是特殊开关)(跳闸信号)(Byte33~37)
    switch(Sys_cfg_info.ups_set.work_mode){
        case 0:		//单机模式
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);
            buf_Num += 2;
            break;
        case 1:		//主从模式
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);

            ERR_UPS_DcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],5);
            ERR_UPS_AcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
            ERR_UPS_BypassInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);

            buf_Num++;
            ERR_UPS_AcOutput_sw[1]= JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_Bypass_Overhaul_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            buf_Num ++;
            break;
        case 2:		//分立模式
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);

            ERR_UPS_DcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],5);
            ERR_UPS_AcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
            ERR_UPS_BypassInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
            buf_Num++;
            ERR_UPS_AcOutput_sw[1]= JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_Bypass_Overhaul_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            /*	   modify by lph   2016.06.30
                   ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
                   ERR_UPS_AcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
                   ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
                   ERR_UPS_DcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);
                   ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],4);
                   ERR_UPS_AcOutput_sw[1]= JudgeBitInByte(Uart1Buf[buf_Num],5);
                   ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
                   ERR_UPS_BypassInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
                   buf_Num++;
                   ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
                   ERR_UPS_Bypass_Overhaul_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
                   */   
            buf_Num ++;
            break;
        case 3:		//并机模式
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);     //10101010一个字节，0是第1位，1是第8位，从右到左
            ERR_UPS_AcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_DcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],4);
            ERR_UPS_AcOutput_sw[1]= JudgeBitInByte(Uart1Buf[buf_Num],5);
            ERR_UPS_AcOutput_sw[2] = JudgeBitInByte(Uart1Buf[buf_Num],6);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],7);
            buf_Num += 2;
            break;
        default:	//单机模式
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);
            buf_Num += 2;
            break;
    }
    for(i=0;i<8;i++){    //i表示8位，1个开关监控
        ERR_UPS_SW_trip[0][i] =  JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_UPS_SW_trip[0][i+8] =  JudgeBitInByte(Uart1Buf[buf_Num+1],i);
        ERR_UPS_SW_trip[0][i+16] =  JudgeBitInByte(Uart1Buf[buf_Num+2],i);
    }
    buf_Num += 3;
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=1;i<6;i++){         //i从1开始，这里的i是开关监控个数
            for(j=0;j<8;j++){    //j表示8位
                ERR_UPS_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                ERR_UPS_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                ERR_UPS_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                ERR_UPS_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }else {
        for(i=1;i<4;i++){         //i从1开始，这里的i是开关监控个数
            for(j=0;j<8;j++){    //j表示8位
                ERR_UPS_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                ERR_UPS_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                ERR_UPS_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                ERR_UPS_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }

    }
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<8;i++){   //i表示位，8位是状态监控通信故障
            ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i); 
        }
        buf_Num++;
    }
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<6;i++){            //i是状态监控个数    UPS特殊开关状态在1号状态监控前面2个字节
            for(j=0;j<8;j++){       //j表示8位
                Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }else {   //(Byte49~52~65)
        for(i=0;i<4;i++){            //i是状态监控个数    UPS特殊开关状态在1号状态监控前面2个字节
            for(j=0;j<8;j++){       //j表示8位
                Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }
    ERR_UPS_Feeder_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);     //UPS馈线开关故障
    ERR_UPS_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);       //ups防雷器故障

#if 1  //UPS模式判断及输入电流值计算    
    for (i = 0; i < 2; i ++)
    {
        //电流值* 百分比(结果保留一位小数)
        unsigned long output_cur = UPS_OUTPUT_CUR * Ups_info[i].output.current / 100;   

        //if (ERR_UPS_Bypass_Overhaul_sw[i])
        if (((i == 0) && Ups_info[0].feederLine[0].state[4])
                || ((i == 1) && Ups_info[0].feederLine[0].state[9]))
        {
            UPS_Mode[i] = UPS_MODE_OVER_HAUL;  //UPS检修模式

            //UPS市电输入电流、UPS直流输入电流、UPS旁路输入电流、
            //UPS输出电流和市电输入频率全为0
            Ups_info[i].input.AC_current = 0;
            Ups_info[i].input.DC_current = 0;
            Ups_info[i].input.bypass_current = 0;
            Ups_info[i].output.current_value = 0;
            Ups_info[i].input.AC_freq = 0;
        }
        else  if ((ERR_UPS_Bypass_output[i] == 0)     //旁路输出遥信无 
                && (ERR_UPS[i] == 0)                  //UPS无故障 
                && (ERR_UPS_Comm[i] == 0))            //ups通信故障    
        {
            if (ERR_UPS_MainsSupply[i] == 0)          //市电输入正常     
            {
                UPS_Mode[i] = UPS_MODE_MAINS_SUPPLY;  //UPS市电模式 

                if (Ups_info[i].input.AC_voltage > 300) //>30V
                {
                    Ups_info[i].input.AC_current = (UPS_OUTPUT_CUR * Ups_info[i].output.current) 
                        * Ups_info[i].output.voltage / (Ups_info[i].input.AC_voltage * 82);
                }
                else
                {
                    Ups_info[i].input.AC_current = 0x00;    
                }

                Ups_info[i].input.DC_current = 0;
                Ups_info[i].input.bypass_current = 0;
                Ups_info[i].output.current_value = output_cur;
                Ups_info[i].input.AC_freq = 500;
            }
            else if (ERR_UPS_DC[i] == 0)              //直流输入正常
            {
                UPS_Mode[i] = UPS_MODE_BAT_SUPPLY;    //UPS电池模式

                Ups_info[i].input.AC_current = 0;

                if (Ups_info[i].input.DC_voltage > 300)  //>30V
                {
                    Ups_info[i].input.DC_current = (UPS_OUTPUT_CUR * Ups_info[i].output.current) 
                        * Ups_info[i].output.voltage / (Ups_info[i].input.DC_voltage * 90);     
                }
                else
                {
                    Ups_info[i].input.DC_current = 0x00;
                }

                Ups_info[i].input.bypass_current = 0;
                Ups_info[i].output.current_value = output_cur;
                Ups_info[i].input.AC_freq = 0;
            }
            else 
            {
                UPS_Mode[i] = UPS_MODE_BY_PASS;       //UPS旁路模式

                Ups_info[i].input.AC_current = 0;
                Ups_info[i].input.DC_current = 0;
                Ups_info[i].input.bypass_current = output_cur;
                Ups_info[i].output.current_value = output_cur;
                Ups_info[i].input.AC_freq = 0;    
            }
        }
        else          
        {
            UPS_Mode[i] = UPS_MODE_BY_PASS;           //UPS旁路模式

            Ups_info[i].input.AC_current = 0;
            Ups_info[i].input.DC_current = 0;
            Ups_info[i].input.bypass_current = output_cur;
            Ups_info[i].output.current_value = output_cur;
            Ups_info[i].input.AC_freq = 0;
        }   
    }

#if 1
    //UPS输出异常判断
    static unsigned char Num_ERR_UPS_Output_High[2] = {0};
    static unsigned char Num_ERR_UPS_Output_Low[2] = {0};
    static unsigned char Num_ERR_UPS_Output_No[2] = {0};

    for (i = 0; i < Sys_set_Ups_Num; i ++)
    {
        if (Ups_info[i].output.voltage < 2000) //200~240 输出欠压
        {
            Num_ERR_UPS_Output_High[i] = 0;
            Num_ERR_UPS_Output_No[i] = 0;
            if (Num_ERR_UPS_Output_Low[i] < 5)
            {
                Num_ERR_UPS_Output_Low[i] ++;    
            }
            else 
            {
                ERR_UPS_OutPut[i] = 1;  //输出异常(欠压) 
            }
        }
        else if (Ups_info[i].output.voltage > 2400) //输出过压
        {
            Num_ERR_UPS_Output_Low[i] = 0;
            Num_ERR_UPS_Output_No[i] = 0;
            if (Num_ERR_UPS_Output_High[i] < 5)
            {
                Num_ERR_UPS_Output_High[i] ++;    
            }
            else 
            {
                ERR_UPS_OutPut[i] = 1;  //输出异常(过压)   
            }
        }
        else  //正常范围
        {
            Num_ERR_UPS_Output_High[i] = 0;
            Num_ERR_UPS_Output_Low[i] = 0;
            if (Num_ERR_UPS_Output_No[i] < 5)
            {
                Num_ERR_UPS_Output_No[i] ++;
            }
            else 
            {
                ERR_UPS_OutPut[i] = 0;     
            }
        }    
    }
#endif 
#endif 
}
void receive_UPS_data_35K()    //35KV工程通信，UPS解析
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;

    //UPS_1	
    Ups_info[0].input.AC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //UPS1交流输入电压
    buf_Num += 2;
    Ups_info[0].output.voltage  = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];      //输出电压
    buf_Num += 2;
    Ups_info[0].input.DC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //直流输入电压
    buf_Num += 2;
    Ups_info[0].output.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //输出电流
    buf_Num += 2;
    Ups_info[0].output.frequency = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //输出频率
    buf_Num += 2;
    Ups_info[0].input.bypass_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; //旁路输入电压
    buf_Num += 2;
    Ups_info[0].output.temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];   //输出温度
    buf_Num += 2;
    ERR_UPS_Bypass[0] = JudgeBitInByte(Uart4Buf[buf_Num],2);        //旁路异常
    ERR_UPS_Overload[0] = JudgeBitInByte(Uart4Buf[buf_Num],3);      //过载
    ERR_UPS[0] = JudgeBitInByte(Uart4Buf[buf_Num],4);               //UPS故障
    ERR_UPS_Bypass_output[0] = JudgeBitInByte(Uart4Buf[buf_Num],5); //旁路输出
    ERR_UPS_DC[0] = JudgeBitInByte(Uart4Buf[buf_Num],6);            //直流异常
    ERR_UPS_MainsSupply[0] = JudgeBitInByte(Uart4Buf[buf_Num],7);   //市电异常
    buf_Num++;
    //UPS_2                                                                            
    Ups_info[1].input.AC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //UPS2交流输入电压
    buf_Num += 2;                                                                   
    Ups_info[1].output.voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //输出电压
    buf_Num += 2;                                                                   
    Ups_info[1].input.DC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //直流输入电压
    buf_Num += 2;                                                                   
    Ups_info[1].output.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //输出电流
    buf_Num += 2;                                                                   
    Ups_info[1].output.frequency = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //输出频率
    buf_Num += 2;                                                                   
    Ups_info[1].input.bypass_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; //旁路输入电压
    buf_Num += 2;                                                                   
    Ups_info[1].output.temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];   //输出温度
    buf_Num += 2;
    ERR_UPS_Bypass[1] = JudgeBitInByte(Uart4Buf[buf_Num],2);        //旁路异常
    ERR_UPS_Overload[1] = JudgeBitInByte(Uart4Buf[buf_Num],3);      //过载
    ERR_UPS[1] = JudgeBitInByte(Uart4Buf[buf_Num],4);               //UPS故障
    ERR_UPS_Bypass_output[1] = JudgeBitInByte(Uart4Buf[buf_Num],5); //旁路输出
    ERR_UPS_DC[1] = JudgeBitInByte(Uart4Buf[buf_Num],6);            //直流异常
    ERR_UPS_MainsSupply[1] = JudgeBitInByte(Uart4Buf[buf_Num],7);   //市电异常
    buf_Num++;
    for(i=0;i<2;i++){
        for(j=0;j<8;j++){  //j表示位
            Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);
            Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    ERR_UPS_Comm[0] = JudgeBitInByte(Uart4Buf[buf_Num],0);   //ups1通信故障
    ERR_UPS_Comm[1] = JudgeBitInByte(Uart4Buf[buf_Num],1);   //ups2通信故障
    buf_Num++;
    for(i=0;i<2;i++){  //i表示位
        ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart4Buf[buf_Num],i); //2个状态监控通信故障
    }
}
void receive_Comm_data_35K(){
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;
    for(i=0;i<4;i++){
        Comm_info[0].module.module_output_v[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //模块输出电压
        buf_Num += 2;
        Comm_info[0].module.module_output_i[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //模块输出电流
        buf_Num += 2;
        Comm_info[0].module.module_output_t[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //模块输出温度
        buf_Num += 2;
        ERR_Comm_Module[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],0);
        buf_Num++;
    }
    for(i=0;i<2;i++){       //2个状态采样盒
        for(j=0;j<8;j++){   //支路状态，32路
            Comm_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);
            Comm_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Comm_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Comm_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<4;i++){
        ERR_Comm_Module_comm[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],i); //一段4个模块通信故障
    }
    buf_Num++;
    for(i=0;i<2;i++){
        ERR_Comm_St_comm[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);     //一段2个状态监控通信故障
    }
}

/**************************************************************************
 *函数名:	receive_DC_FD_data
 *函数功能:	将接收直流电源分电柜馈线信息，包括显示用变量和故障标志变量。
 *参数:		void
 *返回值:	void
 ***************************************************************************/
void receive_DC_FD_data(unsigned int group)
{
    unsigned int buf_Num = 2; 
    //int i =0,j=0;
    buf_Num++;		//子功能码
    //AC_sneak_voltage =  (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; // 交流窜入电压
}
