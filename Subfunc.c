#include "Subfunc.h"
#include "global_define.h"
#include "Global_Para_Ex.h"
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <stdlib.h>
#include <globel_Ex.h>
#include "receive_data.h"

//#include <global_Qt.h>

NowFaultStruct *pHead = NULL;	//��ǰ��������ͷ
NowFaultStruct *His_Head = NULL;	//��ʷ���ϱ�ͷ

void Calc_crc16(INT16U *crc, INT8U  crcbuf)
{
    INT8U  i,TT;

    *crc=*crc^crcbuf;
    for(i=0;i<8;i++)
    {
        TT=*crc&1;
        *crc=*crc>>1;
        *crc=*crc&0x7fff;
        if (TT==1)
            (*crc)=(*crc)^0xa001;
        *crc=*crc&0xffff;
    }
}

INT16U Load_crc(INT8U cnt,INT8U *dat)
{
    INT16U i;
    INT16U crc=0xffff;

    for(i=0;(i<cnt)&(i<256);i++)
    {
        Calc_crc16(&crc,dat[i]); //  
    }
    return crc;
}

//ASCLL��ת16����
INT16U AscToHex(unsigned char aChar)
{
    if((aChar>=0x30)&&(aChar<=0x39))
        aChar -= 0x30;
    else if((aChar>=0x41)&&(aChar<=0x46))//��д��ĸ
        aChar -= 0x37;
    else if((aChar>=0x61)&&(aChar<=0x66))//Сд��ĸ
        aChar -= 0x57;
    else aChar = 0xff;
    return aChar; 
} 

unsigned int atoh(unsigned char *hex)
{
    unsigned int v = 0;

    if ( !hex ) return 0;
    if ( hex[0] == '0' && (hex[1] == 'X' || hex[1] == 'x' ) )
        hex ++, hex ++;

    while ( *hex ) {
        if (*hex >= '0' && *hex <= '9') {
            v = v * 16 + (*hex) - '0';
        } else if (*hex >= 'A' && *hex <= 'F') {
            v = v * 16 + (*hex) - 'A' + 10;
        } else if (*hex >= 'a' && *hex <= 'f') {
            v = v * 16 + (*hex) - 'a' + 10;
        } else break;
        hex ++;
    }

    return v;
}


INT8U JudgeBitInByte(INT8U ByteData,INT8U BitPos)
{
    if(((ByteData >> BitPos)& 0x01) == 0x01)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/****************��Ե������������******************/



void MoveData_BackupToSet()
{}
void MoveData_RS2324ToGlobalData(INT8U Addr,INT8U Len)
{}

void MoveData_RS2323ToGlobalData(INT8U Addr,INT8U Len)
{}

//���ڽ��յ�����ת��61850���棨gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]��
void MoveData_RS232ToServer()
{
    INT16U j,j2,j1;  //,j5,j3,j4,j6;
    INT16U k,m;
    INT8U Bufin[8];
    INT16U i,group,num_st;

    // <!-- ******** ң����(��ַ: 0x0000): ��Ӧ�����ļ�config_mx_dc.cfg ******** -->
    //��1��ֱ�����������
    j = 0x0000;
    group = 0;
    for(i=0;i<2;i++){									//0 :һ·        1:��·
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;  //����һ·AB�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;	 //����һ·BC�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;	 //����һ·CA�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;           //�������
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;        //��ص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;     
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;        //������ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8; //��ĸ��ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;        //��ص���
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8; //��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;	 //��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		 //��������
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		 //�¶�
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;	 //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8; //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8; //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //�������������ѹ����ĸ�߶Եص��衢��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //���������ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    //��2��ֱ�����������
    j = 0x1000;
    group = 1;
    for(i=0;i<2;i++){									//0 :һ·        1:��·
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;  //����һ·AB�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;	 //����һ·BC�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;	 //����һ·CA�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;           //�������
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;		 //��ص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;		 //������ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8; //��ĸ��ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;		 //��ص���
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8; //��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;	 //��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		 //��������
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		 //�¶�
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;	 //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8; //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8; //��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //�������������ѹ����ĸ�߶Եص��衢��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //���������ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    //��3��ֱ�����������
    j = 0x2000;	//���������ң��
    group = 2;
    for(i=0;i<2;i++){									//0 :һ·        1:��·
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;    	//����һ·AB�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;		//����һ·BC�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;		//����һ·CA�ߵ�ѹ
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;     //�������
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;		//��ص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;		//������ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8;		//��ĸ��ѹ
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;		//��ص���
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8;		//��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;		//��ĸ����
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		//��������
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		//�¶�
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;		//��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8;		//��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8;		//��ĸ���Եص�ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //�������������ѹ����ĸ�߶Եص��衢��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //���������ѹ
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //��ĸ�߶Եص���
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    j = 0x0100;												//�����Ϣ
    for(j2 = 0;j2<108;j2++)
    {
        gIEC61850_Data[j++] = Dc_info[0].battery.single_v[j2]>>8;
        gIEC61850_Data[j++] = Dc_info[0].battery.single_v[j2];
    }
    j = 0x1100;
    for(j2 = 0;j2<108;j2++)
    {
        gIEC61850_Data[j++] = Dc_info[1].battery.single_v[j2]>>8;
        gIEC61850_Data[j++] = Dc_info[1].battery.single_v[j2];
    }
    j = 0x2100;
    for(j2 = 0;j2<108;j2++)
    {
        gIEC61850_Data[j++] = Dc_info[2].battery.single_v[j2]>>8;
        gIEC61850_Data[j++] = Dc_info[2].battery.single_v[j2];
    }

    j = 0x0200;												//����������
#if 0
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[0]>>8;			// 1·�������ѹ
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[0]>>8;			// 2·�������ѹ
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[2];

    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[0]>>8;			// 1·ĸ�����ѹ
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[0]>>8;			// 1·ĸ�������
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[0];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[1];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[2];

    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ActivePower[3]+Ac_info.ac_in_data[1].ActivePower[3])>>8;		//�й�����
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ActivePower[3]+Ac_info.ac_in_data[1].ActivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ReactivePower[3]+Ac_info.ac_in_data[1].ReactivePower[3])>>8;		//�޹�����
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ReactivePower[3]+Ac_info.ac_in_data[1].ReactivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ApparentPower[3]+Ac_info.ac_in_data[1].ApparentPower[3])>>8;	//���ڹ���
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ApparentPower[3]+Ac_info.ac_in_data[1].ApparentPower[3];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].PowerFactor[3]>>8;									//��������
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].PowerFactor[3];

    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[0]>>8;			// 3·�������ѹ
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[0]>>8;			// 4·�������ѹ
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[2];

    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[0]>>8;			// 2·ĸ�����ѹ
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[0]>>8;			// 2·ĸ�������
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[0];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[1];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[2];

    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ActivePower[3]+Ac_info.ac_in_data[3].ActivePower[3])>>8;		//�й�����
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ActivePower[3]+Ac_info.ac_in_data[3].ActivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ReactivePower[3]+Ac_info.ac_in_data[3].ReactivePower[3])>>8;		//�޹�����
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ReactivePower[3]+Ac_info.ac_in_data[3].ReactivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ApparentPower[3]+Ac_info.ac_in_data[3].ApparentPower[3])>>8;	//���ڹ���
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ApparentPower[3]+Ac_info.ac_in_data[3].ApparentPower[3];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].PowerFactor[3]>>8;									//��������
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].PowerFactor[3];
#else //������·�����й��޹����ʵ����ݴ���̨.
    //4·�������ߵ�ѹ���������й��޹����ʡ���������
    for (i = 0; i < 4; i++)  
    {
        //�������ߵ�ѹ 
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[0] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[0];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[1];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[2];

        //�������ߵ���
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[0] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[0];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[1];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[2];

        //���������й�����
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ActivePower[3] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ActivePower[3];

        //���������޹�����
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ReactivePower[3] >> 8;	
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ReactivePower[3];

        //�����������ڹ���
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ApparentPower[3] >> 8;	
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ApparentPower[3];

        //�������߹�������
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].PowerFactor[3] >> 8;									
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].PowerFactor[3];
    }

    //2·����ĸ�ߵ�ѹ������
    for (i = 0; i < 2; i++)  
    {
        //����ĸ��ABC���ѹ
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[0] >> 8;			
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[0];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[1];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[2];

        //����ĸ��ABC�����
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[0] >> 8;			
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[0];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[1];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[2];
    }
#endif 	

    j = 0x0300;												//UPS�Ӽ������
    gIEC61850_Data[j++] = Ups_info[0].input.AC_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.AC_voltage;
    gIEC61850_Data[j++] = Ups_info[0].output.voltage>>8;
    gIEC61850_Data[j++] = Ups_info[0].output.voltage;
    gIEC61850_Data[j++] = Ups_info[0].input.DC_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.DC_voltage;
    gIEC61850_Data[j++] = Ups_info[0].output.current>>8;
    gIEC61850_Data[j++] = Ups_info[0].output.current;
    gIEC61850_Data[j++] = Ups_info[0].output.frequency>>8;
    gIEC61850_Data[j++] = Ups_info[0].output.frequency;
    gIEC61850_Data[j++] = Ups_info[0].input.bypass_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.bypass_voltage;
    gIEC61850_Data[j++] = Ups_info[0].output.temperature>>8;
    gIEC61850_Data[j++] = Ups_info[0].output.temperature;

    gIEC61850_Data[j++] = Ups_info[1].input.AC_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.AC_voltage;
    gIEC61850_Data[j++] = Ups_info[1].output.voltage>>8;
    gIEC61850_Data[j++] = Ups_info[1].output.voltage;
    gIEC61850_Data[j++] = Ups_info[1].input.DC_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.DC_voltage;
    gIEC61850_Data[j++] = Ups_info[1].output.current>>8;
    gIEC61850_Data[j++] = Ups_info[1].output.current;
    gIEC61850_Data[j++] = Ups_info[1].output.frequency>>8;
    gIEC61850_Data[j++] = Ups_info[1].output.frequency;
    gIEC61850_Data[j++] = Ups_info[1].input.bypass_voltage>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.bypass_voltage;
    gIEC61850_Data[j++] = Ups_info[1].output.temperature>>8;
    gIEC61850_Data[j++] = Ups_info[1].output.temperature;

#if 0
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[0]>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[0];
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[1]>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[1];
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[2]>>8;
    gIEC61850_Data[j++] = Ups_info[0].input.voltage[2];
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[0]>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[0];
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[1]>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[1];
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[2]>>8;
    gIEC61850_Data[j++] = Ups_info[1].input.voltage[2];
#else  //����: UPS1�е�������� + UPS2�е��������
    for (i = 0; i < 2; i ++)
    {
        gIEC61850_Data[j++] = Ups_info[i].input.AC_current >> 8;
        gIEC61850_Data[j++] = Ups_info[i].input.AC_current;
        gIEC61850_Data[j++] = Ups_info[i].input.DC_current >> 8;
        gIEC61850_Data[j++] = Ups_info[i].input.DC_current ;
        gIEC61850_Data[j++] = Ups_info[i].input.bypass_current >> 8;
        gIEC61850_Data[j++] = Ups_info[i].input.bypass_current;
        gIEC61850_Data[j++] = Ups_info[i].output.current_value >> 8;
        gIEC61850_Data[j++] = Ups_info[i].output.current_value;
        gIEC61850_Data[j++] = Ups_info[i].input.AC_freq >> 8;
        gIEC61850_Data[j++] = Ups_info[i].input.AC_freq;    
    } 
#endif 

    j = 0x0400;												//ͨ���Ӽ����Ϣ
    group = 0;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i;
    for(j2 = 0;j2<16;j2++)  //scl_srvr������ʱ,ֻ����8��
    {
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2];
    }

    j = 0x1400;
    group = 1;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i;
    for(j2 = 0;j2<16;j2++) //scl_srvr������ʱ,ֻ����8��
    {
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2];
    }


    j = 0x0500;												//���ģ����Ϣ
    for(j2 = 0;j2<8;j2++)
    {
        gIEC61850_Data[j++] = Dc_info[0].module[j2].voltage>>8;
        gIEC61850_Data[j++] = Dc_info[0].module[j2].voltage;
        gIEC61850_Data[j++] = Dc_info[0].module[j2].current>>8;
        gIEC61850_Data[j++] = Dc_info[0].module[j2].current;
        gIEC61850_Data[j++] = Dc_info[0].module[j2].temperature>>8;
        gIEC61850_Data[j++] = Dc_info[0].module[j2].temperature;
    }
    j = 0x1500;
    for(j2 = 0;j2<8;j2++)
    {
        gIEC61850_Data[j++] = Dc_info[1].module[j2].voltage>>8;
        gIEC61850_Data[j++] = Dc_info[1].module[j2].voltage;
        gIEC61850_Data[j++] = Dc_info[1].module[j2].current>>8;
        gIEC61850_Data[j++] = Dc_info[1].module[j2].current;
        gIEC61850_Data[j++] = Dc_info[1].module[j2].temperature>>8;
        gIEC61850_Data[j++] = Dc_info[1].module[j2].temperature;
    }

    j = 0x0600;												//ͨ�ŵ�Դ�����Ϣ
    for(j2 = 0;j2<48;j2++)
    {
        if (j2 < 12){
            gIEC61850_Data[j++] = Comm_DC_info[0].battery.TXsingle_v[j2]>>8;
            gIEC61850_Data[j++] = Comm_DC_info[0].battery.TXsingle_v[j2];
        }else if (j2 < 24){
            gIEC61850_Data[j++] = Comm_DC_info[1].battery.TXsingle_v[j2-12]>>8;
            gIEC61850_Data[j++] = Comm_DC_info[1].battery.TXsingle_v[j2-12];
        }else if (j2 < 36){
            gIEC61850_Data[j++] = Comm_DC_info[2].battery.TXsingle_v[j2-24]>>8;
            gIEC61850_Data[j++] = Comm_DC_info[2].battery.TXsingle_v[j2-24];	
        }else if (j2 < 48){
            gIEC61850_Data[j++] = Comm_DC_info[3].battery.TXsingle_v[j2-36]>>8;
            gIEC61850_Data[j++] = Comm_DC_info[3].battery.TXsingle_v[j2-36];
        }
    }

    //--------------------------------------------------------------- 
    //0x0800(��): <!--  ֱ����Դ1����һ·����(1��ֱ��ϵͳ��һ�齻���������)--> 
    //ң����	
    j = 0x0800;				  //�����Ϣ
    if(ERR_DC_AcVoltage_GY[0][0] == 1 || ERR_DC_AcVoltage_QY[0][0] == 1 
            || ERR_DC_Ac_PowerCut[0][0] == 1 || ERR_DC_Ac_PhaseLoss[0][0] == 1)  
    {
        Bufin[0] = 0x01;      //����һ·����
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[0][1] == 1 || ERR_DC_AcVoltage_QY[0][1] == 1 
            || ERR_DC_Ac_PowerCut[0][1] == 1 || ERR_DC_Ac_PhaseLoss[0][1] == 1)  
    {
        Bufin[1] = 0x01;       					//������·����
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[0][0])&0x01;   //����һ·����
    Bufin[3] = (~ERR_DC_Ac1_state[0][1])&0x01;	 //������·����
    Bufin[4] = ERR_DC_AcSPD[0];                  //�������׹���

