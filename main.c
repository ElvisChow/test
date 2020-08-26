#include "integration_main.h"
#include "myinputcontex.h"
#include <QApplication>
#include "special.h"
#include "globel_Ex.h"
#include <QTextCodec>
#include "global_Qt.h"
#include <QSplashScreen>
#include <QDesktopWidget>
#include "Ertu_start_61850s.h"
#include "Ertu_start_RS232.h"
#include "Ertu_start_interact.h"
#include "Ertu_start_interact1.h"
#include "Ertu_start_sio.h"
#include "Ertu_start_sio1.h"
#include "Ertu_start_Time.h"
#include "application.h"
#include <QMessageBox>
#include <QSettings>
#include <QWidget>
#include <QTimer>

#include <QtXml/QDomDocument>
#include "version.h"

extern "C"{
#include "global_define.h"
#include "scl_srvr.h"
#include "scl.h"
#include "ertu_thread.h"
#include "glbtypes.h"
#include "Global_Para_Ex.h"
}
extern NowFaultStruct *His_Head;

//参数初始化
void ParaInit()
{
    QString str_time;
    gCmdAddrEnable = 0;
    gCmdAddrEnable1 = 0;
    gCmdAddrEnable11 = 0;

    qDebug()<<"ParaInit....";
   
    //若操作记录文件不存在,创建一个 
    QFile *file;  
    if(!file->exists("operation.xml"))	
        create_xml("operation.xml"); 

    //历史故障记录文件hisfault.xml
    if(!file->exists("hisfault.xml"))
    {
        create_xml("hisfault.xml"); 
    }
    else
    {
        QFile file("hisfault.xml");
        if(!file.open(QIODevice::ReadOnly | QFile::Text))
        {
            qDebug()<<"open for read error....";
        }
        
        QString errorStr;
        int errorLine;
        int errorColum;
        unsigned int id_start = 0;

        QDomDocument doc;
        if(!doc.setContent(&file,false,&errorStr,&errorLine,&errorColum)){
            qDebug()<<"ParaInit :: setcontent error....";
            file.close();
            system("rm hisfault.xml");
            system("rm operation.xml");
            //system("cp copy_fault hisfault.xml");
            system("reboot");
        }
        else
        {
            file.close();
        }

        QDomElement root = doc.documentElement();
        if(root.tagName() != "config")
        {
            qDebug()<<"root.tagname != config...";
        }
        
        QDomNode node = root.firstChild();
        QDomNode node1 = root.lastChild();
        if(node1.toElement().attributeNode("id").value().toInt() > 100)
        {
            id_start = node1.toElement().attributeNode("id").value().toInt() - 100;
        }
        else
        {
            id_start = 0;
        }    
        
        while(!node.isNull())
        {
            if(node.isElement())
            {
                QDomElement element = node.toElement();
                if(element.attributeNode("id").value().toInt() > id_start)
                {
                    QDomNodeList list = element.childNodes();
                    QString type = list.at(0).toElement().text();
                    QString group = list.at(1).toElement().text();
                    QString element = list.at(2).toElement().text();
                    QString num = list.at(3).toElement().text();
                    QString start_time = list.at(4).toElement().text();
                    QString recover_time = list.at(5).toElement().text();
                    NowFaultStruct *p = (NowFaultStruct*)malloc(sizeof(NowFaultStruct));
                    p->FaultType = type.toInt();
                    p->FaultGroup = group.toInt();
                    p->FaultElement = element.toInt();
                    p->num = num.toInt();
                    strcpy(p->start_time,start_time.toStdString().c_str());
                    strcpy(p->recover_time,recover_time.toStdString().c_str());
                    p->next = His_Head->next;
                    His_Head->next = p;
                    His_Fault_dis_Num++;
                }
            }
            node = node.nextSibling();
        }
    }
 
#if 1
    Sys_cfg_info.sys_set.battery_mode = 0x01;  //默认值为"不独立"
#endif 

    //读配置文件--settings.ini 
    QFile f_set("settings.ini");	
    if(f_set.exists() && (f_set.size() > 0))
    {
        QSettings *configIni_R = new QSettings("settings.ini",QSettings::IniFormat); 	
        Sys_cfg_info.sys_set.sound = configIni_R->value("sys/Sys_set_Sound").toInt();
        Sys_cfg_info.sys_set.backlight_time = configIni_R->value("sys/Sys_set_Backlight_time").toInt();
        Sys_cfg_info.sys_set.sampling_box_num = configIni_R->value("sys/Sys_set_Sampling_box_num").toInt();
        Sys_cfg_info.sys_set.sampling_box1_num = configIni_R->value("sys/Sys_set_Sampling_box1_num").toInt();
        current_set_password = configIni_R->value("sys/current_set_password").toString();
        current_ctl_password = configIni_R->value("sys/current_ctl_password").toString();
        Sys_cfg_info.sys_set.insulate_mode = configIni_R->value("sys/Sys_set_JY_set").toInt();
        Sys_cfg_info.sys_set.battery_mode = configIni_R->value("sys/Sys_set_Battery_set").toInt();
        Sys_cfg_info.sys_set.baud = configIni_R->value("sys/Sys_set_Baud_set").toInt();
        Sys_cfg_info.sys_set.comm_addr = configIni_R->value("sys/Sys_set_CommAddr").toInt();
        Sys_cfg_info.sys_set.comm_protocol = configIni_R->value("sys/Sys_set_CommProtocol").toInt();

#if 1   //"系统设置"-"通讯协议"下拉选项改为只能选"MODBUS".
        if (Sys_cfg_info.sys_set.comm_protocol != 0x00)
        {
            Sys_cfg_info.sys_set.comm_protocol = 0x00;
        }
#endif 

        Sys_cfg_info.ac_set.duan_num = configIni_R->value("ac/AC_set_duan_amount").toInt();
        Sys_cfg_info.ac_set.ATSE_num= configIni_R->value("ac/AC_set_ATSE_amount").toInt();
        Sys_cfg_info.ac_set.diancao_num = configIni_R->value("ac/AC_set_diancao_amount").toInt();
        Sys_cfg_info.ac_set.ac_sampling_num = configIni_R->value("ac/AC_set_sample_amount").toInt();
        Sys_cfg_info.ac_set.switch_monitor_num = configIni_R->value("ac/Ac_set_SwitchGroup_amount").toInt();
        Sys_cfg_info.ac_set.current_sampling_num = configIni_R->value("ac/AC_set_I_sample_amount").toInt();
        Sys_cfg_info.ac_set.control_mode = configIni_R->value("ac/AC_set_Control_set").toInt();
        Sys_cfg_info.ac_set.state_monitor_num = configIni_R->value("ac/AC_state_monitor_amount").toInt();
        for(int i=0; i<30;i++)
        {
            QString str = "ac/AC_state_"+QString::number(i,10);
            Sys_cfg_info.ac_set.state_num[i] = configIni_R->value(str).toInt();
        }

#if 1  //定制功能：新增交流进线信息来源（有ATS）下拉框选择.
        Sys_cfg_info.ac_set.source_of_ac_data = configIni_R->value("ac/AC_set_source_of_ac_data").toInt();; 
#endif
#if 0
        //Sys_cfg_info.battery_set.battery_group_num = configIni_R->value("battery/battery_group_num").toInt();
        Sys_cfg_info.battery_set.voltage_precision = configIni_R->value("battery/voltage_precision").toInt();
        Sys_cfg_info.battery_set.charge_mode = configIni_R->value("battery/charge_mode").toInt();
        Sys_cfg_info.battery_set.battery_amount = configIni_R->value("battery/battery_amount").toInt();
        Sys_cfg_info.battery_set.single_capacity = configIni_R->value("battery/single_capacity").toInt();
        Sys_cfg_info.battery_set.polling_num = configIni_R->value("battery/polling_num").toInt();
        Sys_cfg_info.battery_set.voltage_upperLimit = configIni_R->value("battery/voltage_upperLimit").toInt();
        Sys_cfg_info.battery_set.voltage_lowerLimit = configIni_R->value("battery/voltage_lowerLimit").toInt();
        Sys_cfg_info.battery_set.single_upperLimit_v = configIni_R->value("battery/single_upperLimit_v").toInt();
        Sys_cfg_info.battery_set.single_lowerLimit_v = configIni_R->value("battery/single_lowerLimit_v").toInt();
        Sys_cfg_info.battery_set.over_current = configIni_R->value("battery/over_current").toInt();
        Sys_cfg_info.battery_set.CYH_communi = configIni_R->value("battery/CYH_communi").toInt();   //从配置文件读数据
        Sys_cfg_info.battery_set.dropout_voltage_limit = configIni_R->value("battery/dropout_voltage_limit").toInt();
#else   //系统设置电池参数从统一设置改成分组设置. 
        Sys_cfg_info.battery_set[0].voltage_precision = configIni_R->value("battery/voltage_precision").toInt();
        Sys_cfg_info.battery_set[0].charge_mode = configIni_R->value("battery/charge_mode").toInt();
        Sys_cfg_info.battery_set[0].battery_amount = configIni_R->value("battery/battery_amount").toInt();
        Sys_cfg_info.battery_set[0].single_capacity = configIni_R->value("battery/single_capacity").toInt();
        Sys_cfg_info.battery_set[0].polling_num = configIni_R->value("battery/polling_num").toInt();
        Sys_cfg_info.battery_set[0].voltage_upperLimit = configIni_R->value("battery/voltage_upperLimit").toInt();
        Sys_cfg_info.battery_set[0].voltage_lowerLimit = configIni_R->value("battery/voltage_lowerLimit").toInt();
        Sys_cfg_info.battery_set[0].single_upperLimit_v = configIni_R->value("battery/single_upperLimit_v").toInt();
        Sys_cfg_info.battery_set[0].single_lowerLimit_v = configIni_R->value("battery/single_lowerLimit_v").toInt();
        Sys_cfg_info.battery_set[0].over_current = configIni_R->value("battery/over_current").toInt();
        Sys_cfg_info.battery_set[0].CYH_communi = configIni_R->value("battery/CYH_communi").toInt();   //从配置文件读数据
        Sys_cfg_info.battery_set[0].dropout_voltage_limit = configIni_R->value("battery/dropout_voltage_limit").toInt();

        Sys_cfg_info.battery_set[1].voltage_precision = configIni_R->value("battery_2/voltage_precision").toInt();
        Sys_cfg_info.battery_set[1].charge_mode = configIni_R->value("battery_2/charge_mode").toInt();
        Sys_cfg_info.battery_set[1].battery_amount = configIni_R->value("battery_2/battery_amount").toInt();
        Sys_cfg_info.battery_set[1].single_capacity = configIni_R->value("battery_2/single_capacity").toInt();
        Sys_cfg_info.battery_set[1].polling_num = configIni_R->value("battery_2/polling_num").toInt();
        Sys_cfg_info.battery_set[1].voltage_upperLimit = configIni_R->value("battery_2/voltage_upperLimit").toInt();
        Sys_cfg_info.battery_set[1].voltage_lowerLimit = configIni_R->value("battery_2/voltage_lowerLimit").toInt();
        Sys_cfg_info.battery_set[1].single_upperLimit_v = configIni_R->value("battery_2/single_upperLimit_v").toInt();
        Sys_cfg_info.battery_set[1].single_lowerLimit_v = configIni_R->value("battery_2/single_lowerLimit_v").toInt();
        Sys_cfg_info.battery_set[1].over_current = configIni_R->value("battery_2/over_current").toInt();
        Sys_cfg_info.battery_set[1].CYH_communi = configIni_R->value("battery_2/CYH_communi").toInt();   //从配置文件读数据
        Sys_cfg_info.battery_set[1].dropout_voltage_limit = configIni_R->value("battery_2/dropout_voltage_limit").toInt();

        Sys_cfg_info.battery_set[2].voltage_precision = configIni_R->value("battery_3/voltage_precision").toInt();
        Sys_cfg_info.battery_set[2].charge_mode = configIni_R->value("battery_3/charge_mode").toInt();
        Sys_cfg_info.battery_set[2].battery_amount = configIni_R->value("battery_3/battery_amount").toInt();
        Sys_cfg_info.battery_set[2].single_capacity = configIni_R->value("battery_3/single_capacity").toInt();
        Sys_cfg_info.battery_set[2].polling_num = configIni_R->value("battery_3/polling_num").toInt();
        Sys_cfg_info.battery_set[2].voltage_upperLimit = configIni_R->value("battery_3/voltage_upperLimit").toInt();
        Sys_cfg_info.battery_set[2].voltage_lowerLimit = configIni_R->value("battery_3/voltage_lowerLimit").toInt();
        Sys_cfg_info.battery_set[2].single_upperLimit_v = configIni_R->value("battery_3/single_upperLimit_v").toInt();
        Sys_cfg_info.battery_set[2].single_lowerLimit_v = configIni_R->value("battery_3/single_lowerLimit_v").toInt();
        Sys_cfg_info.battery_set[2].over_current = configIni_R->value("battery_3/over_current").toInt();
        Sys_cfg_info.battery_set[2].CYH_communi = configIni_R->value("battery_3/CYH_communi").toInt();   //从配置文件读数据
        Sys_cfg_info.battery_set[2].dropout_voltage_limit = configIni_R->value("battery_3/dropout_voltage_limit").toInt();
#endif          

        Sys_cfg_info.comm_set[0].module_num = configIni_R->value("communication_1/module_num").toInt();
        Sys_cfg_info.comm_set[0].switch_monitor_num = configIni_R->value("communication_1/switch_monitor_num").toInt();
        Sys_cfg_info.comm_set[0].module_output_v = configIni_R->value("communication_1/module_output_v").toInt();
        Sys_cfg_info.comm_set[0].module_GY_value = configIni_R->value("communication_1/module_GY_value").toInt();
        Sys_cfg_info.comm_set[0].module_QY_value = configIni_R->value("communication_1/module_QY_value").toInt();
        Sys_cfg_info.comm_set[0].shuntfactor = configIni_R->value("communication_1/shuntfactor").toInt();
        Sys_cfg_info.comm_set[0].state_monitor_num = configIni_R->value("communication_1/state_monitor_num").toInt();
        Sys_cfg_info.comm_set[0].battery_num = configIni_R->value("communication_1/battery_num").toInt();
        for(int i=0; i<30;i++)
        {
            QString str = "communication_1/comm_state_"+QString::number(i,10);
            Sys_cfg_info.comm_set[0].state_num[i] = configIni_R->value(str).toInt();
        }
        
        Sys_cfg_info.comm_set[1].module_num = configIni_R->value("communication_2/module_num").toInt();
        Sys_cfg_info.comm_set[1].switch_monitor_num = configIni_R->value("communication_2/switch_monitor_num").toInt();
        Sys_cfg_info.comm_set[1].module_output_v = configIni_R->value("communication_2/module_output_v").toInt();
        Sys_cfg_info.comm_set[1].module_GY_value = configIni_R->value("communication_2/module_GY_value").toInt();
        Sys_cfg_info.comm_set[1].module_QY_value = configIni_R->value("communication_2/module_QY_value").toInt();
        Sys_cfg_info.comm_set[1].shuntfactor = configIni_R->value("communication_2/shuntfactor").toInt();
        Sys_cfg_info.comm_set[1].state_monitor_num = configIni_R->value("communication_2/state_monitor_num").toInt();
        Sys_cfg_info.comm_set[1].battery_num = configIni_R->value("communication_2/battery_num").toInt();
        for(int i=0; i<30;i++)
        {
            QString str = "communication_2/comm_state_"+QString::number(i,10);
            Sys_cfg_info.comm_set[1].state_num[i] = configIni_R->value(str).toInt();
        }

        for(int i=0; i<3;i++)
        {
            QString tab = "dc_"+QString::number(i,10);
            configIni_R->beginGroup(tab);
            Sys_cfg_info.dc_set[i].AcPowerSupply_mode = configIni_R->value("AcPowerSupply_mode").toInt();
            Sys_cfg_info.dc_set[i].module_num = configIni_R->value("module_num").toInt();
            Sys_cfg_info.dc_set[i].switch_monitor_num = configIni_R->value("switch_monitor_num").toInt();
            Sys_cfg_info.dc_set[i].busTie_num = configIni_R->value("busTie_num").toInt();
            Sys_cfg_info.dc_set[i].AcInput_GY = configIni_R->value("AcInput_GY").toInt();
            Sys_cfg_info.dc_set[i].AcInput_QY = configIni_R->value("AcInput_QY").toInt();
            Sys_cfg_info.dc_set[i].charger_GY = configIni_R->value("charger_GY").toInt();
            Sys_cfg_info.dc_set[i].charger_QY = configIni_R->value("charger_QY").toInt();
            Sys_cfg_info.dc_set[i].control_busbar_GY = configIni_R->value("control_busbar_GY").toInt();
            Sys_cfg_info.dc_set[i].control_busbar_QY = configIni_R->value("control_busbar_QY").toInt();
            Sys_cfg_info.dc_set[i].EqualCharge_V = configIni_R->value("EqualCharge_V").toInt();
            Sys_cfg_info.dc_set[i].FloatCharge_V = configIni_R->value("FloatCharge_V").toInt();
            Sys_cfg_info.dc_set[i].temperature_compensation = configIni_R->value("temperature_compensation").toInt();
            Sys_cfg_info.dc_set[i].BatteryCharge_I = configIni_R->value("BatteryCharge_I").toInt();
            Sys_cfg_info.dc_set[i].control_busbar_V = configIni_R->value("control_busbar_V").toInt();
            Sys_cfg_info.dc_set[i].EqualCharge_trigger = configIni_R->value("EqualCharge_trigger").toInt();
            Sys_cfg_info.dc_set[i].FloatCharge_trigger = configIni_R->value("FloatCharge_trigger").toInt();
            Sys_cfg_info.dc_set[i].EqualCharge_timer = configIni_R->value("EqualCharge_timer").toInt();
            Sys_cfg_info.dc_set[i].EqualCharge_delay = configIni_R->value("EqualCharge_delay").toInt();
            Sys_cfg_info.dc_set[i].EqualCharge_regule = configIni_R->value("EqualCharge_regule").toInt();
            Sys_cfg_info.dc_set[i].ChargerSplitter_factor = configIni_R->value("ChargerSplitter_factor").toInt();
            Sys_cfg_info.dc_set[i].BatterySplitter_factor = configIni_R->value("BatterySplitter_factor").toInt();
            Sys_cfg_info.dc_set[i].insulate_polling_num = configIni_R->value("insulate_polling_num").toInt();
            Sys_cfg_info.dc_set[i].Insulate_DropoutVoltage_limit = configIni_R->value("Insulate_DropoutVoltage_limit").toInt();
            Sys_cfg_info.dc_set[i].Insulate_GND_R_limit = configIni_R->value("Insulate_GND_R_limit").toInt();
            Sys_cfg_info.dc_set[i].state_monitor_num = configIni_R->value("state_monitor_num").toInt();

#ifdef _ANM03_DC_SWITCH_STATE_NUM_40
            for(int j = 0; j < 40; j++)  //扩到40组
#else
			for(int j=0; j<30;j++)
#endif                         
			{
                QString str = "DC_state_"+QString::number(j,10);
                Sys_cfg_info.dc_set[i].state_K_num[j] = configIni_R->value(str).toInt();
            }
            configIni_R->endGroup();
        }

        configIni_R->beginGroup("jy_0"); 	
        Sys_cfg_info.insulate_set[0].polling_num = configIni_R->value("polling_num").toInt();
        Sys_cfg_info.insulate_set[0].switch_monitor_num = configIni_R->value("switch_monitor_num").toInt();
        Sys_cfg_info.insulate_set[0].state_monitor_num = configIni_R->value("state_monitor_num").toInt();
        Sys_cfg_info.insulate_set[0].dropout_voltage_limit = configIni_R->value("dropout_voltage_limit").toInt();
        Sys_cfg_info.insulate_set[0].GND_R_limit = configIni_R->value("GND_R_limit").toInt();
        for(int j=0; j<30;j++)
        {
            QString str = "insulate_state_"+QString::number(j,10);
            Sys_cfg_info.insulate_set[0].state_num[j] = configIni_R->value(str).toInt();
        }
        configIni_R->endGroup();

        configIni_R->beginGroup("ups"); 	
        Sys_cfg_info.ups_set.ups_num = configIni_R->value("ups_num").toInt();
        Sys_cfg_info.ups_set.switch_monitor_num = configIni_R->value("switch_monitor_num").toInt();
        Sys_cfg_info.ups_set.state_monitor_num = configIni_R->value("state_monitor_num").toInt();
        Sys_cfg_info.ups_set.work_mode = configIni_R->value("work_mode").toInt();
        Sys_cfg_info.ups_set.output_GY_value = configIni_R->value("output_GY_value").toInt();
        Sys_cfg_info.ups_set.output_QY_value = configIni_R->value("output_QY_value").toInt();
        Sys_cfg_info.ups_set.output_GZ_value = configIni_R->value("output_GZ_value").toInt();
        Sys_cfg_info.ups_set.DcInput_GY_value = configIni_R->value("DcInput_GY_value").toInt();
        Sys_cfg_info.ups_set.DcInput_QY_value = configIni_R->value("DcInput_QY_value").toInt();
        Sys_cfg_info.ups_set.AcInput_GY_value = configIni_R->value("AcInput_GY_value").toInt();
        Sys_cfg_info.ups_set.AcInput_QY_value = configIni_R->value("AcInput_QY_value").toInt();
        Sys_cfg_info.ups_set.bypass_Input_GY_value = configIni_R->value("bypass_Input_GY_value").toInt();
        Sys_cfg_info.ups_set.bypass_Input_QY_value = configIni_R->value("bypass_Input_QY_value").toInt();		
        for(int j=0; j<30;j++){
            QString str = "ups_state_"+QString::number(j,10);
            Sys_cfg_info.ups_set.state_num[j] = configIni_R->value(str).toInt();
        }
#if 1 //新增配置"UPS容量(VA)"
        Sys_cfg_info.ups_set.Capacity = configIni_R->value("ups_capacity_value").toInt();
#endif 		
        configIni_R->endGroup();
        
        delete configIni_R;	
    }
    else  //settings.ini不存在或大小等于0
    {
        system("rm settings.ini");
        system("cp copy_set settings.ini");
        system("reboot");
    }
}
int main(int argc, char *argv[])
{
#if 1
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));//设置编码格式，使支持中文显示
    QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
