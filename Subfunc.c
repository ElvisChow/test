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

NowFaultStruct *pHead = NULL;	//当前故障链表头
NowFaultStruct *His_Head = NULL;	//历史故障表头

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

//ASCLL码转16进制
INT16U AscToHex(unsigned char aChar)
{
    if((aChar>=0x30)&&(aChar<=0x39))
        aChar -= 0x30;
    else if((aChar>=0x41)&&(aChar<=0x46))//大写字母
        aChar -= 0x37;
    else if((aChar>=0x61)&&(aChar<=0x66))//小写字母
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

/****************绝缘不在这里设置******************/



void MoveData_BackupToSet()
{}
void MoveData_RS2324ToGlobalData(INT8U Addr,INT8U Len)
{}

void MoveData_RS2323ToGlobalData(INT8U Addr,INT8U Len)
{}

//串口接收的数据转存61850缓存（gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]）
void MoveData_RS232ToServer()
{
    INT16U j,j2,j1;  //,j5,j3,j4,j6;
    INT16U k,m;
    INT8U Bufin[8];
    INT16U i,group,num_st;

    // <!-- ******** 遥测量(地址: 0x0000): 对应配置文件config_mx_dc.cfg ******** -->
    //第1组直流充电监控数据
    j = 0x0000;
    group = 0;
    for(i=0;i<2;i++){									//0 :一路        1:二路
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;  //交流一路AB线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;	 //交流一路BC线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;	 //交流一路CA线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;           //输入电流
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;        //电池电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;     
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;        //充电机电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8; //控母电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;        //电池电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8; //控母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;	 //合母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		 //充电机电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		 //温度
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;	 //控母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8; //合母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8; //合母负对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //新增交流串入电压、正母线对地电阻、负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //交流串入电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //正母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    //第2组直流充电监控数据
    j = 0x1000;
    group = 1;
    for(i=0;i<2;i++){									//0 :一路        1:二路
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;  //交流一路AB线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;	 //交流一路BC线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;	 //交流一路CA线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;           //输入电流
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;		 //电池电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;		 //充电机电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8; //控母电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;		 //电池电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8; //控母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;	 //合母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		 //充电机电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		 //温度
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;	 //控母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8; //合母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8; //合母负对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //新增交流串入电压、正母线对地电阻、负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //交流串入电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //正母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    //第3组直流充电监控数据
    j = 0x2000;	//第三组充电机遥测
    group = 2;
    for(i=0;i<2;i++){									//0 :一路        1:二路
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i]>>8;    	//交流一路AB线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_A[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i]>>8;		//交流一路BC线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_B[i];
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i]>>8;		//交流一路CA线电压
        gIEC61850_Data[j++] = Dc_info[group].input.voltage_C[i];
    }
    gIEC61850_Data[j++] = Dc_info[group].input.current>>8;     //输入电流
    gIEC61850_Data[j++] = Dc_info[group].input.current;
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v>>8;		//电池电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v>>8;		//充电机电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v>>8;		//控母电压
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_v;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i>>8;		//电池电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i>>8;		//控母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.control_busbar_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;		//合母电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.switching_busbar_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i>>8;		//充电机电流
    gIEC61850_Data[j++] = Dc_info[group].busbar.charger_i;        //
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t>>8;		//温度
    gIEC61850_Data[j++] = Dc_info[group].busbar.battery_t;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v>>8;		//控母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.control_bus_v;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1>>8;		//合母正对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v1;        //
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2>>8;		//合母负对地电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.switching_bus_v2;

#if 1  //新增交流串入电压、正母线对地电阻、负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol>>8;	 //交流串入电压
    gIEC61850_Data[j++] = Dc_info[group].insulate.ACInVol;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes>>8; //正母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.PlusBusGroudRes;        
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes>>8; //负母线对地电阻
    gIEC61850_Data[j++] = Dc_info[group].insulate.MinusBusGroudRes;
#endif 

    j = 0x0100;												//电池信息
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

    j = 0x0200;												//交流屏数据
#if 0
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[0]>>8;			// 1路进线相电压
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[0]>>8;			// 2路进线相电压
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[1].Voltage[2];

    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[0]>>8;			// 1路母线相电压
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[0]>>8;			// 1路母线相电流
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[0];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[1];
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[0].Current[2];

    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ActivePower[3]+Ac_info.ac_in_data[1].ActivePower[3])>>8;		//有功功率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ActivePower[3]+Ac_info.ac_in_data[1].ActivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ReactivePower[3]+Ac_info.ac_in_data[1].ReactivePower[3])>>8;		//无功功率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ReactivePower[3]+Ac_info.ac_in_data[1].ReactivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[0].ApparentPower[3]+Ac_info.ac_in_data[1].ApparentPower[3])>>8;	//视在公率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].ApparentPower[3]+Ac_info.ac_in_data[1].ApparentPower[3];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].PowerFactor[3]>>8;									//功率因数
    gIEC61850_Data[j++] = Ac_info.ac_in_data[0].PowerFactor[3];

    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[0]>>8;			// 3路进线相电压
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[0]>>8;			// 4路进线相电压
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.ac_in_data[3].Voltage[2];

    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[0]>>8;			// 2路母线相电压
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[0];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[1];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Voltage[2];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[0]>>8;			// 2路母线相电流
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[0];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[1]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[1];
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[2]>>8;
    gIEC61850_Data[j++] = Ac_info.busbar[1].Current[2];

    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ActivePower[3]+Ac_info.ac_in_data[3].ActivePower[3])>>8;		//有功功率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ActivePower[3]+Ac_info.ac_in_data[3].ActivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ReactivePower[3]+Ac_info.ac_in_data[3].ReactivePower[3])>>8;		//无功功率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ReactivePower[3]+Ac_info.ac_in_data[3].ReactivePower[3];
    gIEC61850_Data[j++] = (Ac_info.ac_in_data[2].ApparentPower[3]+Ac_info.ac_in_data[3].ApparentPower[3])>>8;	//视在公率
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].ApparentPower[3]+Ac_info.ac_in_data[3].ApparentPower[3];
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].PowerFactor[3]>>8;									//功率因数
    gIEC61850_Data[j++] = Ac_info.ac_in_data[2].PowerFactor[3];
#else //新增四路进线有功无功功率等数据传后台.
    //4路交流进线电压、电流、有功无功功率、功率因数
    for (i = 0; i < 4; i++)  
    {
        //交流进线电压 
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[0] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[0];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[1];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Voltage[2];

        //交流进线电流
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[0] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[0];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[1];
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].Current[2];

        //交流进线有功功率
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ActivePower[3] >> 8;		
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ActivePower[3];

        //交流进线无功功率
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ReactivePower[3] >> 8;	
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ReactivePower[3];

        //交流进线视在公率
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ApparentPower[3] >> 8;	
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].ApparentPower[3];

        //交流进线功率因数
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].PowerFactor[3] >> 8;									
        gIEC61850_Data[j++] = Ac_info.ac_in_data[i].PowerFactor[3];
    }

    //2路交流母线电压、电流
    for (i = 0; i < 2; i++)  
    {
        //交流母线ABC相电压
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[0] >> 8;			
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[0];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[1];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Voltage[2];

        //交流母线ABC相电流
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[0] >> 8;			
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[0];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[1] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[1];
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[2] >> 8;
        gIEC61850_Data[j++] = Ac_info.busbar[i].Current[2];
    }