#if 0
    if(Special_35KV_flag == 2){  //��������1~2�Ͽ�����
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0)  ){
            Bufin[5] = 0x00;         
        }else{
            Bufin[5] = 0x01;						//�������ع���
        }
    }else{                       //��������1~4�Ͽ�����
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0) 
                && (ERR_DC_AcSwitch[0][2] == 0)&& (ERR_DC_AcSwitch[0][3] == 0) ){
            Bufin[5] = 0x00;         
        }else{
            Bufin[5] = 0x01;						//�������ع���
        }
    }
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����.
    if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
    {   //��������1~2�Ͽ�����
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0))
        {
            Bufin[5] = 0x00;         
        }
        else
        {
            Bufin[5] = 0x01;					
        }
    }
    else  //ͬʱ��UPS��ͨ�ż��ʱ
    {   //��������1~4�Ͽ�����
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0) 
                && (ERR_DC_AcSwitch[0][2] == 0)&& (ERR_DC_AcSwitch[0][3] == 0))
        {
            Bufin[5] = 0x00;         
        }
        else
        {
            Bufin[5] = 0x01;					
        }
    }
#endif 
    Bufin[6] = ERR_DC_AcSample_comm[0];				//������ص�ԪͨѶ����
    
    //<!--  ֱ����Դ1���Ѳ����ͨѶ�Ͽ� -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//�������Ѳ�����
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[0];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0800(��)

    //--------------------------------------------------------------- 
    //0x0800(��): <!--  ֱ����Դ2����һ·����(2��ֱ��ϵͳ��һ�齻���������)-->  
    if(ERR_DC_AcVoltage_GY[1][0] == 1 || ERR_DC_AcVoltage_QY[1][0] == 1 || ERR_DC_Ac_PowerCut[1][0] == 1 || ERR_DC_Ac_PhaseLoss[1][0] == 1)  
    {
        Bufin[0] = 0x01;      //����һ·����
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[1][1] == 1 || ERR_DC_AcVoltage_QY[1][1] == 1 || ERR_DC_Ac_PowerCut[1][1] == 1 || ERR_DC_Ac_PhaseLoss[1][1] == 1)  
    {
        Bufin[1] = 0x01;       					//������·����
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[1][0])&0x01;   //����һ·����
    Bufin[3] = (~ERR_DC_Ac1_state[1][1])&0x01;	 //������·����
    Bufin[4] = ERR_DC_AcSPD[1];                  //�������׹���
    if((ERR_DC_AcSwitch[1][0] == 0) && (ERR_DC_AcSwitch[1][1] == 0) && (ERR_DC_AcSwitch[1][2] == 0) && (ERR_DC_AcSwitch[1][3] == 0))
    {
        Bufin[5] = 0x00;         
    }
    else
    {
        Bufin[5] = 0x01;						//�������ع���
    }
    Bufin[6] = ERR_DC_AcSample_comm[1];				//������ص�ԪͨѶ����
    
    //<!--  ֱ����Դ2���Ѳ����ͨѶ�Ͽ� -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//�������Ѳ�����
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0800(��)

    //--------------------------------------------------------------- 
    //0x0801(��): <!--  ֱ����Դ1���ģ�����(1��ֱ��ϵͳ����ģ�����) --> 
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++)  //��ԭ8·����12·���ģ��
    {
        Bufin[0] |= ERR_DC_Module[0][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)    //��ԭ8·����12·���ģ��
    {
        Bufin[1] |= ERR_DC_Module_comm[0][j2];
    }
    if(Sys_cfg_info.dc_set[0].switch_monitor_num>0)
    {
        Bufin[2] = 0x00;
        for(j2 = 0;j2<16;j2++)   //ǰ16·
        {
            for(j1 = 0;j1<32;j1++)
            {
                Bufin[2] |= ERR_DC_SW_trip[0][j2][j1];
            }
        }
        //       Bufin[2] |= ERR_DC_SW_Spe[29];

#if 1   //������źš�һ�����߿�����բ��
        for(j2 = 0;j2<24;j2++)  //��24·
        {
            for(j1 = 0;j1<16;j1++) 
            {
                Bufin[2] |= ERR_DC_FG_SW_trip[0][j2][j1];
            }
        }

#if 0
#if 0   
        Bufin[2] |= (ERR_DC_AcSwitch[0][3] | ERR_DC_AcSwitch[0][4]);
#else 
        if(Special_35KV_flag == 2)
        {
            Bufin[2] |= ERR_DC_AcSwitch[0][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[0][3] | ERR_DC_AcSwitch[0][4]);        
        }
#endif
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����. 
        //ֱ��X�����߿�����բ
        if ((Special_35KV_flag == 0x02)
                || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02)
                || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            Bufin[2] |= ERR_DC_AcSwitch[0][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[0][3] | ERR_DC_AcSwitch[0][4]);        
        }
#endif 
#endif 
    }
    else
    {
        Bufin[2] = ERR_DC_External[0][0];
    }
#if 0
    Bufin[3] = ERR_DC_Battery_SW[0][0];  //��ؿ��ع���
#else   //ת��λ��
    Bufin[3] = 0x00;
#endif 
    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[0][0];
    Bufin[4] |= ERR_DC_BatteryFuse[0][1];
    Bufin[5] = ERR_DC_Battery_SW[0][1];  				//����������բ����
    Bufin[6] = 0x00;
    for(j2=0;j2<16;j2++)
    {
        Bufin[6] |= ERR_DC_SW_Sample_comm[0][j2];
    }
    Bufin[7] = 0x00;
    for(j2=0;j2<8;j2++)
    {
        Bufin[7] |= ERR_DC_St_Sample_comm[0][j2];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0801(��)

    //--------------------------------------------------------------- 
    //0x0801(��): <!--  ֱ����Դ2���ģ�����(2��ֱ��ϵͳ����ģ�����) --> 
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++) //��ԭ8·����12·���ģ��
    {
        Bufin[0] |= ERR_DC_Module[1][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)  //��ԭ8·����12·���ģ��
    {
        Bufin[1] |= ERR_DC_Module_comm[1][j2];
    }
    if(Sys_cfg_info.dc_set[1].switch_monitor_num>0)
    {
        Bufin[2] = 0x00;
        for(j2 = 0;j2<16;j2++)
        {
            for(j1 = 0;j1<32;j1++)
            {
                Bufin[2] |= ERR_DC_SW_trip[1][j2][j1];
            }
        }
        //       Bufin[2] |= ERR_DC_SW_Spe[29];
        
#if 1   //������źš��������߿�����բ��
        for(j2 = 0;j2<24;j2++)  //��24·
        {
            for(j1 = 0;j1<16;j1++) 
            {
                Bufin[2] |= ERR_DC_FG_SW_trip[1][j2][j1];
            }
        }

#if 0
#if 0   
        Bufin[2] |= (ERR_DC_AcSwitch[1][3] | ERR_DC_AcSwitch[1][4]);
#else 
        if(Special_35KV_flag == 2)
        {
            Bufin[2] |= ERR_DC_AcSwitch[1][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[1][3] | ERR_DC_AcSwitch[1][4]);        
        }
#endif 
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����.
        if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //�ñ�־λ���ܰ�����ȥ
            //ERR_DC_AcSwitch[1][3]: ��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
            Bufin[2] |= ERR_DC_AcSwitch[1][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[1][3] | ERR_DC_AcSwitch[1][4]);        
        }
#endif 
#endif 
    }
    else
    {
        Bufin[2] = ERR_DC_External[1][0];
    }

#if 0
    Bufin[3] = ERR_DC_Battery_SW[1][0]; //��ؿ��ع���
#else   //ת��λ��
    Bufin[3] = 0x00;
#endif 

    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[1][0];
    Bufin[4] |= ERR_DC_BatteryFuse[1][1];
    Bufin[5] = ERR_DC_Battery_SW[1][1];  				//����������բ����
    Bufin[6] = 0x00;
    for(j2=0;j2<16;j2++)
    {
        Bufin[6] |= ERR_DC_SW_Sample_comm[1][j2];
    }
    Bufin[7] = 0x00;
    for(j2=0;j2<8;j2++)
    {
        Bufin[7] |= ERR_DC_St_Sample_comm[1][j2];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0801(��)

    //--------------------------------------------------------------- 
    //0x0802(��): <!--  һ�γ�����ѹ -->   
    Bufin[0] = ERR_DC_Charger_GY[0];
    Bufin[1] = ERR_DC_Charger_QY[0];
    //#ifdef S35KV
    //    Bufin[2] = ERR_DC_SW_Spe[25];
    //    Bufin[3] = ERR_DC_SW_Spe[26];
    //#else
    Bufin[2] = ERR_DC_KM_GY[0];
    Bufin[3] = ERR_DC_KM_QY[0];
    //#endif

#if 0
    Bufin[4] = ERR_DC_Battery_QY[0];
    Bufin[5] = sys_ctl_info.battery_mode_ctl[0];
#else //ת��λ��
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif 

    Bufin[6] = ERR_DC_DcSample_comm[0];
    Bufin[7] = 0;                   //���Ѳ�쵥ԪͨѶ����
    Bufin[7] |= ERR_DC_BatteryPolling_comm[0][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[0][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0802(��)

    //--------------------------------------------------------------- 
    //0x0802(��): <!--  ���γ�����ѹ --> 
    Bufin[0] = ERR_DC_Charger_GY[1];
    Bufin[1] = ERR_DC_Charger_QY[1];
    //#ifdef S35KV
    //    Bufin[2] = ERR_DC_SW_Spe[25];
    //    Bufin[3] = ERR_DC_SW_Spe[26];
    //#else
    Bufin[2] = ERR_DC_KM_GY[1];
    Bufin[3] = ERR_DC_KM_QY[1];
    //#endif

#if 0
    Bufin[4] = ERR_DC_Battery_QY[1];
    Bufin[5] = sys_ctl_info.battery_mode_ctl[1];
#else //ת��λ��
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif 

    Bufin[6] = ERR_DC_DcSample_comm[1];
    Bufin[7] = 0;                   //���Ѳ�쵥ԪͨѶ����
    Bufin[7] |= ERR_DC_BatteryPolling_comm[1][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[1][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0802(��)

    //--------------------------------------------------------------- 
    //0x0803(��): <!--  һ�鵥���ع���(1��ֱ��ϵͳ�����쳣)--> 

#if 0
    for(j2=0; j2<108; j2++){
        Bufin[0] |= ERR_DC_BatterySingle_GY[0][j2];            							// 1�鵥���ع���
        Bufin[0] |= ERR_DC_BatterySingle_QY[0][j2];
    }
#else  //ת��λ�� 
    Bufin[0] = 0x00;
#endif 

#if 0
#if 0
    Bufin[1] = ERR_DC_JY_VF[0];     // 1��ѹ��澯 ��Ϊ ��طŵ�
#else  
    Bufin[1] = Dc_info[0].battery.FG_discharging;     //��طŵ�
#endif
#else  //ת��λ��
    Bufin[1] = 0x00;
#endif  

    Bufin[2] = ERR_DC_JY_detection[0][0]|ERR_DC_JY_detection[0][1];
    Bufin[3] = 0;   //�ž�ԵѲ��ͨѶ����
    for(j2=0;j2<16;j2++)
    {
        Bufin[3] |= ERR_DC_JY_Sample_comm[0][j2];  //��Ե�Ӽ��ͨѶ����
    }
    if(Sys_cfg_info.sys_set.insulate_mode == 0)
    {
        Bufin[4] = ERR_MainIsu; //# ������Ե�Ӽ��ͨ�ŶϿ�
    }
    else
    {
        Bufin[4] = 0;
    }
    //    if(bgCfg_DC_IsCool == 1)
    //    {
    //        Bufin[5] = ERR_MainDC_COOL;
    //    }
    //    else
    //    {
    Bufin[5] = ERR_MainDC[0];
    //    }
#if 1  
    Bufin[6] = 0;
    Bufin[7] = 0;
#else  //ת��λ��
    //<!--  һ�鵥����Ƿѹ--> 
    for(j2=0; j2<108; j2++){          			
        Bufin[6] |= ERR_DC_BatterySingle_QY[0][j2];
    }

    //<!--  һ�鵥���ع�ѹ-->
    for(j2=0; j2<108; j2++){
        Bufin[7] |= ERR_DC_BatterySingle_GY[0][j2];            				
    }
#endif 
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0803(��)

    //--------------------------------------------------------------- 
    //0x0803(��): <!--  ���鵥���ع���(2��ֱ��ϵͳ�����쳣) -->
#if 0
    for(j2=0; j2<108; j2++){
        Bufin[0] |= ERR_DC_BatterySingle_GY[1][j2];            							// 1�鵥���ع���
        Bufin[0] |= ERR_DC_BatterySingle_QY[1][j2];
    }
#else  //ת��λ��
    Bufin[0] = 0x00;
#endif 

#if 0
#if 0
    Bufin[1] = ERR_DC_JY_VF[1];     // 2��ѹ��澯
#else  
    Bufin[1] = Dc_info[1].battery.FG_discharging;     //��طŵ�
#endif
#else  //ת��λ��
    Bufin[1] = 0x00;
#endif  

    Bufin[2] = ERR_DC_JY_detection[1][0]|ERR_DC_JY_detection[1][1];
    Bufin[3] = 0x00;
    for(j2=0;j2<16;j2++)
    {
        Bufin[3] |= ERR_DC_JY_Sample_comm[1][j2];
    }
    if(Sys_cfg_info.sys_set.insulate_mode == 0)
    {
        Bufin[4] = ERR_MainIsu1;
    }
    else
    {
        Bufin[4] = 0;
    }
    //    if(bgCfg_DC_IsCool == 1){
    //        Bufin[5] = ERR_MainDC_COOL;
    //    }
    //    else{
    Bufin[5] = ERR_MainDC[1];
    //    }

#if 1
    Bufin[6] = 0;
    Bufin[7] = 0;
#else  //ת��λ��
    //<!--  ���鵥����Ƿѹ--> 
    for(j2=0; j2<108; j2++){          			
        Bufin[6] |= ERR_DC_BatterySingle_QY[1][j2];
    }

    //<!--  ���鵥���ع�ѹ-->
    for(j2=0; j2<108; j2++){
        Bufin[7] |= ERR_DC_BatterySingle_GY[1][j2];            				
    }
#endif 

    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0803(��)


    /***********************    AC  0808   *************************************/
    //--------------------------------------------------------------- 
    //0x0804(��): <!--  �������߶�·��һ��բ(վ�õ�1��ATS����1��·����բ) -->
    Bufin[0] = Ac_info.ac_in_data[0].SW_state;
    Bufin[1] = Ac_info.ac_in_data[1].SW_state;
    Bufin[2] = ERR_AC_in_SW_trip[0];  //�������߶�·��һ��բ
    Bufin[3] = ERR_AC_in_SW_trip[1];  //�������߶�·������բ
    Bufin[4] = ERR_AC_SPD[0];   //������1����
    Bufin[5] = ERR_AC_in_V[0];
    Bufin[6] = ERR_AC_in_V[1];
    Bufin[7] = ERR_AC_mu_V[0];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0804(��)

    //--------------------------------------------------------------- 
    //0x0804(��): <!--  �������߶�·������բ(վ�õ�2��ATS����1��·����բ) -->
    Bufin[0] = Ac_info.ac_in_data[2].SW_state;
    Bufin[1] = Ac_info.ac_in_data[3].SW_state;
#if 1   //�޸��ϱ�����̨�ġ��������߶�·������բ����"�������߶�·������բ"�ź�.
    Bufin[2] = ERR_AC_in_SW_trip[2]; //�������߶�·������բ(ĸ��������բ)
    Bufin[3] = ERR_AC_in_SW_trip[3]; //�������߶�·������բ
#else  //���������߹�ĸ��������բ�ź��ϴ���̨
    if (Sys_cfg_info.ac_set.control_mode == 2)  //��ٿ���
    {
        Bufin[2] = 0x00;
        Bufin[3] = 0x00;        
    }
    else   //������� 
    {
        Bufin[2] = ERR_AC_in_SW_trip[2]; //�������߶�·������բ
        Bufin[3] = ERR_AC_in_SW_trip[3]; //�������߶�·������բ    
    }
#endif 
    Bufin[4] = ERR_AC_SPD[1];   //������2����
    Bufin[5] = ERR_AC_in_V[2];
    Bufin[6] = ERR_AC_in_V[3];
    Bufin[7] = ERR_AC_mu_V[1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0804(��)

    /************************??????????????????????????????????**********************/    
    //--------------------------------------------------------------- 
    //0x0805(��): <!--  ������ԴATSE1һ·��բ -->
#if 0
    if (Sys_cfg_info.ac_set.ATSE_num >0)
    {
        if(Ac_info.atse_stat[0].sw == 1)
        {
            Bufin[0] = 1;
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;
        }
    }
    else
    {
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
    }
    Bufin[3] = ERR_MainATS; 
#else //������ATSE����ĸ��ź��ϴ���̨ 
    for (i = 0; i < 4; i ++)
        Bufin[i] = 0;
    if (((Sys_cfg_info.ac_set.control_mode == 1)
            || (Sys_cfg_info.ac_set.control_mode == 3)) //ATSE����  //�����ͺ���ATSͨѶ֧��.
            && (Sys_cfg_info.ac_set.ATSE_num >0))  //ATSE��������0 
    {
        if(Ac_info.atse_stat[0].sw == 1)
        {
            Bufin[0] = 1;  //������ԴATSE1һ·��բ
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;  //������ԴATSE1��·��բ
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;  //������ԴATSE1��բ
        }
        Bufin[3] = ERR_MainATS; //������ԴATSE1ͨѶ���� 
    }
#endif 

#if 0
    /*    
          if(Sys_cfg_info.ac_set.switch_monitor_num > 0)
          {
          if(Sys_cfg_info.ac_set.switch_monitor_num <= AC1_Switch_Num)
          {
          Bufin[4] = 0x00;
          for(j2 = 0;j2<Sys_cfg_info.ac_set.switch_monitor_num;j2++)
          {
          for(j1 = 0;j1<32;j1++)
          {
          Bufin[4] |= ERR_AC_SW[j2][j1];
          }
          }
          Bufin[4] |= ERR_AC_Sw_Spe[30];
          Bufin[5] = 0x00;
          }
          else
          {
          Bufin[4] = 0x00;
          for(j2 = 0;j2<AC1_Switch_Num;j2++)
          {
          for(j1 = 0;j1<32;j1++)
          {
          Bufin[4] |= ERR_AC_SW[j2][j1];
          }
          }
          Bufin[4] |= ERR_AC_Sw_Spe[30];
          Bufin[5] = 0x00;
          for(j2 = AC1_Switch_Num;j2<Sys_cfg_info.ac_set.switch_monitor_num;j2++)
          {
          for(j1 = 0;j1<32;j1++)
          {
          Bufin[5] |= ERR_AC_SW[j2][j1];
          }
          }
          Bufin[5] |= ERR_AC_Sw_Spe[31];
          }
          }
          else
          {
          Bufin[4] = ERR_AC_Sw_Spe[30];				 
          Bufin[5] = ERR_AC_Sw_Spe[31];
          }*/
#endif

#if 0
    Bufin[4] = ERR_AC_Feeder_Duan_trip_1;
#else   //����������բ���źŻ򵽽�����������բ
    Bufin[4] = 0x00;
    for(i = 0;i < (Sys_cfg_info.ac_set.state_monitor_num / 2); i++)  //ǰһ����Ϊһ��
    {
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2];   
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+8];
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+16];
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+24];
        }
    }
    Bufin[4] |= ERR_AC_Feeder_Duan_trip_1;
#endif 

    Bufin[5] = 0x00;
#if 0
    Bufin[6] = ERR_DC_Battery_GY[0];����//һ�������ѹ
#else  //ת��λ��
    Bufin[6] = 0x00;
#endif 
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0805(��)

    //--------------------------------------------------------------- 
    //0x0805(��): <!--  ������ԴATSE2һ·��բ -->
#if 0
    if(Sys_cfg_info.ac_set.ATSE_num > 1)
    {
        if(Ac_info.atse_stat[1].sw == 1)
        {
            Bufin[0] = 1;
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;
        }
    }
    else
    {
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
    }
    Bufin[3] = ERR_MainATS1;
#else  //������ATSE����ĸ��ź��ϴ���̨ 
    for (i = 0; i < 4; i ++)
        Bufin[i] = 0;
    if (((Sys_cfg_info.ac_set.control_mode == 1)
                ||(Sys_cfg_info.ac_set.control_mode == 3))    //ATSE����  //�����ͺ���ATSͨѶ֧��.
            && (Sys_cfg_info.ac_set.ATSE_num > 1)) //ATSE��������1 
    {
        if(Ac_info.atse_stat[1].sw == 1)
        {
            Bufin[0] = 1;  //������ԴATSE2һ·��բ
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;  //������ԴATSE2��·��բ
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;  //������ԴATSE2��բ
        }
        Bufin[3] = ERR_MainATS1; //������ԴATSE2ͨѶ���� 
    }
#endif 

#if 0
    Bufin[4] = ERR_AC_Feeder_Duan_trip_2;
#else   //����������բ���źŻ򵽽�����������բ
    Bufin[4] = 0x00;
    for(i = (Sys_cfg_info.ac_set.state_monitor_num / 2);
            i < Sys_cfg_info.ac_set.state_monitor_num; i++)  //��һ����Ϊ����
    {
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2];   
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+8];
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+16];
        }
        for(j2=0;j2<8;j2++){
            Bufin[4] |= ERR_AC_Feeder_SW_trip[i][j2+24];
        }
    }
    Bufin[4] |= ERR_AC_Feeder_Duan_trip_2;
