#include "receive_data.h"
#include "global_define.h"
#include "globel_Ex.h"
#include "Global_Para_Ex.h"
#include "Subfunc.h"
#include "version.h"

#if 0
#define MAXCOUNT_GY_QY  3                //��Ƿѹȥ������
#else   //������ص����Ƿѹ����,�˲�������3������8��. 
#define MAXCOUNT_GY_QY  8                //��Ƿѹȥ������
#endif 
#define UPS_MODE_MAINS_SUPPLY   (0x01)   //UPS�е�ģʽ 
#define UPS_MODE_BAT_SUPPLY     (0x02)   //UPS���ģʽ
#define UPS_MODE_BY_PASS        (0x04)   //UPS��·ģʽ
#define UPS_MODE_OVER_HAUL      (0x08)   //UPS����ģʽ  
//#define  UPS_OUTPUT_CUR          (36)  //UPS��������36A
#define UPS_OUTPUT_CUR          (Sys_cfg_info.ups_set.Capacity * 0.8 / 220.0) //UPS��������

static char ERR_DC_BatterySingle_GY_CNT[3][127] = {{0}};   // ��ѹ����
static char ERR_DC_BatterySingle_GYHF_CNT[3][127] = {{0}}; // ��ѹ�ָ�����
static char ERR_DC_BatterySingle_QY_CNT[3][127] = {{0}};   // Ƿѹ����
static char ERR_DC_BatterySingle_QYHF_CNT[3][127] = {{0}}; // Ƿѹ�ָ�����

extern INT8U JudgeBitInByte(INT8U ByteData,INT8U BitPos);

//#define  TEST_DC_REC_NUM    //���ڲ���: ��ӡֱ����ط������ݵĴ���
#ifdef TEST_DC_REC_NUM
static unsigned int DC_REC_NUM_0x20 = 0;
static unsigned int DC_REC_NUM_0x28 = 0;
#endif 

/**********************************************************************************
 *������:	 receive_data
 *��������:  �������յ�����Ϣ,�����豸�ĵ�ַ��֡���Ⱥ͹����룬�ֱ��������ĺ���ת������
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
void receive_data(char Addr,char Len,char mark)
{
#if 1   //��������桱-��ϵͳ���á�-��ֱ��ϵͳ����������ѡ��ޡ�.  --ת��λ��
    //���ս���������� 
    if(Addr == 0x10 && Len == 0xC4){       //ȡ��������ص�Ԫ����--01
        receive_AC_data();            
    }else if(Addr == 0x11 && Len == 0x80){ //ȡ���߿���״̬����
        receive_AC_FeederLine_data();
    }else if(Addr == 0x10 && Len == 0xC6){ //ȡ��������ص�Ԫ����--02
        receive_AC_dc_data();
    } 

    //��������ͨ�ŵ�Դ������� 
    if(Addr == 0x31 && Len == 0x31 && mark == 0x31){
        receive_LNMKComm_data(0);   //ģ��ģ��������1��
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x31){
        receive_LNMKComm_data(1);   //ģ��ģ��������2��
    }else if(Addr == 0x31 && Len == 0x31 && mark == 0x33){
        receive_LNMKComm_status(0); //ģ�鿪����״̬1��
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x33){
        receive_LNMKComm_status(1); //ģ�鿪����״̬2��
    }else if(Addr == 0x31 && Len == 0x31 && mark == 0x34){
        receive_LNMKComm_alarm(0);  //�澯��Ϣ1��
    }else if(Addr == 0x32 && Len == 0x31 && mark == 0x34){
        receive_LNMKComm_alarm(1);  //�澯��Ϣ2��
    }else if(Addr == 0x31 && Len == 0x32 && mark == 0x31){
        receive_LNDCComm_data(0);   //ֱ��ģ���������Ϣ1��
    }else if(Addr == 0x32 && Len == 0x32 && mark == 0x31){
        receive_LNDCComm_data(1);   //ֱ��ģ���������Ϣ2��
    }


    if(Addr == 0x30 && Len == 0x8F){  //ȡ��1��ͨ�ŵ�Դ��ص�Ԫ����
        receive_Comm_data(0);
    }else if(Addr == 0x31 && Len == 0x8F){  //ȡ��2��ͨ�ŵ�Դ��ص�Ԫ����
        receive_Comm_data(1);
    }else if(Addr == 0x40 && Len == 0x42){  //ȡUPS��ص�Ԫ����
        receive_UPS_data();
    }
#endif

    unsigned int group;
    for(group = 0;group < Sys_set_DC_duan; group ++)  //Sys_set_DC_duan 1,2,3
    {  
#if 0   //��������桱-��ϵͳ���á�-��ֱ��ϵͳ����������ѡ��ޡ�.  --ת��λ��
        //���ս���������� 
        if(Addr == 0x10 && Len == 0xC4){       //ȡ��������ص�Ԫ����--01
            receive_AC_data();            
        }else if(Addr == 0x11 && Len == 0x80){ //ȡ���߿���״̬����
            receive_AC_FeederLine_data();
        }else if(Addr == 0x10 && Len == 0xC6){ //ȡ��������ص�Ԫ����--02
            receive_AC_dc_data();
        } 

        //��������ͨ�ŵ�Դ������� 
        if(Addr == 0x31 && Len == 0x31 && mark == 0x31){
            receive_LNMKComm_data(0);   //ģ��ģ��������1��
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x31){
            receive_LNMKComm_data(1);   //ģ��ģ��������2��
        }else if(Addr == 0x31 && Len == 0x31 && mark == 0x33){
            receive_LNMKComm_status(0); //ģ�鿪����״̬1��
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x33){
            receive_LNMKComm_status(1); //ģ�鿪����״̬2��
        }else if(Addr == 0x31 && Len == 0x31 && mark == 0x34){
            receive_LNMKComm_alarm(0);  //�澯��Ϣ1��
        }else if(Addr == 0x32 && Len == 0x31 && mark == 0x34){
            receive_LNMKComm_alarm(1);  //�澯��Ϣ2��
        }else if(Addr == 0x31 && Len == 0x32 && mark == 0x31){
            receive_LNDCComm_data(0);   //ֱ��ģ���������Ϣ1��
        }else if(Addr == 0x32 && Len == 0x32 && mark == 0x31){
            receive_LNDCComm_data(1);   //ֱ��ģ���������Ϣ2��
        } 
#endif

        //���յ�ز���������(ͨ�ŵ�Դ�����Ϣ)
        if (Addr == 0x81 && Len == 0x60){
            receive_TX_Battery_data(0);
        }else if (Addr == 0x81 && Len == 0x61){
            receive_TX_Battery_data(1);
        }else if (Addr == 0x81 && Len == 0x62){
            receive_TX_Battery_data(2);
        }else if (Addr == 0x81 && Len == 0x63){
            receive_TX_Battery_data(3);
        }

        //����ֱ�����ϵͳ����
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
                receive_DC_module_data(0);        //ȡ���ģ������
            }else if(Addr == 0x20 && Len == 0x72){
#ifdef TEST_DC_REC_NUM  //just for test
                if (DC_REC_NUM_0x20 < 0xFFFF)
                {
                    DC_REC_NUM_0x20 ++;
                }
                printf("DC_REC_NUM_0x20(0x72): %d.\n", DC_REC_NUM_0x20);
#endif           

                receive_DC_module_data_NUM12(0);  //ȡ���ģ������(12�����ģ��)
            }else if(Addr == 0x21 && Len == 0x21){
                receive_DC_monitor_data(0);       //ȡֱ��ĸ������
            }else if(Addr == 0x22){
                receive_DC_JY_data(0);            //ȡ��Ե����
            }else if(Addr == 0x23 && Len == 0x84){
                receive_DC_Switch_data(0);        //ȡ���߿�������
            }else if(Addr == 0x24 && Len == 0xDA 
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //�޶����ĵ��Ѳ��: ���ݴ�PSM-3��ȡ
                receive_DC_Battery_data(0);       //ȡ����������: �¶ȡ���ѹ 
            }else if(Addr == 0x50 && Len == 0x80){
                receive_DC_FenGui_data(0);        //���߿����������
            }else if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
            {
                if (Addr == 0xB0 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(0); //ȡ����������: ��ص�ѹ 
                }
                else if (Addr == 0xB0 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(0); //ȡ����������: �������
                }
                else if (Addr == 0xB0 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(0);  //ȡ����������: ���ң����
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
                receive_DC_module_data_NUM12(1);  //ȡ���ģ������(12�����ģ��)
            }else if(Addr == 0x29 && Len == 0x21){
                receive_DC_monitor_data(1);
            }else if(Addr == 0x2A){
                receive_DC_JY_data(1);
            }else if(Addr == 0x2B && Len == 0x84){
                receive_DC_Switch_data(1);
            }else if(Addr == 0x2C && Len == 0xDA
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //�޶����ĵ��Ѳ��: ���ݴ�PSM-3��ȡ
                receive_DC_Battery_data(1);
            }else if(Addr == 0x51 && Len == 0x80){
                receive_DC_FenGui_data(1);
            }else if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
            {
                if (Addr == 0xB1 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(1); //ȡ����������: ��ص�ѹ 
                }
                else if (Addr == 0xB1 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(1); //ȡ����������: �������
                }
                else if (Addr == 0xB1 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(1);  //ȡ����������: ���ң����
                }                
            }
        }
        else if (group == 2)
        {
            if(Addr == 0x25 && Len == 0x39){
                receive_DC_module_data(2);
            }else if(Addr == 0x25 && Len == 0x72){
                receive_DC_module_data_NUM12(2);  //ȡ���ģ������(12�����ģ��)
            }else if(Addr == 0x26 && Len == 0x21){
                receive_DC_monitor_data(2);
            }else if(Addr == 0x27){
                receive_DC_JY_data(2);
            }else if(Addr == 0x2D && Len == 0x84){
                receive_DC_Switch_data(2);
            }else if(Addr == 0x2E && Len == 0xDA
                    && (Fg_SysSet_BatteryCheckMode == 1)){ //�޶����ĵ��Ѳ��: ���ݴ�PSM-3��ȡ
                receive_DC_Battery_data(2);
            }else if(Addr == 0x52 && Len == 0x80){
                receive_DC_FenGui_data(2);
            }else if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
            {
                if (Addr == 0xB2 && Len == 0xDC && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Vol(2); //ȡ����������: ��ص�ѹ 
                }
                else if (Addr == 0xB2 && Len == 0xDA && mark == 0x03)
                {
                    receive_DC_Battery_data_PSMXB_Res(2); //ȡ����������: �������
                }
                else if (Addr == 0xB2 && Len == 0x10 && mark == 0x04)
                {
                    receive_DC_Battery_data_PSMXB_ST(2);  //ȡ����������: ���ң����
                }                
            }
        }

#if 0   //��������桱-��ϵͳ���á�-��ֱ��ϵͳ����������ѡ��ޡ�.  --ת��λ��
        if(Addr == 0x50 + group * 5 && Len == 0x01){
            //receive_DC_FD_data(group);		//�ֵ�
        }else if(Addr == 0x30 && Len == 0x8F){  //ȡ��1��ͨ�ŵ�Դ��ص�Ԫ����
            receive_Comm_data(0);
        }else if(Addr == 0x31 && Len == 0x8F){  //ȡ��2��ͨ�ŵ�Դ��ص�Ԫ����
            receive_Comm_data(1);
        }else if(Addr == 0x40 && Len == 0x42){  //ȡUPS��ص�Ԫ����
            receive_UPS_data();
        }else if(Addr == 0x80 && Len == 0x01){
            ;
        }else {;}
#endif 
    }

#if 0
    //���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3����¡�ͨ�ŵ�Դ��ַ 0x25
    //���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3����¡�����Դ��ַ 0x26
    if(Addr == 0x25 && Special_35KV_flag == 2){       
        receive_Comm_data_35K();
    }else if(Addr == 0x26 && Special_35KV_flag == 2){ 
        receive_UPS_data_35K();
    }else{;}
#else //������������ͨ�š�UPS��Դ��ص�ѡ����. 
    //���⹤�̣�ͨ�ŵ�Դ������Դ����PSM-3����¡�ͨ�ŵ�Դ��ַ 0x25������Դ��ַ 0x26
    if(Addr == 0x25 && ((Special_35KV_flag == 2) 
                || (Special_35KV_flag_NoCommMon_WithUpsMon == 2)))
    {   //����ͨ�ŵ�Դ���ʱ    
        receive_Comm_data_35K();
    }
    else if(Addr == 0x26 && ((Special_35KV_flag == 2)
                || (Special_35KV_flag_NoUpsMon_WithCommMon == 2)))
    {   //����UPS���ʱ 
        receive_UPS_data_35K();
    }    
#endif 

    //ȡATSE����
    if (Sys_cfg_info.ac_set.control_mode == 1){       //ATS(��̩)
        if(Addr == ATSE_Addr_1 && Len == 0x01){       //ATSE_Addr_1 0x01
            receive_ATSE_state(0);
        }else if(Addr == ATSE_Addr_1 && Len == 0x03){
#if 0
            receive_ATSE_data(0);
#else    //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��.
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
#else    //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��.
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data(1);
            }
#endif 
        }
    }
    else if (Sys_cfg_info.ac_set.control_mode == 3)   //ATS(����)  //�����ͺ���ATSͨѶ֧��.
    {
        if(Addr == ATSE_Addr_1 && Len == 0x03 && (Uart1Buf[2] == 0x14))
        {    
            receive_ATSE_state_HanKwang(0); //����ң������(500~509)
        }
        else if(Addr == ATSE_Addr_1 && Len == 0x03 && (Uart1Buf[2] == 0x98))
        {
#if 0
            receive_ATSE_data_HanKwang(0);  //����ң������(1000~1075)
#else    //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��.
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data_HanKwang(0);  //����ң������(1000~1075)            
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
#else   //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��. 
            if (Sys_cfg_info.ac_set.source_of_ac_data == 0x00)
            {
                receive_ATSE_data_HanKwang(1);
            }
#endif 
        }    
    }
}
/**************************************************************************
 *������:	receive_ATSE_state(��̩)
 *��������:	�����յ�ATSE����״̬��ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:		void
 *����ֵ:	void
 ***************************************************************************/