#endif 	

    j = 0x0300;												//UPS子监控数据
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
#else  //新增: UPS1市电输入电流 + UPS2市电输入电流
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

    j = 0x0400;												//通信子监控信息
    group = 0;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_v;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_v;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.input_i;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i>>8;
    gIEC61850_Data[j++] = Comm_info[group].module.output_i;
    for(j2 = 0;j2<16;j2++)  //scl_srvr送数据时,只送了8组
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
    for(j2 = 0;j2<16;j2++) //scl_srvr送数据时,只送了8组
    {
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_v[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_i[j2];
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2]>>8;
        gIEC61850_Data[j++] = Comm_info[group].module.module_output_t[j2];
    }


    j = 0x0500;												//充电模块信息
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

    j = 0x0600;												//通信电源电池信息
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
    //0x0800(高): <!--  直流电源1交流一路故障(1号直流系统第一组交流输入故障)--> 
    //遥信量	
    j = 0x0800;				  //电池信息
    if(ERR_DC_AcVoltage_GY[0][0] == 1 || ERR_DC_AcVoltage_QY[0][0] == 1 
            || ERR_DC_Ac_PowerCut[0][0] == 1 || ERR_DC_Ac_PhaseLoss[0][0] == 1)  
    {
        Bufin[0] = 0x01;      //交流一路故障
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[0][1] == 1 || ERR_DC_AcVoltage_QY[0][1] == 1 
            || ERR_DC_Ac_PowerCut[0][1] == 1 || ERR_DC_Ac_PhaseLoss[0][1] == 1)  
    {
        Bufin[1] = 0x01;       					//交流二路故障
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[0][0])&0x01;   //交流一路工作
    Bufin[3] = (~ERR_DC_Ac1_state[0][1])&0x01;	 //交流二路工作
    Bufin[4] = ERR_DC_AcSPD[0];                  //交流防雷故障

#if 0
    if(Special_35KV_flag == 2){  //交流开关1~2断开故障
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0)  ){
            Bufin[5] = 0x00;         
        }else{
            Bufin[5] = 0x01;						//交流开关故障
        }
    }else{                       //交流开关1~4断开故障
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0) 
                && (ERR_DC_AcSwitch[0][2] == 0)&& (ERR_DC_AcSwitch[0][3] == 0) ){
            Bufin[5] = 0x00;         
        }else{
            Bufin[5] = 0x01;						//交流开关故障
        }
    }