#endif 

    Bufin[5] = 0;
#if 0
    Bufin[6] = ERR_DC_Battery_GY[1]; //���������ѹ
#else  //ת��λ��
    Bufin[6] = 0x00;
#endif 
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0805(��)

    //--------------------------------------------------------------- 
    //0x0806(��): <!--  ������Դ����쳣�澯 -->	
    Bufin[0] = 0;    
#if 1  //�޸��ܼ�غͺ�̨�������1��2ͨѶ���ϵ�����.
    Bufin[0] |= ERR_AC_Meter_comm[0];    
    Bufin[0] |= ERR_AC_Meter_comm[1];    
#endif 
    Bufin[0] |= ERR_AC_Meter_comm[2];    
    Bufin[0] |= ERR_AC_Meter_comm[3];    

    Bufin[1] = Ac_info.ML_state;		//    Bufin[1] = gAC_MLine;        //???????????????
    Bufin[2] = 0;
    for(j2 =0;j2<16;j2++)
    {
        Bufin[2] |= ERR_AC_SW_comm[j2];
    }
    Bufin[3] = 0;
    for(j2 =0;j2<16;j2++)
    {
        Bufin[3] |= ERR_AC_St_comm[j2];
    }
    Bufin[4] = 0;
    for(j2 =0;j2<2;j2++)
    {
        Bufin[4] |= ERR_AC_CurrentSample_comm[j2];
    }
    Bufin[5] = 0;
    for(j2 =0;j2<2;j2++)
    {
        Bufin[5] |= ERR_AC_AcSample_comm[j2];
    }
    Bufin[6] = ERR_MainAC;
#if 0
    Bufin[7] = 0;				 //Bufin[7] = ERR_AC_Thunder;   //ĸ��������բ                  //?????????????
#else  //���������߹�ĸ��������բ�ź��ϴ���̨ 
    Bufin[7] = 0;
    if (Sys_cfg_info.ac_set.control_mode == 2)  //��ٿ���
    {
        Bufin[7] = ERR_AC_in_SW_trip[2];  //ĸ��������բ
    }
#endif 
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(��)

    //--------------------------------------------------------------- 
    //0x0806(��): 	<DOI name="Ind1" desc="ͨ�ŵ�Դ1ģ��ͨѶ����">
    if (LN_MK_communication == 2)  //(����)��������ģ��ʱ
    {
#if 0
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[0][j2];   //ģ��ͨ�Ź���
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[0][j2];	
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[2] |= ERR_Comm_Module_output_QY[0][j2];
        }
        Bufin[3] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[3] |= ERR_Comm_Module_AC_import_GY[0][j2];
        }
        Bufin[4] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module_AC_import_QY[0][j2];
        }
        Bufin[5] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[5] |= ERR_LN_Comm_Module[0][j2];   //ģ�����
        }
        Bufin[6] = ERR_MainComPw;                //�Ӽ��ͨ�Ź���
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[0][j2]; 
        }											
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(��) 
#else 
        gIEC61850_Data[j++] = 0;        //0x0806(��)
#endif 
    }else if (LN_MK_communication != 2){   //����������ģ��ʱ
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_Comm_Module_comm[0][j2];
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            Bufin[1] |= ERR_Comm_SW_comm[0][j2];
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            Bufin[2] |= ERR_Comm_St_comm[0][j2];
        }
        Bufin[3] = ERR_Comm_SPD[0];  
        Bufin[4] = 0x00;

#if 0
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module[0][j2];
        }
#else   //�޸�ͨ�ŵ�Դ��������쳣ʱ�󱨸���̨ͨ��ģ����ϵ����⣨���ֻ֧��14��ģ�飩. 
        for(j2=0;j2<14;j2++)
        {
            Bufin[4] |= ERR_Comm_Module[0][j2];
        }
#endif 
        Bufin[5] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            for(j1=0;j1<32;j1++)
            {
                Bufin[5] |= ERR_Comm_SW_trip[0][j2][j1];
            }
        }
        Bufin[5] |= ERR_Comm_Feeder_sw[0];

#if 0
        if(Special_35KV_flag == 2)
            Bufin[5] = ERR_DC_AcSwitch[0][2];  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����. 
        if ((Special_35KV_flag == 0x02) || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //����ͨ�ŵ�Դ���ʱ
            Bufin[5] = ERR_DC_AcSwitch[0][2];  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
        }  
#endif 
        Bufin[6] = ERR_MainComPw;
        Bufin[7] = 0;          
        Bufin[7] |= ERR_Comm_Module[0][14];  
        Bufin[7] |= ERR_Comm_Module[0][15]; 												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(��)
    }

    //--------------------------------------------------------------- 
    //0x0807(��): <!--  UPS1�е������쳣(1��UPS���������쳣) -->
    /********	080E		// UPS				**************/
    Bufin[0] = ERR_UPS_MainsSupply[0];
    Bufin[1] = ERR_UPS_DC[0];
    Bufin[2] = ERR_UPS_Bypass_output[0];
    Bufin[3] = ERR_UPS[0];
    Bufin[4] = ERR_UPS_Bypass[0];
    Bufin[5] = ERR_UPS_Overload[0];
    Bufin[6] = ERR_UPS_Comm[0];
    Bufin[7] = ERR_UPS_OutPut[0];	 //Bufin[7] = ERR_UPS_SW_Spe[31]; UPS1����쳣
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 	//0x0807(��)

    //--------------------------------------------------------------- 
    //0x0807(��): <!--  UPS2�е������쳣(2��UPS���������쳣) -->
    Bufin[0] = ERR_UPS_MainsSupply[1];
    Bufin[1] = ERR_UPS_DC[1];
    Bufin[2] = ERR_UPS_Bypass_output[1];
    Bufin[3] = ERR_UPS[1];
    Bufin[4] = ERR_UPS_Bypass[1];
    Bufin[5] = ERR_UPS_Overload[1];
    Bufin[6] = ERR_UPS_Comm[1];
    Bufin[7] = ERR_UPS_OutPut[1];	//Bufin[7] = ERR_UPS_SW_Spe[31]; UPS2����쳣
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0807(��)

    //--------------------------------------------------------------- 
    //0x0808(��): <!--  UPS�������߿�����բ�澯 -->
    Bufin[0] = 0;
    for(j2 = 0;j2<6;j2++)
    {
        for(j1=0;j1<32;j1++)
        {
            Bufin[0] |= ERR_UPS_SW_trip[j2][j1];
        }
    }
    Bufin[0] |= ERR_UPS_Feeder_sw[0];

#if 0
    if(Special_35KV_flag == 2)
        Bufin[0] = ERR_DC_AcSwitch[0][3];  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����. 
    if ((Special_35KV_flag == 0x02) || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02))
    {
        //����UPS���ʱ
        Bufin[0] = ERR_DC_AcSwitch[0][3];  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
    }       
#endif 
    
    Bufin[1] = 0x00;
    for(j2 = 0;j2<4;j2++)
    {
        Bufin[1] |= ERR_UPS_SW_Comm[j2];
    }
    Bufin[2] = 0x00;
    for(j2 = 0;j2<4;j2++)
    {
        Bufin[2] |= ERR_UPS_State_Comm[j2];
    }
    Bufin[3] = 0;
    Bufin[4] = ERR_MainUPS;    //����Դ�Ӽ��ͨѶ����
    Bufin[5] = ERR_UPS_SPD[0]; //UPS����������
    Bufin[6] = 0;
    Bufin[7] = 0;

    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0808(��)

    //--------------------------------------------------------------- 
    //0x0808(��): ��
#if 0
#if 0
    if (Sys_cfg_info.battery_set.CYH_communi == 1  || LN_MK_communication == 2){
#else //ϵͳ���õ�ز�����ͳһ���øĳɷ�������.
    if (Sys_cfg_info.battery_set[0].CYH_communi == 1  || LN_MK_communication == 2){
#endif 
        for(j2=0; j2<12; j2++){
            Bufin[0] |= ERR_comm_BatterySingle_GY[0][j2];   // 1�鵥���ع���
            Bufin[0] |= ERR_comm_BatterySingle_QY[0][j2];         												 
            Bufin[0] |= ERR_comm_BatterySingle_GY[1][j2];
            Bufin[0] |= ERR_comm_BatterySingle_QY[1][j2];
            Bufin[1] |= ERR_comm_BatterySingle_GY[2][j2];   // 2�鵥���ع���   
            Bufin[1] |= ERR_comm_BatterySingle_QY[2][j2];
            Bufin[1] |= ERR_comm_BatterySingle_GY[3][j2];
            Bufin[1] |= ERR_comm_BatterySingle_QY[3][j2];
        }
        Bufin[2] = 0x00;
        /*for(j2=0;j2<4;j2++)
          {
          for(j1=0;j1<32;j1++)
          {
          Bufin[2] |= ERR_Comm_SW_trip[0][j2][j1];
          }
          }
          Bufin[2] |= ERR_Comm_Feeder_sw[0];*/
        if(Special_35KV_flag == 2)
            //Bufin[2] = 1;
            //debug_printf(0,"Bufin[2]==%d\n",Bufin[2]);
            Bufin[2] = ERR_DC_AcSwitch[0][2];  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); //0x0808(��)
    }else {
        gIEC61850_Data[j++] = 0;         //����һ���ֽ�		        //0x0808(��)

    }	
#else  //��������ģ���ź���ģ���ļ���. 
    if (LN_MK_communication == 2)
    {
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[0][j2];   //ͨ�ŵ�Դ1ģ��ͨ�Ź���
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[0][j2]; //ͨ�ŵ�Դ1ģ�������ѹ�ػ�����	
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[2] |= ERR_Comm_Module_output_QY[0][j2];
        }
        Bufin[3] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[3] |= ERR_Comm_Module_AC_import_GY[0][j2];
        }
        Bufin[4] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module_AC_import_QY[0][j2];
        }
        Bufin[5] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[5] |= ERR_LN_Comm_Module[0][j2];   //ģ�����
        }
        Bufin[6] = ERR_MainComPw;                //�Ӽ��ͨ�Ź���
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[0][j2]; 
        }											
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0808(��)     
    }
    else 
    {
        gIEC61850_Data[j++] = 0;        //0x0808(��)
    }
#endif 