void receive_ATSE_state(unsigned char num)
{
    Ac_info.atse_stat[num].state = JudgeBitInByte(Uart1Buf[3],4); //�Զ����ֶ�:	Ϊ1�Զ���Ϊ0�ֶ�
    if(JudgeBitInByte(Uart1Buf[3],0) == 1){       //1#����״̬: Ϊ1��բ��Ϊ0��բ
        Ac_info.atse_stat[num].sw = 1;  //1·
    }else if(JudgeBitInByte(Uart1Buf[3],2) == 1){ //2#����״̬: Ϊ1��բ��Ϊ0��բ
        Ac_info.atse_stat[num].sw = 2;  //2·
    }else{
        Ac_info.atse_stat[num].sw = 3;  //��բ
    }
}
/**************************************************************************
 *������:	receive_ATSE_data(��̩)
 *��������:	�����յ�ATSE�������ݸ�ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:		void
 *����ֵ:	void
 ***************************************************************************/
void receive_ATSE_data(unsigned char num)
{
    unsigned int tmp_data = 0x00; //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    unsigned int buf_Num = 15; 
    int j=0;	
    num = num*2;
    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num].Voltage[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*10;		//һ·���ߵ�ѹABC
        buf_Num +=2;
    }
    Ac_info.ac_in_data[num].Voltage[3] = (Ac_info.ac_in_data[num].Voltage[0]+Ac_info.ac_in_data[num].Voltage[1]+Ac_info.ac_in_data[num].Voltage[2])/3;  //��ѹ����

    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num+1].Voltage[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*10;		//��·���ߵ�ѹABC
        buf_Num +=2;
    }
    Ac_info.ac_in_data[num+1].Voltage[3] = (Ac_info.ac_in_data[num+1].Voltage[0]+Ac_info.ac_in_data[num+1].Voltage[1]+Ac_info.ac_in_data[num+1].Voltage[2])/3;  //��������

#if 0
    for(j=0;j<3;j++){
        Ac_info.ac_in_data[num].Current[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//һ·���ߵ��� A
        Ac_info.ac_in_data[num+1].Current[j] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//��·���ߵ��� A
        buf_Num +=2;
    }
#else //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS). 
    for(j = 0; j < 3; j++)
    {
        //���ߵ���
        tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;  
        buf_Num += 0x02;

        Ac_info.ac_in_data[num].Current[j] = 0x00;     //һ·���ߵ��� A
        Ac_info.ac_in_data[num + 1].Current[j] = 0x00; //��·���ߵ��� A  
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
        Ac_info.ac_in_data[num].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//һ·����Ƶ��
    }
    buf_Num +=2;
    for(j=0;j<4;j++){
        Ac_info.ac_in_data[num+1].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//��·����Ƶ��
    }
    buf_Num +=2;

#if 0
    Ac_info.ac_in_data[num].ActivePower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//һ·�����й�����;
    Ac_info.ac_in_data[num+1].ActivePower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//��·�����й�����;
    Ac_info.ac_in_data[num].ActivePower[0] = Ac_info.ac_in_data[num].ActivePower[1] = Ac_info.ac_in_data[num].ActivePower[2] = Ac_info.ac_in_data[num].ActivePower[3]/3;
    Ac_info.ac_in_data[num+1].ActivePower[0] = Ac_info.ac_in_data[num+1].ActivePower[1] = Ac_info.ac_in_data[num+1].ActivePower[2] = Ac_info.ac_in_data[num+1].ActivePower[3]/3;
    buf_Num +=2;

    Ac_info.ac_in_data[num].ApparentPower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//һ·�������ڹ���
    Ac_info.ac_in_data[num+1].ApparentPower[3] = ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1])*100;		//��·�������ڹ���
    Ac_info.ac_in_data[num].ApparentPower[0] = Ac_info.ac_in_data[num].ApparentPower[1] = Ac_info.ac_in_data[num].ApparentPower[2] = Ac_info.ac_in_data[num].ApparentPower[3]/3;
    Ac_info.ac_in_data[num+1].ApparentPower[0] = Ac_info.ac_in_data[num+1].ApparentPower[1] = Ac_info.ac_in_data[num+1].ApparentPower[2] = Ac_info.ac_in_data[num+1].ApparentPower[3]/3;
    buf_Num +=2;		

    Ac_info.ac_in_data[num].ReactivePower[3] = Ac_info.ac_in_data[num].ApparentPower[3] - Ac_info.ac_in_data[num].ActivePower[3];
    Ac_info.ac_in_data[num+1].ReactivePower[3] = Ac_info.ac_in_data[num+1].ApparentPower[3] - Ac_info.ac_in_data[num+1].ActivePower[3];
    Ac_info.ac_in_data[num].ReactivePower[0] = Ac_info.ac_in_data[num].ReactivePower[1] = Ac_info.ac_in_data[num].ReactivePower[2] = Ac_info.ac_in_data[num].ReactivePower[3]/3;
    Ac_info.ac_in_data[num+1].ReactivePower[0] = Ac_info.ac_in_data[num+1].ReactivePower[1] = Ac_info.ac_in_data[num+1].ReactivePower[2] = Ac_info.ac_in_data[num+1].ReactivePower[3]/3;

    Ac_info.ac_in_data[num].PowerFactor[3]	= ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]);		//һ·���߹�������
    Ac_info.ac_in_data[num+1].PowerFactor[3]	= ((Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]);		//��·���߹�������
#else  //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS). 
    //�й����� -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;	
    buf_Num += 0x02;

    //����
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

    //ABC�� 
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ActivePower[j] = Ac_info.ac_in_data[num].ActivePower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ActivePower[j] = Ac_info.ac_in_data[num + 1].ActivePower[3] / 3;    
    }    

    //���ڹ��� -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 100;	
    buf_Num += 0x02;

    //����
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
    //ABC��
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ApparentPower[j] = Ac_info.ac_in_data[num].ApparentPower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ApparentPower[j] = Ac_info.ac_in_data[num + 1].ApparentPower[3] / 3;    
    }

    //�޹����� -----------------------------------------------------------
#if 0
    //����
    Ac_info.ac_in_data[num].ReactivePower[3] = Ac_info.ac_in_data[num].ApparentPower[3] - Ac_info.ac_in_data[num].ActivePower[3];
    Ac_info.ac_in_data[num + 1].ReactivePower[3] = Ac_info.ac_in_data[num + 1].ApparentPower[3] - Ac_info.ac_in_data[num + 1].ActivePower[3];    
#else  //�޸��й��޹����ڹ��ʼ�������.
    Ac_info.ac_in_data[num].ReactivePower[3] = 
        sqrt(Ac_info.ac_in_data[num].ApparentPower[3] * Ac_info.ac_in_data[num].ApparentPower[3]  
                - Ac_info.ac_in_data[num].ActivePower[3] * Ac_info.ac_in_data[num].ActivePower[3]);
    Ac_info.ac_in_data[num + 1].ReactivePower[3] = 
        sqrt(Ac_info.ac_in_data[num + 1].ApparentPower[3] * Ac_info.ac_in_data[num + 1].ApparentPower[3] 
                - Ac_info.ac_in_data[num + 1].ActivePower[3] * Ac_info.ac_in_data[num + 1].ActivePower[3]); 
#endif 
    //ABC��
    for (j = 0; j < 3; j ++)
    {
        Ac_info.ac_in_data[num].ReactivePower[j] = Ac_info.ac_in_data[num].ReactivePower[3] / 3; 
        Ac_info.ac_in_data[num + 1].ReactivePower[j] = Ac_info.ac_in_data[num + 1].ReactivePower[3] / 3;    
    }

    //�������� -----------------------------------------------------------
    tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]);	
    buf_Num += 0x02;

    //����
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
 *������:	receive_ATSE_state_HanKwang(����)(����ң������500~509)
 *��������:	�����յ�ATSE����״̬��ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:		void
 *����ֵ:	void
 ***************************************************************************/
void receive_ATSE_state_HanKwang(unsigned char num)
{
    //500
    //�Զ����ֶ�:	Ϊ1�Զ���Ϊ0�ֶ�
    Ac_info.atse_stat[num].state = JudgeBitInByte(Uart1Buf[3],0); 

    //507
    if (JudgeBitInByte(Uart1Buf[18],4) == 1) //1#����״̬: Ϊ1��բ��Ϊ0��բ
    {           
        Ac_info.atse_stat[num].sw = 1;  //1·
    }
    else if (JudgeBitInByte(Uart1Buf[18],5) == 1) //2#����״̬: Ϊ1��բ��Ϊ0��բ
    { 
        Ac_info.atse_stat[num].sw = 2;  //2·
    }
    else
    {
        Ac_info.atse_stat[num].sw = 3;  //��բ
    }
}
/**************************************************************************
 *������:	receive_ATSE_data_HanKwang(����)(����ң������1000~1075)
 *��������:	�����յ�ATSE�������ݸ�ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:		void
 *����ֵ:	void
 ***************************************************************************/
void receive_ATSE_data_HanKwang(unsigned char num)
{
    unsigned int buf_Num = 3; 
    int i = 0;
    int j = 0;	
    num = num * 2;
    unsigned int tmp_data = 0x00; //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).

    //1000: 
    buf_Num += 0x06;

    //1003~5: ��ѹUA1 UB1 UC1
    for (j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num].Voltage[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10; //һ·���ߵ�ѹABC
        buf_Num += 0x02;
    }
    Ac_info.ac_in_data[num].Voltage[3] = (Ac_info.ac_in_data[num].Voltage[0]
            + Ac_info.ac_in_data[num].Voltage[1] + Ac_info.ac_in_data[num].Voltage[2]) / 3;  //��ѹ����

    //1006~8
    buf_Num += 0x06;

    //1009: Ƶ��1  �����ԣ���������
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //��ֵʱ��Ϊ0����
            Ac_info.ac_in_data[num].Frequency[j] = 0x00; 
        }
        else //��ֵʱ��������
        {
            Ac_info.ac_in_data[num].Frequency[j] 
                = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) / 10; //һ·����Ƶ��
        }        
    }
    buf_Num += 0x02;

    //1010~1019
    buf_Num += 0x14;

    //1020:
    buf_Num += 0x06;

    //---------------------------------------------------
    //1023~1025: ��ѹUA2 UB2 UC2
    for (j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num + 1].Voltage[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10; //��·���ߵ�ѹABC
        buf_Num += 2;
    }
    Ac_info.ac_in_data[num + 1].Voltage[3] = (Ac_info.ac_in_data[num + 1].Voltage[0]
            + Ac_info.ac_in_data[num + 1].Voltage[1] + Ac_info.ac_in_data[num + 1].Voltage[2]) / 3;  //��������

    //1026~1028
    buf_Num += 0x06;

    //1029: Ƶ��2   �����ԣ���������
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //��ֵʱ��Ϊ0����
            Ac_info.ac_in_data[num + 1].Frequency[j] = 0x00; 
        }
        else //��ֵʱ��������
        {
            Ac_info.ac_in_data[num + 1].Frequency[j] 
                = ((Uart1Buf[buf_Num] << 8)| Uart1Buf[buf_Num + 1]) / 10; //��·����Ƶ��
        }
    }
    buf_Num += 0x02;

    //---------------------------------------------------
    //1030~1039
    buf_Num += 0x14;

    //1040~1042
    //ABC�����
#if 0
    for(j = 0; j < 3; j++)
    {
        Ac_info.ac_in_data[num].Current[j] = ((Uart1Buf[buf_Num] << 8) 
                | Uart1Buf[buf_Num + 1]) * 10;	   //һ·���ߵ��� A
        Ac_info.ac_in_data[num + 1].Current[j] = ((Uart1Buf[buf_Num] << 8)
                | Uart1Buf[buf_Num + 1]) * 10;     //��·���ߵ��� A
        buf_Num += 0x02;
    }
#else   //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    for(j = 0; j < 3; j++)
    {
        //���ߵ��� 
        tmp_data = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;
        buf_Num += 0x02;

        Ac_info.ac_in_data[num].Current[j] = 0x00;     //һ·���ߵ��� A
        Ac_info.ac_in_data[num + 1].Current[j] = 0x00; //��·���ߵ��� A  
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

    //1048~1055: �й�����: 16�ֽ�(4*4)  �����ԣ���������
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //��ֵʱ��Ϊ0����
            Ac_info.ac_in_data[num].ActivePower[j] = 0x00;
        }
        else //��ֵʱ�������� 
        {   //�޸�����ATSͨѶ���й��޹����ʽ�������.
            Ac_info.ac_in_data[num].ActivePower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //һ·�����й�����
        }

        Ac_info.ac_in_data[num + 1].ActivePower[j] = Ac_info.ac_in_data[num].ActivePower[j];                   //��·�����й�����
        buf_Num += 0x04;
    }
#else   //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    for (j = 0; j < 4; j++)
    {
        //�й����� 
        if (Uart1Buf[buf_Num] & 0x80) //��ֵʱ��Ϊ0����
        {
            tmp_data = 0x00;   
        }
        else //��ֵʱ�������� 
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

    //1056~1063: �޹�����: 16�ֽ�(4*4)  �����ԣ���������
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //��ֵʱ��Ϊ0����
            Ac_info.ac_in_data[num].ReactivePower[j] = 0x00;
        }
        else //��ֵʱ�������� 
        {    //�޸�����ATSͨѶ���й��޹����ʽ�������.
            Ac_info.ac_in_data[num].ReactivePower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //һ·�����޹�����
        }

        Ac_info.ac_in_data[num + 1].ReactivePower[j] = Ac_info.ac_in_data[num].ReactivePower[j];               //��·�����޹�����
        buf_Num += 0x04;
    }
#else   //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    for (j = 0; j < 4; j++)
    {
        //�޹�����
        if (Uart1Buf[buf_Num] & 0x80) //��ֵʱ��Ϊ0����
        {
            tmp_data = 0x00;   
        }
        else //��ֵʱ�������� 
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

    //1064~1071: ���ڹ���: 16�ֽ�(4*4)   �����ԣ���������
#if 0
    for (j = 0; j < 4; j++)
    {
        if (Uart1Buf[buf_Num] & 0x80)
        {
            //��ֵʱ��Ϊ0����
            Ac_info.ac_in_data[num].ApparentPower[j] = 0x00;
        }
        else //��ֵʱ�������� 
        {    //�޸�����ATSͨѶ���й��޹����ʽ�������.
            Ac_info.ac_in_data[num].ApparentPower[j] = ((Uart1Buf[buf_Num] << 8) 
                    | Uart1Buf[buf_Num + 1]) * 10;     //һ·�������ڹ���
        }

        Ac_info.ac_in_data[num + 1].ApparentPower[j] = Ac_info.ac_in_data[num].ApparentPower[j];               //��·�������ڹ���
        buf_Num += 0x04;
    }
#else   //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    for (j = 0; j < 4; j++)
    {
        //���ڹ���
        if (Uart1Buf[buf_Num] & 0x80) //��ֵʱ��Ϊ0����
        {
            tmp_data = 0x00;   
        }
        else //��ֵʱ�������� 
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

    //1072~1075: ��������(����������*100 * 10 / 1000������ʾ)         
#if 0
    for (j = 0; j < 4; j++)
    {
        Ac_info.ac_in_data[num].PowerFactor[j] = ((Uart1Buf[buf_Num] << 8) | Uart1Buf[buf_Num + 1]) * 10;     //һ·���߹�������
        Ac_info.ac_in_data[num + 1].PowerFactor[j] = Ac_info.ac_in_data[num].PowerFactor[j];                  //��·���߹�������
        buf_Num += 0x02;
    }
#else   //����ATS��բ��һ·���������й����޹������ڡ�������������(��̩+����ATS).
    for (j = 0; j < 4; j++)
    {
        //��������
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
 *������:			receive_AC_data   0X10(0xC4)
 *��������:		�����յĽ���������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void receive_AC_data(){
    unsigned int buf_Num = 2; 
    int i =0,j=0;

#if 1  //�޸��ͺ�̨֮���3����ٵ�ң��ָ��:��̨ң�ص���߼�����
    for (i = 0; i < 5; i ++)
    {
        if (diancao_cmd_num[i] > 0x00)
        {
            diancao_cmd_num[i] --;
        }
    }
#endif 
    if ((Sys_cfg_info.ac_set.control_mode != 1) 
            && (Sys_cfg_info.ac_set.control_mode != 3))  //�����ͺ���ATSͨѶ֧��.
    {    //������ʽ������ATS������ʽʱ����������Ľ�������ʽ���ڽ������ý������õ�
        for(i=0;i<4;i++){
            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//һ·���ߵ�ѹ
                buf_Num +=2;
            }
            for(j=0;j<3;j++)
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//Ƶ��(������ֻ��һ��Ƶ�����ݣ�Ϊ����ʾ3������ͬһ��ֵ�ŵ�3��λ���ϣ�for��Ϊ�˷�λ��)		
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//һ·���ߵ���
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//���й�����
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//�й�����
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//���޹�����
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//�޹�����
                buf_Num +=2;
            }

#if 0
            Ac_info.ac_in_data[i].ApparentPower[3] = Ac_info.ac_in_data[i].ReactivePower[3] + Ac_info.ac_in_data[i].ActivePower[3];  //���ڹ���
#else       //�޸��й��޹����ڹ��ʼ�������.
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);  //���ڹ���                        
            }
#endif 

            Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//��������
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//��������
                buf_Num +=2;
            }
        }
    }