#else   //新增两种有无通信、UPS电源监控的选择项.
    if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
    {   //交流开关1~2断开故障
        if((ERR_DC_AcSwitch[0][0] == 0) && (ERR_DC_AcSwitch[0][1] == 0))
        {
            Bufin[5] = 0x00;         
        }
        else
        {
            Bufin[5] = 0x01;					
        }
    }
    else  //同时有UPS、通信监控时
    {   //交流开关1~4断开故障
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
    Bufin[6] = ERR_DC_AcSample_comm[0];				//交流监控单元通讯故障
    
    //<!--  直流电源1电池巡检监控通讯断开 -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//独立电池巡检故障
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[0];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0800(高)

    //--------------------------------------------------------------- 
    //0x0800(低): <!--  直流电源2交流一路故障(2号直流系统第一组交流输入故障)-->  
    if(ERR_DC_AcVoltage_GY[1][0] == 1 || ERR_DC_AcVoltage_QY[1][0] == 1 || ERR_DC_Ac_PowerCut[1][0] == 1 || ERR_DC_Ac_PhaseLoss[1][0] == 1)  
    {
        Bufin[0] = 0x01;      //交流一路故障
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[1][1] == 1 || ERR_DC_AcVoltage_QY[1][1] == 1 || ERR_DC_Ac_PowerCut[1][1] == 1 || ERR_DC_Ac_PhaseLoss[1][1] == 1)  
    {
        Bufin[1] = 0x01;       					//交流二路故障
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[1][0])&0x01;   //交流一路工作
    Bufin[3] = (~ERR_DC_Ac1_state[1][1])&0x01;	 //交流二路工作
    Bufin[4] = ERR_DC_AcSPD[1];                  //交流防雷故障
    if((ERR_DC_AcSwitch[1][0] == 0) && (ERR_DC_AcSwitch[1][1] == 0) && (ERR_DC_AcSwitch[1][2] == 0) && (ERR_DC_AcSwitch[1][3] == 0))
    {
        Bufin[5] = 0x00;         
    }
    else
    {
        Bufin[5] = 0x01;						//交流开关故障
    }
    Bufin[6] = ERR_DC_AcSample_comm[1];				//交流监控单元通讯故障
    
    //<!--  直流电源2电池巡检监控通讯断开 -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//独立电池巡检故障
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0800(低)

    //--------------------------------------------------------------- 
    //0x0801(高): <!--  直流电源1充电模块故障(1号直流系统充电机模块故障) --> 
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++)  //从原8路扩到12路充电模块
    {
        Bufin[0] |= ERR_DC_Module[0][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)    //从原8路扩到12路充电模块
    {
        Bufin[1] |= ERR_DC_Module_comm[0][j2];
    }
    if(Sys_cfg_info.dc_set[0].switch_monitor_num>0)
    {
        Bufin[2] = 0x00;
        for(j2 = 0;j2<16;j2++)   //前16路
        {
            for(j1 = 0;j1<32;j1++)
            {
                Bufin[2] |= ERR_DC_SW_trip[0][j2][j1];
            }
        }
        //       Bufin[2] |= ERR_DC_SW_Spe[29];

#if 1   //添加总信号“一段馈线开关跳闸”
        for(j2 = 0;j2<24;j2++)  //后24路
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
#else   //新增两种有无通信、UPS电源监控的选择项. 
        //直流X段馈线开关跳闸
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
    Bufin[3] = ERR_DC_Battery_SW[0][0];  //电池开关故障
#else   //转移位置
    Bufin[3] = 0x00;
#endif 
    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[0][0];
    Bufin[4] |= ERR_DC_BatteryFuse[0][1];
    Bufin[5] = ERR_DC_Battery_SW[0][1];  				//充电机开关跳闸故障
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
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0801(高)

    //--------------------------------------------------------------- 
    //0x0801(低): <!--  直流电源2充电模块故障(2号直流系统充电机模块故障) --> 
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++) //从原8路扩到12路充电模块
    {
        Bufin[0] |= ERR_DC_Module[1][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)  //从原8路扩到12路充电模块
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
        
#if 1   //添加总信号“二段馈线开关跳闸”
        for(j2 = 0;j2<24;j2++)  //后24路
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
#else   //新增两种有无通信、UPS电源监控的选择项.
        if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //该标志位不能包含进去
            //ERR_DC_AcSwitch[1][3]: 无专用的UPS监控和通信电源监控：UPS馈线开关跳闸
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
    Bufin[3] = ERR_DC_Battery_SW[1][0]; //电池开关故障
#else   //转移位置
    Bufin[3] = 0x00;
#endif 

    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[1][0];
    Bufin[4] |= ERR_DC_BatteryFuse[1][1];
    Bufin[5] = ERR_DC_Battery_SW[1][1];  				//充电机开关跳闸故障
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
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0801(低)

    //--------------------------------------------------------------- 
    //0x0802(高): <!--  一段充电机过压 -->   
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
#else //转移位置
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif 

    Bufin[6] = ERR_DC_DcSample_comm[0];
    Bufin[7] = 0;                   //电池巡检单元通讯故障
    Bufin[7] |= ERR_DC_BatteryPolling_comm[0][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[0][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0802(高)

    //--------------------------------------------------------------- 
    //0x0802(低): <!--  二段充电机过压 --> 
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
#else //转移位置
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif 

    Bufin[6] = ERR_DC_DcSample_comm[1];
    Bufin[7] = 0;                   //电池巡检单元通讯故障
    Bufin[7] |= ERR_DC_BatteryPolling_comm[1][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[1][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0802(低)

    //--------------------------------------------------------------- 
    //0x0803(高): <!--  一组单体电池故障(1号直流系统蓄电池异常)--> 

#if 0
    for(j2=0; j2<108; j2++){
        Bufin[0] |= ERR_DC_BatterySingle_GY[0][j2];            							// 1组单体电池故障
        Bufin[0] |= ERR_DC_BatterySingle_QY[0][j2];
    }
#else  //转移位置 
    Bufin[0] = 0x00;
#endif 

#if 0
#if 0
    Bufin[1] = ERR_DC_JY_VF[0];     // 1段压差告警 改为 电池放电
#else  
    Bufin[1] = Dc_info[0].battery.FG_discharging;     //电池放电
#endif
#else  //转移位置
    Bufin[1] = 0x00;
#endif  

    Bufin[2] = ERR_DC_JY_detection[0][0]|ERR_DC_JY_detection[0][1];
    Bufin[3] = 0;   //号绝缘巡检通讯故障
    for(j2=0;j2<16;j2++)
    {
        Bufin[3] |= ERR_DC_JY_Sample_comm[0][j2];  //绝缘子监控通讯故障
    }
    if(Sys_cfg_info.sys_set.insulate_mode == 0)
    {
        Bufin[4] = ERR_MainIsu; //# 独立绝缘子监控通信断开
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
#else  //转移位置
    //<!--  一组单体电池欠压--> 
    for(j2=0; j2<108; j2++){          			
        Bufin[6] |= ERR_DC_BatterySingle_QY[0][j2];
    }

    //<!--  一组单体电池过压-->
    for(j2=0; j2<108; j2++){
        Bufin[7] |= ERR_DC_BatterySingle_GY[0][j2];            				
    }
#endif 
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0803(高)

    //--------------------------------------------------------------- 
    //0x0803(低): <!--  二组单体电池故障(2号直流系统蓄电池异常) -->
#if 0
    for(j2=0; j2<108; j2++){
        Bufin[0] |= ERR_DC_BatterySingle_GY[1][j2];            							// 1组单体电池故障
        Bufin[0] |= ERR_DC_BatterySingle_QY[1][j2];
    }
#else  //转移位置
    Bufin[0] = 0x00;
#endif 

#if 0
#if 0
    Bufin[1] = ERR_DC_JY_VF[1];     // 2段压差告警
#else  
    Bufin[1] = Dc_info[1].battery.FG_discharging;     //电池放电
#endif
#else  //转移位置
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
#else  //转移位置
    //<!--  二组单体电池欠压--> 
    for(j2=0; j2<108; j2++){          			
        Bufin[6] |= ERR_DC_BatterySingle_QY[1][j2];
    }

    //<!--  二组单体电池过压-->
    for(j2=0; j2<108; j2++){
        Bufin[7] |= ERR_DC_BatterySingle_GY[1][j2];            				
    }
#endif 

    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0803(低)


    /***********************    AC  0808   *************************************/
    //--------------------------------------------------------------- 
    //0x0804(高): <!--  交流进线断路器一合闸(站用电1号ATS进线1断路器合闸) -->
    Bufin[0] = Ac_info.ac_in_data[0].SW_state;
    Bufin[1] = Ac_info.ac_in_data[1].SW_state;
    Bufin[2] = ERR_AC_in_SW_trip[0];  //交流进线断路器一跳闸
    Bufin[3] = ERR_AC_in_SW_trip[1];  //交流进线断路器二跳闸
    Bufin[4] = ERR_AC_SPD[0];   //防雷器1故障
    Bufin[5] = ERR_AC_in_V[0];
    Bufin[6] = ERR_AC_in_V[1];
    Bufin[7] = ERR_AC_mu_V[0];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0804(高)

    //--------------------------------------------------------------- 
    //0x0804(低): <!--  交流进线断路器三合闸(站用电2号ATS进线1断路器合闸) -->
    Bufin[0] = Ac_info.ac_in_data[2].SW_state;
    Bufin[1] = Ac_info.ac_in_data[3].SW_state;
#if 1   //修复上报给后台的“交流进线断路器三跳闸”、"交流进线断路器四跳闸"信号.
    Bufin[2] = ERR_AC_in_SW_trip[2]; //交流进线断路器三跳闸(母联开关跳闸)
    Bufin[3] = ERR_AC_in_SW_trip[3]; //交流进线断路器四跳闸
#else  //梳理交流进线柜母联开关跳闸信号上传后台
    if (Sys_cfg_info.ac_set.control_mode == 2)  //电操控制
    {
        Bufin[2] = 0x00;
        Bufin[3] = 0x00;        
    }
    else   //其他情况 
    {
        Bufin[2] = ERR_AC_in_SW_trip[2]; //交流进线断路器三跳闸
        Bufin[3] = ERR_AC_in_SW_trip[3]; //交流进线断路器四跳闸    
    }
#endif 
    Bufin[4] = ERR_AC_SPD[1];   //防雷器2故障
    Bufin[5] = ERR_AC_in_V[2];
    Bufin[6] = ERR_AC_in_V[3];
    Bufin[7] = ERR_AC_mu_V[1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0804(低)

    /************************??????????????????????????????????**********************/    
    //--------------------------------------------------------------- 
    //0x0805(高): <!--  交流电源ATSE1一路合闸 -->
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
#else //整理了ATSE相关四个信号上传后台 
    for (i = 0; i < 4; i ++)
        Bufin[i] = 0;
    if (((Sys_cfg_info.ac_set.control_mode == 1)
            || (Sys_cfg_info.ac_set.control_mode == 3)) //ATSE控制  //新增和韩光ATS通讯支持.
            && (Sys_cfg_info.ac_set.ATSE_num >0))  //ATSE个数大于0 
    {
        if(Ac_info.atse_stat[0].sw == 1)
        {
            Bufin[0] = 1;  //交流电源ATSE1一路合闸
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;  //交流电源ATSE1二路合闸
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[0].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;  //交流电源ATSE1开闸
        }
        Bufin[3] = ERR_MainATS; //交流电源ATSE1通讯故障 
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
#else   //交流馈线跳闸分信号或到交流馈线总跳闸
    Bufin[4] = 0x00;
    for(i = 0;i < (Sys_cfg_info.ac_set.state_monitor_num / 2); i++)  //前一半判为一段
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
    Bufin[6] = ERR_DC_Battery_GY[0];　　//一组电池组过压
#else  //转移位置
    Bufin[6] = 0x00;
#endif 
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0805(高)

    //--------------------------------------------------------------- 
    //0x0805(低): <!--  交流电源ATSE2一路合闸 -->
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
#else  //整理了ATSE相关四个信号上传后台 
    for (i = 0; i < 4; i ++)
        Bufin[i] = 0;
    if (((Sys_cfg_info.ac_set.control_mode == 1)
                ||(Sys_cfg_info.ac_set.control_mode == 3))    //ATSE控制  //新增和韩光ATS通讯支持.
            && (Sys_cfg_info.ac_set.ATSE_num > 1)) //ATSE个数大于1 
    {
        if(Ac_info.atse_stat[1].sw == 1)
        {
            Bufin[0] = 1;  //交流电源ATSE2一路合闸
            Bufin[1] = 0;
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 2)
        {
            Bufin[0] = 0;
            Bufin[1] = 1;  //交流电源ATSE2二路合闸
            Bufin[2] = 0;
        }
        else if(Ac_info.atse_stat[1].sw == 3)
        {
            Bufin[0] = 0;
            Bufin[1] = 0;
            Bufin[2] = 1;  //交流电源ATSE2开闸
        }
        Bufin[3] = ERR_MainATS1; //交流电源ATSE2通讯故障 
    }
#endif 

#if 0
    Bufin[4] = ERR_AC_Feeder_Duan_trip_2;
#else   //交流馈线跳闸分信号或到交流馈线总跳闸
    Bufin[4] = 0x00;
    for(i = (Sys_cfg_info.ac_set.state_monitor_num / 2);
            i < Sys_cfg_info.ac_set.state_monitor_num; i++)  //后一半判为二段
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
    Bufin[6] = ERR_DC_Battery_GY[1]; //二组电池组过压
#else  //转移位置
    Bufin[6] = 0x00;
#endif 
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0805(低)

    //--------------------------------------------------------------- 
    //0x0806(高): <!--  交流电源电表异常告警 -->	
    Bufin[0] = 0;    
#if 1  //修复总监控和后台不报电表1、2通讯故障的问题.
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
    Bufin[7] = 0;				 //Bufin[7] = ERR_AC_Thunder;   //母联开关跳闸                  //?????????????
#else  //梳理交流进线柜母联开关跳闸信号上传后台 
    Bufin[7] = 0;
    if (Sys_cfg_info.ac_set.control_mode == 2)  //电操控制
    {
        Bufin[7] = ERR_AC_in_SW_trip[2];  //母联开关跳闸
    }
#endif 
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(高)

    //--------------------------------------------------------------- 
    //0x0806(低): 	<DOI name="Ind1" desc="通信电源1模块通讯故障">
    if (LN_MK_communication == 2)  //(特殊)当用雷能模块时
    {
#if 0
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[0][j2];   //模块通信故障
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
            Bufin[5] |= ERR_LN_Comm_Module[0][j2];   //模块故障
        }
        Bufin[6] = ERR_MainComPw;                //子监控通信故障
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[0][j2]; 
        }											
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(低) 
#else 
        gIEC61850_Data[j++] = 0;        //0x0806(低)
#endif 
    }else if (LN_MK_communication != 2){   //当不用雷能模块时
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
#else   //修复通信电源输入输出异常时误报给后台通信模块故障的问题（最大只支持14个模块）. 
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
            Bufin[5] = ERR_DC_AcSwitch[0][2];  //无专用的UPS监控和通信电源监控：通信电源馈线跳闸
#else   //新增两种有无通信、UPS电源监控的选择项. 
        if ((Special_35KV_flag == 0x02) || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //当无通信电源监控时
            Bufin[5] = ERR_DC_AcSwitch[0][2];  //无专用的UPS监控和通信电源监控：通信电源馈线跳闸
        }  
#endif 
        Bufin[6] = ERR_MainComPw;
        Bufin[7] = 0;          
        Bufin[7] |= ERR_Comm_Module[0][14];  
        Bufin[7] |= ERR_Comm_Module[0][15]; 												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0806(低)
    }

    //--------------------------------------------------------------- 
    //0x0807(高): <!--  UPS1市电输入异常(1号UPS交流输入异常) -->
    /********	080E		// UPS				**************/
    Bufin[0] = ERR_UPS_MainsSupply[0];
    Bufin[1] = ERR_UPS_DC[0];
    Bufin[2] = ERR_UPS_Bypass_output[0];
    Bufin[3] = ERR_UPS[0];
    Bufin[4] = ERR_UPS_Bypass[0];
    Bufin[5] = ERR_UPS_Overload[0];
    Bufin[6] = ERR_UPS_Comm[0];
    Bufin[7] = ERR_UPS_OutPut[0];	 //Bufin[7] = ERR_UPS_SW_Spe[31]; UPS1输出异常
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); 	//0x0807(高)

    //--------------------------------------------------------------- 
    //0x0807(低): <!--  UPS2市电输入异常(2号UPS交流输入异常) -->
    Bufin[0] = ERR_UPS_MainsSupply[1];
    Bufin[1] = ERR_UPS_DC[1];
    Bufin[2] = ERR_UPS_Bypass_output[1];
    Bufin[3] = ERR_UPS[1];
    Bufin[4] = ERR_UPS_Bypass[1];
    Bufin[5] = ERR_UPS_Overload[1];
    Bufin[6] = ERR_UPS_Comm[1];
    Bufin[7] = ERR_UPS_OutPut[1];	//Bufin[7] = ERR_UPS_SW_Spe[31]; UPS2输出异常
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0807(低)

    //--------------------------------------------------------------- 
    //0x0808(高): <!--  UPS交流馈线开关跳闸告警 -->
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
        Bufin[0] = ERR_DC_AcSwitch[0][3];  //无专用的UPS监控和通信电源监控：UPS馈线开关跳闸
#else   //新增两种有无通信、UPS电源监控的选择项. 
    if ((Special_35KV_flag == 0x02) || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02))
    {
        //当无UPS监控时
        Bufin[0] = ERR_DC_AcSwitch[0][3];  //无专用的UPS监控和通信电源监控：UPS馈线开关跳闸
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
    Bufin[4] = ERR_MainUPS;    //逆变电源子监控通讯故障
    Bufin[5] = ERR_UPS_SPD[0]; //UPS防雷器故障
    Bufin[6] = 0;
    Bufin[7] = 0;

    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0808(高)

    //--------------------------------------------------------------- 
    //0x0808(低): 空
#if 0
#if 0
    if (Sys_cfg_info.battery_set.CYH_communi == 1  || LN_MK_communication == 2){
#else //系统设置电池参数从统一设置改成分组设置.
    if (Sys_cfg_info.battery_set[0].CYH_communi == 1  || LN_MK_communication == 2){
#endif 
        for(j2=0; j2<12; j2++){
            Bufin[0] |= ERR_comm_BatterySingle_GY[0][j2];   // 1组单体电池故障
            Bufin[0] |= ERR_comm_BatterySingle_QY[0][j2];         												 
            Bufin[0] |= ERR_comm_BatterySingle_GY[1][j2];
            Bufin[0] |= ERR_comm_BatterySingle_QY[1][j2];
            Bufin[1] |= ERR_comm_BatterySingle_GY[2][j2];   // 2组单体电池故障   
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
            Bufin[2] = ERR_DC_AcSwitch[0][2];  //无专用的UPS监控和通信电源监控：通信电源馈线跳闸
        Bufin[3] = 0;
        Bufin[4] = 0;
        Bufin[5] = 0;
        Bufin[6] = 0;
        Bufin[7] = 0;
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); //0x0808(低)
    }else {
        gIEC61850_Data[j++] = 0;         //空了一个字节		        //0x0808(低)

    }	
#else  //新增雷能模块信号于模型文件中. 
    if (LN_MK_communication == 2)
    {
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[0][j2];   //通信电源1模块通信故障
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[0][j2]; //通信电源1模块输出过压关机故障	
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
            Bufin[5] |= ERR_LN_Comm_Module[0][j2];   //模块故障
        }
        Bufin[6] = ERR_MainComPw;                //子监控通信故障
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[0][j2]; 
        }											
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0808(低)     
    }
    else 
    {
        gIEC61850_Data[j++] = 0;        //0x0808(低)
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
          Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //模块通信故障
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
          Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //模块故障
          }
          Bufin[6] = ERR_MainComPw1;                //子监控通信故障
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


    Bufin[0] = 0;              //分电柜一段母线不平衡
    Bufin[0] |= ERR_FenDian[0][48];
    Bufin[0] |= ERR_FenDian[1][48];
    Bufin[0] |= ERR_FenDian[2][48];
    Bufin[0] |= ERR_FenDian[3][48];
    Bufin[1] = 0;				//分电柜二段母线不平衡
    Bufin[1] |= ERR_FenDian[0][49];
    Bufin[1] |= ERR_FenDian[1][49];
    Bufin[1] |= ERR_FenDian[2][49];
    Bufin[1] |= ERR_FenDian[3][49];
    Bufin[2] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[2] |= ERR_DC_standard_cell_comm[0][j2];    //直流电源1绝缘标准单元通信故障 
    }
    Bufin[3] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[3] |= ERR_DC_standard_cell_comm[1][j2];    //直流电源2绝缘标准单元通信故障
    }
    Bufin[4] = 0;
    for(j2=0;j2<30;j2++)
    {
        Bufin[4] |= ERR_DC_standard_cell_comm[2][j2];    //直流电源3绝缘标准单元通信故障 
    }
    Bufin[5] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[5] |= ERR_DC_FG_SW_trip[0][i][j2];      //1#开关量监控采样值（跳闸）（16路）
            Bufin[5] |= ERR_DC_FG_SW_trip[0][i][j2+8];
        }
    }
    Bufin[6] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[6] |= ERR_DC_FG_SW_trip[1][i][j2];      //2#开关量监控采样值（跳闸）（16路）
            Bufin[6] |= ERR_DC_FG_SW_trip[1][i][j2+8];
        }
    }
    Bufin[7] = 0;
    for(i=0;i<14;i++){
        for(j2=0;j2<8;j2++){
            Bufin[7] |= ERR_DC_FG_SW_trip[2][i][j2];      //3#开关量监控采样值（跳闸）（16路）
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

    /*gIEC61850_Data[j++] = 0;        //空了一个字节(69)		    //0x0818
      gIEC61850_Data[j++] = 0;         //空了一个字节		       //0x0819
      gIEC61850_Data[j++] = 0;         //空了一个字节(70)		       //0x081a
      gIEC61850_Data[j++] = 0;         //空了一个字节		       //0x081b
      gIEC61850_Data[j++] = 0;         //空了一个字节(71)		       //0x081c */
#endif

    //--------------------------------------------------------------- 
    //0x0809(高):  

#if 0
    Bufin[0] = 0;
    Bufin[1] = 0;
    Bufin[2] = 0; 
    Bufin[3] = 0;
    Bufin[4] = 0;
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x0809(高)
#else  //新增雷能模块信号于模型文件中.  
    if (LN_MK_communication == 2)  //(特殊)当用雷能模块时
    {
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //通信电源2模块通信故障
        }
        Bufin[1] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[1] |= ERR_Comm_Module_output_GY[1][j2]; //通信电源2模块输出过压关机故障
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
            Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //模块故障
        }
        Bufin[6] = ERR_MainComPw1;                //子监控通信故障
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[1][j2];   
        }												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	    
    }
    else 
    {
        gIEC61850_Data[j++] = 0x00;	 //0x0809(高)
    }