#if 0
    /*    Bufin[0] = ERR_MainUPS;
          Bufin[1] = 0;
          Bufin[2] = ERR_UPS_SPD[0];
          Bufin[3] = 0;
          Bufin[4] = 0;
          Bufin[5] = 0;
          Bufin[6] = 0;
          Bufin[7] = 0;
          gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0812

          if(LN_MK_communication == 2)
          {
          Bufin[0] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //ģ��ͨ�Ź���
          }
          Bufin[1] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[1] |= ERR_Comm_Module_output_GY[1][j2];
          }
          Bufin[2] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[2] |= ERR_Comm_Module_output_QY[1][j2];
          }
          Bufin[3] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[3] |= ERR_Comm_Module_AC_import_GY[1][j2];
          }
          Bufin[4] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[4] |= ERR_Comm_Module_AC_import_QY[1][j2];
          }
          Bufin[5] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //ģ�����
          }
          Bufin[6] = ERR_MainComPw1;                //�Ӽ��ͨ�Ź���
          Bufin[7] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[7] |= ERR_Comm_Module_bu_advection[1][j2];   
          }												
          gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0813  
          }else if (LN_MK_communication != 2){
          Bufin[0] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[0] |= ERR_Comm_Module_comm[1][j2];
          }
          Bufin[1] = 0x00;
          for(j2=0;j2<4;j2++)
          {
          Bufin[1] |= ERR_Comm_SW_comm[1][j2];
          }
          Bufin[2] = 0x00;
          for(j2=0;j2<4;j2++)
          {
          Bufin[2] |= ERR_Comm_St_comm[1][j2];
          }
          Bufin[3] = ERR_Comm_SPD[1];  
          Bufin[4] = 0x00;
          for(j2=0;j2<16;j2++)
          {
          Bufin[4] |= ERR_Comm_Module[1][j2];
          }
    Bufin[5] = 0x00;
    for(j2=0;j2<4;j2++)
    {
        for(j1=0;j1<32;j1++)
        {
            Bufin[5] |= ERR_Comm_SW_trip[1][j2][j1];
        }
    }
    Bufin[5] |= ERR_Comm_Feeder_sw[1];
    Bufin[6] = ERR_MainComPw1;
    Bufin[7] = 0;          
    Bufin[7] |= ERR_Comm_Module[1][14];  
    Bufin[7] |= ERR_Comm_Module[1][15]; 	 
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0813


    Bufin[0] = 0;              //�ֵ��һ��ĸ�߲�ƽ��
    Bufin[0] |= ERR_FenDian[0][48];
    Bufin[0] |= ERR_FenDian[1][48];
    Bufin[0] |= ERR_FenDian[2][48];
    Bufin[0] |= ERR_FenDian[3][48];
    Bufin[1] = 0;				//�ֵ�����ĸ�߲�ƽ��
    Bufin[1] |= ERR_FenDian[0][49];
    Bufin[1] |= ERR_FenDian[1][49];
    Bufin[1] |= ERR_FenDian[2][49];
    Bufin[1] |= ERR_FenDian[3][49];
    Bufin[2] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[2] |= ERR_DC_standard_cell_comm[0][j2];    //ֱ����Դ1��Ե��׼��Ԫͨ�Ź��� 
    }
    Bufin[3] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[3] |= ERR_DC_standard_cell_comm[1][j2];    //ֱ����Դ2��Ե��׼��Ԫͨ�Ź���
    }
    Bufin[4] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[4] |= ERR_DC_standard_cell_comm[2][j2];    //ֱ����Դ3��Ե��׼��Ԫͨ�Ź��� 
    }
    Bufin[5] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[5] |= ERR_DC_FG_SW_trip[0][i][j2];      //1#��������ز���ֵ����բ����16·��
            Bufin[5] |= ERR_DC_FG_SW_trip[0][i][j2+8];
        }
    }
    Bufin[6] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[6] |= ERR_DC_FG_SW_trip[1][i][j2];      //2#��������ز���ֵ����բ����16·��
            Bufin[6] |= ERR_DC_FG_SW_trip[1][i][j2+8];
        }
    }
    Bufin[7] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[7] |= ERR_DC_FG_SW_trip[2][i][j2];      //3#��������ز���ֵ����բ����16·��
            Bufin[7] |= ERR_DC_FG_SW_trip[2][i][j2+8];
        }
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);     //0x0814


    Bufin[0] = ERR_GLOD_Dev[0][0];
    Bufin[1] = ERR_GLOD_Dev[0][1];
    Bufin[2] = ERR_GLOD_Dev[0][2];
    Bufin[3] = ERR_GLOD_Dev[0][3];
    Bufin[4] = ERR_GLOD_Dev[0][4];
    Bufin[5] = ERR_GLOD_Dev[0][5];
    Bufin[6] = ERR_GLOD_Dev[0][6];
    Bufin[7] = ERR_GLOD_Dev[0][7];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);        //0x0817           */  

    /*gIEC61850_Data[j++] = 0;        //����һ���ֽ�(69)		    //0x0818
      gIEC61850_Data[j++] = 0;         //����һ���ֽ�		       //0x0819
      gIEC61850_Data[j++] = 0;         //����һ���ֽ�(70)		       //0x081a
      gIEC61850_Data[j++] = 0;         //����һ���ֽ�		       //0x081b
      gIEC61850_Data[j++] = 0;         //����һ���ֽ�(71)		       //0x081c */
#endif

    //--------------------------------------------------------------- 
    //0x0809(��):  

#if 0
    Bufin[0] = 0;
    Bufin[1] = 0;
    Bufin[2] = 0; 
    Bufin[3] = 0;
    Bufin[4] = 0;
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0809(��)
#else  //��������ģ���ź���ģ���ļ���.  
    if (LN_MK_communication == 2)  //(����)��������ģ��ʱ
    {
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //ͨ�ŵ�Դ2ģ��ͨ�Ź���
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[1][j2]; //ͨ�ŵ�Դ2ģ�������ѹ�ػ�����
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[2] |= ERR_Comm_Module_output_QY[1][j2];
        }
        Bufin[3] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[3] |= ERR_Comm_Module_AC_import_GY[1][j2];
        }
        Bufin[4] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module_AC_import_QY[1][j2];
        }
        Bufin[5] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //ģ�����
        }
        Bufin[6] = ERR_MainComPw1;                //�Ӽ��ͨ�Ź���
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[1][j2];   
        }												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	    
    }
    else 
    {
        gIEC61850_Data[j++] = 0x00;	 //0x0809(��)
    }
#endif 

    //--------------------------------------------------------------- 
    //0x0809(��): �� 48
    if(LN_MK_communication == 2)  //(����)��������ģ��ʱ
    {
#if 0
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //ģ��ͨ�Ź���
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[1][j2];
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[2] |= ERR_Comm_Module_output_QY[1][j2];
        }
        Bufin[3] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[3] |= ERR_Comm_Module_AC_import_GY[1][j2];
        }
        Bufin[4] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module_AC_import_QY[1][j2];
        }
        Bufin[5] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //ģ�����
        }
        Bufin[6] = ERR_MainComPw1;                //�Ӽ��ͨ�Ź���
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[1][j2];   
        }												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0809(��) 
#else 
        gIEC61850_Data[j++] = 0x00;	 //0x0809(��)
#endif 
    }else if (LN_MK_communication != 2){  //����������ģ��ʱ
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_Comm_Module_comm[1][j2];
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            Bufin[1] |= ERR_Comm_SW_comm[1][j2];
        }
        Bufin[2] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            Bufin[2] |= ERR_Comm_St_comm[1][j2];
        }
        Bufin[3] = ERR_Comm_SPD[1];  
        Bufin[4] = 0x00;

#if 0
        for(j2=0;j2<16;j2++)
        {
            Bufin[4] |= ERR_Comm_Module[1][j2];
        }
#else   //�޸�ͨ�ŵ�Դ��������쳣ʱ�󱨸���̨ͨ��ģ����ϵ����⣨���ֻ֧��14��ģ�飩.
        for(j2=0;j2<14;j2++)
        {
            Bufin[4] |= ERR_Comm_Module[1][j2];
        }
#endif 

        Bufin[5] = 0x00;
        for(j2=0;j2<4;j2++)
        {
            for(j1=0;j1<32;j1++)
            {
                Bufin[5] |= ERR_Comm_SW_trip[1][j2][j1];
            }
        }
        Bufin[5] |= ERR_Comm_Feeder_sw[1];
        Bufin[6] = ERR_MainComPw1;
        Bufin[7] = 0;          
        Bufin[7] |= ERR_Comm_Module[1][14];  
        Bufin[7] |= ERR_Comm_Module[1][15]; 	 
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0809(��)
    }

    //--------------------------------------------------------------- 
    //0x080A(��): <!--  ֱ����Դ3����һ·����(3��ֱ��ϵͳ��һ�齻���������)-->  
    //ֱ����3��
    if(ERR_DC_AcVoltage_GY[2][0] == 1 || ERR_DC_AcVoltage_QY[2][0] == 1 
            || ERR_DC_Ac_PowerCut[2][0] == 1 || ERR_DC_Ac_PhaseLoss[2][0] == 1)  
    {
        Bufin[0] = 0x01;      //����һ·����
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[2][1] == 1 || ERR_DC_AcVoltage_QY[2][1] == 1 
            || ERR_DC_Ac_PowerCut[2][1] == 1 || ERR_DC_Ac_PhaseLoss[2][1] == 1)  
    {
        Bufin[1] = 0x01;       					//������·����
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[2][0])&0x01;   //����һ·����
    Bufin[3] = (~ERR_DC_Ac1_state[2][1])&0x01;	 //������·����
    Bufin[4] = ERR_DC_AcSPD[2];                  //�������׹���

    if((ERR_DC_AcSwitch[2][0] == 0) && (ERR_DC_AcSwitch[2][1] == 0) 
            && (ERR_DC_AcSwitch[2][2] == 0)&& (ERR_DC_AcSwitch[2][3] == 0) )
    {
        Bufin[5] = 0x00;         
    }else{
        Bufin[5] = 0x01;						//�������ع���
    }

    Bufin[6] = ERR_DC_AcSample_comm[2];				//������ص�ԪͨѶ����
    
    //<!--  ֱ����Դ3���Ѳ����ͨѶ�Ͽ� -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//�������Ѳ�����
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[2];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x080A(��)

    //--------------------------------------------------------------- 
    //0x080A(��): <!--  ֱ����Դ3���ģ�����(3��ֱ��ϵͳ����ģ�����) -->
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++)  //��ԭ8·����12·���ģ��
    {
        Bufin[0] |= ERR_DC_Module[2][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)    //��ԭ8·����12·���ģ��
    {
        Bufin[1] |= ERR_DC_Module_comm[2][j2];
    }
    if(Sys_cfg_info.dc_set[2].switch_monitor_num>0)
    {
        Bufin[2] = 0x00;
        for(j2 = 0;j2<16;j2++)
        {
            for(j1 = 0;j1<32;j1++)
            {
                Bufin[2] |= ERR_DC_SW_trip[2][j2][j1];
            }
        }
        // 	Bufin[2] |= ERR_DC_SW_Spe[29];

#if 1   //������źš��������߿�����բ��
        for(j2 = 0;j2<24;j2++)  //��24·
        {
            for(j1 = 0;j1<16;j1++) 
            {
                Bufin[2] |= ERR_DC_FG_SW_trip[2][j2][j1];
            }
        }

#if 0
#if 0   
        Bufin[2] |= (ERR_DC_AcSwitch[2][3] | ERR_DC_AcSwitch[2][4]);
#else 
        if(Special_35KV_flag == 2)
        {
            Bufin[2] |= ERR_DC_AcSwitch[2][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[2][3] | ERR_DC_AcSwitch[2][4]);        
        }
#endif
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����.
        if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //�ñ�־λ���ܰ�����ȥ
            //ERR_DC_AcSwitch[2][3]: ��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
            Bufin[2] |= ERR_DC_AcSwitch[2][4];        
        }
        else 
        {
            Bufin[2] |= (ERR_DC_AcSwitch[2][3] | ERR_DC_AcSwitch[2][4]);        
        }
#endif 
#endif 
    }
    else
    {
        Bufin[2] = ERR_DC_External[2][0];
    }
#if 0
    Bufin[3] = ERR_DC_Battery_SW[2][0]; //��ؿ��ع���
#else   //ת��λ��
    Bufin[3] = 0x00;
#endif 

    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[2][0];
    Bufin[4] |= ERR_DC_BatteryFuse[2][1];
    Bufin[5] = ERR_DC_Battery_SW[2][1]; 				//����������բ����
    Bufin[6] = 0x00;
    for(j2=0;j2<16;j2++)
    {
        Bufin[6] |= ERR_DC_SW_Sample_comm[2][j2];
    }
    Bufin[7] = 0x00;
    for(j2=0;j2<8;j2++)
    {
        Bufin[7] |= ERR_DC_St_Sample_comm[2][j2];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); //0x080A(��)

    //--------------------------------------------------------------- 
    //0x080B(��): <!--  1��ֱ��ϵͳ����1#���ģ����� --> 
    //����1~3��ֱ��ϵͳ����1~8#���ģ�����
    //1��ֱ��ϵͳ����1~8#���ģ�����
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[0][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080B(��)
    for(k = 8; k < 12; k++)
    {
        Bufin[k - 8] = 0x00;
        Bufin[k - 8] |= ERR_DC_Module[0][k];
    }
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
    Bufin[6] = 0x00;
    Bufin[7] = 0x00;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080C(��): 2��ֱ��ϵͳ����1~8#���ģ�����
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[1][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080C(��)
    for(k = 8; k < 12; k++)
    {
        Bufin[k - 8] = 0x00;
        Bufin[k - 8] |= ERR_DC_Module[1][k];
    }
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
    Bufin[6] = 0x00;
    Bufin[7] = 0x00;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080D(��): 3��ֱ��ϵͳ����1~8#���ģ�����
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[2][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080D(��)
    for(k = 8; k < 12; k++)
    {
        Bufin[k - 8] = 0x00;
        Bufin[k - 8] |= ERR_DC_Module[2][k];
    }
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
    Bufin[6] = 0x00;
    Bufin[7] = 0x00;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //--------------------------------------------------------------- 
    //0x080E(��): <!--  ���鵥���ع���(3��ֱ��ϵͳ�����쳣)-->
    //<!--  ֱ����Դ3�Ӽ��ͨѶ����(3��ֱ��ϵͳ���װ��ͨ���ж�)--> 
#if 0
    for(j2=0; j2<108; j2++)
    {
        Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];        // 1�鵥���ع���
        Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
    }
     Bufin[1] = ERR_DC_JY_VF[2];   // 1��ѹ��澯 ��Ϊ ��طŵ�
#else  //ת��
    Bufin[0] = 0x00;
    Bufin[1] = 0x00;
#endif  
   
    Bufin[2] = ERR_DC_JY_detection[2][0]|ERR_DC_JY_detection[2][1];
    Bufin[3] = 0;
    for(j2=0;j2<16;j2++)
    {
        Bufin[3] |= ERR_DC_JY_Sample_comm[2][j2];
    }
    if(Sys_cfg_info.sys_set.insulate_mode == 0)
    {
        Bufin[4] = ERR_MainIsu;
    }
    else
    {
        Bufin[4] = 0;
    }
    //  if(bgCfg_DC_IsCool == 1)
    //  {
    //  	Bufin[5] = ERR_MainDC_COOL;
    //  }
    //  else
    //  {
    Bufin[5] = ERR_MainDC[2];
    //  }
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x080E(��): <!--  �������߶�·��һ��բ(վ�õ�1��ATS����1��·����բ) -->
    Bufin[0] = 0;
    Bufin[1] = 0;
    Bufin[2] = 0;
    Bufin[3] = 0;
    Bufin[4] = ERR_AC_SPD[2];   //������3����
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x080F(��): <!--  ���γ�����ѹ -->   
    Bufin[0] = ERR_DC_Charger_GY[2];
    Bufin[1] = ERR_DC_Charger_QY[2];
    //#ifdef S35KV
    //  Bufin[2] = ERR_DC_SW_Spe[25];
    //  Bufin[3] = ERR_DC_SW_Spe[26];
    //#else
    Bufin[2] = ERR_DC_KM_GY[2];
    Bufin[3] = ERR_DC_KM_QY[2];
    //#endif

#if 0    
    Bufin[4] = ERR_DC_Battery_QY[2];
    Bufin[5] = sys_ctl_info.battery_mode_ctl[2];
#else //ת��λ��
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif    

    Bufin[6] = ERR_DC_DcSample_comm[2];
    Bufin[7] = 0;                   //���Ѳ�쵥ԪͨѶ����
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x080F(��): <!--  UPS1�е�ģʽ -->
    gIEC61850_Data[j++] = (UPS_Mode[1] << 4) | UPS_Mode[0];

    //--------------------------------------------------------------- 
    //0x0810(��): һ����-1    <!--  һ���ع�ѹ -->  
    Bufin[0] = ERR_DC_Battery_GY[0]; //һ���ع�ѹ 
    Bufin[1] = ERR_DC_Battery_QY[0]; //һ����Ƿѹ

    //һ�鵥���ع�ѹ
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[0][i];            				
    }   

    //һ�鵥����Ƿѹ
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[0][i];
    }

    //һ�鵥���ع���
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[0][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[0][i];
    }
    
    Bufin[5] = ERR_DC_Battery_SW[0][0];           //һ���ؿ��ع���
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[0];  //һ���ؾ���
#else  //�޸��ϱ���̨�ĵ�ؾ����źŴ���. 
    Bufin[6] = battery_current_state[0];