#else  //文本编码统一成UTF-8
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);	
#endif

    QApplication QApp(argc, argv);								//app
    Application bApp(argc,argv);								//设置全局响应窗体
    QApp.setFont(QFont("wenquanyi_160_75.qpf",8,QFont::Bold));	//设置字体(文泉驿字库)
   
    //开机画面 
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/image/m/image/0.png"));	//设置程序初始化界面
    splash->move(0,0);
    splash->show();        //显示开机画面 

    //遮掩主界面启动时刷初始界面前的空白
    QWidget *init_widget;  
    init_widget = new QWidget();
    init_widget->resize(800,600);
    init_widget->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
   
#if 0 
    //打印版本信息
    qDebug()<<"------------------------------------------------";
    qDebug()<<"-----  Version: ANM1030001-V1.2.9(181031) ------";
    qDebug()<<"------------------------------------------------";
#else  //软件版本号显示方式修改--01.     
    qDebug() << "------------------------------------------------";
    qDebug() << "-----  Version:"<<ANM_VERSION<<"------";
    qDebug() << "------------------------------------------------";
#endif 

    //读工程配置
    char sys_flag = 0;
    QFile file_cfg("sys_cfg.ini");
    if(file_cfg.exists())   //当配置文件sys_cfg.ini存在时
    {
        QSettings *configIni_R = new QSettings("sys_cfg.ini",QSettings::IniFormat);
        QString system = configIni_R->value("system").toString();
        
        if(system == "integration")  //当为一体化系统
        {
            qDebug()<<"******** main :: sys_cfg.ini:::222222::::  start" << sys_flag;		
            
            configIni_R->beginGroup("integration"); 	
            Sys_set_DC_duan = configIni_R->value("Sys_set_DC_duan").toInt();
            Sys_set_battery_group_num = configIni_R->value("Sys_set_battery_group_num").toInt();
            //Sys_tu_Path = configIni_R->value("Sys_tu_Path").toString();
            //DC_tu_Path = configIni_R->value("DC_tu_Path").toString();
            Sys_set_AC_cfg = configIni_R->value("Sys_set_AC_cfg").toInt();
            Sys_set_Comm_Num = configIni_R->value("Sys_set_Comm_Num").toInt();
            Sys_set_Ups_Num = configIni_R->value("Sys_set_Ups_Num").toInt();
            Special_35KV_flag = configIni_R->value("Special_35KV_flag").toInt();

#if 1 //新增两种有无通信、UPS电源监控的选择项.
            Special_35KV_flag_NoUpsMon_WithCommMon = configIni_R->value("Special_35KV_flag_NoUpsMon_WithCommMon").toInt();
            Special_35KV_flag_NoCommMon_WithUpsMon = configIni_R->value("Special_35KV_flag_NoCommMon_WithUpsMon").toInt();

            //依优先级排序只能一个被选中
            if (Special_35KV_flag == 0x02)  //如果它被选中,其他两项全不选中
            {
                Special_35KV_flag_NoUpsMon_WithCommMon = 0x00;
                Special_35KV_flag_NoCommMon_WithUpsMon = 0x00;
            }
            else if (Special_35KV_flag_NoUpsMon_WithCommMon == 0x02) //如果它被选中,第三项不选中
            {
                Special_35KV_flag_NoCommMon_WithUpsMon = 0x00; 
            }
#endif
            
            Comm_Special_sw_flag = configIni_R->value("Comm_Special_sw_flag").toInt();
            UPS_Special_sw_flag = configIni_R->value("UPS_Special_sw_flag").toInt();
            J4_GPIO_ALL = configIni_R->value("J4_GPIO_ALL").toInt();
            Battery_12V_flag = configIni_R->value("Battery_12V_flag").toInt();
            Feeder_Line_Name_flag = configIni_R->value("Feeder_Line_Name_flag").toInt();
            Battery_num_108 = configIni_R->value("Battery_num_108").toInt();
            SYS_JY_J3 = configIni_R->value("SYS_JY_J3").toInt();
            LN_MK_communication = configIni_R->value("LN_MK_communication").toInt();
            configIni_R->endGroup();
            
            sys_flag = 1;
        }
        else if(system == "dc")
        {
            sys_flag = 2;
        }
        else
        {
            sys_flag = 0;
        }
        
        delete configIni_R;
    }
    else  //当配置文件sys_cfg.ini不存在时
    {
        sys_flag = 0;
        qDebug()<<"main :: sys_cfg.ini:::::::  start";
        qDebug()<<sys_flag;	
    }

    //开机画面后的一个过渡画面
    init_widget->setStyleSheet("border-image:url(../image/black.png);");
    init_widget->show();
    
    qDebug()<<"main :: sys_cfg.ini:::111::::  start";
    qDebug()<<sys_flag;
    
    //Dialog ver_ctl;   //主界面实例
    Creat_LinkList();	//创建故障链表头
    ParaInit();         //参数初始化