#endif 

    //--------------------------------------------------------------- 
    //0x0809(低): 空 48
    if(LN_MK_communication == 2)  //(特殊)当用雷能模块时
    {
#if 0
        Bufin[0] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[0] |= ERR_LN_Comm_Module_comm[1][j2];   //模块通信故障
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
            Bufin[5] |= ERR_LN_Comm_Module[1][j2];   //模块故障
        }
        Bufin[6] = ERR_MainComPw1;                //子监控通信故障
        Bufin[7] = 0x00;
        for(j2=0;j2<16;j2++)
        {
            Bufin[7] |= ERR_Comm_Module_bu_advection[1][j2];   
        }												
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0809(低) 
#else 
        gIEC61850_Data[j++] = 0x00;	 //0x0809(低)
#endif 
    }else if (LN_MK_communication != 2){  //当不用雷能模块时
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
#else   //修复通信电源输入输出异常时误报给后台通信模块故障的问题（最大只支持14个模块）.
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
        gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);		//0x0809(低)
    }

    //--------------------------------------------------------------- 
    //0x080A(高): <!--  直流电源3交流一路故障(3号直流系统第一组交流输入故障)-->  
    //直流第3段
    if(ERR_DC_AcVoltage_GY[2][0] == 1 || ERR_DC_AcVoltage_QY[2][0] == 1 
            || ERR_DC_Ac_PowerCut[2][0] == 1 || ERR_DC_Ac_PhaseLoss[2][0] == 1)  
    {
        Bufin[0] = 0x01;      //交流一路故障
    }
    else
    {
        Bufin[0] = 0x00;
    }
    if(ERR_DC_AcVoltage_GY[2][1] == 1 || ERR_DC_AcVoltage_QY[2][1] == 1 
            || ERR_DC_Ac_PowerCut[2][1] == 1 || ERR_DC_Ac_PhaseLoss[2][1] == 1)  
    {
        Bufin[1] = 0x01;       					//交流二路故障
    }
    else
    {
        Bufin[1] = 0x00;
    }
    Bufin[2] = (~ERR_DC_Ac1_state[2][0])&0x01;   //交流一路工作
    Bufin[3] = (~ERR_DC_Ac1_state[2][1])&0x01;	 //交流二路工作
    Bufin[4] = ERR_DC_AcSPD[2];                  //交流防雷故障

    if((ERR_DC_AcSwitch[2][0] == 0) && (ERR_DC_AcSwitch[2][1] == 0) 
            && (ERR_DC_AcSwitch[2][2] == 0)&& (ERR_DC_AcSwitch[2][3] == 0) )
    {
        Bufin[5] = 0x00;         
    }else{
        Bufin[5] = 0x01;						//交流开关故障
    }

    Bufin[6] = ERR_DC_AcSample_comm[2];				//交流监控单元通讯故障
    
    //<!--  直流电源3电池巡检监控通讯断开 -->
    //    Bufin[7] = ERR_DC_SW_Spe[31];				//独立电池巡检故障
    Bufin[7] = ERR_MainDC_PSMX_B_Comm[2];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x080A(高)

    //--------------------------------------------------------------- 
    //0x080A(低): <!--  直流电源3充电模块故障(3号直流系统充电机模块故障) -->
    Bufin[0] = 0x00;
    for(j2 = 0;j2<12;j2++)  //从原8路扩到12路充电模块
    {
        Bufin[0] |= ERR_DC_Module[2][j2];
    }
    Bufin[1] = 0x00;
    for(j2=0;j2<12;j2++)    //从原8路扩到12路充电模块
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