#if 1   //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��.
    else if (Sys_cfg_info.ac_set.source_of_ac_data != 0x00)   //��ATSʱ,���Ǵ�ATS��(�ӽ�����ض�)
    {
        for(i = 0; i < 2; i++)  //��������������ݡ�����ATS״̬ת������·��������
        {
            //һ·���ߵ�ѹ
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3·��ѹ=1·��ѹ	    
                Ac_info.ac_in_data[i + 2].Voltage[j] = Ac_info.ac_in_data[i].Voltage[j];      
                buf_Num +=2;
            }

            //Ƶ��
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3·Ƶ��=1·Ƶ��
                Ac_info.ac_in_data[i + 2].Frequency[j] = Ac_info.ac_in_data[i].Frequency[j]; 
            }                
            buf_Num +=2;

            //-------------------------------------
            //������13��24·����
            //���ߵ���
            for(j = 0; j < 3;j++)
            {
                Ac_info.ac_in_data[i].Current[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].Current[j] = 0x00;	   
            }

            //�й�����
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ActivePower[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].ActivePower[j] = 0x00;
            }

            //�޹�����
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ReactivePower[j] = 0x00;
                Ac_info.ac_in_data[i + 2].ReactivePower[j] = 0x00;
            }

            //��������
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].PowerFactor[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].PowerFactor[j] = 0x00;
            }

            //-------------------------------------
            //����ATS1��2����բ״̬����4·��������
            if ((Ac_info.atse_stat[0].sw == (0x01 + i))         //ATS1--1·��բ �� ATS1--2·��բ
                    && (Ac_info.atse_stat[1].sw == (0x01 + i))) //ATS2--1·��բ �� ATS2--2·��բ
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].Current[j] = Ac_info.ac_in_data[i].Current[j];
                }

                //���й�����
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ActivePower[3] = Ac_info.ac_in_data[i].ActivePower[3];

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = Ac_info.ac_in_data[i].ActivePower[j];
                }

                //���޹�����
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = Ac_info.ac_in_data[i].ReactivePower[3];

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = Ac_info.ac_in_data[i].ReactivePower[j];
                }

                //��������
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = Ac_info.ac_in_data[i].PowerFactor[3];
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = Ac_info.ac_in_data[i].PowerFactor[j];
                }
            }
            else if (Ac_info.atse_stat[0].sw == (0x01 + i)) //ֻ��(ATS1--1·��բ �� ATS1--2·��բ)
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���й�����
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���޹�����
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //��������
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else if (Ac_info.atse_stat[1].sw == (0x01 + i)) //ֻ��(ATS2--1·��բ �� ATS2--2·��բ)
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���й�����
                Ac_info.ac_in_data[i + 2].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���޹�����
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //��������
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else  //ATS1��բ �� ATS2��բ
            {
                buf_Num += 30;
            }

            //---------------------------------------------
            //���ڹ��� 
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
        Ac_info.busbar[0].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ�ߵ�ѹ
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ�ߵ���
        buf_Num +=2;
    }
    Ac_info.busbar[0].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ���������
    buf_Num +=2;

    for(i=0;i<3;i++){
        Ac_info.busbar[1].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[1].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
        buf_Num +=2;
    }
    Ac_info.busbar[1].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
    buf_Num +=2;

    Ac_info.ac_measure_I[0] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];					//��������
    buf_Num +=2;
    Ac_info.ac_measure_I[1] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;
    Ac_info.ac_measure_I[2] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;


    Ac_info.ac_in_data[0].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],0);		//���߿��ص��״̬		
    Ac_info.ac_in_data[1].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],1);
    if(Sys_cfg_info.ac_set.diancao_num == 3){       //���3
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],2);
    }else if(Sys_cfg_info.ac_set.diancao_num == 5){ //���5
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//���߿��ص��״̬		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],4);
    }else{ //�������: ATS���ƻ�(��ٲ���3��5)
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//���߿��ص��״̬		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        ERR_AC_in_SW_trip[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);               //���߿��ص����բ
    }

    ERR_AC_in_SW_trip[0] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_AC_in_SW_trip[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_in_SW_trip[2] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;

    ERR_AC_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);							//����������
    ERR_AC_SPD[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<6;i++)
        ERR_AC_in_V[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//���ߵ�ѹ�쳣
    ERR_AC_mu_V[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_mu_V[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);							//ĸ�ߵ�ѹ�쳣
    buf_Num++;

    ERR_AC_AcSample_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);				//��������ͨѶ����
    ERR_AC_AcSample_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_SW_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//����������ͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_SW_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_SW_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_SW_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);	
    for(i=2;i<8;i++)
        ERR_AC_St_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//����״̬��ͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_St_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_St_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_St_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_CurrentSample_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);	//��������������ԪͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_CurrentSample_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    for(i=0;i<6;i++)
        ERR_AC_CurrentSample_comm[i+14] = JudgeBitInByte(Uart1Buf[buf_Num],i);
#if 0
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);					//������ATSEͨѶ�쳣
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
#else  //�޸��ܼ�غͺ�̨�������1��2ͨѶ���ϵ�����. 
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);					//������ATSEͨѶ�쳣
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
#endif 

    buf_Num++;
    ERR_AC_Feeder_Duan_trip_1 = JudgeBitInByte(Uart1Buf[buf_Num],0); 			//һ��������բ
    ERR_AC_Feeder_Duan_trip_2 = JudgeBitInByte(Uart1Buf[buf_Num],1);

    ERR_AC_SPD[2] = JudgeBitInByte(Uart1Buf[buf_Num],2);                 //3#����������
    ERR_AC_Meter_comm[2] = JudgeBitInByte(Uart1Buf[buf_Num],3);					//���ͨѶ�쳣
    ERR_AC_Meter_comm[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);	
}
/**************************************************************************
 *������:			receive_AC_dc_data   0X10(0xC6)
 *��������:		�����յĽ���������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void receive_AC_dc_data(){
    unsigned int buf_Num = 2; 
    int i =0,j=0;

    if ((Sys_cfg_info.ac_set.control_mode != 1)
            && (Sys_cfg_info.ac_set.control_mode != 3))  //�����ͺ���ATSͨѶ֧��.
    {    //������ʽ������ATS������ʽʱ����������Ľ�������ʽ���ڽ������ý������õ�
        for(i=0;i<4;i++){
            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//һ·���ߵ�ѹ
                buf_Num +=2;
            }
            for(j=0;j<3;j++)
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//Ƶ��(������ֻ��һ��Ƶ�����ݣ�Ϊ����ʾ3������ͬһ��ֵ�ŵ�3��λ���ϣ�for��Ϊ�˷�λ��)		
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//һ·���ߵ���
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//���й�����
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//�й�����
                buf_Num +=2;
            }

            Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//���޹�����
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//�޹�����
                buf_Num +=2;
            }

#if 0
            Ac_info.ac_in_data[i].ApparentPower[3] = Ac_info.ac_in_data[i].ReactivePower[3] + Ac_info.ac_in_data[i].ActivePower[3];  //���ڹ���
#else       //�޸��й��޹����ڹ��ʼ�������.
            for (j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ApparentPower[j] = 
                    sqrt(Ac_info.ac_in_data[i].ReactivePower[j] * Ac_info.ac_in_data[i].ReactivePower[j] 
                            + Ac_info.ac_in_data[i].ActivePower[j] * Ac_info.ac_in_data[i].ActivePower[j]);  //���ڹ���                        
            }
#endif 

            Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//��������
            buf_Num +=2;

            for(j=0;j<3;j++){
                Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		//��������
                buf_Num +=2;
            }
        }
    }
#if 1   //���ƹ��ܣ���������������Ϣ��Դ����ATS��������ѡ��.
    else if (Sys_cfg_info.ac_set.source_of_ac_data != 0x00)   //��ATSʱ,���Ǵ�ATS��(�ӽ�����ض�)
    {
        for(i = 0; i < 2; i++)  //��������������ݡ�����ATS״̬ת������·��������
        {
            //һ·���ߵ�ѹ
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Voltage[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3·��ѹ=1·��ѹ	    
                Ac_info.ac_in_data[i + 2].Voltage[j] = Ac_info.ac_in_data[i].Voltage[j];      
                buf_Num +=2;
            }

            //Ƶ��
            for(j=0;j<3;j++)
            {
                Ac_info.ac_in_data[i].Frequency[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
                //3·Ƶ��=1·Ƶ��
                Ac_info.ac_in_data[i + 2].Frequency[j] = Ac_info.ac_in_data[i].Frequency[j]; 
            }                
            buf_Num +=2;

            //-------------------------------------
            //������13��24·����
            //���ߵ���
            for(j = 0; j < 3;j++)
            {
                Ac_info.ac_in_data[i].Current[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].Current[j] = 0x00;	   
            }

            //�й�����
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ActivePower[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].ActivePower[j] = 0x00;
            }

            //�޹�����
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].ReactivePower[j] = 0x00;
                Ac_info.ac_in_data[i + 2].ReactivePower[j] = 0x00;
            }

            //��������
            for(j = 0; j < 4; j++)
            {
                Ac_info.ac_in_data[i].PowerFactor[j] = 0x00;		
                Ac_info.ac_in_data[i + 2].PowerFactor[j] = 0x00;
            }

            //-------------------------------------
            //����ATS1��2����բ״̬����4·��������
            if ((Ac_info.atse_stat[0].sw == (0x01 + i))         //ATS1--1·��բ �� ATS1--2·��բ
                    && (Ac_info.atse_stat[1].sw == (0x01 + i))) //ATS2--1·��բ �� ATS2--2·��բ
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].Current[j] = Ac_info.ac_in_data[i].Current[j];
                }

                //���й�����
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ActivePower[3] = Ac_info.ac_in_data[i].ActivePower[3];

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = Ac_info.ac_in_data[i].ActivePower[j];
                }

                //���޹�����
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = Ac_info.ac_in_data[i].ReactivePower[3];

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = Ac_info.ac_in_data[i].ReactivePower[j];
                }

                //��������
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = Ac_info.ac_in_data[i].PowerFactor[3];
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = Ac_info.ac_in_data[i].PowerFactor[j];
                }
            }
            else if (Ac_info.atse_stat[0].sw == (0x01 + i)) //ֻ��(ATS1--1·��բ �� ATS1--2·��բ)
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���й�����
                Ac_info.ac_in_data[i].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���޹�����
                Ac_info.ac_in_data[i].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //��������
                Ac_info.ac_in_data[i].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else if (Ac_info.atse_stat[1].sw == (0x01 + i)) //ֻ��(ATS2--1·��բ �� ATS2--2·��բ)
            {
                //���ߵ���
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].Current[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���й�����
                Ac_info.ac_in_data[i + 2].ActivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�й�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ActivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //���޹�����
                Ac_info.ac_in_data[i + 2].ReactivePower[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;

                //�޹�����
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].ReactivePower[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }

                //��������
                Ac_info.ac_in_data[i + 2].PowerFactor[3] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                buf_Num +=2;
                //��������
                for(j=0;j<3;j++){
                    Ac_info.ac_in_data[i + 2].PowerFactor[j] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];		
                    buf_Num +=2;
                }
            }
            else  //ATS1��բ �� ATS2��բ
            {
                buf_Num += 30;
            }

            //---------------------------------------------
            //���ڹ��� 
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
        Ac_info.busbar[0].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ�ߵ�ѹ
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[0].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ�ߵ���
        buf_Num +=2;
    }
    Ac_info.busbar[0].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//һ��ĸ���������
    buf_Num +=2;

    for(i=0;i<3;i++){
        Ac_info.busbar[1].Voltage[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
        buf_Num +=2;
    }
    for(i=0;i<3;i++){
        Ac_info.busbar[1].Current[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
        buf_Num +=2;
    }
    Ac_info.busbar[1].Residual_Current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];				//����ĸ��
    buf_Num +=2;

    Ac_info.ac_measure_I[0] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];					//��������
    buf_Num +=2;
    Ac_info.ac_measure_I[1] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;
    Ac_info.ac_measure_I[2] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];
    buf_Num +=2;


    Ac_info.ac_in_data[0].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],0);		//���߿��ص��״̬		
    Ac_info.ac_in_data[1].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],1);
    if(Sys_cfg_info.ac_set.diancao_num == 3){       //���3
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],2);
    }else if(Sys_cfg_info.ac_set.diancao_num == 5){ //���5
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//���߿��ص��״̬		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        Ac_info.ML_state = JudgeBitInByte(Uart1Buf[buf_Num],4);
    }else{  //�������: ATS���ƻ�(��ٲ���3��5)
        Ac_info.ac_in_data[2].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],2);		//���߿��ص��״̬		
        Ac_info.ac_in_data[3].SW_state = JudgeBitInByte(Uart1Buf[buf_Num],3);
        ERR_AC_in_SW_trip[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);               //���߿��ص����բ
    }

    ERR_AC_in_SW_trip[0] = JudgeBitInByte(Uart1Buf[buf_Num],5);
    ERR_AC_in_SW_trip[1] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_in_SW_trip[2] = JudgeBitInByte(Uart1Buf[buf_Num],7);
    buf_Num++;

    ERR_AC_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);							//����������
    ERR_AC_SPD[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<6;i++)
        ERR_AC_in_V[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//���ߵ�ѹ�쳣
    ERR_AC_mu_V[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);
    ERR_AC_mu_V[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);							//ĸ�ߵ�ѹ�쳣
    buf_Num++;

    ERR_AC_AcSample_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);				//��������ͨѶ����
    ERR_AC_AcSample_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_SW_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//����������ͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_SW_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_SW_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_SW_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);	
    for(i=2;i<8;i++)
        ERR_AC_St_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);					//����״̬��ͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_St_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    ERR_AC_St_comm[14] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_AC_St_comm[15] = JudgeBitInByte(Uart1Buf[buf_Num],1);
    for(i=2;i<8;i++)
        ERR_AC_CurrentSample_comm[i-2] = JudgeBitInByte(Uart1Buf[buf_Num],i);	//��������������ԪͨѶ����
    buf_Num++;
    for(i=0;i<8;i++)
        ERR_AC_CurrentSample_comm[i+6] = JudgeBitInByte(Uart1Buf[buf_Num],i);
    buf_Num++;

    for(i=0;i<6;i++)
        ERR_AC_CurrentSample_comm[i+14] = JudgeBitInByte(Uart1Buf[buf_Num],i);

#if 0
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);					//������ATSEͨѶ�쳣
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
#else  //�޸��ܼ�غͺ�̨�������1��2ͨѶ���ϵ�����. 
    ERR_AC_Meter_comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);					//������ATSEͨѶ�쳣
    ERR_AC_Meter_comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],7);
#endif 

    buf_Num++;
    ERR_AC_Feeder_Duan_trip_1 = JudgeBitInByte(Uart1Buf[buf_Num],0); 			//һ��������բ
    ERR_AC_Feeder_Duan_trip_2 = JudgeBitInByte(Uart1Buf[buf_Num],1);

    ERR_AC_SPD[2] = JudgeBitInByte(Uart1Buf[buf_Num],2);                 //3#����������
    ERR_AC_Meter_comm[2] = JudgeBitInByte(Uart1Buf[buf_Num],3);					//���ͨѶ�쳣
    ERR_AC_Meter_comm[3] = JudgeBitInByte(Uart1Buf[buf_Num],4);
    ERR_AC_BT_device =	JudgeBitInByte(Uart1Buf[buf_Num],5);       //��Ͷװ�ù���
    Ac_info.BTDZ_device[0] = JudgeBitInByte(Uart1Buf[buf_Num],6);    //1#��Ͷװ�ñ�Ͷ����
    Ac_info.BTDZ_device[1]= JudgeBitInByte(Uart1Buf[buf_Num],7);    //2#��Ͷװ�ñ�Ͷ����
    buf_Num++;
    ERR_AC_BT_device_he[0]  = JudgeBitInByte(Uart1Buf[buf_Num],0);    //1#��غ�բʧ��
    ERR_AC_BT_device_fen[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);    //1#��ط�բʧ��
    ERR_AC_BT_device_he[1]  = JudgeBitInByte(Uart1Buf[buf_Num],2);    //2#��غ�բʧ��
    ERR_AC_BT_device_fen[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);    //2#��ط�բʧ��
    ERR_AC_BT_device_he[2]  = JudgeBitInByte(Uart1Buf[buf_Num],4);    //3#��غ�բʧ��
    ERR_AC_BT_device_fen[2] = JudgeBitInByte(Uart1Buf[buf_Num],5);    //3#��ط�բʧ��
    ERR_AC_BT_device_he[3]  = JudgeBitInByte(Uart1Buf[buf_Num],6);    //4#��غ�բʧ��
    ERR_AC_BT_device_fen[3] = JudgeBitInByte(Uart1Buf[buf_Num],7);    //4#��ط�բʧ��
}


/**************************************************************************
 *������:			receive_AC_FeederLine_data   0X11
 *��������:		���մ���������״̬�͸澯�źš�
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void receive_AC_FeederLine_data(){
    unsigned int buf_Num = 2;     //ǰ��2���ֽ��ǵ�ַ������
    int i = 0;
    int j = 0;
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            ERR_AC_Feeder_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);         //��բ��һ���ֽ�
            ERR_AC_Feeder_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            ERR_AC_Feeder_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            ERR_AC_Feeder_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }

    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            Ac_info.feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);     //����״̬��һ���ֽ�
            Ac_info.feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            Ac_info.feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            Ac_info.feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
}

/**************************************************************************
 *������:			receive_DC_module_data    0X20/0X28/0X25
 *��������:		�����յ�ֱ����س��ģ�����ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			unsigned int group	ֱ������
 *����ֵ:			void
 ***************************************************************************/
void receive_DC_module_data(unsigned int group)    //group 0,1,2
{
    unsigned int buf_Num = 2;
    int i = 0;
#if 0  //�޸��ϱ���̨�ĵ�ؾ����źŴ���.
    //battery_current_state[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
#endif 
    for(i = 0; i<8;i++){    //i��ʾģ����
        Dc_info[group].module[i].power_supply_mode = JudgeBitInByte(Uart4Buf[buf_Num],0);   //ģ�鹩�緽ʽ
        ERR_DC_Module[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],4);
        if(JudgeBitInByte(Uart4Buf[buf_Num],1) == 0){								//ģ�鿪��
            if(JudgeBitInByte(Uart4Buf[buf_Num],4) == 1){								//ģ�����
                Dc_info[group].module[i].state = 2;  //������2����ʾ����
            }else
                Dc_info[group].module[i].state = 1;   //������1����ʾ����
        }else																			//ģ��ػ�
            Dc_info[group].module[i].state = 0;   //������0�ǹػ�
        buf_Num ++;
        Dc_info[group].module[i].current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//�������
        buf_Num += 2;
        Dc_info[group].module[i].voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//�����ѹ
        buf_Num += 2;
        Dc_info[group].module[i].temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//����¶�
        buf_Num += 2;
    }
    for(i=0;i<8;i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);		//ģ��ͨ�Ź���
}

/**************************************************************************
 *������:	  receive_DC_module_data_NUM12    0X20/0X28/0X25
 *��������:	  �����յ�ֱ����س��ģ�����ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:		  unsigned int group	ֱ������
 *����ֵ:	  void
 ***************************************************************************/