#endif 
    Bufin[7] = Dc_info[0].battery.FG_discharging; //һ���طŵ�
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0810(��): һ����-2    
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
    {
        //<!--  һ���ز�����ͨѶ�ж� --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[0][i];
        }
        //<!--  һ�鵥�������賬�� -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[0][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0811(��): ������-1    <!--  �����ع�ѹ -->  
    Bufin[0] = ERR_DC_Battery_GY[1]; //�����ع�ѹ; 
    Bufin[1] = ERR_DC_Battery_QY[1]; //������Ƿѹ
    
    //���鵥���ع�ѹ
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[1][i];            				
    }   

    //���鵥����Ƿѹ
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[1][i];
    }
    
    //���鵥���ع���
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[1][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[1][i];
    }

    Bufin[5] = ERR_DC_Battery_SW[1][0];           //�����ؿ��ع���
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[1];  //�����ؾ���
#else  //�޸��ϱ���̨�ĵ�ؾ����źŴ���. 
    Bufin[6] = battery_current_state[1];
#endif 
    Bufin[7] = Dc_info[1].battery.FG_discharging; //�����طŵ�
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0811(��): ������-2
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
    {
        //<!--  һ���ز�����ͨѶ�ж� --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[1][i];
        }
        //<!--  һ�鵥�������賬�� -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[1][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0812(��): ������-1
    Bufin[0] = ERR_DC_Battery_GY[2]; //�����ع�ѹ 
    Bufin[1] = ERR_DC_Battery_QY[2]; //������Ƿѹ

    //���鵥���ع�ѹ
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[2][i];            				
    }   

    //���鵥����Ƿѹ
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[2][i];
    }

    //���鵥���ع���
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[2][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[2][i];
    }
    
    Bufin[5] = ERR_DC_Battery_SW[2][0];           //�����ؿ��ع���
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[2];  //�����ؾ���
#else    //�޸��ϱ���̨�ĵ�ؾ����źŴ���. 
    Bufin[6] = battery_current_state[2];
#endif 
    Bufin[7] = Dc_info[2].battery.FG_discharging; //�����طŵ�
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0812(��): ������-2
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
    {
        //<!--  һ���ز�����ͨѶ�ж� --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[2][i];
        }
        //<!--  һ�鵥�������賬�� -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[2][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0813(��): �ֵ���Ӽ��ͨѶ����
    Bufin[0] = ERR_DC_FG_comm[0][0];    //ֱ����Դ1��1#�ֵ���Ӽ��ͨѶ����
    Bufin[1] = ERR_DC_FG_comm[0][1];    //ֱ����Դ1��2#�ֵ���Ӽ��ͨѶ����
    Bufin[2] = ERR_DC_FG_comm[0][2];	//ֱ����Դ1��3#�ֵ���Ӽ��ͨѶ����
    Bufin[3] = ERR_DC_FG_comm[0][3];	//ֱ����Դ1��4#�ֵ���Ӽ��ͨѶ����
    Bufin[4] = ERR_DC_FG_comm[1][0];    //ֱ����Դ2��1#�ֵ���Ӽ��ͨѶ����
    Bufin[5] = ERR_DC_FG_comm[1][1];    //ֱ����Դ2��2#�ֵ���Ӽ��ͨѶ����
    Bufin[6] = ERR_DC_FG_comm[1][2];    //ֱ����Դ2��3#�ֵ���Ӽ��ͨѶ����
    Bufin[7] = ERR_DC_FG_comm[1][3];    //ֱ����Դ2��4#�ֵ���Ӽ��ͨѶ����
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);     

    //0x0813(��)
    Bufin[0] = ERR_DC_FG_comm[2][0];    //ֱ����Դ3��1#�ֵ���Ӽ��ͨѶ����
    Bufin[1] = ERR_DC_FG_comm[2][1];    //ֱ����Դ3��2#�ֵ���Ӽ��ͨѶ����
    Bufin[2] = ERR_DC_FG_comm[2][2];	//ֱ����Դ3��3#�ֵ���Ӽ��ͨѶ����
    Bufin[3] = ERR_DC_FG_comm[2][3];	//ֱ����Դ3��4#�ֵ���Ӽ��ͨѶ����
    Bufin[4] = 0;    
    Bufin[5] = 0;    
    Bufin[6] = 0;    
    Bufin[7] = 0;    
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);       

#if 0
    /*	for(j2=0; j2<108; j2++)
        {
        Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];            							// 1�鵥���ع���
        Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
        }
        Bufin[1] = ERR_DC_JY_VF[2];           												 // 1��ѹ��澯 ��Ϊ ��طŵ�
        Bufin[2] = ERR_DC_JY_detection[2][0]|ERR_DC_JY_detection[2][1];
        Bufin[3] = 0;
        for(j2=0;j2<16;j2++)
        {
        Bufin[1] |= ERR_DC_Module_comm[2][j2];
        }
        if(Sys_cfg_info.dc_set[2].switch_monitor_num>0)
        {
        Bufin[2] = 0x00;
        for(j2 = 0;j2<16;j2++)
        {
        for(j1 = 0;j1<32;j1++)
        {
        Bufin[2] |= ERR_DC_SW_trip[2][j2][j1];
        }
        }
    // 	Bufin[2] |= ERR_DC_SW_Spe[29];
    }
    else
    {
    Bufin[2] = ERR_DC_External[2][0];
    }
    Bufin[3] = ERR_DC_Battery_SW[2][0];
    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[2][0];
    Bufin[4] |= ERR_DC_BatteryFuse[2][1];
    Bufin[5] = ERR_DC_Battery_SW[2][1]; 				//����������բ����
    Bufin[6] = 0x00;
    for(j2=0;j2<16;j2++)
    {
    Bufin[6] |= ERR_DC_SW_Sample_comm[2][j2];
    }
    Bufin[7] = 0x00;
    for(j2=0;j2<8;j2++)
    {
    Bufin[7] |= ERR_DC_St_Sample_comm[2][j2];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); //0x081e

    Bufin[0] = ERR_DC_Charger_GY[2];
    Bufin[1] = ERR_DC_Charger_QY[2];
    //#ifdef S35KV
    //  Bufin[2] = ERR_DC_SW_Spe[25];
    //  Bufin[3] = ERR_DC_SW_Spe[26];
    //#else
    Bufin[2] = ERR_DC_KM_GY[2];
    Bufin[3] = ERR_DC_KM_QY[2];
    //#endif
    Bufin[4] = ERR_DC_Battery_QY[2];
    Bufin[5] = sys_ctl_info.battery_mode_ctl[2];
    Bufin[6] = ERR_DC_DcSample_comm[2];
    Bufin[7] = 0;                   //���Ѳ�쵥ԪͨѶ����
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x081f

    for(j2=0; j2<108; j2++)
    {
    Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];            							// 1�鵥���ع���
    Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
    }
    Bufin[1] = ERR_DC_JY_VF[2];           												 // 1��ѹ��澯 ��Ϊ ��طŵ�
    Bufin[2] = ERR_DC_JY_detection[2][0]|ERR_DC_JY_detection[2][1];
    Bufin[3] = 0;
    for(j2=0;j2<16;j2++)
    {
        Bufin[3] |= ERR_DC_JY_Sample_comm[2][j2];
    }
    if(Sys_cfg_info.sys_set.insulate_mode == 0)
    {
        Bufin[4] = ERR_MainIsu;
    }
    else
    {
        Bufin[4] = 0;
    }
    //  if(bgCfg_DC_IsCool == 1)
    //  {
    //  	Bufin[5] = ERR_MainDC_COOL;
    //  }
    //  else
    //  {
    Bufin[5] = ERR_MainDC[2];
    //  }
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0820*/

    /*
       Bufin[0] = 0;              //�ֵ��һ��ĸ�߲�ƽ��
       Bufin[0] |= ERR_FenDian[0][48];
       Bufin[0] |= ERR_FenDian[1][48];
       Bufin[0] |= ERR_FenDian[2][48];
       Bufin[0] |= ERR_FenDian[3][48];
       Bufin[1] = 0;				//�ֵ�����ĸ�߲�ƽ��
       Bufin[1] |= ERR_FenDian[0][49];
       Bufin[1] |= ERR_FenDian[1][49];
       Bufin[1] |= ERR_FenDian[2][49];
       Bufin[1] |= ERR_FenDian[3][49];
       Bufin[2] = ERR_MainIsu2[0];
       Bufin[3] = ERR_MainIsu2[1];
       Bufin[4] = ERR_MainIsu2[2];
       Bufin[5] = ERR_MainIsu2[3];
       Bufin[6] = 0;
       Bufin[7] = 0;
       gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);   //fen1

    //�ֵ�����Ϣ��Ϊ��һ�£�ÿ�ζ��ر�����
    Bufin[0] = 0;
    for(j2=0;j2<16;j2++)
    {
    Bufin[0] |= ERR_FenDian[0][j2];
    }
    Bufin[1] = 0;
    for(j2=32;j2<48;j2++)
    {
    Bufin[1] |= ERR_FenDian[0][j2];
    }
    Bufin[2] = 0;
    for(j2 = 0;j2<16;j2++)
    {
    for(j1 = 0;j1<32;j1++)
    {
    Bufin[2] |= ERR_FenDian1_FL[j2][j1];
    }
    }
    Bufin[2] |= ERR_DC_ACSwt4;
    Bufin[2] |= ERR_DC1_ACSwt4;

    if(gFaultFenDian1IsuNum>0)    	Bufin[3] = 1;
    else 							Bufin[3] = 0;


    Bufin[4] = 0;
    for(j2=0;j2<16;j2++)
    {
    Bufin[4] |= ERR_FenDian[1][j2];
    }
    Bufin[5] = 0;
    for(j2=32;j2<48;j2++)
    {
    Bufin[5] |= ERR_FenDian[1][j2];
    }
    Bufin[6] = 0;
    for(j2 = 0;j2<16;j2++)
    {
    for(j1 = 0;j1<32;j1++)
    {
    Bufin[6] |= ERR_FenDian2_FL[j2][j1];
    }
    }
    Bufin[6] |= ERR_DC_ACSwt5;
    Bufin[6] |= ERR_DC1_ACSwt5;

    if(gFaultFenDian2IsuNum>0)    	Bufin[7] = 1;
    else 							Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);   //fen2
    Bufin[0] = 0;
    for(j2=0;j2<16;j2++)
    {
        Bufin[0] |= ERR_FenDian[2][j2];
    }
    Bufin[1] = 0;
    for(j2=32;j2<48;j2++)
    {
        Bufin[1] |= ERR_FenDian[2][j2];
    }
    Bufin[2] = 0;
    for(j2 = 0;j2<16;j2++)
    {
        for(j1 = 0;j1<32;j1++)
        {
            Bufin[2] |= ERR_FenDian3_FL[j2][j1];
        }
    }
    Bufin[2] |= ERR_DC_ACSwt6;
    Bufin[2] |= ERR_DC1_ACSwt6;

    if(gFaultFenDian3IsuNum>0)    	Bufin[3] = 1;
    else 							Bufin[3] = 0;


    Bufin[4] = 0;
    for(j2=0;j2<16;j2++)
    {
        Bufin[4] |= ERR_FenDian[3][j2];
    }
    Bufin[5] = 0;
    for(j2=32;j2<48;j2++)
    {
        Bufin[5] |= ERR_FenDian[3][j2];
    }
    Bufin[6] = 0;
    for(j2 = 0;j2<16;j2++)
    {
        for(j1 = 0;j1<32;j1++)
        {
            Bufin[6] |= ERR_FenDian4_FL[j2][j1];
        }
    }
    Bufin[6] |= ERR_DC_ACSwt7;
    Bufin[6] |= ERR_DC1_ACSwt7;

    if(gFaultFenDian4IsuNum>0)    	Bufin[7] = 1;
    else 							Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);   //fen3

    Bufin[0] = ERR_GLOD_Dev[0][0];
    Bufin[1] = ERR_GLOD_Dev[0][1];
    Bufin[2] = ERR_GLOD_Dev[0][2];
    Bufin[3] = ERR_GLOD_Dev[0][3];
    Bufin[4] = ERR_GLOD_Dev[0][4];
    Bufin[5] = ERR_GLOD_Dev[0][5];
    Bufin[6] = ERR_GLOD_Dev[0][6];
    Bufin[7] = ERR_GLOD_Dev[0][7];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);     //fen4

    Bufin[0] = ERR_GLOD_Dev[0][8];
    Bufin[1] = ERR_MainPWR;    //�������ڵ��Ѳ����ͨ�Ź���
    Bufin[2] = 0;
    Bufin[3] = 0;
    Bufin[4] = 0;
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    Bufin[0] = ERR_GLOD_Dev[1][0];
    Bufin[1] = ERR_GLOD_Dev[1][1];
    Bufin[2] = ERR_GLOD_Dev[1][2];
    Bufin[3] = ERR_GLOD_Dev[1][3];
    Bufin[4] = ERR_GLOD_Dev[1][4];
    Bufin[5] = ERR_GLOD_Dev[1][5];
    Bufin[6] = ERR_GLOD_Dev[1][6];
    Bufin[7] = ERR_GLOD_Dev[1][7];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    Bufin[0] = ERR_GLOD_Dev[1][8];
    Bufin[1] = 0;
    Bufin[2] = 0;
    Bufin[3] = 0;
    Bufin[4] = 0;
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    */
#endif


    /******************************************************************************************/  //ֱ������	
    //��1��ֱ������    
    j = 0x0900;  //״̬��(�ֺ�)  ���40* 4 = 160�ֽ�
    group = 0;   
    num_st = (Sys_cfg_info.dc_set[0].state_monitor_num > Sys_cfg_info.dc_set[0].switch_monitor_num)
        ?(Sys_cfg_info.dc_set[0].state_monitor_num):(Sys_cfg_info.dc_set[0].switch_monitor_num);

    if (num_st > 40)
    {
        num_st = 40; 
    }

    if(num_st < 17){  //1~16 
        for(i=0;i<num_st;i++){     //ÿ�����32�տ�
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2];  //���μ��鼸·״̬
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }
    else if (num_st > 16) //1~(17~40)
    {   
        for(i=0;i<16;i++){     //ÿ�����32�տ�
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2];  //���μ��鼸·״̬
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[0].FGfeederLine[i].FGstate[j2];  //1�ηֹ񼸶μ��鼸·״̬
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[0].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x0E00;   //������(��բ)  ���40* 4 = 160�ֽ�
    if (num_st < 17){
        for(i=0;i<num_st;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2];    //���μ��鼸·��բ(������)
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);  

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }
    else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2];    //���μ��鼸·��բ(������)
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);  

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16��,����Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[0][i][j2];  //1�ηֹ񼸶μ��鼸·��բ
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[0][i][j2+8]; 
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x0F00;   //�ӵ���Ϣ ���40* 4 = 160�ֽ�
    //if (num_st <= 40){
    for(i=0;i<num_st;i++){
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];  //���μ��鼸·��Ե(��ĸ����ĸ)
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 

        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 

        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 
    }
    //}