#if 1   //添加总信号“三段馈线开关跳闸”
        for(j2 = 0;j2<24;j2++)  //后24路
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
#else   //新增两种有无通信、UPS电源监控的选择项.
        if ((Special_35KV_flag == 0x02) 
            || (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) 
            || (Special_35KV_flag_NoCommMon_WithUpsMon == 0x02))
        {
            //该标志位不能包含进去
            //ERR_DC_AcSwitch[2][3]: 无专用的UPS监控和通信电源监控：UPS馈线开关跳闸
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
    Bufin[3] = ERR_DC_Battery_SW[2][0]; //电池开关故障
#else   //转移位置
    Bufin[3] = 0x00;
#endif 

    Bufin[4] = 0x00;
    Bufin[4] |= ERR_DC_BatteryFuse[2][0];
    Bufin[4] |= ERR_DC_BatteryFuse[2][1];
    Bufin[5] = ERR_DC_Battery_SW[2][1]; 				//充电机开关跳闸故障
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
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin); //0x080A(低)

    //--------------------------------------------------------------- 
    //0x080B(高): <!--  1号直流系统充电机1#充电模块故障 --> 
    //新增1~3号直流系统充电机1~8#充电模块故障
    //1号直流系统充电机1~8#充电模块故障
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[0][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080B(低)
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

    //0x080C(高): 2号直流系统充电机1~8#充电模块故障
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[1][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080C(低)
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

    //0x080D(高): 3号直流系统充电机1~8#充电模块故障
    for(k = 0; k < 8; k++)
    {
        Bufin[k] = 0x00;
        Bufin[k] |= ERR_DC_Module[2][k];
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

    //0x080D(低)
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
    //0x080E(高): <!--  三组单体电池故障(3号直流系统蓄电池异常)-->
    //<!--  直流电源3子监控通讯故障(3号直流系统监控装置通信中断)--> 
#if 0
    for(j2=0; j2<108; j2++)
    {
        Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];        // 1组单体电池故障
        Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
    }
     Bufin[1] = ERR_DC_JY_VF[2];   // 1段压差告警 改为 电池放电
#else  //转移
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
    //0x080E(低): <!--  交流进线断路器一合闸(站用电1号ATS进线1断路器合闸) -->
    Bufin[0] = 0;
    Bufin[1] = 0;
    Bufin[2] = 0;
    Bufin[3] = 0;
    Bufin[4] = ERR_AC_SPD[2];   //防雷器3故障
    Bufin[5] = 0;
    Bufin[6] = 0;
    Bufin[7] = 0;
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x080F(高): <!--  三段充电机过压 -->   
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
#else //转移位置
    Bufin[4] = 0x00;
    Bufin[5] = 0x00;
#endif    

    Bufin[6] = ERR_DC_DcSample_comm[2];
    Bufin[7] = 0;                   //电池巡检单元通讯故障
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x080F(低): <!--  UPS1市电模式 -->
    gIEC61850_Data[j++] = (UPS_Mode[1] << 4) | UPS_Mode[0];

    //--------------------------------------------------------------- 
    //0x0810(高): 一组电池-1    <!--  一组电池过压 -->  
    Bufin[0] = ERR_DC_Battery_GY[0]; //一组电池过压 
    Bufin[1] = ERR_DC_Battery_QY[0]; //一组电池欠压

    //一组单体电池过压
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[0][i];            				
    }   

    //一组单体电池欠压
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[0][i];
    }

    //一组单体电池故障
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[0][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[0][i];
    }
    
    Bufin[5] = ERR_DC_Battery_SW[0][0];           //一组电池开关故障
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[0];  //一组电池均充
#else  //修复上报后台的电池均充信号处理. 
    Bufin[6] = battery_current_state[0];