void receive_DC_module_data_NUM12(unsigned int group)    //group 0,1,2
{
    unsigned int buf_Num = 2;
    int i = 0;
#if 0  //�޸��ϱ���̨�ĵ�ؾ����źŴ���.
    //battery_current_state[group] = JudgeBitInByte(Uart4Buf[buf_Num],2);
#endif 
    for(i = 0; i < 12; i++)  //i��ʾģ����
    {    
        Dc_info[group].module[i].power_supply_mode = JudgeBitInByte(Uart4Buf[buf_Num],0);   //ģ�鹩�緽ʽ
        ERR_DC_Module[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],4);
        if(JudgeBitInByte(Uart4Buf[buf_Num],1) == 0){								//ģ�鿪��
            if(JudgeBitInByte(Uart4Buf[buf_Num],4) == 1){								//ģ�����
                Dc_info[group].module[i].state = 2;  //������2����ʾ����
            }else
                Dc_info[group].module[i].state = 1;   //������1����ʾ����
        }else																			//ģ��ػ�
            Dc_info[group].module[i].state = 0;   //������0�ǹػ�
        buf_Num ++;
        Dc_info[group].module[i].current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//�������
        buf_Num += 2;
        Dc_info[group].module[i].voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//�����ѹ
        buf_Num += 2;
        Dc_info[group].module[i].temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//����¶�
        buf_Num += 2;
    }

    //���4��ģ����Ϣ����
    buf_Num += (7 * 4);

    for(i=0;i<8;i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);		//ģ��ͨ�Ź���
    buf_Num ++;

    //��8~11ģ��ͨ�Ź��� 
    for(i = 8; i < 12; i++)
        ERR_DC_Module_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],(i - 8));//ģ��ͨ�Ź���
}

/**************************************************************************
 *������:			receive_DC_monitor_data  0X21/0X29/0X26
 *��������:		�����յ�ֱ�����ĸ�����ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			unsigned int group	ֱ������
 *����ֵ:			void
 ***************************************************************************/
void receive_DC_monitor_data(unsigned int group)
{
    static INT8U Num_battery_discharging[3] = {0};  //�ŵ�
    static INT8U Num_battery_charging[3] = {0};           

    unsigned int buf_Num = 2;
    int i = 0;				
    for(i = 0;i<2;i++){																	// 1·��2·
        Dc_info[group].input.voltage_A[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];			//A�������ѹ
        buf_Num += 2;
        Dc_info[group].input.voltage_B[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];
        buf_Num += 2;
        Dc_info[group].input.voltage_C[i]= (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];
        buf_Num += 2;
    }
    Dc_info[group].input.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //�������
    buf_Num += 2;
    Dc_info[group].input.state = Uart4Buf[buf_Num];
    ERR_DC_Ac1_state[group][0] = JudgeBitInByte(Uart4Buf[buf_Num],0); //=0 �պ�  =1 �Ͽ�
    ERR_DC_Ac1_state[group][1] = JudgeBitInByte(Uart4Buf[buf_Num],1);
    buf_Num ++;		//BYTE15
    buf_Num ++;		//BYTE16

    Dc_info[group].busbar.battery_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//��ص�ѹ
    Dc_info[group].battery.voltage = Dc_info[group].busbar.battery_v;
    buf_Num += 2;
    Dc_info[group].busbar.switching_busbar_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//��ĸ��ѹ��������ѹ
    Dc_info[group].busbar.charger_v = Dc_info[group].busbar.switching_busbar_v;
    buf_Num += 2;
    Dc_info[group].busbar.control_busbar_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//��ĸ��ѹ
    buf_Num += 2;

#if 0
    Dc_info[group].busbar.battery_i = ((Uart4Buf[buf_Num]&0x7f)<<8)|Uart4Buf[buf_Num+1];		//��ص���
#else  //�������λ(����λ) 
    Dc_info[group].busbar.battery_i = ((Uart4Buf[buf_Num])<<8)|Uart4Buf[buf_Num+1];		//��ص���
#endif 

    Dc_info[group].battery.current = Dc_info[group].busbar.battery_i;

#if 0  //ת��λ��: �ŵ��־��Ϊ��ȡֱ������ϱ�������,��ֱ����ز��ϱ�ʱ�������ж�.
    //������طŵ��־λ����
    if ((Uart4Buf[buf_Num] & 0x80))  //�ŵ�
    {
        Num_battery_charging[group] = 0;            
        if (Num_battery_discharging[group] >= 3)
        {
            Dc_info[group].battery.FG_discharging = 1; //��طŵ��־��1    
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
            Dc_info[group].battery.FG_discharging = 0; //��طŵ��־��0    
        }
        else 
        {
            Num_battery_charging[group] ++;
        }        
    }
#endif

    buf_Num += 2;
    Dc_info[group].busbar.charger_i = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//��������
    buf_Num += 2;
    Dc_info[group].busbar.battery_t = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//�¶�
    buf_Num += 2;

#if 0  //������ĸ����ֵ���� -----�ŵ��־��Ϊ��ȡֱ������ϱ�������,��ֱ����ز��ϱ�ʱ�������ж�.
    if (Dc_info[group].battery.FG_discharging)
    {
        //�ŵ�ʱ��ĸ�ߵ������ڳ�������+(��ص����ľ���ֵ)
        Dc_info[group].busbar.control_busbar_i = Dc_info[group].busbar.charger_i + (Dc_info[group].busbar.battery_i & 0x7FFF); 
    }
    else 
    {
        //���ʱ,ĸ�ߵ������ڳ�������-��ص���
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

#if 1  //����������ѹ
    //Byte33-bit5: ������ѹ
    ERR_DC_Battery_GY[group] = JudgeBitInByte(Uart4Buf[buf_Num],5);
#endif

#if 1 //�ŵ��־��Ϊ��ȡֱ������ϱ�������,��ֱ����ز��ϱ�ʱ�������ж�.
    /* -------------------------------------------------------- 
     * BIT6��0----���                   1-----�ŵ磨������
     * BIT7: 0----��ŵ��־��Ч��BIT6�� 1-----��ŵ��־��Ч��������                         
     * ��ע������ŵ��־��Чʱ���ܼ�����и��ݵ����������жϳ�ŵ�״̬��
     *       ����ֱ��ȡֱ������ϱ���״̬��
     * -------------------------------------------------------*/
    if (JudgeBitInByte(Uart4Buf[buf_Num],7))  //ȡֱ������ϴ��ı�־λ
    {
        //����������ŵ��־λ
        Dc_info[group].battery.FG_discharging = JudgeBitInByte(Uart4Buf[buf_Num],6);        
    }
    else //�����ܼ�����и��ݵ����������жϳ�ŵ��־
    {
        //������طŵ��־λ����
        if ((Dc_info[group].busbar.battery_i & 0x8000))  //����������Ϊ�ŵ�
        {
            Num_battery_charging[group] = 0;            
            if (Num_battery_discharging[group] >= 3)
            {
                Dc_info[group].battery.FG_discharging = 1; //��طŵ��־��1    
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
                Dc_info[group].battery.FG_discharging = 0; //��طŵ��־��0    
            }
            else 
            {
                Num_battery_charging[group] ++;
            }        
        }    
    }
#endif 

#if 1  //������ĸ����ֵ����---------�ŵ��־��Ϊ��ȡֱ������ϱ�������,��ֱ����ز��ϱ�ʱ�������ж�.
    if (Dc_info[group].battery.FG_discharging)
    {
        //�ŵ�ʱ��ĸ�ߵ������ڳ�������+(��ص����ľ���ֵ)
        Dc_info[group].busbar.control_busbar_i = Dc_info[group].busbar.charger_i + (Dc_info[group].busbar.battery_i & 0x7FFF); 
    }
    else 
    {
        //���ʱ,ĸ�ߵ������ڳ�������-��ص���
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
 *������:			receive_DC_JY_data     0X22/0X2A/0X27
 *��������:		�����յ�ֱ����������еľ�Ե������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			unsigned int group	ֱ������
 *����ֵ:			void
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
    Dc_info[group].insulate.control_bus_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];		//��ĸ���Ե�
    buf_Num += 2;
    Dc_info[group].insulate.switching_bus_v1 = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //��ĸ���Ե�
    buf_Num += 2;
    Dc_info[group].insulate.switching_bus_v2 = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];	//��ĸ���Ե�
    buf_Num += 2;
    for(i=0;i<16;i++)  //ԭ��16��
        for(j=0;j<32;j++){
            ERR_DC_SW_K_JY[group][i][j] = 0;    //��������
            ERR_DC_SW_H_JY[group][i][j] = 0;    //��������
        }

    for(i=0;i<24;i++)  //������24��(17~40��)
        for(j=0;j<32;j++){
            ERR_DC_SW_K_JY_Add[group][i][j] = 0;    //��������
            ERR_DC_SW_H_JY_Add[group][i][j] = 0;    //��������
        }

#if 1   //����
    if ((Uart4Buf[buf_Num] == 0x55) 
            && (Uart4Buf[buf_Num + 1] == 0xAA))
    {
        buf_Num += 2;

        //���������ѹ
        Dc_info[group].insulate.ACInVol = (Uart4Buf[buf_Num]<<8) | Uart4Buf[buf_Num+1];	
        buf_Num += 2;
        //��ĸ�߶Եص���
        Dc_info[group].insulate.PlusBusGroudRes = (Uart4Buf[buf_Num]<<8) | Uart4Buf[buf_Num+1];  
        buf_Num += 2;
        //��ĸ�߶Եص��� 
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
        ERR_DC_JY_detection[group][0] = 1;  //<!--  һ�ξ�Ե����(1��ֱ��ϵͳ��Ե����)-->
        Dc_info[group].ERR_DC_muxian = 1;   //1: ĸ�߽ӵ�
        for(i=0;i<JY_ERR_Num;i++){
            if (Uart4Buf[buf_Num] <= 16)    //ԭ��16��
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
            else if (Uart4Buf[buf_Num] <= 40) //������24��(17~40��)
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
        Dc_info[group].ERR_DC_muxian = 0;  //0: ����
        for(i=0;i<16;i++)
            for(j=0;j<32;j++){
                ERR_DC_SW_K_JY[group][i][j] = 0;
                ERR_DC_SW_H_JY[group][i][j] = 0;
            }

        for(i=0;i<24;i++)  //������24��(17~40��)
            for(j=0;j<32;j++){
                ERR_DC_SW_K_JY_Add[group][i][j] = 0;
                ERR_DC_SW_H_JY_Add[group][i][j] = 0;
            }
    }
}
/**************************************************************************
 *������:			receive_DC_Switch_data    0X23/0X2B/0X2D
 *��������:		�����յ�ֱ����������е����߿���������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			unsigned int group	ֱ������
 *����ֵ:			void
 ***************************************************************************/
void receive_DC_Switch_data(unsigned int group)
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j = 0;
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            ERR_DC_SW_trip[group][i][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      //��������ز���ֵ����բ����32·��
            ERR_DC_SW_trip[group][i][j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            ERR_DC_SW_trip[group][i][j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            ERR_DC_SW_trip[group][i][j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<16;i++){
        for(j=0;j<8;j++){
            Dc_info[group].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    //״̬����ز���ֵ���Ϸ֣���32·��
            Dc_info[group].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Dc_info[group].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Dc_info[group].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<8;i++){
        ERR_DC_SW_Sample_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);    //���������ͨ�Ź��ϣ�16����أ�
        ERR_DC_SW_Sample_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<8;i++){
        ERR_DC_St_Sample_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);   //״̬�����ͨ�Ź��ϣ�16����أ�
        ERR_DC_St_Sample_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
}
/**************************************************************************
 *������:			receive_DC_Battery_data   0X24/0X2C/0X2E
 *��������:		�����յ�ֱ��������ݵ��������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			unsigned int group	ֱ������
 *����ֵ:			void
 ***************************************************************************/
void receive_DC_Battery_data(unsigned int group)
{
    unsigned int buf_Num = 2;
    unsigned int i = 0;
    int battery_flag = 1;      //��Ϊ��׼λ����ʾ��ص�ѹ

    if(Battery_12V_flag == 2)
    {
        battery_flag = 10;
    }
    else
    {
        battery_flag = 1;
    }

    //����¶�
    Dc_info[group].battery.temperature = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];  //X<<8,X��ֵ����8λ������һ�ֽ�
    buf_Num += 2;      

#if 0
    //��ص�ѹ�����С��ʼֵ
    Dc_info[group].battery.single_max_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]; //�����1�ڵ������ߵ�ѹ����ð��һ�����ȹ�ȥ��
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;
#else  //���ƹ��ܣ��޸�12Vʱ������С��ѹֵ�Ƚ���ʾ�����⡣ 
    //��ص�ѹ�����С��ʼֵ
    Dc_info[group].battery.single_max_v = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;
#endif 

#if 0
    //ͨ��ð�ݵķ����Ƚϳ����е���е����ֵ����Сֵ����ͨ���Ƚϲ�������ֵ��ȷ��ÿһ�ڵ���Ƿ�Ƿѹ���߹�ѹ    
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
        for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
#endif 
        {
            //ÿһ�ڵ�ص�ѹ
            Dc_info[group].battery.single_v[i] =( (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;   
            buf_Num += 2;

            //ȡ�����ѹ���Сֵ
            if(Dc_info[group].battery.single_v[i] > Dc_info[group].battery.single_max_v)
            {
                Dc_info[group].battery.single_max_v = Dc_info[group].battery.single_v[i]; //ȡ�����ѹ���ֵ
            }
            else if(Dc_info[group].battery.single_v[i] < Dc_info[group].battery.single_min_v)
            {
                Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_v[i]; //ȡ�����ѹ��Сֵ
            }

#if 0
            //�����ѹ�͹�ѹ�ָ�
            if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v * 100) //��ѹ
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
                //�����ѹ�͹�ѹ�ָ�
                if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v * 100) //��ѹ
#endif 
                {
                    ERR_DC_BatterySingle_GYHF_CNT[group][i] = 0;   //��ѹ�ָ�������0                  
                    if (ERR_DC_BatterySingle_GY_CNT[group][i] < MAXCOUNT_GY_QY)      
                    {
                        ERR_DC_BatterySingle_GY_CNT[group][i] ++;  //��ѹ������1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 1;  //�ж�Ϊ��ѹ
                    }
                }
                else  //��ѹ�ָ� 
                {
                    ERR_DC_BatterySingle_GY_CNT[group][i] = 0;  //��ѹ������0                    
                    if (ERR_DC_BatterySingle_GYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_GYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 0;  //�ж�Ϊ��ѹ�ָ�
                    }
                }

#if 0
            //����Ƿѹ��Ƿѹ�ָ�
            if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100) //Ƿѹ
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
                //����Ƿѹ��Ƿѹ�ָ�
                if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100) //Ƿѹ
#endif 
                {
                    ERR_DC_BatterySingle_QYHF_CNT[group][i] = 0;                    
                    if (ERR_DC_BatterySingle_QY_CNT[group][i] < MAXCOUNT_GY_QY)     
                    {
                        ERR_DC_BatterySingle_QY_CNT[group][i] ++; //Ƿѹ������1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 1;  //�ж�ΪǷѹ
                    }
                }
                else  //Ƿѹ�ָ�
                {
                    ERR_DC_BatterySingle_QY_CNT[group][i] = 0;                      
                    if (ERR_DC_BatterySingle_QYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_QYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 0;  //�ж�ΪǷѹ�ָ�
                    }
                }

#if 0   //���� //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
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
        }     //ͨ��ð�ݵķ����Ƚϳ����е���е����ֵ����Сֵ����ͨ���Ƚϲ�������ֵ��ȷ��ÿһ�ڵ���Ƿ�Ƿѹ���߹�ѹ
}