#if 1  //移到此处(在函数ParaInit()后面)
    Dialog ver_ctl;		//主界面实例
#endif

    if(sys_flag)
    {
        if(sys_flag == 1) //当为一体化系统
        {
            SCL_INFO scl_info;			
            IEC61850S_INI(&scl_info);			 //61850服务初始化
            IECC61850_GetDevCfg_INI(&g_Dev_cfg); //设备初始化: 读取9个配置文件(后续可考虑返回出错情况来报警)
            gpio_Init();						 //需要用的GPIO口
            BackLight_pwm_init();				 //背光控制

            //7大子线程
            Ertu_Start_61850s    *ertu_61850 ;
            Ertu_Start_Rs232     *ertu_rs232;
            Ertu_Start_Interact  *ertu_interact;
            Ertu_Start_Interact1 *ertu_interact1;
            Ertu_Start_Sio       *ertu_sio;
            Ertu_Start_Sio1      *ertu_sio1;
            Ertu_Start_Time      *ertu_time;

            ertu_61850 = new Ertu_Start_61850s();       //61850服务线程
            ertu_rs232 = new Ertu_Start_Rs232();  		//后台RS485线程	
            ertu_interact =  new Ertu_Start_Interact(); //ACUPS通信RS485发送线程
            ertu_interact1 = new Ertu_Start_Interact1();//直流RS485发送线程
            ertu_sio =  new Ertu_Start_Sio();           //ACUPS通信RS485接收线程
            ertu_sio1 = new Ertu_Start_Sio1();          //直流RS485接收线程
            ertu_time = new Ertu_Start_Time();          //500ms定时线程

            //ver_ctl.on_integration_show();      //显示一体化主界面
            qDebug()<<"main :: ver_ctl.w.show();";
            ver_ctl.w.show();
            
            //线程运行
            ertu_sio->start();   //子类化QThread，调用start()会触发调用run().
            qDebug()<<"ertu_sio->start()....";
            ertu_interact->start();
            qDebug()<<"ertu_interact->start()....";
            ertu_interact1->start();
            qDebug()<<"ertu_interact1->start()....";
            ertu_61850->start();
            qDebug()<<"ertu_61850->start()....";
            ertu_rs232->start();
            qDebug()<<"ertu_rs232->start()....";
            ertu_sio1->start();
            qDebug()<<"ertu_sio1->start()....";
            ertu_time->start();
            qDebug()<<"ertu_time->start()....";
        }
        else if(sys_flag == 2) //当为直流主界面(预留)
        {
            ; //ver_ctl.DC_show(); //预留 直流主界面
        }
    }
    else  //当配置文件sys_cfg.ini不存在时
    {
        ver_ctl.show();  //显示配置界面,参数配置后生成文件sys_cfg.ini
    }

    bApp.setWindowInstance(&ver_ctl);       //屏保				
    
    myInputContex *ic = new myInputContex; 	//输入子系统,数字键盘
    QApp.setInputContext(ic);               //InputContext和Application关联 
    
    //设置按键 和下拉按键风格
    qApp->setStyleSheet("QPushButton,QComboBox{color: rgb(255, 255, 255);background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(155, 155, 155, 255));};\n""QComboBox{border:1px;solid:gray;border-radius:3px;padding:1px18px1px3px;};\n");
    //qApp->setStyleSheet("QComboBox{border:1px;solid:gray;border-radius:3px;padding:1px18px1px3px;min-height:40px;}"
    //                        "QComboBox{border:1px;solid:blue;}""QPushButton{background-color: rgb(0, 255, 255);}");

    //不停地重绘 
    for(int i=0;i<8;i++)
        splash->repaint();  
    
    //当主窗口启动后，开机画面隐藏 
    splash->finish(init_widget);  
  
    //保持显示画面1000ms后关闭显示 
    QTimer* init_timer = new QTimer();
    init_timer->start(1000);
    QObject::connect(init_timer,SIGNAL(timeout()),init_widget,SLOT(close()));
    
    delete splash;
    
    qDebug()<<"main :: enter exec()";
   
    //进入事件循环 
    return QApp.exec();			  
}