#endif 
    Bufin[7] = Dc_info[0].battery.FG_discharging; //一组电池放电
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0810(低): 一组电池-2    
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
    {
        //<!--  一组电池采样盒通讯中断 --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[0][i];
        }
        //<!--  一组单体电池内阻超限 -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[0][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0811(高): 二组电池-1    <!--  二组电池过压 -->  
    Bufin[0] = ERR_DC_Battery_GY[1]; //二组电池过压; 
    Bufin[1] = ERR_DC_Battery_QY[1]; //二组电池欠压
    
    //二组单体电池过压
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[1][i];            				
    }   

    //二组单体电池欠压
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[1][i];
    }
    
    //二组单体电池故障
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[1][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[1][i];
    }

    Bufin[5] = ERR_DC_Battery_SW[1][0];           //二组电池开关故障
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[1];  //二组电池均充
#else  //修复上报后台的电池均充信号处理. 
    Bufin[6] = battery_current_state[1];
#endif 
    Bufin[7] = Dc_info[1].battery.FG_discharging; //二组电池放电
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0811(低): 二组电池-2
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
    {
        //<!--  一组电池采样盒通讯中断 --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[1][i];
        }
        //<!--  一组单体电池内阻超限 -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[1][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0812(高): 三组电池-1
    Bufin[0] = ERR_DC_Battery_GY[2]; //三组电池过压 
    Bufin[1] = ERR_DC_Battery_QY[2]; //三组电池欠压

    //三组单体电池过压
    Bufin[2] = 0x00;
    for(i = 0; i < 108; i ++)
    {
        Bufin[2] |= ERR_DC_BatterySingle_GY[2][i];            				
    }   

    //三组单体电池欠压
    Bufin[3] = 0x00;
    for(i = 0; i < 108; i++)
    {          			
        Bufin[3] |= ERR_DC_BatterySingle_QY[2][i];
    }

    //三组单体电池故障
    Bufin[4] = 0x00;
    for(i = 0; i < 108; i++)
    {
        Bufin[4] |= ERR_DC_BatterySingle_GY[2][i];            						
        Bufin[4] |= ERR_DC_BatterySingle_QY[2][i];
    }
    
    Bufin[5] = ERR_DC_Battery_SW[2][0];           //三组电池开关故障