/**
 * �����յ�PSMX-B�����ĵ�ص�ѹ��ֵ����ر�����(��ַ: 0XB0/0XB1/0XB2)
 * @param  group: ֱ������
 */
void receive_DC_Battery_data_PSMXB_Vol(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;
    int battery_flag = 1;      //��Ϊ��׼λ����ʾ��ص�ѹ

#if 0  //�˴�2V��12V���һ������
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
    //����¶�: Ĭ��25.0��
    Dc_info[group].battery.temperature = 250;
#endif

    //������ؽ���: ���Դ˴��ϴ��ĵ�ؽ���Ϊ׼���Խ����趨Ϊ׼
    //Sys_cfg_info.battery_set.battery_amount = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];
    //if (Sys_cfg_info.battery_set.battery_amount > 108)
    //{
    //    Sys_cfg_info.battery_set.battery_amount = 108;
    //}
    buf_Num += 2; 

    //��ص�ѹ: ���Դ˴��ϴ��ĵ�ѹΪ׼,��ֱ������ϴ�����Ϊ׼
    //Dc_info[group].busbar.battery_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];		
    //Dc_info[group].battery.voltage = Dc_info[group].busbar.battery_v;
    buf_Num += 2;

    //��ص�ѹ�����С��ʼֵ
    Dc_info[group].battery.single_max_v = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]; //�����1�ڵ������ߵ�ѹ����ð��һ�����ȹ�ȥ��
    Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_max_v;

#if 0
    //ͨ��ð�ݵķ����Ƚϳ����е���е����ֵ����Сֵ����ͨ���Ƚϲ�������ֵ��ȷ��ÿһ�ڵ���Ƿ�Ƿѹ���߹�ѹ    
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
        //ͨ��ð�ݵķ����Ƚϳ����е���е����ֵ����Сֵ����ͨ���Ƚϲ�������ֵ��ȷ��ÿһ�ڵ���Ƿ�Ƿѹ���߹�ѹ    
        for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
#endif 
        {
            //ÿһ�ڵ�ص�ѹ
            Dc_info[group].battery.single_v[i] =( (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]) * battery_flag;   
            buf_Num += 2;

            //ȡ�����ѹ���Сֵ
            if(Dc_info[group].battery.single_v[i] > Dc_info[group].battery.single_max_v)
            {
                Dc_info[group].battery.single_max_v = Dc_info[group].battery.single_v[i]; //ȡ�����ѹ���ֵ
            }
            else if(Dc_info[group].battery.single_v[i] < Dc_info[group].battery.single_min_v)
            {
                Dc_info[group].battery.single_min_v = Dc_info[group].battery.single_v[i]; //ȡ�����ѹ��Сֵ
            }

#if 0
            //�����ѹ�͹�ѹ�ָ�
            if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v * 100) //��ѹ
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                //�����ѹ�͹�ѹ�ָ�
                if (Dc_info[group].battery.single_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v * 100) //��ѹ
#endif 
                {
                    ERR_DC_BatterySingle_GYHF_CNT[group][i] = 0;   //��ѹ�ָ�������0                  
                    if (ERR_DC_BatterySingle_GY_CNT[group][i] < MAXCOUNT_GY_QY)      
                    {
                        ERR_DC_BatterySingle_GY_CNT[group][i] ++;  //��ѹ������1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 1;  //�ж�Ϊ��ѹ
                    }
                }
                else  //��ѹ�ָ� 
                {
                    ERR_DC_BatterySingle_GY_CNT[group][i] = 0;  //��ѹ������0                    
                    if (ERR_DC_BatterySingle_GYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_GYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_GY[group][i] = 0;  //�ж�Ϊ��ѹ�ָ�
                    }
                }

#if 0
            //����Ƿѹ��Ƿѹ�ָ�
            if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100) //Ƿѹ
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
                //����Ƿѹ��Ƿѹ�ָ�
                if(Dc_info[group].battery.single_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100) //Ƿѹ
#endif 
                {
                    ERR_DC_BatterySingle_QYHF_CNT[group][i] = 0;                    
                    if (ERR_DC_BatterySingle_QY_CNT[group][i] < MAXCOUNT_GY_QY)     
                    {
                        ERR_DC_BatterySingle_QY_CNT[group][i] ++; //Ƿѹ������1
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 1;  //�ж�ΪǷѹ
                    }
                }
                else  //Ƿѹ�ָ�
                {
                    ERR_DC_BatterySingle_QY_CNT[group][i] = 0;                      
                    if (ERR_DC_BatterySingle_QYHF_CNT[group][i] < MAXCOUNT_GY_QY)    
                    {
                        ERR_DC_BatterySingle_QYHF_CNT[group][i] ++;
                    }
                    else
                    {
                        ERR_DC_BatterySingle_QY[group][i] = 0;  //�ж�ΪǷѹ�ָ�
                    }
                }

#if 0   //����: //ϵͳ���õ�ز�����ͳһ���øĳɷ�������. 
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
 * �����յ�PSMX-B�����ĵ�����踳ֵ����ر�����(��ַ: 0XB0/0XB1/0XB2)
 * @param  group: ֱ������
 */
void receive_DC_Battery_data_PSMXB_Res(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;

#if 0
    for(i = 0;i < Sys_cfg_info.battery_set.battery_amount; i ++)
    {
        //ÿһ�ڵ������
        Dc_info[group].battery.single_res[i] 
            = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]);   
        buf_Num += 2;
    }
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
    for(i = 0;i < Sys_cfg_info.battery_set[group].battery_amount; i ++)
    {
        //ÿһ�ڵ������
        Dc_info[group].battery.single_res[i] 
            = ((Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1]);   
        buf_Num += 2;
    }
#endif 

    //����¶�
    buf_Num = 219;
    Dc_info[group].battery.temperature = (Uart4Buf[buf_Num] << 8) | Uart4Buf[buf_Num + 1];  //X<<8,X��ֵ����8λ������һ�ֽ�
}

/**
 * �����յ�PSMX-B�����ĵ��ң�Ÿ�ֵ����ر�����(��ַ: 0XB0/0XB1/0XB2)
 * @param  group: ֱ������
 */
void receive_DC_Battery_data_PSMXB_ST(unsigned int group)
{
    unsigned int buf_Num = 3;
    unsigned int i = 0;
    unsigned int j = 0;

#if 0
    //0x0307~0x030D: 1~108�ŵ�����賬�ޱ�ʾλ(14 �ֽ�* 8 = 112) 
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
#else  //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
    //0x0307~0x030D: 1~108�ŵ�����賬�ޱ�ʾλ(14 �ֽ�* 8 = 112) 
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

    //0x030E(���ֽ�Bit0~3): 1~4�Ų�����ͨѶ�쳣
    buf_Num = 18;
    for (i = 0; i < 4; i ++)
    {
        ERR_DC_BatterySamplingBox_CommError[group][i] 
            = JudgeBitInByte(Uart4Buf[buf_Num], i);
    }
}

/**************************************************************************
 *������:	receive_DC_FenGui_data   0X50/0X51/0X52
 *��������:	�����յ�ֱ����������е����߿���������ݸ�ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:	    unsigned int group	ֱ������
 *����ֵ:	void
 ***************************************************************************/