#if 0  //�������ô���
    else if ( num_st > 16)
    {
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];  //���μ��鼸·��Ե(��ĸ����ĸ)
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //1�ηֹ񼸶μ��鼸·��Ե
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }
#endif 

    //��2��ֱ������
    j = 0x1900;  //״̬��(�ֺ�)  ���40* 4 = 160�ֽ�
    group = 1;
    if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[1].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[1].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[1].FGfeederLine[i].FGstate[j2];  //2�ηֹ񼸶μ��鼸·״̬
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[1].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x1E00;   //������(��բ)  ���40* 4 = 160�ֽ�
    if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[1].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[1][i][j2];  //2�ηֹ񼸶μ��鼸·��բ
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[1][i][j2+8]; ;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x1F00;   //�ӵ���Ϣ ���40* 4 = 160�ֽ�
    //if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[1].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
#if 0   //�������ô���
    }else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //2�ηֹ񼸶μ��鼸·��Ե
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }
#endif 

    j = 0x2900;  //״̬��(�ֺ�)  ���40* 4 = 160�ֽ�
    group = 2;
    if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[2].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_info[2].feederLine[i].state[j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[2].FGfeederLine[i].FGstate[j2];  //3�ηֹ񼸶μ��鼸·״̬
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[2].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x2E00;   //������(��բ)  ���40* 4 = 160�ֽ�
    if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[2].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }else if ( num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[2][i][j2];  //3�ηֹ񼸶μ��鼸·��բ
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[2][i][j2+8]; 
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //����
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x2F00;   //�ӵ���Ϣ ���40* 4 = 160�ֽ�
    //if (num_st < 17){
        for(i=0;i<Sys_cfg_info.dc_set[2].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
#if 0   //�������ô���
    }else if (num_st > 16){
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+8] | ERR_DC_SW_H_JY[group][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+16] | ERR_DC_SW_H_JY[group][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2+24] | ERR_DC_SW_H_JY[group][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }

        for(i=0;i<(num_st-16);i++){     //ÿ��16������Э��
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //3�ηֹ񼸶μ��鼸·��Ե
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 
    }
#endif 

    //��������
    j = 0x0A00;
    for(i=0;i<Sys_cfg_info.ac_set.state_monitor_num;i++){
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ac_info.feederLine[i].state[j2];    //���鼸·״̬
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ac_info.feederLine[i].state[j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ac_info.feederLine[i].state[j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ac_info.feederLine[i].state[j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(i=0;i<Sys_cfg_info.ac_set.state_monitor_num;i++){
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_AC_Feeder_SW_trip[i][j2];   //���鼸·��բ��2���������2�Σ�1��1��33��2��2��34!
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_AC_Feeder_SW_trip[i][j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_AC_Feeder_SW_trip[i][j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_AC_Feeder_SW_trip[i][j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }

    //UPS����
    j = 0x0B00;
#if 0
    for(i=0;i<Sys_cfg_info.ups_set.state_monitor_num;i++){  // UPS���⿪��״̬��1��״̬���ǰ��2���ֽ�
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[i].state[j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[i].state[j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[i].state[j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[i].state[j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }                

    if(UPS_Special_sw_flag == 2)  //UPS���⿪��
    {
        Bufin[0] = ERR_UPS_DcInput_sw[0];   //UPS���⿪�ؿ�����
        Bufin[1] = ERR_UPS_AcInput_sw[0];
        Bufin[2] = ERR_UPS_BypassInput_sw[0];
        Bufin[3] = ERR_UPS_AcOutput_sw[0];
        Bufin[4] = ERR_UPS_Bypass_Overhaul_sw[0];
        Bufin[5] = ERR_UPS_DcInput_sw[1];
        Bufin[6] = ERR_UPS_AcInput_sw[1];
        Bufin[7] = ERR_UPS_BypassInput_sw[1];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_UPS_AcOutput_sw[1];
        Bufin[1] = ERR_UPS_Bypass_Overhaul_sw[1];
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(i=0;i<Sys_cfg_info.ups_set.state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }       
    }
    else
    {
        for(i=0;i<Sys_cfg_info.ups_set.state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_UPS_SW_trip[i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 

    }
#else  //��ʱֻ�ϱ�UPS���⿪�ص�״̬����բ�źŸ���̨(8�ֽ�) 
    if(UPS_Special_sw_flag == 2)  //UPS���⿪��״̬��
    {
        //ǰ4���ֽ�Ϊ���⿪�ص�״̬���ֺ�բ
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[0].state[j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[0].state[j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[0].state[j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ups_info[0].feederLine[0].state[j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

        //��4���ֽ�Ϊ���⿪�ص���բ�ź�        
        Bufin[0] = ERR_UPS_DcInput_sw[0];   //UPS���⿪�ؿ�����
        Bufin[1] = ERR_UPS_AcInput_sw[0];
        Bufin[2] = ERR_UPS_BypassInput_sw[0];
        Bufin[3] = ERR_UPS_AcOutput_sw[0];
        Bufin[4] = ERR_UPS_Bypass_Overhaul_sw[0];
        Bufin[5] = ERR_UPS_DcInput_sw[1];
        Bufin[6] = ERR_UPS_AcInput_sw[1];
        Bufin[7] = ERR_UPS_BypassInput_sw[1];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_UPS_AcOutput_sw[1];
        Bufin[1] = ERR_UPS_Bypass_Overhaul_sw[1];
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = 0;
        Bufin[1] = 0;
        Bufin[2] = 0;
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
#endif 

    j = 0x0C00;
    //	j += 4;
    for(i=0;i<Sys_cfg_info.comm_set[0].state_monitor_num;i++){    //һ����״̬
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[0].feederLine[i].state[j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[0].feederLine[i].state[j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[0].feederLine[i].state[j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[0].feederLine[i].state[j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    } 
    if(Comm_Special_sw_flag == 2)  //���ڵ�ģʽ�����ȥ��(ģ��ͨ�����⿪��)
    {
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[0][j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[0][j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[0][j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[0][j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

        for(i=0;i<Sys_cfg_info.comm_set[0].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 
    }
    else
    {//	j += 4;
        for(i=0;i<Sys_cfg_info.comm_set[0].state_monitor_num;i++){    //һ������բ
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[0][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 
    }


    j = 0x1C00;
    for(i=0;i<Sys_cfg_info.comm_set[1].state_monitor_num;i++){    //������״̬
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[1].feederLine[i].state[j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[1].feederLine[i].state[j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[1].feederLine[i].state[j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Comm_info[1].feederLine[i].state[j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    } 
    if(Comm_Special_sw_flag == 2)   //���ڵ�ģʽ�����ȥ��
    {
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[1][j2];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[1][j2+8];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[1][j2+16];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_Comm_SW_specialty[1][j2+24];
        }
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

        for(i=0;i<Sys_cfg_info.comm_set[1].state_monitor_num;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 
    }
    else
    {
        for(i=0;i<Sys_cfg_info.comm_set[1].state_monitor_num;i++){    //��������բ
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+16];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_Comm_SW_trip[1][i][j2+24];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        } 
    }

#if 0
    /*	
        j=0x0D00;   
        gIEC61850_Data[j++] = DC_II_1.Row[7][1];
        gIEC61850_Data[j++] = DC_II_1.Row[7][0];
        gIEC61850_Data[j++] = DC_II_1.Row[8][1];
        gIEC61850_Data[j++] = DC_II_1.Row[8][0];
        gIEC61850_Data[j++] = DC_II_1.Row[9][1];
        gIEC61850_Data[j++] = DC_II_1.Row[9][0];
        gIEC61850_Data[j++] = DC_II_2.Row[7][1];
        gIEC61850_Data[j++] = DC_II_2.Row[7][0];
        gIEC61850_Data[j++] = DC_II_2.Row[8][1];
        gIEC61850_Data[j++] = DC_II_2.Row[8][0];
        gIEC61850_Data[j++] = DC_II_2.Row[9][1];
        gIEC61850_Data[j++] = DC_II_2.Row[9][0];


        gIEC61850_Data[j++] = DC_II_1.Row[10][1];
        gIEC61850_Data[j++] = DC_II_1.Row[10][0];
        gIEC61850_Data[j++] = DC_II_1.Row[11][1];
        gIEC61850_Data[j++] = DC_II_1.Row[11][0];
        gIEC61850_Data[j++] = DC_II_1.Row[12][1];
        gIEC61850_Data[j++] = DC_II_1.Row[12][0];
        gIEC61850_Data[j++] = DC_II_2.Row[10][1];
        gIEC61850_Data[j++] = DC_II_2.Row[10][0];
        gIEC61850_Data[j++] = DC_II_2.Row[11][1];
        gIEC61850_Data[j++] = DC_II_2.Row[11][0];
        gIEC61850_Data[j++] = DC_II_2.Row[12][1];
        gIEC61850_Data[j++] = DC_II_2.Row[12][0];

        gIEC61850_Data[j++] = DC_II_1.Row[13][1];
        gIEC61850_Data[j++] = DC_II_1.Row[13][0];
        gIEC61850_Data[j++] = DC_II_1.Row[14][1];
        gIEC61850_Data[j++] = DC_II_1.Row[14][0];
        gIEC61850_Data[j++] = DC_II_1.Row[15][1];
        gIEC61850_Data[j++] = DC_II_1.Row[15][0];
        gIEC61850_Data[j++] = DC_II_2.Row[13][1];
        gIEC61850_Data[j++] = DC_II_2.Row[13][0];
        gIEC61850_Data[j++] = DC_II_2.Row[14][1];
        gIEC61850_Data[j++] = DC_II_2.Row[14][0];
        gIEC61850_Data[j++] = DC_II_2.Row[15][1];
        gIEC61850_Data[j++] = DC_II_2.Row[15][0];


        gIEC61850_Data[j++] = DC_II_1.Row[13+3][1];
        gIEC61850_Data[j++] = DC_II_1.Row[13+3][0];
        gIEC61850_Data[j++] = DC_II_1.Row[14+3][1];
        gIEC61850_Data[j++] = DC_II_1.Row[14+3][0];
        gIEC61850_Data[j++] = DC_II_1.Row[15+3][1];
        gIEC61850_Data[j++] = DC_II_1.Row[15+3][0];
        gIEC61850_Data[j++] = DC_II_2.Row[13+3][1];
        gIEC61850_Data[j++] = DC_II_2.Row[13+3][0];
        gIEC61850_Data[j++] = DC_II_2.Row[14+3][1];
        gIEC61850_Data[j++] = DC_II_2.Row[14+3][0];
        gIEC61850_Data[j++] = DC_II_2.Row[15+3][1];
        gIEC61850_Data[j++] = DC_II_2.Row[15+3][0];

        gIEC61850_Data[j++] = DC_II_1.Row[13+6][1];
        gIEC61850_Data[j++] = DC_II_1.Row[13+6][0];
        gIEC61850_Data[j++] = DC_II_1.Row[14+6][1];
        gIEC61850_Data[j++] = DC_II_1.Row[14+6][0];
        gIEC61850_Data[j++] = DC_II_1.Row[15+6][1];
        gIEC61850_Data[j++] = DC_II_1.Row[15+6][0];
        gIEC61850_Data[j++] = DC_II_2.Row[13+6][1];
        gIEC61850_Data[j++] = DC_II_2.Row[13+6][0];
        gIEC61850_Data[j++] = DC_II_2.Row[14+6][1];
        gIEC61850_Data[j++] = DC_II_2.Row[14+6][0];
        gIEC61850_Data[j++] = DC_II_2.Row[15+6][1];
        gIEC61850_Data[j++] = DC_II_2.Row[15+6][0];


        gIEC61850_Data[j++] = DC_II_1.Row[13+9][1];
    gIEC61850_Data[j++] = DC_II_1.Row[13+9][0];
    gIEC61850_Data[j++] = DC_II_1.Row[14+9][1];
    gIEC61850_Data[j++] = DC_II_1.Row[14+9][0];
    gIEC61850_Data[j++] = DC_II_1.Row[15+9][1];
    gIEC61850_Data[j++] = DC_II_1.Row[15+9][0];
    gIEC61850_Data[j++] = DC_II_2.Row[13+9][1];
    gIEC61850_Data[j++] = DC_II_2.Row[13+9][0];
    gIEC61850_Data[j++] = DC_II_2.Row[14+9][1];
    gIEC61850_Data[j++] = DC_II_2.Row[14+9][0];
    gIEC61850_Data[j++] = DC_II_2.Row[15+9][1];
    gIEC61850_Data[j++] = DC_II_2.Row[15+9][0];



    j = 0x0E00;
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = Sta_FenDian1[j2][0];
        Bufin[1] = Sta_FenDian1[j2][1];
        Bufin[2] = Sta_FenDian1[j2][2];
        Bufin[3] = Sta_FenDian1[j2][3];
        Bufin[4] = Sta_FenDian1[j2][4];
        Bufin[5] = Sta_FenDian1[j2][5];
        Bufin[6] = Sta_FenDian1[j2][6];
        Bufin[7] = Sta_FenDian1[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian1[j2][8];
        Bufin[1] = Sta_FenDian1[j2][9];
        Bufin[2] = Sta_FenDian1[j2][10];
        Bufin[3] = Sta_FenDian1[j2][11];
        Bufin[4] = Sta_FenDian1[j2][12];
        Bufin[5] = Sta_FenDian1[j2][13];
        Bufin[6] = Sta_FenDian1[j2][14];
        Bufin[7] = Sta_FenDian1[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian1[j2][16];
        Bufin[1] = Sta_FenDian1[j2][17];
        Bufin[2] = Sta_FenDian1[j2][18];
        Bufin[3] = Sta_FenDian1[j2][19];
        Bufin[4] = Sta_FenDian1[j2][20];
        Bufin[5] = Sta_FenDian1[j2][21];
        Bufin[6] = Sta_FenDian1[j2][22];
        Bufin[7] = Sta_FenDian1[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian1[j2][24];
        Bufin[1] = Sta_FenDian1[j2][25];
        Bufin[2] = Sta_FenDian1[j2][26];
        Bufin[3] = Sta_FenDian1[j2][27];
        Bufin[4] = Sta_FenDian1[j2][28];
        Bufin[5] = Sta_FenDian1[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = Sta_FenDian2[j2][0];
        Bufin[1] = Sta_FenDian2[j2][1];
        Bufin[2] = Sta_FenDian2[j2][2];
        Bufin[3] = Sta_FenDian2[j2][3];
        Bufin[4] = Sta_FenDian2[j2][4];
        Bufin[5] = Sta_FenDian2[j2][5];
        Bufin[6] = Sta_FenDian2[j2][6];
        Bufin[7] = Sta_FenDian2[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian2[j2][8];
        Bufin[1] = Sta_FenDian2[j2][9];
        Bufin[2] = Sta_FenDian2[j2][10];
        Bufin[3] = Sta_FenDian2[j2][11];
        Bufin[4] = Sta_FenDian2[j2][12];
        Bufin[5] = Sta_FenDian2[j2][13];
        Bufin[6] = Sta_FenDian2[j2][14];
        Bufin[7] = Sta_FenDian2[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian2[j2][16];
        Bufin[1] = Sta_FenDian2[j2][17];
        Bufin[2] = Sta_FenDian2[j2][18];
        Bufin[3] = Sta_FenDian2[j2][19];
        Bufin[4] = Sta_FenDian2[j2][20];
        Bufin[5] = Sta_FenDian2[j2][21];
        Bufin[6] = Sta_FenDian2[j2][22];
        Bufin[7] = Sta_FenDian2[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian2[j2][24];
        Bufin[1] = Sta_FenDian2[j2][25];
        Bufin[2] = Sta_FenDian2[j2][26];
        Bufin[3] = Sta_FenDian2[j2][27];
        Bufin[4] = Sta_FenDian2[j2][28];
        Bufin[5] = Sta_FenDian2[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = Sta_FenDian3[j2][0];
        Bufin[1] = Sta_FenDian3[j2][1];
        Bufin[2] = Sta_FenDian3[j2][2];
        Bufin[3] = Sta_FenDian3[j2][3];
        Bufin[4] = Sta_FenDian3[j2][4];
        Bufin[5] = Sta_FenDian3[j2][5];
        Bufin[6] = Sta_FenDian3[j2][6];
        Bufin[7] = Sta_FenDian3[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian3[j2][8];
        Bufin[1] = Sta_FenDian3[j2][9];
        Bufin[2] = Sta_FenDian3[j2][10];
        Bufin[3] = Sta_FenDian3[j2][11];
        Bufin[4] = Sta_FenDian3[j2][12];
        Bufin[5] = Sta_FenDian3[j2][13];
        Bufin[6] = Sta_FenDian3[j2][14];
        Bufin[7] = Sta_FenDian3[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian3[j2][16];
        Bufin[1] = Sta_FenDian3[j2][17];
        Bufin[2] = Sta_FenDian3[j2][18];
        Bufin[3] = Sta_FenDian3[j2][19];
        Bufin[4] = Sta_FenDian3[j2][20];
        Bufin[5] = Sta_FenDian3[j2][21];
        Bufin[6] = Sta_FenDian3[j2][22];
        Bufin[7] = Sta_FenDian3[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian3[j2][24];
        Bufin[1] = Sta_FenDian3[j2][25];
        Bufin[2] = Sta_FenDian3[j2][26];
        Bufin[3] = Sta_FenDian3[j2][27];
        Bufin[4] = Sta_FenDian3[j2][28];
        Bufin[5] = Sta_FenDian3[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = Sta_FenDian4[j2][0];
        Bufin[1] = Sta_FenDian4[j2][1];
        Bufin[2] = Sta_FenDian4[j2][2];
        Bufin[3] = Sta_FenDian4[j2][3];
        Bufin[4] = Sta_FenDian4[j2][4];
        Bufin[5] = Sta_FenDian4[j2][5];
        Bufin[6] = Sta_FenDian4[j2][6];
        Bufin[7] = Sta_FenDian4[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian4[j2][8];
        Bufin[1] = Sta_FenDian4[j2][9];
        Bufin[2] = Sta_FenDian4[j2][10];
        Bufin[3] = Sta_FenDian4[j2][11];
        Bufin[4] = Sta_FenDian4[j2][12];
        Bufin[5] = Sta_FenDian4[j2][13];
        Bufin[6] = Sta_FenDian4[j2][14];
        Bufin[7] = Sta_FenDian4[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian4[j2][16];
        Bufin[1] = Sta_FenDian4[j2][17];
        Bufin[2] = Sta_FenDian4[j2][18];
        Bufin[3] = Sta_FenDian4[j2][19];
        Bufin[4] = Sta_FenDian4[j2][20];
        Bufin[5] = Sta_FenDian4[j2][21];
        Bufin[6] = Sta_FenDian4[j2][22];
        Bufin[7] = Sta_FenDian4[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = Sta_FenDian4[j2][24];
        Bufin[1] = Sta_FenDian4[j2][25];
        Bufin[2] = Sta_FenDian4[j2][26];
        Bufin[3] = Sta_FenDian4[j2][27];
        Bufin[4] = Sta_FenDian4[j2][28];
        Bufin[5] = Sta_FenDian4[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }

    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = ERR_FenDian1_FL[j2][0];
        Bufin[1] = ERR_FenDian1_FL[j2][1];
        Bufin[2] = ERR_FenDian1_FL[j2][2];
        Bufin[3] = ERR_FenDian1_FL[j2][3];
        Bufin[4] = ERR_FenDian1_FL[j2][4];
        Bufin[5] = ERR_FenDian1_FL[j2][5];
        Bufin[6] = ERR_FenDian1_FL[j2][6];
        Bufin[7] = ERR_FenDian1_FL[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian1_FL[j2][8];
        Bufin[1] = ERR_FenDian1_FL[j2][9];
        Bufin[2] = ERR_FenDian1_FL[j2][10];
        Bufin[3] = ERR_FenDian1_FL[j2][11];
        Bufin[4] = ERR_FenDian1_FL[j2][12];
        Bufin[5] = ERR_FenDian1_FL[j2][13];
        Bufin[6] = ERR_FenDian1_FL[j2][14];
        Bufin[7] = ERR_FenDian1_FL[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian1_FL[j2][16];
        Bufin[1] = ERR_FenDian1_FL[j2][17];
        Bufin[2] = ERR_FenDian1_FL[j2][18];
        Bufin[3] = ERR_FenDian1_FL[j2][19];
        Bufin[4] = ERR_FenDian1_FL[j2][20];
        Bufin[5] = ERR_FenDian1_FL[j2][21];
        Bufin[6] = ERR_FenDian1_FL[j2][22];
        Bufin[7] = ERR_FenDian1_FL[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian1_FL[j2][24];
        Bufin[1] = ERR_FenDian1_FL[j2][25];
        Bufin[2] = ERR_FenDian1_FL[j2][26];
        Bufin[3] = ERR_FenDian1_FL[j2][27];
        Bufin[4] = ERR_FenDian1_FL[j2][28];
        Bufin[5] = ERR_FenDian1_FL[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = ERR_FenDian2_FL[j2][0];
        Bufin[1] = ERR_FenDian2_FL[j2][1];
        Bufin[2] = ERR_FenDian2_FL[j2][2];
        Bufin[3] = ERR_FenDian2_FL[j2][3];
        Bufin[4] = ERR_FenDian2_FL[j2][4];
        Bufin[5] = ERR_FenDian2_FL[j2][5];
        Bufin[6] = ERR_FenDian2_FL[j2][6];
        Bufin[7] = ERR_FenDian2_FL[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian2_FL[j2][8];
        Bufin[1] = ERR_FenDian2_FL[j2][9];
        Bufin[2] = ERR_FenDian2_FL[j2][10];
        Bufin[3] = ERR_FenDian2_FL[j2][11];
        Bufin[4] = ERR_FenDian2_FL[j2][12];
        Bufin[5] = ERR_FenDian2_FL[j2][13];
        Bufin[6] = ERR_FenDian2_FL[j2][14];
        Bufin[7] = ERR_FenDian2_FL[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian2_FL[j2][16];
        Bufin[1] = ERR_FenDian2_FL[j2][17];
        Bufin[2] = ERR_FenDian2_FL[j2][18];
        Bufin[3] = ERR_FenDian2_FL[j2][19];
        Bufin[4] = ERR_FenDian2_FL[j2][20];
        Bufin[5] = ERR_FenDian2_FL[j2][21];
        Bufin[6] = ERR_FenDian2_FL[j2][22];
        Bufin[7] = ERR_FenDian2_FL[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian2_FL[j2][24];
        Bufin[1] = ERR_FenDian2_FL[j2][25];
        Bufin[2] = ERR_FenDian2_FL[j2][26];
        Bufin[3] = ERR_FenDian2_FL[j2][27];
        Bufin[4] = ERR_FenDian2_FL[j2][28];
        Bufin[5] = ERR_FenDian2_FL[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }

    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = ERR_FenDian3_FL[j2][0];
        Bufin[1] = ERR_FenDian3_FL[j2][1];
        Bufin[2] = ERR_FenDian3_FL[j2][2];
        Bufin[3] = ERR_FenDian3_FL[j2][3];
        Bufin[4] = ERR_FenDian3_FL[j2][4];
        Bufin[5] = ERR_FenDian3_FL[j2][5];
        Bufin[6] = ERR_FenDian3_FL[j2][6];
        Bufin[7] = ERR_FenDian3_FL[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian3_FL[j2][8];
        Bufin[1] = ERR_FenDian3_FL[j2][9];
        Bufin[2] = ERR_FenDian3_FL[j2][10];
        Bufin[3] = ERR_FenDian3_FL[j2][11];
        Bufin[4] = ERR_FenDian3_FL[j2][12];
        Bufin[5] = ERR_FenDian3_FL[j2][13];
        Bufin[6] = ERR_FenDian3_FL[j2][14];
        Bufin[7] = ERR_FenDian3_FL[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian3_FL[j2][16];
        Bufin[1] = ERR_FenDian3_FL[j2][17];
        Bufin[2] = ERR_FenDian3_FL[j2][18];
        Bufin[3] = ERR_FenDian3_FL[j2][19];
        Bufin[4] = ERR_FenDian3_FL[j2][20];
        Bufin[5] = ERR_FenDian3_FL[j2][21];
        Bufin[6] = ERR_FenDian3_FL[j2][22];
        Bufin[7] = ERR_FenDian3_FL[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian3_FL[j2][24];
        Bufin[1] = ERR_FenDian3_FL[j2][25];
        Bufin[2] = ERR_FenDian3_FL[j2][26];
        Bufin[3] = ERR_FenDian3_FL[j2][27];
        Bufin[4] = ERR_FenDian3_FL[j2][28];
        Bufin[5] = ERR_FenDian3_FL[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }
    for(j2 = 0;j2<FENDIAN_FL_NUM;j2++)
    {
        Bufin[0] = ERR_FenDian4_FL[j2][0];
        Bufin[1] = ERR_FenDian4_FL[j2][1];
        Bufin[2] = ERR_FenDian4_FL[j2][2];
        Bufin[3] = ERR_FenDian4_FL[j2][3];
        Bufin[4] = ERR_FenDian4_FL[j2][4];
        Bufin[5] = ERR_FenDian4_FL[j2][5];
        Bufin[6] = ERR_FenDian4_FL[j2][6];
        Bufin[7] = ERR_FenDian4_FL[j2][7];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian4_FL[j2][8];
        Bufin[1] = ERR_FenDian4_FL[j2][9];
        Bufin[2] = ERR_FenDian4_FL[j2][10];
        Bufin[3] = ERR_FenDian4_FL[j2][11];
        Bufin[4] = ERR_FenDian4_FL[j2][12];
        Bufin[5] = ERR_FenDian4_FL[j2][13];
        Bufin[6] = ERR_FenDian4_FL[j2][14];
        Bufin[7] = ERR_FenDian4_FL[j2][15];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian4_FL[j2][16];
        Bufin[1] = ERR_FenDian4_FL[j2][17];
        Bufin[2] = ERR_FenDian4_FL[j2][18];
        Bufin[3] = ERR_FenDian4_FL[j2][19];
        Bufin[4] = ERR_FenDian4_FL[j2][20];
        Bufin[5] = ERR_FenDian4_FL[j2][21];
        Bufin[6] = ERR_FenDian4_FL[j2][22];
        Bufin[7] = ERR_FenDian4_FL[j2][23];
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        Bufin[0] = ERR_FenDian4_FL[j2][24];
        Bufin[1] = ERR_FenDian4_FL[j2][25];
        Bufin[2] = ERR_FenDian4_FL[j2][26];
        Bufin[3] = ERR_FenDian4_FL[j2][27];
        Bufin[4] = ERR_FenDian4_FL[j2][28];
        Bufin[5] = ERR_FenDian4_FL[j2][29];
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
    }

    for(j2=0;j2<8;j2++)   //��Ե����û�����ã���״̬��һ��
    {
        gIEC61850_Data[0x0E00+64+j2] = 0;
    }
    for(j2=0;j2<25;j2++)
    {
        j6 = ERR_FenDian1_Isu[j2][1]&0x7F;
        if((ERR_FenDian1_Isu[j2][1]&0x80) ==0x00)
        {
#ifdef JY_DONEWITH_SPE
            j6 = j6 + DC_JYHM_NUM[ERR_FenDian1_Isu[j2][0]-1];
#endif
        }
        if(ERR_FenDian1_Isu[j2][0] != 0 && j6 != 0)
        {
            if((j6%8)==0)
            {
                j5 = ERR_FenDian1_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j3 = j3-1;
                j4 = 8;
                gIEC61850_Data[0x0E00+64+j5+j3] |= (1<<(j4-1));
            }
            else
            {
                j5 = ERR_FenDian1_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j4 = j6%8;
                gIEC61850_Data[0x0E00+64+j5+j3] |= (1<<(j4-1));
            }
        }
    }
    for(j2=0;j2<8;j2++)   //��Ե����û�����ã���״̬��һ��
    {
        gIEC61850_Data[0x0E00+72+j2] = 0;
    }
    for(j2=0;j2<25;j2++)
    {
        j6 = ERR_FenDian2_Isu[j2][1]&0x7F;
        if((ERR_FenDian2_Isu[j2][1]&0x80) ==0x00)
        {
#ifdef JY_DONEWITH_SPE
            j6 = j6 + DC_JYHM_NUM[ERR_FenDian2_Isu[j2][0]-1];
#endif
        }
        if(ERR_FenDian2_Isu[j2][0] != 0 && j6 != 0)
        {
            if((j6%8)==0)
            {
                j5 = ERR_FenDian2_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j3 = j3-1;
                j4 = 8;
                gIEC61850_Data[0x0E00+72+j5+j3] |= (1<<(j4-1));
            }
            else
            {
                j5 = ERR_FenDian2_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j4 = j6%8;
                gIEC61850_Data[0x0E00+72+j5+j3] |= (1<<(j4-1));
            }
        }
    }

    for(j2=0;j2<8;j2++)   //��Ե����û�����ã���״̬��һ��
    {
        gIEC61850_Data[0x0E00+80+j2] = 0;
    }
    for(j2=0;j2<25;j2++)
    {
        j6 = ERR_FenDian3_Isu[j2][1]&0x7F;
        if((ERR_FenDian3_Isu[j2][1]&0x80) ==0x00)
        {
#ifdef JY_DONEWITH_SPE
            j6 = j6 + DC_JYHM_NUM[ERR_FenDian3_Isu[j2][0]-1];
#endif
        }
        if(ERR_FenDian3_Isu[j2][0] != 0 && j6 != 0)
        {
            if((j6%8)==0)
            {
                j5 = ERR_FenDian3_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j3 = j3-1;
                j4 = 8;
                gIEC61850_Data[0x0E00+80+j5+j3] |= (1<<(j4-1));
            }
            else
            {
                j5 = ERR_FenDian3_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j4 = j6%8;
                gIEC61850_Data[0x0E00+80+j5+j3] |= (1<<(j4-1));
            }
        }
    }
    for(j2=0;j2<8;j2++)   //��Ե����û�����ã���״̬��һ��
    {
        gIEC61850_Data[0x0E00+88+j2] = 0;
    }
    for(j2=0;j2<25;j2++)
    {
        j6 = ERR_FenDian4_Isu[j2][1]&0x7F;
        if((ERR_FenDian4_Isu[j2][1]&0x80) ==0x00)
        {
#ifdef JY_DONEWITH_SPE
            j6 = j6 + DC_JYHM_NUM[ERR_FenDian4_Isu[j2][0]-1];
#endif
        }
        if(ERR_FenDian4_Isu[j2][0] != 0 && j6 != 0)
        {
            if((j6%8)==0)
            {
                j5 = ERR_FenDian4_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j3 = j3-1;
                j4 = 8;
                gIEC61850_Data[0x0E00+88+j5+j3] |= (1<<(j4-1));
            }
            else
            {
                j5 = ERR_FenDian4_Isu[j2][0]-1;
                j5 = j5*JYBYTES;
                j3 = j6/8;
                j4 = j6%8;
                gIEC61850_Data[0x0E00+88+j5+j3] |= (1<<(j4-1));
            }
        }
    }
    */
#endif 
}

INT8U ChgData_8BytesToByte(INT8U *bufin)
{
    INT8U DataOut[9];
    INT8U j;
    DataOut[8] = 0;
    for(j = 0;j < 8;j++)
    {
        DataOut[j] = bufin[j] & 0x01;
        DataOut[j] = DataOut[j]<<j;
        DataOut[8] += DataOut[j];
    }
    return DataOut[8];
}


/*******************************************************************************
 ** ��������:void creat_LinkList(void)
 ** ��������:����һ���µ�����
 ** ��ʽ����:��
 ** �в�˵��:��
 ** ���ز���:��
 ********************************************************************************/
void Creat_LinkList()
{
    printf("Creat_LinkList.......\n");
    pHead = (NowFaultStruct *)malloc(sizeof(NowFaultStruct));
    pHead->next = NULL;

    His_Head = (NowFaultStruct *)malloc(sizeof(NowFaultStruct));
    His_Head->next = NULL;
}

/*******************************************************************************
 ** ��������:void Add_LinkList(unsigned char type, unsigned char group, unsigned char element)
 ** ��������:ͷ�巨��������Ԫ��
 ** ��ʽ����:unsigned char type, unsigned char group, unsigned char element, unsigned int num
 ** �в�˵��:      ��������   		 ��������       	����Ԫ���������е�λ��   ���ϸ�����Ϣ
 ** ���ز���:��
 ********************************************************************************/
void Add_LinkList(unsigned int type, unsigned int group, unsigned int element ,unsigned int num)  //���ϣ�����һ�ζ��Σ�����Ĺ��ϣ����鼸·
{
    NowFaultStruct *p;
    NowFaultStruct *his;
    NowFaultStruct *qq;
    char* str = get_time();
    char* str_recover = "Waiting....";
    p = (NowFaultStruct *)malloc(sizeof(NowFaultStruct));

    p->FaultType = type;
    p->FaultGroup = group;
    p->FaultElement = element;
    p->num = num;
    strcpy(p->start_time , str);

    p->next = pHead->next;
    pHead->next = p;
    Fault_appear = 1;

    his = (NowFaultStruct *)malloc(sizeof(NowFaultStruct));
    his->FaultType = type;
    his->FaultGroup = group;
    his->FaultElement = element;
    his->num = num;
    strcpy(his->start_time , str);
    strcpy(his->recover_time , str_recover);
    his->next = His_Head->next;
    His_Head->next = his;
    His_Fault_save(his);
    if(His_Fault_dis_Num <300){
        His_Fault_dis_Num++;
    }else{
        qq = his->next;
        while(qq->next != NULL){
            his = qq;
            qq = qq->next;
        }
        his->next = NULL;
        free(qq);
        His_Fault_dis_Num = 300;
    }

    switch(type){
        case AC_ERR: 
            if (Light_AC < 500)
            {
                Light_AC ++;    
            }
            break;
        case DC_ERR:
            if (Light_DC < 500)
            {
                Light_DC ++;
            }
            break;
        case Comm_ERR:
            if (Light_Comm < 500)
            {
                Light_Comm ++;    
            }
            break;
        case UPS_ERR:
            if (Light_UPS < 500)
            {
                Light_UPS ++;
            }
            break;
        case Main_ERR:
            if (Light_Main < 500)
            {
                Light_Main ++;
            }
            break;
        default:
            break;
    }
    printf("Add_LinkList ::	 %d,%d,%d,%d\n",type,group,element,num);
}


/*******************************************************************************
 ** ��������:void Delete_LinkList(unsigned char type, unsigned char element1, unsigned char element2)
 ** ��������:���ݶ�Ӧ��ֵ�ҵ���Ӧֵ�Ľڵ㲢ɾ���ýڵ�
 ** ��ʽ����:unsigned char type, unsigned char element
 ** �в�˵��:      ��������    ����Ԫ��
 ** ���ز���:��
 ** ʹ��˵��:��
 ********************************************************************************/
void Delete_LinkList(unsigned int type, unsigned int group, unsigned int element,unsigned int num)
{
    NowFaultStruct *p = pHead;
    NowFaultStruct *q = pHead->next;

    NowFaultStruct *pp = His_Head;
    NowFaultStruct *qq = His_Head->next;
    //printf("Delete_LinkList\n");
    while (((q->FaultType != type) || (q->FaultElement != element) || (q->FaultGroup != group) || (q->num != num))
            && (q != NULL)&&(q->next != NULL))
    {
        p = q;
        q = q->next;
    }
    if ((q->FaultType == type) && (q->FaultElement == element) && (q->FaultGroup == group)&&(q->num == num))
    {
        char* str = get_time();
        strcpy(q->recover_time , str);

        p->next = q->next;
        q->next = NULL;
        free(q);

        while (((qq->FaultType != type) || (qq->FaultElement != element) || (qq->FaultGroup != group) || (qq->num != num))
                && (qq != NULL)&&(qq->next != NULL)){
            pp = qq;
            qq = qq->next;
        }
        if ((qq->FaultType == type) && (qq->FaultElement == element) && (qq->FaultGroup == group)&&(qq->num == num))
        {
            printf("add hisfault time\n");
            strcpy(qq->recover_time , str);
            modify_xml("hisfault.xml",qq);
        }

        switch(type){
            case AC_ERR: 
                if (Light_AC > 0)
                {
                    Light_AC--;    
                }
                break;
            case DC_ERR:
                if (Light_DC > 0)
                {
                    Light_DC--;
                }
                break;
            case Comm_ERR:
                if (Light_Comm > 0)
                {
                    Light_Comm--;    
                }
                break;
            case UPS_ERR:
                if (Light_UPS > 0)
                {
                    Light_UPS--;
                }
                break;
            case Main_ERR:
                if (Light_Main > 0)
                {
                    Light_Main--;
                }
                break;
            default:
                break;
        }
        printf("Delete_LinkList ::	 %d,%d,%d,%d\n",type,group,element,num);
    }
}

/*******************************************************************************
 ** ��������:void clear_LinkList(void)
 ** ��������:��������г�ͷ�ڵ������еĽڵ�
 ** ��ʽ����:��
 ** �в�˵��:��
 ** ���ز���:��
 ** ʹ��˵��:��
 ********************************************************************************/
void Clear_LinkList(NowFaultStruct *head)
{
    NowFaultStruct *p = head->next;
    while (p != NULL)
    {
        head->next = p->next;
        free(p);
        p = head->next;
    }
    //	free(head);
}

//ͨ��������MODBUS��Լ���̨ͨѶ
void SendData_RS232ToSever(INT16U SlvAddr,INT8U Func,INT16U StaAddr,INT16U Length)
{
    INT16U j = 0;
    INT16U len;
    INT16U CrcValue;

#if 0
    if(SlvAddr == 0x01)
#else  //����̨ͨѶ��RS485��ַ�ӹ̶���Ϊ������裬������Ч 
    if (SlvAddr == Sys_cfg_info.sys_set.comm_addr)
#endif 
    {
        if(Func == 0x03)
        {
#if 0  //�޸�����ң�����ݲ��ܱ���̨RS485��ȡ������.
            if(StaAddr < 0x0800)
            {
#endif 
                len = Length*2;
                Uart3Buf[j++] = SlvAddr;
                Uart3Buf[j++] = Func;
                Uart3Buf[j++] = len;
                while(len--)
                {
                    Uart3Buf[j++] = gIEC61850_Data[StaAddr++];
                }
                CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
                Uart3Buf[j++] = CrcValue;
                Uart3Buf[j++] = CrcValue>>8;
                Uart3BufCnt = j;

#if 0  //�޸�����ң�����ݲ��ܱ���̨RS485��ȡ������.
            }
            else if(StaAddr >= 0x0800 && StaAddr < 0x0B00)
            {
                len = Length*2;
                Uart3Buf[j++] = SlvAddr;
                Uart3Buf[j++] = Func;
                Uart3Buf[j++] = len;
                while(len--)
                {
                    Uart3Buf[j++] = gIEC61850_Data[StaAddr++];
                }
                CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
                Uart3Buf[j++] = CrcValue;
                Uart3Buf[j++] = CrcValue>>8;
                Uart3BufCnt = j;
            }
#endif 
        }
        else if(Func == 0x04)
        {
            len = Length*2;
            Uart3Buf[j++] = SlvAddr;
            Uart3Buf[j++] = Func;
            Uart3Buf[j++] = len;
            while(len--)
            {
                Uart3Buf[j++] = gIEC61850_Data[StaAddr++];
            }
            CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
            Uart3Buf[j++] = CrcValue;
            Uart3Buf[j++] = CrcValue>>8;
            Uart3BufCnt = j;
        }
        else if(Func == 0x06)
        {
            if(StaAddr == 0x0002)
            {
                Sys_cfg_info.dc_set[0].control_busbar_V = Length;   //�趨��ĸ��ѹ
                Uart3Buf[j++] = SlvAddr;
                Uart3Buf[j++] = Func;
                Uart3Buf[j++] = 2;
                Uart3Buf[j++] = Length>>8;
                Uart3Buf[j++] = Length;
                CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
                Uart3Buf[j++] = CrcValue;
                Uart3Buf[j++] = CrcValue>>8;
                Uart3BufCnt = j;
            }
            else if(StaAddr == 0x0004)
            {
                Sys_cfg_info.dc_set[0].EqualCharge_V = Length;   //�趨��ĸ��ѹ
                Uart3Buf[j++] = SlvAddr;
                Uart3Buf[j++] = Func;
                Uart3Buf[j++] = 2;
                Uart3Buf[j++] = Length>>8;
                Uart3Buf[j++] = Length;
                CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
                Uart3Buf[j++] = CrcValue;
                Uart3Buf[j++] = CrcValue>>8;
                Uart3BufCnt = j;
            }
            else if(StaAddr == 0x0006)
            {
                Sys_cfg_info.dc_set[0].FloatCharge_V = Length;   //�趨��ĸ��ѹ
                Uart3Buf[j++] = SlvAddr;
                Uart3Buf[j++] = Func;
                Uart3Buf[j++] = 2;
                Uart3Buf[j++] = Length>>8;
                Uart3Buf[j++] = Length;
                CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
                Uart3Buf[j++] = CrcValue;
                Uart3Buf[j++] = CrcValue>>8;
                Uart3BufCnt = j;
            }
        }
        else if(Func == 0x0F)
        {
            /*
               if(StaAddr == 0x0101) Sys_ctl_info.battery_mode_ctl[0] = Uart3Buf[7];
               else if(StaAddr == 0x0102) Sys_ctl_info.battery_mode_ctl[0] = Uart3Buf[7];
               else if(StaAddr == 0x0103) bSysOpt_ModuleSet_MD1Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0104) bSysOpt_ModuleSet_MD2Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0105) bSysOpt_ModuleSet_MD3Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0106) bSysOpt_ModuleSet_MD4Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0107) bSysOpt_ModuleSet_MD5Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0108) bSysOpt_ModuleSet_MD6Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x0109) bSysOpt_ModuleSet_MD7Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x010A) bSysOpt_ModuleSet_MD8Sel_Yes_No[0]= Uart3Buf[7];
               else if(StaAddr == 0x010B) bSysOpt_ModuleSet_MD1Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x010C) bSysOpt_ModuleSet_MD2Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x010D) bSysOpt_ModuleSet_MD3Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x010E) bSysOpt_ModuleSet_MD4Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x010F) bSysOpt_ModuleSet_MD5Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x0110) bSysOpt_ModuleSet_MD6Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x0111) bSysOpt_ModuleSet_MD7Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x0112) bSysOpt_ModuleSet_MD8Sel_Yes_No[1]= Uart3Buf[7];
               else if(StaAddr == 0x0113) bSysOpt_ACSet_DCao1_HD_AT0        = Uart3Buf[7];
               else if(StaAddr == 0x0114) {bSysOpt_ACSet_DCao1_HD_AT0 = 1;bSysOpt_ACSet_DCao1_H_F = Uart3Buf[7];bSysOpt_ACSet_DCao1_H_F =(~bSysOpt_ACSet_DCao1_H_F)&0x0001;}
               else if(StaAddr == 0x0115) bSysOpt_ACSet_DCao2_HD_AT0        = Uart3Buf[7];
               else if(StaAddr == 0x0116) {bSysOpt_ACSet_DCao2_HD_AT0 = 1;bSysOpt_ACSet_DCao2_H_F = Uart3Buf[7];bSysOpt_ACSet_DCao2_H_F =(~bSysOpt_ACSet_DCao2_H_F)&0x0001;}
               else if(StaAddr == 0x0117) bSysOpt_ACSet_DCao3_HD_AT0        = Uart3Buf[7];
               else if(StaAddr == 0x0118) {bSysOpt_ACSet_DCao3_HD_AT0 = 1;bSysOpt_ACSet_DCao3_H_F = Uart3Buf[7];bSysOpt_ACSet_DCao3_H_F =(~bSysOpt_ACSet_DCao3_H_F)&0x0001;}
               else if(StaAddr == 0x0119) bSysOpt_ACSet_DCao4_HD_AT0        = Uart3Buf[7];
               else if(StaAddr == 0x011A) {bSysOpt_ACSet_DCao4_HD_AT0 = 1;bSysOpt_ACSet_DCao4_H_F = Uart3Buf[7];bSysOpt_ACSet_DCao4_H_F =(~bSysOpt_ACSet_DCao4_H_F)&0x0001;}
               else if(StaAddr == 0x011B)
               {
               if(Uart3Buf[7] == 1)
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 24;
               }
               else
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 20;
               bSysOpt_ACSet_HD1_1_2_F = 0x55;
               }
               }
               else if(StaAddr == 0x011C)
               {
               if(Uart3Buf[7] == 1)
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 21;
               }
               }
               else if(StaAddr == 0x011D)
               {
               if(Uart3Buf[7] == 1)
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 23;
               }
               }
               else if(StaAddr == 0x011E)
               {
               if(Uart3Buf[7] == 1)
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 22;
               }
               }
               else if(StaAddr == 0x011F)
               {
               if(Uart3Buf[7] == 1)
               {
               gCmdAddrEnable = 1;
               gCmdAddr = 29;
        }
               else
               {
                   gCmdAddrEnable = 1;
                   gCmdAddr = 25;
                   bSysOpt_ACSet_HD2_1_2_F = 0x55;
               }
        }
               else if(StaAddr == 0x0120)
               {
                   if(Uart3Buf[7] == 1)
                   {
                       gCmdAddrEnable = 1;
                       gCmdAddr = 26;
                   }
               }
               else if(StaAddr == 0x0121)
               {
                   if(Uart3Buf[7] == 1)
                   {
                       gCmdAddrEnable = 1;
                       gCmdAddr = 28;
                   }
               }
               else if(StaAddr == 0x0122)
               {
                   if(Uart3Buf[7] == 1)
                   {
                       gCmdAddrEnable = 1;
                       gCmdAddr = 27;
                   }
               }
               Uart3Buf[j++] = SlvAddr;
               Uart3Buf[j++] = Func;
               Uart3Buf[j++] = StaAddr;
               Uart3Buf[j++] = 0;
               Uart3Buf[j++] = 1;
               CrcValue = Load_crc(j,(INT8U *)&Uart3Buf);
               Uart3Buf[j++] = CrcValue;
               Uart3Buf[j++] = CrcValue>>8;
               Uart3BufCnt = j;
               */
        }
    }
}