#if 0
    Bufin[6] = sys_ctl_info.battery_mode_ctl[2];  //三组电池均充
#else    //修复上报后台的电池均充信号处理. 
    Bufin[6] = battery_current_state[2];
#endif 
    Bufin[7] = Dc_info[2].battery.FG_discharging; //三组电池放电
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0812(低): 三组电池-2
    for (i = 0; i < 8; i ++)
    {
        Bufin[i] = 0x00;
    }

    if (Fg_SysSet_BatteryCheckMode == 0) //独立的电池巡检: PSMX-B
    {
        //<!--  一组电池采样盒通讯中断 --> 
        for(i = 0; i < 4; i++)
        {          			
            Bufin[0] |= ERR_DC_BatterySamplingBox_CommError[2][i];
        }
        //<!--  一组单体电池内阻超限 -->  
        for(i = 0; i < 108; i++)
        {          			
            Bufin[1] |= ERR_DC_BatterySingle_ResOver[2][i];
        }    
    }
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	

    //--------------------------------------------------------------- 
    //0x0813(高): 分电柜子监控通讯故障
    Bufin[0] = ERR_DC_FG_comm[0][0];    //直流电源1：1#分电柜子监控通讯故障
    Bufin[1] = ERR_DC_FG_comm[0][1];    //直流电源1：2#分电柜子监控通讯故障
    Bufin[2] = ERR_DC_FG_comm[0][2];	//直流电源1：3#分电柜子监控通讯故障
    Bufin[3] = ERR_DC_FG_comm[0][3];	//直流电源1：4#分电柜子监控通讯故障
    Bufin[4] = ERR_DC_FG_comm[1][0];    //直流电源2：1#分电柜子监控通讯故障
    Bufin[5] = ERR_DC_FG_comm[1][1];    //直流电源2：2#分电柜子监控通讯故障
    Bufin[6] = ERR_DC_FG_comm[1][2];    //直流电源2：3#分电柜子监控通讯故障
    Bufin[7] = ERR_DC_FG_comm[1][3];    //直流电源2：4#分电柜子监控通讯故障
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);     

    //0x0813(低)
    Bufin[0] = ERR_DC_FG_comm[2][0];    //直流电源3：1#分电柜子监控通讯故障
    Bufin[1] = ERR_DC_FG_comm[2][1];    //直流电源3：2#分电柜子监控通讯故障
    Bufin[2] = ERR_DC_FG_comm[2][2];	//直流电源3：3#分电柜子监控通讯故障
    Bufin[3] = ERR_DC_FG_comm[2][3];	//直流电源3：4#分电柜子监控通讯故障
    Bufin[4] = 0;    
    Bufin[5] = 0;    
    Bufin[6] = 0;    
    Bufin[7] = 0;    
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);       