void receive_DC_FenGui_data(unsigned int group)
{
    unsigned int buf_Num = 10;
    int i = 0;
    int j = 0;

    //��Ե��׼��Ԫͨ�Ź��ϣ��Ϸ֣���30·�� (8 Bytes)
    for(j=0;j<8;j++)
    {
        //1~32 
        ERR_DC_standard_cell_comm[group][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      
        ERR_DC_standard_cell_comm[group][j + 8] = JudgeBitInByte(Uart4Buf[buf_Num + 1],j);
        ERR_DC_standard_cell_comm[group][j + 16] = JudgeBitInByte(Uart4Buf[buf_Num + 2],j);
        ERR_DC_standard_cell_comm[group][j + 24] = JudgeBitInByte(Uart4Buf[buf_Num + 3],j);

#ifdef  _ANM03_DC_SWITCH_STATE_NUM_40       
        //����64��(33~64)
        ERR_DC_standard_cell_comm[group][j + 32] = JudgeBitInByte(Uart4Buf[buf_Num + 4],j);      
        ERR_DC_standard_cell_comm[group][j + 40] = JudgeBitInByte(Uart4Buf[buf_Num + 5],j);
        ERR_DC_standard_cell_comm[group][j + 48] = JudgeBitInByte(Uart4Buf[buf_Num + 6],j);
        ERR_DC_standard_cell_comm[group][j + 56] = JudgeBitInByte(Uart4Buf[buf_Num + 7],j);
#endif 
    }
    buf_Num += 8;  //8���ֽ��Ǳ�׼��Ԫͨ�Ź���

    //�ֹ�ͨ�Ź��ϣ�10����أ� (4 Bytes)
    for(i=0;i<8;i++)
    {
        ERR_DC_FG_comm[group][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);      
        ERR_DC_FG_comm[group][i+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],i);
    }
    buf_Num += 4;  //2���ֽ��Ƿֹ�ͨ�Ź��ϣ�2���ֽڱ���

#ifdef  _ANM03_DC_SWITCH_STATE_NUM_40 
    if (Uart4Buf[buf_Num - 1] == 40)  //����40�鴦��
    {
        //״̬����ز���ֵ���Ϸ֣���16·�� (30 Bytes)
        for(i=0;i<24;i++){
            for(j=0;j<8;j++){
                Dc_FG_info[group].FGfeederLine[i].FGstate[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    
                Dc_FG_info[group].FGfeederLine[i].FGstate[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            }
            buf_Num += 2;
        }
        buf_Num += 2;  //2���ֽڱ���

        //��������ز���ֵ����բ����16·�� (28 Bytes)
        for(i=0;i<24;i++){
            for(j=0;j<8;j++){
                ERR_DC_FG_SW_trip[group][i][j] = JudgeBitInByte(Uart4Buf[buf_Num],j);      
                ERR_DC_FG_SW_trip[group][i][j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            }
            buf_Num += 2;    
        } 
    }
#else   
    //ԭ30�鴦��
    //״̬����ز���ֵ���Ϸ֣���16·�� (30 Bytes)
    for(i=0;i<14;i++){
        for(j=0;j<8;j++){
            Dc_FG_info[group].FGfeederLine[i].FGstate[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);    
            Dc_FG_info[group].FGfeederLine[i].FGstate[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
        }
        buf_Num += 2;
    }
    buf_Num += 2;  //2���ֽڱ���

    //��������ز���ֵ����բ����16·�� (28 Bytes)
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
  Dc_info[group].battery.TXsingle_v[i] =((Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]);   //ÿһ�ڵ�ص�ѹ
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

    Comm_DC_info[group].battery.TXsingle_max_v = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];//�����1�ڵ������ߵ�ѹ����ð��һ�����ȹ�ȥ��
    Comm_DC_info[group].battery.TXsingle_min_v = Comm_DC_info[group].battery.TXsingle_max_v;
    for(i=0;i<j;i++){
        Comm_DC_info[group].battery.TXsingle_v[i] =( (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]);   //ÿһ�ڵ�ص�ѹ
        buf_Num += 2;
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Comm_DC_info[group].battery.TXsingle_max_v){
            Comm_DC_info[group].battery.TXsingle_max_v = Comm_DC_info[group].battery.TXsingle_v[i];	//ȡ�����ѹ���ֵ
        }else if(Comm_DC_info[group].battery.TXsingle_v[i] < Comm_DC_info[group].battery.TXsingle_min_v){
            Comm_DC_info[group].battery.TXsingle_min_v = Comm_DC_info[group].battery.TXsingle_v[i];	//ȡ�����ѹ��Сֵ
        }else{;}

#if 0
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Sys_cfg_info.battery_set.single_upperLimit_v*100){
            ERR_comm_BatterySingle_GY[group][i] = 1;				  //��ص�ѹ���ڹ�ѹ�趨ֵ��X100��������ѹ�澯
        }else {
            ERR_comm_BatterySingle_GY[group][i] = 0;
        }
        if(Comm_DC_info[group].battery.TXsingle_v[i] < Sys_cfg_info.battery_set.single_lowerLimit_v*100){
            ERR_comm_BatterySingle_QY[group][i] = 1;				  //��ص�ѹС��Ƿѹ�趨ֵ����Ƿѹ�澯
        }else{
            ERR_comm_BatterySingle_QY[group][i] = 0;;
        }
#else   //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.  
        if(Comm_DC_info[group].battery.TXsingle_v[i] > Sys_cfg_info.battery_set[group].single_upperLimit_v*100){
            ERR_comm_BatterySingle_GY[group][i] = 1;				  //��ص�ѹ���ڹ�ѹ�趨ֵ��X100��������ѹ�澯
        }else {
            ERR_comm_BatterySingle_GY[group][i] = 0;
        }
        if(Comm_DC_info[group].battery.TXsingle_v[i] < Sys_cfg_info.battery_set[group].single_lowerLimit_v*100){
            ERR_comm_BatterySingle_QY[group][i] = 1;				  //��ص�ѹС��Ƿѹ�趨ֵ����Ƿѹ�澯
        }else{
            ERR_comm_BatterySingle_QY[group][i] = 0;;
        }
#endif 
    }	  //ͨ��ð�ݵķ����Ƚϳ����е���е����ֵ����Сֵ����ͨ���Ƚϲ�������ֵ��ȷ��ÿһ�ڵ���Ƿ�Ƿѹ���߹�ѹ
}


/**************************************************************************
 *������:	receive_Comm_data   0X30  0X31
 *��������:	�����յ�ͨ�ŵ�Դ������ݵ��������ݸ�ֵ����ر�����������ʾ�ñ����͹��ϱ�־������
 *����:		unsigned int groupͨ�ŵ�Դ����
 *����ֵ:	void
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
    for(i=0;i<16;i++){   //16��ģ�������ѹ
        Comm_info[group].module.module_output_v[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];  //16��ģ�������ѹ
        buf_Num += 2;
    }
    for(i=0;i<16;i++){   //16��ģ���������
        Comm_info[group].module.module_output_i[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //16��ģ���������
        buf_Num += 2;
    }
    for(i=0;i<16;i++){   //16��ģ������¶�
        Comm_info[group].module.module_output_t[i] = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //16��ģ������¶�
        buf_Num += 2;
    }
    for(i=0;i<8;i++){    //һ���ֽ�8λ�������i��8λ����˼��һ��5���ֽ�
        ERR_Comm_SW_trip[group][0][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_SW_trip[group][0][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
        ERR_Comm_SW_trip[group][0][i+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],i);
        ERR_Comm_SW_specialty[group][i] = JudgeBitInByte(Uart1Buf[buf_Num+3],i);
        ERR_Comm_SW_specialty[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+4],i);
    }
    buf_Num += 5;
    for(i=1;i<4;i++){   //ʣ��3�����ؼ��ֵ��i��1��ʼ�������i�ǿ��ؼ�ظ���
        for(j=0;j<8;j++){
            ERR_Comm_SW_trip[group][i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
            ERR_Comm_SW_trip[group][i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            ERR_Comm_SW_trip[group][i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            ERR_Comm_SW_trip[group][i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<4;i++){   //i״̬��ظ���
        for(j=0;j<8;j++){    //j��ʾλ��0-7,1���ֽ�8λ
            Comm_info[group].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
            Comm_info[group].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
            Comm_info[group].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
            Comm_info[group].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<8;i++){   //i��ʾһ���ֽ�8λ��2���ֽڱ�ʾ16��ģ��ͨѶ����
        ERR_Comm_Module_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_Module_comm[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<8;i++){    //i��ʾһ���ֽ�8λ��2���ֽڱ�ʾ16��ģ�����
        ERR_Comm_Module[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_Module[group][i+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],i);
    }
    buf_Num += 2;
    for(i=0;i<4;i++){  //i��ʾλ��ǰ��4λ��ʾ���ؼ��ͨѶ���ϣ���4λ��״̬���ͨ�Ź���
        ERR_Comm_SW_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_Comm_St_comm[group][i] = JudgeBitInByte(Uart1Buf[buf_Num],i+4);
    }
    buf_Num ++;
    ERR_Comm_Feeder_sw[group] = JudgeBitInByte(Uart1Buf[buf_Num],0);
    ERR_Comm_SPD[group]= JudgeBitInByte(Uart1Buf[buf_Num],1);
}
/**************************************************************************
 *������:			receive_LNMKComm_data   �����ܣ�
 *��������:		����������ͨ�ŵ�Դ�������ģ������ --->ģ��ģ��������1��2��
 *����:			unsigned int groupͨ�ŵ�Դ����
 *����ֵ:			void
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
    //j = 0;   //j��λ
    //x = atoh(a);    //�ַ��������
    z = 0;  //z��λ
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);    //f�Ǹ����ͣ�memcpy�ǿ�������int�͵�x,������float�͵�f
    y = (int)((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].module.output_v = y;   //�����ѹ
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
        //j = 0;   //j��λ
        //x = atoh(a);    //�ַ��������
        z = 0;  //z��λ
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_i[i] = y;   //16��ģ���������(����������)
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
        //j = 0;   //j��λ
        //x = atoh(a);    //�ַ��������
        z = 0;  //z��λ
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_v[i] = y;  //16��ģ�������ѹ(�������������ѹ)
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
        //j = 0;   //j��λ
        //x = atoh(a);    //�ַ��������
        z = 0;  //z��λ
        for (x=0;x<4;x++)
        {
            z=(z<<8)|(a[x]);
        }
        memcpy(&f,&z,4);   
        y = (int) ((f)*10);
        //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
        Comm_info[group].module.module_output_t[i] = y;   //16��ģ������¶�(��������DCDC �����¶�)
        buf_Num += 26;
    }	
}
/**************************************************************************
 *������:			receive_LNMKComm_status 
 *��������:		����������ͨ�ŵ�Դ�������ģ��״̬ --->ģ�鿪����״̬1��2��
 *����:			unsigned int groupͨ�ŵ�Դ����
 *����ֵ:			void
 ***************************************************************************/
void receive_LNMKComm_status(unsigned int group)   //group 0,1
{
    unsigned int buf_Num = 15;
    int i = 0;
    //int j =0;
    int MK_num = (AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]));
    for(i=0;i< MK_num;i++)
    {   
        buf_Num += 2;    //����/�ػ�
        Comm_info[group].module.module_statekg[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 2;    //����/������
        Comm_info[group].module.module_statexl[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 2;    //����/����
        Comm_info[group].module.module_statejfc[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
        buf_Num += 4;    //��λ/����λ
        Comm_info[group].module.module_statezw[i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));
    }

}
/**************************************************************************
 *������:			receive_LNMKComm_alarm 
 *��������:		����������ͨ�ŵ�Դ�������ģ��澯 --->�澯��Ϣ1��2��
 *����:			unsigned int groupͨ�ŵ�Դ����
 *����ֵ:			void
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
        ERR_LN_Comm_Module[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));           //ģ�����
        buf_Num += 6;
        ERR_Comm_Module_output_GY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));    //�����ѹ�ػ�����
        buf_Num += 2;
        ERR_Comm_Module_output_QY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));    //���Ƿѹ
        buf_Num += 2;
        ERR_Comm_Module_AC_import_GY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //�����ѹ
        buf_Num += 2;
        ERR_Comm_Module_AC_import_QY[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //����Ƿѹ
        buf_Num += 22;
        ERR_Comm_Module_bu_advection[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1]))); //������
        buf_Num += 4;
        ERR_LN_Comm_Module_comm[group][i] = ((AscToHex(Uart1Buf[buf_Num])<<4) | (AscToHex(Uart1Buf[buf_Num+1])));      //ģ����Ӽ��ͨ�Ź���
    }			
}
/**************************************************************************
 *������:			receive_LNDCComm_data   
 *��������:		����������ͨ�ŵ�Դ������ݵ���������--->ֱ��ģ���������Ϣ1��2��
 *����:			unsigned int groupͨ�ŵ�Դ����
 *����ֵ:			void
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
    //j = 0;   //j��λ
    //x = atoh(a);    //�ַ��������
    z = 0;  //z��λ
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.dcoutput_v = y;   //ֱ�������ѹ
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
    //j = 0;   //j��λ
    //x = atoh(a);    //�ַ��������
    z = 0;  //z��λ
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.dcoutput_i = y;   //ֱ���������
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
    //j = 0;   //j��λ
    //x = atoh(a);    //�ַ��������
    z = 0;  //z��λ
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.battery_output_i = y;   //��ص���
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
    //j = 0;   //j��λ
    //x = atoh(a);    //�ַ��������
    z = 0;  //z��λ
    for (i=0;i<4;i++)
    {
        z=(z<<8)|(a[i]);
    }
    memcpy(&f,&z,4);   
    y = (int) ((f)*10);
    //debug_printf(0,"11111,, z == %4x,, &f == %f,, y == %d,, group ==  %4d\n",z,f,y,group);
    Comm_info[group].battery.battery_output_t = y;   //����¶�		
}
/**************************************************************************
 *������:			receive_UPS_data   0x40
 *��������:		�����յ�����Դ������ݵ��������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void receive_UPS_data()
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;

    //UPS_1	ң��
    Ups_info[0].input.AC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1���������ѹ
    buf_Num += 2;
    Ups_info[0].input.DC_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1ֱ�������ѹ 
    buf_Num += 2;
    Ups_info[0].output.voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //UPS1�����ѹ
    buf_Num += 2;
    Ups_info[0].output.current = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];   //����������ذٷֱ�
    buf_Num += 2;
    Ups_info[0].output.frequency = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1���Ƶ��
    buf_Num += 2;
    Ups_info[0].input.bypass_voltage = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1]; //UPS1��·�����ѹ
    buf_Num += 2;
    Ups_info[0].output.temperature = (Uart1Buf[buf_Num]<<8)|Uart1Buf[buf_Num+1];//UPS1�¶�
    buf_Num += 2;

    //UPS_2  ң��
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

    //UPS1~2״̬(Byte29~30)
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
    ERR_UPS_Comm[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);   //UPS1ͨ�Ź���
    ERR_UPS_Comm[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);   //UPS2ͨ�Ź���
    buf_Num++;

    //kgcom_error
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<8;i++){   //i��ʾλ��ǰ��8λ��ʾ���ؼ��ͨѶ����
            ERR_UPS_SW_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
        }
        buf_Num++;
    }else {
        for(i=0;i<4;i++){   //i��ʾλ��ǰ��4λ��ʾ���ؼ��ͨѶ���ϣ���4λ��״̬���ͨ�Ź���
            ERR_UPS_SW_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i);
            ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i+4); 
        }
        buf_Num++;
    }

    //����λ�ø澯(5�ֽ�: ǰ2�ֽ������⿪��)(��բ�ź�)(Byte33~37)
    switch(Sys_cfg_info.ups_set.work_mode){
        case 0:		//����ģʽ
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);
            buf_Num += 2;
            break;
        case 1:		//����ģʽ
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
        case 2:		//����ģʽ
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
        case 3:		//����ģʽ
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);     //10101010һ���ֽڣ�0�ǵ�1λ��1�ǵ�8λ�����ҵ���
            ERR_UPS_AcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_DcInput_sw[1] = JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],4);
            ERR_UPS_AcOutput_sw[1]= JudgeBitInByte(Uart1Buf[buf_Num],5);
            ERR_UPS_AcOutput_sw[2] = JudgeBitInByte(Uart1Buf[buf_Num],6);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],7);
            buf_Num += 2;
            break;
        default:	//����ģʽ
            ERR_UPS_AcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);
            ERR_UPS_DcInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);
            ERR_UPS_BypassInput_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],2);
            ERR_UPS_AcOutput_sw[0]= JudgeBitInByte(Uart1Buf[buf_Num],3);
            ERR_UPS_Bypass_Overhaul_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],4);
            buf_Num += 2;
            break;
    }
    for(i=0;i<8;i++){    //i��ʾ8λ��1�����ؼ��
        ERR_UPS_SW_trip[0][i] =  JudgeBitInByte(Uart1Buf[buf_Num],i);
        ERR_UPS_SW_trip[0][i+8] =  JudgeBitInByte(Uart1Buf[buf_Num+1],i);
        ERR_UPS_SW_trip[0][i+16] =  JudgeBitInByte(Uart1Buf[buf_Num+2],i);
    }
    buf_Num += 3;
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=1;i<6;i++){         //i��1��ʼ�������i�ǿ��ؼ�ظ���
            for(j=0;j<8;j++){    //j��ʾ8λ
                ERR_UPS_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                ERR_UPS_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                ERR_UPS_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                ERR_UPS_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }else {
        for(i=1;i<4;i++){         //i��1��ʼ�������i�ǿ��ؼ�ظ���
            for(j=0;j<8;j++){    //j��ʾ8λ
                ERR_UPS_SW_trip[i][j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                ERR_UPS_SW_trip[i][j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                ERR_UPS_SW_trip[i][j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                ERR_UPS_SW_trip[i][j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }

    }
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<8;i++){   //i��ʾλ��8λ��״̬���ͨ�Ź���
            ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart1Buf[buf_Num],i); 
        }
        buf_Num++;
    }
    if (Sys_cfg_info.ups_set.state_monitor_num == 5){
        for(i=0;i<6;i++){            //i��״̬��ظ���    UPS���⿪��״̬��1��״̬���ǰ��2���ֽ�
            for(j=0;j<8;j++){       //j��ʾ8λ
                Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }else {   //(Byte49~52~65)
        for(i=0;i<4;i++){            //i��״̬��ظ���    UPS���⿪��״̬��1��״̬���ǰ��2���ֽ�
            for(j=0;j<8;j++){       //j��ʾ8λ
                Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart1Buf[buf_Num],j);
                Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart1Buf[buf_Num+1],j);
                Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart1Buf[buf_Num+2],j);
                Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart1Buf[buf_Num+3],j);
            }
            buf_Num += 4;
        }
    }
    ERR_UPS_Feeder_sw[0] = JudgeBitInByte(Uart1Buf[buf_Num],0);     //UPS���߿��ع���
    ERR_UPS_SPD[0] = JudgeBitInByte(Uart1Buf[buf_Num],1);       //ups����������

#if 1  //UPSģʽ�жϼ��������ֵ����    
    for (i = 0; i < 2; i ++)
    {
        //����ֵ* �ٷֱ�(�������һλС��)
        unsigned long output_cur = UPS_OUTPUT_CUR * Ups_info[i].output.current / 100;   

        //if (ERR_UPS_Bypass_Overhaul_sw[i])
        if (((i == 0) && Ups_info[0].feederLine[0].state[4])
                || ((i == 1) && Ups_info[0].feederLine[0].state[9]))
        {
            UPS_Mode[i] = UPS_MODE_OVER_HAUL;  //UPS����ģʽ

            //UPS�е����������UPSֱ�����������UPS��·���������
            //UPS����������е�����Ƶ��ȫΪ0
            Ups_info[i].input.AC_current = 0;
            Ups_info[i].input.DC_current = 0;
            Ups_info[i].input.bypass_current = 0;
            Ups_info[i].output.current_value = 0;
            Ups_info[i].input.AC_freq = 0;
        }
        else  if ((ERR_UPS_Bypass_output[i] == 0)     //��·���ң���� 
                && (ERR_UPS[i] == 0)                  //UPS�޹��� 
                && (ERR_UPS_Comm[i] == 0))            //upsͨ�Ź���    
        {
            if (ERR_UPS_MainsSupply[i] == 0)          //�е���������     
            {
                UPS_Mode[i] = UPS_MODE_MAINS_SUPPLY;  //UPS�е�ģʽ 

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
            else if (ERR_UPS_DC[i] == 0)              //ֱ����������
            {
                UPS_Mode[i] = UPS_MODE_BAT_SUPPLY;    //UPS���ģʽ

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
                UPS_Mode[i] = UPS_MODE_BY_PASS;       //UPS��·ģʽ

                Ups_info[i].input.AC_current = 0;
                Ups_info[i].input.DC_current = 0;
                Ups_info[i].input.bypass_current = output_cur;
                Ups_info[i].output.current_value = output_cur;
                Ups_info[i].input.AC_freq = 0;    
            }
        }
        else          
        {
            UPS_Mode[i] = UPS_MODE_BY_PASS;           //UPS��·ģʽ

            Ups_info[i].input.AC_current = 0;
            Ups_info[i].input.DC_current = 0;
            Ups_info[i].input.bypass_current = output_cur;
            Ups_info[i].output.current_value = output_cur;
            Ups_info[i].input.AC_freq = 0;
        }   
    }

#if 1
    //UPS����쳣�ж�
    static unsigned char Num_ERR_UPS_Output_High[2] = {0};
    static unsigned char Num_ERR_UPS_Output_Low[2] = {0};
    static unsigned char Num_ERR_UPS_Output_No[2] = {0};

    for (i = 0; i < Sys_set_Ups_Num; i ++)
    {
        if (Ups_info[i].output.voltage < 2000) //200~240 ���Ƿѹ
        {
            Num_ERR_UPS_Output_High[i] = 0;
            Num_ERR_UPS_Output_No[i] = 0;
            if (Num_ERR_UPS_Output_Low[i] < 5)
            {
                Num_ERR_UPS_Output_Low[i] ++;    
            }
            else 
            {
                ERR_UPS_OutPut[i] = 1;  //����쳣(Ƿѹ) 
            }
        }
        else if (Ups_info[i].output.voltage > 2400) //�����ѹ
        {
            Num_ERR_UPS_Output_Low[i] = 0;
            Num_ERR_UPS_Output_No[i] = 0;
            if (Num_ERR_UPS_Output_High[i] < 5)
            {
                Num_ERR_UPS_Output_High[i] ++;    
            }
            else 
            {
                ERR_UPS_OutPut[i] = 1;  //����쳣(��ѹ)   
            }
        }
        else  //������Χ
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
void receive_UPS_data_35K()    //35KV����ͨ�ţ�UPS����
{
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;

    //UPS_1	
    Ups_info[0].input.AC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //UPS1���������ѹ
    buf_Num += 2;
    Ups_info[0].output.voltage  = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];      //�����ѹ
    buf_Num += 2;
    Ups_info[0].input.DC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //ֱ�������ѹ
    buf_Num += 2;
    Ups_info[0].output.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //�������
    buf_Num += 2;
    Ups_info[0].output.frequency = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //���Ƶ��
    buf_Num += 2;
    Ups_info[0].input.bypass_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; //��·�����ѹ
    buf_Num += 2;
    Ups_info[0].output.temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];   //����¶�
    buf_Num += 2;
    ERR_UPS_Bypass[0] = JudgeBitInByte(Uart4Buf[buf_Num],2);        //��·�쳣
    ERR_UPS_Overload[0] = JudgeBitInByte(Uart4Buf[buf_Num],3);      //����
    ERR_UPS[0] = JudgeBitInByte(Uart4Buf[buf_Num],4);               //UPS����
    ERR_UPS_Bypass_output[0] = JudgeBitInByte(Uart4Buf[buf_Num],5); //��·���
    ERR_UPS_DC[0] = JudgeBitInByte(Uart4Buf[buf_Num],6);            //ֱ���쳣
    ERR_UPS_MainsSupply[0] = JudgeBitInByte(Uart4Buf[buf_Num],7);   //�е��쳣
    buf_Num++;
    //UPS_2                                                                            
    Ups_info[1].input.AC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //UPS2���������ѹ
    buf_Num += 2;                                                                   
    Ups_info[1].output.voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //�����ѹ
    buf_Num += 2;                                                                   
    Ups_info[1].input.DC_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //ֱ�������ѹ
    buf_Num += 2;                                                                   
    Ups_info[1].output.current = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];       //�������
    buf_Num += 2;                                                                   
    Ups_info[1].output.frequency = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];     //���Ƶ��
    buf_Num += 2;                                                                   
    Ups_info[1].input.bypass_voltage = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; //��·�����ѹ
    buf_Num += 2;                                                                   
    Ups_info[1].output.temperature = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];   //����¶�
    buf_Num += 2;
    ERR_UPS_Bypass[1] = JudgeBitInByte(Uart4Buf[buf_Num],2);        //��·�쳣
    ERR_UPS_Overload[1] = JudgeBitInByte(Uart4Buf[buf_Num],3);      //����
    ERR_UPS[1] = JudgeBitInByte(Uart4Buf[buf_Num],4);               //UPS����
    ERR_UPS_Bypass_output[1] = JudgeBitInByte(Uart4Buf[buf_Num],5); //��·���
    ERR_UPS_DC[1] = JudgeBitInByte(Uart4Buf[buf_Num],6);            //ֱ���쳣
    ERR_UPS_MainsSupply[1] = JudgeBitInByte(Uart4Buf[buf_Num],7);   //�е��쳣
    buf_Num++;
    for(i=0;i<2;i++){
        for(j=0;j<8;j++){  //j��ʾλ
            Ups_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);
            Ups_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Ups_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Ups_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    ERR_UPS_Comm[0] = JudgeBitInByte(Uart4Buf[buf_Num],0);   //ups1ͨ�Ź���
    ERR_UPS_Comm[1] = JudgeBitInByte(Uart4Buf[buf_Num],1);   //ups2ͨ�Ź���
    buf_Num++;
    for(i=0;i<2;i++){  //i��ʾλ
        ERR_UPS_State_Comm[i] = JudgeBitInByte(Uart4Buf[buf_Num],i); //2��״̬���ͨ�Ź���
    }
}
void receive_Comm_data_35K(){
    unsigned int buf_Num = 2;
    int i = 0;
    int j =0;
    for(i=0;i<4;i++){
        Comm_info[0].module.module_output_v[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //ģ�������ѹ
        buf_Num += 2;
        Comm_info[0].module.module_output_i[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //ģ���������
        buf_Num += 2;
        Comm_info[0].module.module_output_t[i] = (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1];  //ģ������¶�
        buf_Num += 2;
        ERR_Comm_Module[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],0);
        buf_Num++;
    }
    for(i=0;i<2;i++){       //2��״̬������
        for(j=0;j<8;j++){   //֧·״̬��32·
            Comm_info[0].feederLine[i].state[j] = JudgeBitInByte(Uart4Buf[buf_Num],j);
            Comm_info[0].feederLine[i].state[j+8] = JudgeBitInByte(Uart4Buf[buf_Num+1],j);
            Comm_info[0].feederLine[i].state[j+16] = JudgeBitInByte(Uart4Buf[buf_Num+2],j);
            Comm_info[0].feederLine[i].state[j+24] = JudgeBitInByte(Uart4Buf[buf_Num+3],j);
        }
        buf_Num += 4;
    }
    for(i=0;i<4;i++){
        ERR_Comm_Module_comm[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],i); //һ��4��ģ��ͨ�Ź���
    }
    buf_Num++;
    for(i=0;i<2;i++){
        ERR_Comm_St_comm[0][i] = JudgeBitInByte(Uart4Buf[buf_Num],i);     //һ��2��״̬���ͨ�Ź���
    }
}

/**************************************************************************
 *������:	receive_DC_FD_data
 *��������:	������ֱ����Դ�ֵ��������Ϣ��������ʾ�ñ����͹��ϱ�־������
 *����:		void
 *����ֵ:	void
 ***************************************************************************/
void receive_DC_FD_data(unsigned int group)
{
    unsigned int buf_Num = 2; 
    //int i =0,j=0;
    buf_Num++;		//�ӹ�����
    //AC_sneak_voltage =  (Uart4Buf[buf_Num]<<8)|Uart4Buf[buf_Num+1]; // ���������ѹ
}