#if 0
    /*	for(j2=0; j2<108; j2++)
        {
        Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];            							// 1组单体电池故障
        Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
        }
        Bufin[1] = ERR_DC_JY_VF[2];           												 // 1段压差告警 改为 电池放电
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
    Bufin[5] = ERR_DC_Battery_SW[2][1]; 				//充电机开关跳闸故障
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
    Bufin[7] = 0;                   //电池巡检单元通讯故障
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][0];
    Bufin[7] |= ERR_DC_BatteryPolling_comm[2][1];
    gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);	//0x081f

    for(j2=0; j2<108; j2++)
    {
    Bufin[0] |= ERR_DC_BatterySingle_GY[2][j2];            							// 1组单体电池故障
    Bufin[0] |= ERR_DC_BatterySingle_QY[2][j2];
    }
    Bufin[1] = ERR_DC_JY_VF[2];           												 // 1段压差告警 改为 电池放电
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
       Bufin[0] = 0;              //分电柜一段母线不平衡
       Bufin[0] |= ERR_FenDian[0][48];
       Bufin[0] |= ERR_FenDian[1][48];
       Bufin[0] |= ERR_FenDian[2][48];
       Bufin[0] |= ERR_FenDian[3][48];
       Bufin[1] = 0;				//分电柜二段母线不平衡
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

    //分电柜的信息因为不一致，每次都特别处理下
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
    Bufin[1] = ERR_MainPWR;    //特殊用于电池巡检仪通信故障
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


    /******************************************************************************************/  //直流馈线	
    //第1组直流馈线    
    j = 0x0900;  //状态量(分合)  最多40* 4 = 160字节
    group = 0;   
    num_st = (Sys_cfg_info.dc_set[0].state_monitor_num > Sys_cfg_info.dc_set[0].switch_monitor_num)
        ?(Sys_cfg_info.dc_set[0].state_monitor_num):(Sys_cfg_info.dc_set[0].switch_monitor_num);

    if (num_st > 40)
    {
        num_st = 40; 
    }

    if(num_st < 17){  //1~16 
        for(i=0;i<num_st;i++){     //每组最多32空开
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2];  //几段几组几路状态
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
        for(i=0;i<16;i++){     //每组最多32空开
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_info[0].feederLine[i].state[j2];  //几段几组几路状态
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[0].FGfeederLine[i].FGstate[j2];  //1段分柜几段几组几路状态
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[0].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x0E00;   //开关量(跳闸)  最多40* 4 = 160字节
    if (num_st < 17){
        for(i=0;i<num_st;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2];    //几段几组几路跳闸(开关量)
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
                Bufin[j2] = ERR_DC_SW_trip[0][i][j2];    //几段几组几路跳闸(开关量)
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

        for(i=0;i<(num_st-16);i++){     //每组16个,根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[0][i][j2];  //1段分柜几段几组几路跳闸
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[0][i][j2+8]; 
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);

            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x0F00;   //接地信息 最多40* 4 = 160字节
    //if (num_st <= 40){
    for(i=0;i<num_st;i++){
        for(j2=0;j2<8;j2++){
            Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];  //几段几组几路绝缘(合母，控母)
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
#if 0  //屏蔽无用代码
    else if ( num_st > 16)
    {
        for(i=0;i<16;i++){
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_SW_K_JY[group][i][j2] | ERR_DC_SW_H_JY[group][i][j2];  //几段几组几路绝缘(合母，控母)
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //1段分柜几段几组几路绝缘
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

    //第2组直流馈线
    j = 0x1900;  //状态量(分合)  最多40* 4 = 160字节
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[1].FGfeederLine[i].FGstate[j2];  //2段分柜几段几组几路状态
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[1].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x1E00;   //开关量(跳闸)  最多40* 4 = 160字节
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[1][i][j2];  //2段分柜几段几组几路跳闸
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[1][i][j2+8]; ;
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x1F00;   //接地信息 最多40* 4 = 160字节
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
#if 0   //屏蔽无用代码
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //2段分柜几段几组几路绝缘
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

    j = 0x2900;  //状态量(分合)  最多40* 4 = 160字节
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = Dc_FG_info[2].FGfeederLine[i].FGstate[j2];  //3段分柜几段几组几路状态
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = Dc_FG_info[2].FGfeederLine[i].FGstate[j2+8];
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x2E00;   //开关量(跳闸)  最多40* 4 = 160字节
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = ERR_DC_FG_SW_trip[2][i][j2];  //3段分柜几段几组几路跳闸
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = ERR_DC_FG_SW_trip[2][i][j2+8]; 
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
            for(j2=0;j2<8;j2++){
                Bufin[j2] = 0;  //留空
            }
            gIEC61850_Data[j++] = ChgData_8BytesToByte((INT8U *)Bufin);
        }
    }

    j = 0x2F00;   //接地信息 最多40* 4 = 160字节
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
#if 0   //屏蔽无用代码
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

        for(i=0;i<(num_st-16);i++){     //每组16个根据协议
            for(j2=0;j2<8;j2++){    
                Bufin[j2] = 0;                              //3段分柜几段几组几路绝缘
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

    //交流馈线
    j = 0x0A00;
    for(i=0;i<Sys_cfg_info.ac_set.state_monitor_num;i++){
        for(j2=0;j2<8;j2++){
            Bufin[j2] = Ac_info.feederLine[i].state[j2];    //几组几路状态
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
            Bufin[j2] = ERR_AC_Feeder_SW_trip[i][j2];   //几组几路跳闸，2组可以理解成2段，1组1段33，2组2段34!
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

    //UPS馈线
    j = 0x0B00;
#if 0
    for(i=0;i<Sys_cfg_info.ups_set.state_monitor_num;i++){  // UPS特殊开关状态在1号状态监控前面2个字节
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

    if(UPS_Special_sw_flag == 2)  //UPS特殊开关
    {
        Bufin[0] = ERR_UPS_DcInput_sw[0];   //UPS特殊开关开关量
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
#else  //暂时只上报UPS特殊开关的状态及跳闸信号给后台(8字节) 
    if(UPS_Special_sw_flag == 2)  //UPS特殊开关状态在
    {
        //前4个字节为特殊开关的状态：分合闸
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

        //后4个字节为特殊开关的跳闸信号        
        Bufin[0] = ERR_UPS_DcInput_sw[0];   //UPS特殊开关开关量
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
    for(i=0;i<Sys_cfg_info.comm_set[0].state_monitor_num;i++){    //一段馈状态
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
    if(Comm_Special_sw_flag == 2)  //现在的模式不会进去的(模块通信特殊开关)
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
        for(i=0;i<Sys_cfg_info.comm_set[0].state_monitor_num;i++){    //一段馈跳闸
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
    for(i=0;i<Sys_cfg_info.comm_set[1].state_monitor_num;i++){    //二段馈状态
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
    if(Comm_Special_sw_flag == 2)   //现在的模式不会进去的
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
        for(i=0;i<Sys_cfg_info.comm_set[1].state_monitor_num;i++){    //二段馈跳闸
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

    for(j2=0;j2<8;j2++)   //绝缘个数没有设置，跟状态量一致
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
    for(j2=0;j2<8;j2++)   //绝缘个数没有设置，跟状态量一致
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

    for(j2=0;j2<8;j2++)   //绝缘个数没有设置，跟状态量一致
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
    for(j2=0;j2<8;j2++)   //绝缘个数没有设置，跟状态量一致
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
 ** 函数名称:void creat_LinkList(void)
 ** 函数功能:建立一个新的链表
 ** 形式参数:无
 ** 行参说明:无
 ** 返回参数:无
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
 ** 函数名称:void Add_LinkList(unsigned char type, unsigned char group, unsigned char element)
 ** 函数功能:头插法增加链表元素
 ** 形式参数:unsigned char type, unsigned char group, unsigned char element, unsigned int num
 ** 行参说明:      故障类型   		 几号屏柜       	故障元素在数组中的位置   故障附加信息
 ** 返回参数:无
 ********************************************************************************/
void Add_LinkList(unsigned int type, unsigned int group, unsigned int element ,unsigned int num)  //故障，区分一段二段，具体的故障，几组几路
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
 ** 函数名称:void Delete_LinkList(unsigned char type, unsigned char element1, unsigned char element2)
 ** 函数功能:根据对应数值找到对应值的节点并删除该节点
 ** 形式参数:unsigned char type, unsigned char element
 ** 行参说明:      故障类型    故障元素
 ** 返回参数:无
 ** 使用说明:无
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
 ** 函数名称:void clear_LinkList(void)
 ** 函数功能:清除链表中除头节点外所有的节点
 ** 形式参数:无
 ** 行参说明:无
 ** 返回参数:无
 ** 使用说明:无
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

//通过串口用MODBUS规约与后台通讯
void SendData_RS232ToSever(INT16U SlvAddr,INT8U Func,INT16U StaAddr,INT16U Length)
{
    INT16U j = 0;
    INT16U len;
    INT16U CrcValue;

#if 0
    if(SlvAddr == 0x01)
#else  //跟后台通讯的RS485地址从固定改为界面可设，立即生效 
    if (SlvAddr == Sys_cfg_info.sys_set.comm_addr)
#endif 
    {
        if(Func == 0x03)
        {
#if 0  //修复部分遥测数据不能被后台RS485读取的问题.
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

#if 0  //修复部分遥测数据不能被后台RS485读取的问题.
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
                Sys_cfg_info.dc_set[0].control_busbar_V = Length;   //设定控母电压
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
                Sys_cfg_info.dc_set[0].EqualCharge_V = Length;   //设定控母电压
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
                Sys_cfg_info.dc_set[0].FloatCharge_V = Length;   //设定控母电压
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
