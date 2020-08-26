/**********************************************************************************
 *������:	Fault_handling
 *��������:	���ϴ�����
 *��������:	
 *��������ֵ:
 ***********************************************************************************/
#include "fault_handling.h"
#include "global_define.h"
#include "Global_Para_Ex.h"
#include "globel_Ex.h"
#include "Subfunc.h"
#include <sys/time.h>
#include "version.h"
//#include <QTimer>
//#include <QTime>

static unsigned char Fg_ClearCurrentError = 0;

#if 1
//������λ: ����ϱ�־λ
void Fault_ClearCurrentError_handling()
{
    static unsigned int NumOfClearCurrentError = 0;

    if (Fg_ClearCurrentError)
    {
        Fg_ClearCurrentError = 0;
    }

    if (NumOfClearCurrentError < 500)  //��Լ5S
    {
        NumOfClearCurrentError ++;
    }
    else 
    {
        NumOfClearCurrentError = 0;

#if 0
        QDateTime time = QDateTime::currentDateTime();   //��ȡ��ǰʱ��  
        int timeT = time.toTime_t();   //����ǰʱ��תΪʱ���  

        if (((timeT % 600) >= 0) && ((timeT % 600) < 10)) //ǰ10�� 
        {
            Fg_ClearCurrentError = 1;  //����ϱ�־��1  
        }
#else 
        Fg_ClearCurrentError = 1;  //����ϱ�־��1 
        //printf("Fg_ClearCurrentError: %d.\n", Fg_ClearCurrentError); 
#endif 
    }

#if 0  //Test
    printf("NumOfClearCurrentError: %d.\n", NumOfClearCurrentError);
    printf("Fg_ClearCurrentError: %d.\n", Fg_ClearCurrentError);
    int i = 0;

    for (i = 0; i < 2; i ++)
    {
        printf("ERR_UPS_MainsSupply[%d]: %d.\n", i,  ERR_UPS_MainsSupply[i]);
        printf("b_ERR_UPS_MainsSupply[%d]: %d.\n", i,  b_ERR_UPS_MainsSupply[i]);
    }

    printf("Light_UPS: %d.\n", Light_UPS);

#endif

}
#endif 

//���ϱ���
void Fault_handling()
{
#if 1  //������������Ĺ���
    Fault_ClearCurrentError_handling();
#endif 

    Fault_Main_handling();
    Fault_AC_handling();

    Fault_DC_handling();
    Fault_Comm_handling();

    Fault_UPS_handling();
}

/**************************************************************************
 *������:			Fault_Main_handling
 *��������:		�����ܼ����ع��ϱ�־
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void Fault_Main_handling()
{
    int i=0;

    if(ERR_MainPWR != bERR_MainPWR){    //��ǰʱ�̹��ϱ�־ERR_MainPWR��ǰһʱ�̹��ϱ�־bERR_MainPWR�����߲���ȵ�ʱ�򣬾��ǲ����˹�����Ҫ��ʾ���߹�����������Ҫɾ��
        if(bERR_MainPWR == 0){          //ǰһʱ�̹��ϱ�־bERR_MainPWR = 0���Ǿ�˵����ǰʱ�̹��ϱ�־ERR_MainPWR = 1���˿��й�����Ҫ��ʾ���ϣ�
            //���˿�ʱ֮ǰһֱû�й��ϣ�ͻȻ���ϳ��֣���Ҫ��ʾ��
            Add_LinkList(Main_ERR,0,7,0);//��ʾ����
            bERR_MainPWR = ERR_MainPWR;
            //Light_Main++;
            //Light_DC++;
        }else{                          //ǰһʱ�̹��ϱ�־bERR_MainPWR != 0���Ǿ�˵����ǰʱ�̹��ϱ�־ERR_MainPWR = 0���˿̹�����ʧ�ˣ���Ҫɾ����
            //���˿�ʱ֮ǰһֱ�й��ϣ�ͻȻ������ʧ����Ҫɾ����ʾ��
            Delete_LinkList(Main_ERR,0,7,0); //ɾ������
            bERR_MainPWR = ERR_MainPWR;
            //Light_Main--;
            //Light_DC--;
        }
    }
    else if(bERR_MainPWR == 0) //����־λȫΪ0 
    {
        //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
        if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,0,7,0); //ɾ������
        }
    }

    if(ERR_MainPWR1 != bERR_MainPWR1){  //ERR_MainPWR1,���ͨ�Ź��ϵ�0,1��λ�����ǿ�8��ͨ���ڣ���û��ͨ�ϣ��о���0���о���1
        if(bERR_MainPWR1 == 0){
            Add_LinkList(Main_ERR,0,8,0);
            bERR_MainPWR1 = ERR_MainPWR1;
            //Light_Main++;
            //Light_DC++;
        }else{
            Delete_LinkList(Main_ERR,0,8,0);
            bERR_MainPWR1 = ERR_MainPWR1;
            //Light_Main--;
            //Light_DC--;
        }
    }
    else if(bERR_MainPWR1 == 0) //����־λȫΪ0 
    {
        //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
        if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,0,8,0);
        }
    }

    for(i=0;i<3;i++){
        if(ERR_MainDC[i] != bERR_MainDC[i]){   //ֱ��3����أ�3������
            if(bERR_MainDC[i] == 0) {
                Add_LinkList(Main_ERR,i+1,0,0);
                bERR_MainDC[i] = ERR_MainDC[i];
                //Light_DC++;
            }else{
                Delete_LinkList(Main_ERR,i+1,0,0);
                bERR_MainDC[i] = ERR_MainDC[i];
                //Light_DC--;
            }
        }
        else if(bERR_MainDC[i] == 0) //����־λȫΪ0 
        {
            //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
            if ((Light_Main > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Main_ERR,i+1,0,0);
            }
        }

#if 1   //����
        if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
        {
            //1~3#ֱ����Դ���Ѳ����ͨѶ�Ͽ�
            if (ERR_MainDC_PSMX_B_Comm[i] != b_ERR_MainDC_PSMX_B_Comm[i])
            {		
                if (b_ERR_MainDC_PSMX_B_Comm[i] == 0)
                {
                    Add_LinkList(Main_ERR,i+1,12,0);
                }
                else
                {
                    Delete_LinkList(Main_ERR,i+1,12,0);
                }
                b_ERR_MainDC_PSMX_B_Comm[i] = ERR_MainDC_PSMX_B_Comm[i];    
            }
            else if (b_ERR_MainDC_PSMX_B_Comm[i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Main_ERR,i+1,12,0);
                }           
            }                 
        }
#endif 
    }

    for(i=0;i<4;i++){
        if(ERR_MainDCTX[i] != bERR_MainDCTX[i]){   //��ز�����4����4������
            if(bERR_MainDCTX[i] == 0) {
                Add_LinkList(Main_ERR,i+1,11,0);
                bERR_MainDCTX[i] = ERR_MainDCTX[i];
                //Light_DC++;
            }else{
                Delete_LinkList(Main_ERR,i+1,11,0);
                bERR_MainDCTX[i] = ERR_MainDCTX[i];
                //Light_DC--;
            }
        }
        else if(bERR_MainDCTX[i] == 0) //����־λȫΪ0 
        {
            //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
            if ((Light_Main > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Main_ERR,i+1,11,0);
            }
        }
    }

    if(ERR_MainIsu != bERR_MainIsu){
        if(bERR_MainIsu == 0){
            Add_LinkList(Main_ERR,1,6,0);
            bERR_MainIsu = ERR_MainIsu;
            //Light_Main++;
            //Light_DC++;
        }else{
            Delete_LinkList(Main_ERR,1,6,0);
            bERR_MainIsu = ERR_MainIsu;
            //Light_Main--;
            //Light_DC--;
        }
    }
    else if(bERR_MainIsu == 0) //����־λȫΪ0 
    {
        //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
        if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,1,6,0);
        }
    }

    if(ERR_MainIsu1 != bERR_MainIsu1) {
        if(bERR_MainIsu1 == 0){
            Add_LinkList(Main_ERR,2,6,0);
            bERR_MainIsu1 = ERR_MainIsu1;
            //Light_Main++;
            //Light_DC++;
        }else{
            Delete_LinkList(Main_ERR,2,6,0);
            bERR_MainIsu1 = ERR_MainIsu1;
            //Light_Main--;
            //Light_DC--;
        }    
    }
    else if(bERR_MainIsu1 == 0) //����־λȫΪ0 
    {
        //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
        if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,2,6,0);
        }
    }

    if(ERR_MainAC != bERR_MainAC){
        if(bERR_MainAC == 0){
            Add_LinkList(Main_ERR,0,1,0);
            bERR_MainAC = ERR_MainAC;
            //Light_AC++;
        }else{
            Delete_LinkList(Main_ERR,0,1,0);
            bERR_MainAC = ERR_MainAC;
            //Light_AC--;
        }
    }
    else if(bERR_MainAC == 0) //����־λȫΪ0 
    {
        //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
        if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,0,1,0);
        }
    }

#if 0
    if(Special_35KV_flag != 2){   //35kV  ���⹤�� ͨ�ŵ�Դ��UPS����ֱ�������
        if(ERR_MainUPS== 1){
            if(bERR_MainUPS!= 1) {
                Add_LinkList(Main_ERR,0,2,0);
                bERR_MainUPS = ERR_MainUPS;
                //Light_UPS++;
            }
        }else{
            if(bERR_MainUPS== 1){
                Delete_LinkList(Main_ERR,0,2,0);
                bERR_MainUPS = ERR_MainUPS;
                //Light_UPS--;
            }
            //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
            else if ((Light_Main > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Main_ERR,0,2,0);
            }           
        }

        if(ERR_MainComPw == 1){
            if(bERR_MainComPw != 1){
                debug_printf(0,"00000,, bERR_MainComPw == %4d \n",bERR_MainComPw);
                Add_LinkList(Main_ERR,1,3,0);  //���ϱ���λ���ж�ԭ����������Ƿ��Ѿ����������û�о�д������������������˾Ͳ����ٰ�ͬһ��������д������
                bERR_MainComPw = ERR_MainComPw;
                debug_printf(0,"11111,, bERR_MainComPw == %4d \n",bERR_MainComPw);
                //Light_Comm++;
            }
        }else{
            if(bERR_MainComPw == 1){
                Delete_LinkList(Main_ERR,1,3,0); //������ʧ�ˣ�����û�б���λ���ж�ԭ����������Ƿ��Ѿ�����������Ǳ����ģ���Ҫɾ��������Ĺ���
                bERR_MainComPw = ERR_MainComPw;
                //Light_Comm--;
            }
            //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
            else if ((Light_Main > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Main_ERR,1,3,0);
            } 
        }

        if(ERR_MainComPw1 == 1){
            if(bERR_MainComPw1 != 1) {
                Add_LinkList(Main_ERR,2,3,0);
                bERR_MainComPw1 = ERR_MainComPw1;
                //Light_Comm++;
            }
        }else{
            if(bERR_MainComPw1 == 1){
                Delete_LinkList(Main_ERR,2,3,0);
                bERR_MainComPw1 = ERR_MainComPw1;
                //Light_Comm--;
            }  
            else if ((Light_Main > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Main_ERR,2,3,0);
            }             
        }
    }
#else  //������������ͨ�š�UPS��Դ��ص�ѡ����. 
    if(Special_35KV_flag != 2)
    {   //35kV  ���⹤�� ͨ�ŵ�Դ��UPS����ֱ�������
        if (Special_35KV_flag_NoUpsMon_WithCommMon != 0x02)  //��UPS���
        {
            if(ERR_MainUPS== 1)
            {
                if(bERR_MainUPS!= 1) 
                {
                    Add_LinkList(Main_ERR,0,2,0);
                    bERR_MainUPS = ERR_MainUPS;
                    //Light_UPS++;
                }
            }
            else
            {
                if(bERR_MainUPS== 1)
                {
                    Delete_LinkList(Main_ERR,0,2,0);
                    bERR_MainUPS = ERR_MainUPS;
                    //Light_UPS--;
                }
                //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
                else if ((Light_Main > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Main_ERR,0,2,0);
                }           
            }
        }

        if (Special_35KV_flag_NoCommMon_WithUpsMon != 0x02)  //��ͨ�ŵ�Դ���
        {
            if(ERR_MainComPw == 1)
            {
                if(bERR_MainComPw != 1)
                {
                    debug_printf(0,"00000,, bERR_MainComPw == %4d \n",bERR_MainComPw);
                    Add_LinkList(Main_ERR,1,3,0);  //���ϱ���λ���ж�ԭ����������Ƿ��Ѿ����������û�о�д������������������˾Ͳ����ٰ�ͬһ��������д������
                    bERR_MainComPw = ERR_MainComPw;
                    debug_printf(0,"11111,, bERR_MainComPw == %4d \n",bERR_MainComPw);
                    //Light_Comm++;
                }
            }
            else
            {
                if(bERR_MainComPw == 1)
                {
                    Delete_LinkList(Main_ERR,1,3,0); //������ʧ�ˣ�����û�б���λ���ж�ԭ����������Ƿ��Ѿ�����������Ǳ����ģ���Ҫɾ��������Ĺ���
                    bERR_MainComPw = ERR_MainComPw;
                    //Light_Comm--;
                }
                //��������: ��ǰ���ϸ�����Ϊ0,���Ҷ�ʱ����ϱ�־��λ!!! 
                else if ((Light_Main > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Main_ERR,1,3,0);
                } 
            }

            if(ERR_MainComPw1 == 1)
            {
                if(bERR_MainComPw1 != 1) 
                {
                    Add_LinkList(Main_ERR,2,3,0);
                    bERR_MainComPw1 = ERR_MainComPw1;
                    //Light_Comm++;
                }
            }
            else
            {
                if(bERR_MainComPw1 == 1)
                {
                    Delete_LinkList(Main_ERR,2,3,0);
                    bERR_MainComPw1 = ERR_MainComPw1;
                    //Light_Comm--;
                }  
                else if ((Light_Main > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Main_ERR,2,3,0);
                }             
            }
        }
    }    
#endif 

    if(ERR_MainATS == 1){
        if(bERR_MainATS!= 1){
            Add_LinkList(Main_ERR,0,4,0);
            bERR_MainATS = ERR_MainATS;
            //Light_AC++;
        }
    }else{
        if(bERR_MainATS== 1) {
            Delete_LinkList(Main_ERR,0,4,0);
            bERR_MainATS = ERR_MainATS;
            //Light_AC--;
        }    
        else if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,0,4,0);
        }            
    }

    if(ERR_MainATS1 == 1){
        if(bERR_MainATS1!= 1) {
            Add_LinkList(Main_ERR,0,5,0);
            bERR_MainATS1 = ERR_MainATS1;
            //Light_AC++;
        }
    }else{
        if(bERR_MainATS1== 1) {
            Delete_LinkList(Main_ERR,0,5,0);
            bERR_MainATS1 = ERR_MainATS1;
            //Light_AC--;
        }
        else if ((Light_Main > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(Main_ERR,0,5,0);
        } 
    }
}
/**************************************************************************
 *������:			Fault_AC_handling
 *��������:		��������ع��ϱ�־
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void Fault_AC_handling()
{
    int i = 0;
    int j = 0;
    for(i=0;i<4;i++)														//���߿�����բ
        if(ERR_AC_in_SW_trip[i] == 1){
            if(b_ERR_AC_in_SW_trip[i] != 1){
                Add_LinkList(AC_ERR,i+1,0,0);   //��ʾ����
                b_ERR_AC_in_SW_trip[i] = ERR_AC_in_SW_trip[i];
            }  //��ǰʱ��ERR_AC_in_SW_trip = 1��b_ERR_AC_in_SW_trip��ǰһʱ�� =0�Ļ���˵�����������֮ǰ�Ǳ�ɾ���ģ���������ERR_AC_in_SW_trip =1��˵�����ϳ����ˣ���Ҫ��ʾ����
        }else if(b_ERR_AC_in_SW_trip[i] == 1){
            Delete_LinkList(AC_ERR,i+1,0,0);   //ɾ����ʾ�����Ĺ���
            b_ERR_AC_in_SW_trip[i] = ERR_AC_in_SW_trip[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,0,0);
        }     

    //��ǰʱ��ERR_AC_in_SW_trip = 0��b_ERR_AC_in_SW_trip��ǰһʱ�� =1�Ļ���˵�����������������ʾ�ڽ����ϵģ�����ERR_AC_in_SW_trip =0��˵��������ʧ�ˣ���Ҫɾ��

    for(i=0;i<3;i++)														//����������
        if(ERR_AC_SPD[i] == 1){
            if(b_ERR_AC_SPD[i] != 1){
                Add_LinkList(AC_ERR,i+1,1,0);
                b_ERR_AC_SPD[i] = ERR_AC_SPD[i];
            }
        }else if(b_ERR_AC_SPD[i] == 1){
            Delete_LinkList(AC_ERR,i+1,1,0);
            b_ERR_AC_SPD[i] = ERR_AC_SPD[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,1,0);
        }     

    for(i=0;i<4;i++)														//���ߵ�ѹ�쳣
        if(ERR_AC_in_V[i] == 1){
            if(b_ERR_AC_in_V[i] != 1){
                Add_LinkList(AC_ERR,i+1,2,0);
                b_ERR_AC_in_V[i] = ERR_AC_in_V[i];
            }
        }else if(b_ERR_AC_in_V[i] == 1){
            Delete_LinkList(AC_ERR,i+1,2,0);
            b_ERR_AC_in_V[i] = ERR_AC_in_V[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,2,0);
        } 

    for(i=0;i<2;i++)														//ĸ�ߵ�ѹ�쳣
        if(ERR_AC_mu_V[i] == 1){
            if(b_ERR_AC_mu_V[i] != 1){
                Add_LinkList(AC_ERR,i+1,3,0);
                b_ERR_AC_mu_V[i] = ERR_AC_mu_V[i];
            }
        }else if(b_ERR_AC_mu_V[i] == 1){
            Delete_LinkList(AC_ERR,i+1,3,0);
            b_ERR_AC_mu_V[i] = ERR_AC_mu_V[i];
        }        
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,3,0);
        } 

    for(i=0;i<2;i++)														//����������Ԫͨ���쳣
        if(ERR_AC_AcSample_comm[i] == 1){
            if(b_ERR_AC_AcSample_comm[i] != 1){
                Add_LinkList(AC_ERR,i+1,4,0);
                b_ERR_AC_AcSample_comm[i] = ERR_AC_AcSample_comm[i];
            }
        }else if(b_ERR_AC_AcSample_comm[i] == 1){
            Delete_LinkList(AC_ERR,i+1,4,0);
            b_ERR_AC_AcSample_comm[i] = ERR_AC_AcSample_comm[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,4,0);
        }

    for(i=0;i<16;i++)														//��������Ԫͨ���쳣
        if(ERR_AC_SW_comm[i] == 1){
            if(b_ERR_AC_SW_comm[i] != 1){
                Add_LinkList(AC_ERR,i+1,5,0);
                b_ERR_AC_SW_comm[i] = ERR_AC_SW_comm[i];
            }
        }else if(b_ERR_AC_SW_comm[i] == 1){
            Delete_LinkList(AC_ERR,i+1,5,0);
            b_ERR_AC_SW_comm[i] = ERR_AC_SW_comm[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,5,0);
        }

    for(i=0;i<16;i++)														//״̬����Ԫͨ���쳣
        if(ERR_AC_St_comm[i] == 1){
            if(b_ERR_AC_St_comm[i] != 1){
                Add_LinkList(AC_ERR,i+1,6,0);
                b_ERR_AC_St_comm[i] = ERR_AC_St_comm[i];
            }
        }else if(b_ERR_AC_St_comm[i] == 1){
            Delete_LinkList(AC_ERR,i+1,6,0);
            b_ERR_AC_St_comm[i] = ERR_AC_St_comm[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,6,0);
        }

    for(i=0;i<16;i++)														//����������Ԫͨ���쳣
        if(ERR_AC_CurrentSample_comm[i] == 1){
            if(b_ERR_AC_CurrentSample_comm[i] != 1){
                Add_LinkList(AC_ERR,i+1,7,0);
                b_ERR_AC_CurrentSample_comm[i] = ERR_AC_CurrentSample_comm[i];
            }
        }else if(b_ERR_AC_CurrentSample_comm[i] == 1){
            Delete_LinkList(AC_ERR,i+1,7,0);
            b_ERR_AC_CurrentSample_comm[i] = ERR_AC_CurrentSample_comm[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,7,0);
        }

    for(i=0;i<4;i++)														//���ATSEͨ���쳣
        if(ERR_AC_Meter_comm[i] == 1){
            if(b_ERR_AC_Meter_comm[i] != 1){
                Add_LinkList(AC_ERR,i+1,8,0);
                b_ERR_AC_Meter_comm[i] = ERR_AC_Meter_comm[i];
            }
        }else if(b_ERR_AC_Meter_comm[i] == 1){
            Delete_LinkList(AC_ERR,i+1,8,0);
            b_ERR_AC_Meter_comm[i] = ERR_AC_Meter_comm[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,8,0);
        }

    if(ERR_AC_Feeder_Duan_trip_1 == 1){									//һ��������բ
        if(b_ERR_AC_Feeder_Duan_trip_1 != 1){
            Add_LinkList(AC_ERR,1,9,0);
            b_ERR_AC_Feeder_Duan_trip_1 = ERR_AC_Feeder_Duan_trip_1;
        }
    }else if(b_ERR_AC_Feeder_Duan_trip_1 == 1){
        Delete_LinkList(AC_ERR,1,9,0);
        b_ERR_AC_Feeder_Duan_trip_1 = ERR_AC_Feeder_Duan_trip_1;
    }
    else if ((Light_AC > 0) && Fg_ClearCurrentError) 
    {        
        Delete_LinkList(AC_ERR,1,9,0);
    }

    if(ERR_AC_Feeder_Duan_trip_2 == 1){									//����������բ
        if(b_ERR_AC_Feeder_Duan_trip_2 != 1){
            Add_LinkList(AC_ERR,2,9,0);
            b_ERR_AC_Feeder_Duan_trip_2 = ERR_AC_Feeder_Duan_trip_2;
        }
    }else if(b_ERR_AC_Feeder_Duan_trip_2 == 1){
        Delete_LinkList(AC_ERR,2,9,0);
        b_ERR_AC_Feeder_Duan_trip_2 = ERR_AC_Feeder_Duan_trip_2;
    }
    else if ((Light_AC > 0) && Fg_ClearCurrentError) 
    {        
        Delete_LinkList(AC_ERR,2,9,0);
    }

    for(i=0;i<16;i++){														//���߿�����բ
        for(j=0;j<30;j++){
            if(ERR_AC_Feeder_SW_trip[i][j] != b_ERR_AC_Feeder_SW_trip[i][j]){
                if(b_ERR_AC_Feeder_SW_trip[i][j] == 0){
                    Add_LinkList(AC_ERR,i+1,10,j+1);
                    b_ERR_AC_Feeder_SW_trip[i][j] = ERR_AC_Feeder_SW_trip[i][j];
                }else {
                    Delete_LinkList(AC_ERR,i+1,10,j+1);
                    b_ERR_AC_Feeder_SW_trip[i][j] = ERR_AC_Feeder_SW_trip[i][j];
                }
            }
            else if (b_ERR_AC_Feeder_SW_trip[i][j] == 0) 
            {
                if ((Light_AC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(AC_ERR,i+1,10,j+1);
                }
            }
        }
    }

    if(ERR_AC_BT_device == 1){									//��Ͷװ�ù���
        if(b_ERR_AC_BT_device != 1){
            Add_LinkList(AC_ERR,0,11,0);
            b_ERR_AC_BT_device = ERR_AC_BT_device;
        }
    }else if(b_ERR_AC_BT_device == 1){
        Delete_LinkList(AC_ERR,0,11,0);
        b_ERR_AC_BT_device = ERR_AC_BT_device;
    }
    else if ((Light_AC > 0) && Fg_ClearCurrentError) 
    {        
        Delete_LinkList(AC_ERR,0,11,0);
    }

    for(i=0;i<4;i++)
    {														//��Ͷװ�õ�ٺ�բʧ�ܸ澯
        if(ERR_AC_BT_device_he[i] == 1){
            if(b_ERR_AC_BT_device_he[i] != 1){
                Add_LinkList(AC_ERR,i+1,12,0);
                b_ERR_AC_BT_device_he[i] = ERR_AC_BT_device_he[i];
            }
        }else if(b_ERR_AC_BT_device_he[i] == 1){
            Delete_LinkList(AC_ERR,i+1,12,0);
            b_ERR_AC_BT_device_he[i] = ERR_AC_BT_device_he[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,12,0);
        }
    }

    for(i=0;i<4;i++)
    {														//��Ͷװ�õ�ٷ�բʧ�ܸ澯
        if(ERR_AC_BT_device_fen[i] == 1){
            if(b_ERR_AC_BT_device_fen[i] != 1){
                Add_LinkList(AC_ERR,i+1,13,0);
                b_ERR_AC_BT_device_fen[i] = ERR_AC_BT_device_fen[i];
            }
        }else if(b_ERR_AC_BT_device_fen[i] == 1){
            Delete_LinkList(AC_ERR,i+1,13,0);
            b_ERR_AC_BT_device_fen[i] = ERR_AC_BT_device_fen[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,i+1,13,0);
        }
    }

    for(i=0;i<30;i++)						//	Ԥ����30������
        if(ERR_AC_Reserve[i] == 1){
            if(b_ERR_AC_Reserve[i] != 1){
                Add_LinkList(AC_ERR,0,11+i,0);
                b_ERR_AC_Reserve[i] = ERR_AC_Reserve[i];
            }
        }else if(b_ERR_AC_Reserve[i] == 1){
            Delete_LinkList(AC_ERR,0,11+i,0);
            b_ERR_AC_Reserve[i] = ERR_AC_Reserve[i];
        }
        else if ((Light_AC > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(AC_ERR,0,11+i,0);
        }
}
/**************************************************************************
 *������:			Fault_DC_handling
 *��������:		�����յ�ֱ��������ݸ�ֵ����ر���������
 *					��ʾ�ñ����͹��ϱ�־������
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void Fault_DC_handling()
{
    int group = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    int num;
    int flag_sw = 0;
    for (group = 0; group < 3; group++)
    {
        for(i=0;i<12;i++){
            if(ERR_DC_Module[group][i] != b_ERR_DC_Module[group][i]){			// ģ�����
                if(b_ERR_DC_Module[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,0,i+1);
                    b_ERR_DC_Module[group][i] = ERR_DC_Module[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,0,i+1);
                    b_ERR_DC_Module[group][i] = ERR_DC_Module[group][i];
                }
            }
            else if (b_ERR_DC_Module[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,0,i+1);
                }           
            }

            if(ERR_DC_Module_comm[group][i] != b_ERR_DC_Module_comm[group][i]){//ģ��ͨѶ����
                if(b_ERR_DC_Module_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,1,i+1);
                    b_ERR_DC_Module_comm[group][i] = ERR_DC_Module_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,1,i+1);
                    b_ERR_DC_Module_comm[group][i] = ERR_DC_Module_comm[group][i];
                }
            }
            else if (b_ERR_DC_Module_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,1,i+1);
                }           
            }
        }

        if(ERR_DC_DcSample_comm[group] != b_ERR_DC_DcSample_comm[group]){//ֱ��������ͨѶ����
            if(b_ERR_DC_DcSample_comm[group] == 0){
                Add_LinkList(DC_ERR,group+1,2,0);
                b_ERR_DC_DcSample_comm[group] = ERR_DC_DcSample_comm[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,2,0);
                b_ERR_DC_DcSample_comm[group] = ERR_DC_DcSample_comm[group];
            }	
        }
        else if (b_ERR_DC_DcSample_comm[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,2,0);
            }           
        }

        if(ERR_DC_AcSample_comm[group] != b_ERR_DC_AcSample_comm[group]){//����������ͨѶ����
            if(b_ERR_DC_AcSample_comm[group] == 0){
                Add_LinkList(DC_ERR,group+1,3,0);
                b_ERR_DC_AcSample_comm[group] = ERR_DC_AcSample_comm[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,3,0);
                b_ERR_DC_AcSample_comm[group] = ERR_DC_AcSample_comm[group];
            }	
        }
        else if (b_ERR_DC_AcSample_comm[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,3,0);
            }           
        }

        if(ERR_DC_Charger_GY[group] != b_ERR_DC_Charger_GY[group]){		//������ѹ
            if(b_ERR_DC_Charger_GY[group] == 0){
                Add_LinkList(DC_ERR,group+1,4,0);
                b_ERR_DC_Charger_GY[group] = ERR_DC_Charger_GY[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,4,0);
                b_ERR_DC_Charger_GY[group] = ERR_DC_Charger_GY[group];
            }	
        }
        else if (b_ERR_DC_Charger_GY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,4,0);
            }           
        }

        if(ERR_DC_Charger_QY[group] != b_ERR_DC_Charger_QY[group]){		//����Ƿѹ
            if(b_ERR_DC_Charger_QY[group] == 0){
                Add_LinkList(DC_ERR,group+1,5,0);
                b_ERR_DC_Charger_QY[group] = ERR_DC_Charger_QY[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,5,0);
                b_ERR_DC_Charger_QY[group] = ERR_DC_Charger_QY[group];
            }	
        }
        else if (b_ERR_DC_Charger_QY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,5,0);
            }           
        }

        if(ERR_DC_KM_GY[group] != b_ERR_DC_KM_GY[group]){		//��ĸ��ѹ ���� 35K ĸ�߹�ѹ
            if(b_ERR_DC_KM_GY[group] == 0){
                if(Special_35KV_flag == 2){
                    Add_LinkList(DC_ERR,group+1,30,0); //30: "ĸ�߹�ѹ\n"
                }else{
                    Add_LinkList(DC_ERR,group+1,6,0);  //6: "��ĸ��ѹ\n"
                }
                b_ERR_DC_KM_GY[group] = ERR_DC_KM_GY[group];	
            }else{
                if(Special_35KV_flag == 2){
                    Delete_LinkList(DC_ERR,group+1,30,0);
                }else{
                    Delete_LinkList(DC_ERR,group+1,6,0);
                }
                b_ERR_DC_KM_GY[group] = ERR_DC_KM_GY[group];
            }	
        }
        else if (b_ERR_DC_KM_GY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if(Special_35KV_flag == 2){
                    Delete_LinkList(DC_ERR,group+1,30,0);
                }else{
                    Delete_LinkList(DC_ERR,group+1,6,0);
                }
            }           
        }

        if(ERR_DC_KM_QY[group] != b_ERR_DC_KM_QY[group]){		//��ĸǷѹ ���� 35K ĸ��Ƿѹ
            if(b_ERR_DC_KM_QY[group] == 0){
                if(Special_35KV_flag == 2){
                    Add_LinkList(DC_ERR,group+1,31,0); //31: "ĸ��Ƿѹ\n" 
                }else{
                    Add_LinkList(DC_ERR,group+1,7,0);  //7: "��ĸǷѹ\n"
                }
                b_ERR_DC_KM_QY[group] = ERR_DC_KM_QY[group];	
            }else{
                if(Special_35KV_flag == 2){
                    Delete_LinkList(DC_ERR,group+1,31,0);
                }else{
                    Delete_LinkList(DC_ERR,group+1,7,0);
                }
                b_ERR_DC_KM_QY[group] = ERR_DC_KM_QY[group];
            }	
        }
        else if (b_ERR_DC_KM_QY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if(Special_35KV_flag == 2){
                    Delete_LinkList(DC_ERR,group+1,31,0);
                }else{
                    Delete_LinkList(DC_ERR,group+1,7,0);
                }
            }           
        }

        for(i=0;i<2;i++)
        {
            if(ERR_DC_AcVoltage_GY[group][i] != b_ERR_DC_AcVoltage_GY[group][i]){		// ������ѹ
                if(b_ERR_DC_AcVoltage_GY[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,8,i+1);
                    b_ERR_DC_AcVoltage_GY[group][i] = ERR_DC_AcVoltage_GY[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,8,i+1);
                    b_ERR_DC_AcVoltage_GY[group][i] = ERR_DC_AcVoltage_GY[group][i];
                }
            }
            else if (b_ERR_DC_AcVoltage_GY[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,8,i+1);
                }           
            }

            if(ERR_DC_AcVoltage_QY[group][i] != b_ERR_DC_AcVoltage_QY[group][i]){	   // ����Ƿѹ
                if(b_ERR_DC_AcVoltage_QY[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,9,i+1);
                    b_ERR_DC_AcVoltage_QY[group][i] = ERR_DC_AcVoltage_QY[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,9,i+1);
                    b_ERR_DC_AcVoltage_QY[group][i] = ERR_DC_AcVoltage_QY[group][i];
                }
            }
            else if (b_ERR_DC_AcVoltage_QY[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,9,i+1);
                }           
            }

            if(ERR_DC_Ac_PowerCut[group][i] != b_ERR_DC_Ac_PowerCut[group][i]){			// ����ͣ��
                if(b_ERR_DC_Ac_PowerCut[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,10,i+1);
                    b_ERR_DC_Ac_PowerCut[group][i] = ERR_DC_Ac_PowerCut[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,10,i+1);
                    b_ERR_DC_Ac_PowerCut[group][i] = ERR_DC_Ac_PowerCut[group][i];
                }
            }
            else if (b_ERR_DC_Ac_PowerCut[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,10,i+1);
                }           
            }

            if(ERR_DC_Ac_PhaseLoss[group][i] != b_ERR_DC_Ac_PhaseLoss[group][i]){		// ����ȱ��
                if(b_ERR_DC_Ac_PhaseLoss[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,11,i+1);
                    b_ERR_DC_Ac_PhaseLoss[group][i] = ERR_DC_Ac_PhaseLoss[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,11,i+1);
                    b_ERR_DC_Ac_PhaseLoss[group][i] = ERR_DC_Ac_PhaseLoss[group][i];
                }
            }
            else if (b_ERR_DC_Ac_PhaseLoss[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,11,i+1);
                }           
            }
        }

        if(ERR_DC_AcSPD[group] != b_ERR_DC_AcSPD[group]){		//��������������
            if(b_ERR_DC_AcSPD[group] == 0){
                Add_LinkList(DC_ERR,group+1,12,0);
                b_ERR_DC_AcSPD[group] = ERR_DC_AcSPD[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,12,0);
                b_ERR_DC_AcSPD[group] = ERR_DC_AcSPD[group];
            }	
        }
        else if (b_ERR_DC_AcSPD[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,12,0);
            }           
        }

        for(i=0;i<2;i++){
            if(ERR_DC_AcSwitch[group][i] != b_ERR_DC_AcSwitch[group][i]){	    // �������ضϿ�����
                if(b_ERR_DC_AcSwitch[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,13,i+1);
                    b_ERR_DC_AcSwitch[group][i] = ERR_DC_AcSwitch[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,13,i+1);
                    b_ERR_DC_AcSwitch[group][i] = ERR_DC_AcSwitch[group][i];
                }
            }
            else if (b_ERR_DC_AcSwitch[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,13,i+1);
                }           
            }
        }

#if 0
        if(ERR_DC_AcSwitch[group][2] != b_ERR_DC_AcSwitch[group][2]){			// ��������3�Ͽ����� ���� ͨ�ŵ�Դ������բ
            if(b_ERR_DC_AcSwitch[group][2] == 0){
                if(Special_35KV_flag == 2){
                    Add_LinkList(Comm_ERR,group+1,10,0);  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }else{
                    Add_LinkList(DC_ERR,group+1,13,3);
                }
                b_ERR_DC_AcSwitch[group][2] = ERR_DC_AcSwitch[group][2];
            }else{
                if(Special_35KV_flag == 2){
                    Delete_LinkList(Comm_ERR,group+1,10,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }else{
                    Delete_LinkList(DC_ERR,group+1,13,3);
                }
                b_ERR_DC_AcSwitch[group][2] = ERR_DC_AcSwitch[group][2];
            }
        }
        else if (b_ERR_DC_AcSwitch[group][2] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if(Special_35KV_flag == 2){
                    Delete_LinkList(Comm_ERR,group+1,10,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }else{
                    Delete_LinkList(DC_ERR,group+1,13,3);
                }
            }           
        }

        if(ERR_DC_AcSwitch[group][3] != b_ERR_DC_AcSwitch[group][3]){			// �������ضϿ�����
            if(b_ERR_DC_AcSwitch[group][3] == 0){
                if(Special_35KV_flag == 2){
                    Add_LinkList(UPS_ERR,group+1,18,0);   //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }else{
                    Add_LinkList(DC_ERR,group+1,29,2);
                }
                b_ERR_DC_AcSwitch[group][3] = ERR_DC_AcSwitch[group][3];
            }else{
                if(Special_35KV_flag == 2){
                    Delete_LinkList(UPS_ERR,group+1,18,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }else{
                    Delete_LinkList(DC_ERR,group+1,29,2);
                }
                b_ERR_DC_AcSwitch[group][3] = ERR_DC_AcSwitch[group][3];
            }
        }
        else if (b_ERR_DC_AcSwitch[group][3] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if(Special_35KV_flag == 2){
                    Delete_LinkList(UPS_ERR,group+1,18,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }else{
                    Delete_LinkList(DC_ERR,group+1,29,2);
                }
            }           
        }
#else   //������������ͨ�š�UPS��Դ��ص�ѡ����.  
        if(ERR_DC_AcSwitch[group][2] != b_ERR_DC_AcSwitch[group][2])
        {			// ��������3�Ͽ����� ���� ͨ�ŵ�Դ������բ
            if(b_ERR_DC_AcSwitch[group][2] == 0)
            {
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoCommMon_WithUpsMon == 2))
                {
                    Add_LinkList(Comm_ERR,group+1,10,0);  //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }
                else
                {
                    Add_LinkList(DC_ERR,group+1,13,3);    //13: "�Ž������ضϿ�����\n", 			
                }
                b_ERR_DC_AcSwitch[group][2] = ERR_DC_AcSwitch[group][2];
            }
            else
            {
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoCommMon_WithUpsMon == 2))
                {
                    Delete_LinkList(Comm_ERR,group+1,10,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }
                else
                {
                    Delete_LinkList(DC_ERR,group+1,13,3);
                }
                b_ERR_DC_AcSwitch[group][2] = ERR_DC_AcSwitch[group][2];
            }
        }
        else if (b_ERR_DC_AcSwitch[group][2] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoCommMon_WithUpsMon == 2))
                {
                    Delete_LinkList(Comm_ERR,group+1,10,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�ͨ�ŵ�Դ������բ
                }
                else
                {
                    Delete_LinkList(DC_ERR,group+1,13,3);
                }
            }           
        }

        if(ERR_DC_AcSwitch[group][3] != b_ERR_DC_AcSwitch[group][3])
        {			// �������ضϿ�����
            if(b_ERR_DC_AcSwitch[group][3] == 0)
            {
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoUpsMon_WithCommMon == 2))
                {
                    Add_LinkList(UPS_ERR,group+1,18,0);   //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }
                else
                {
                    Add_LinkList(DC_ERR,group+1,29,2);
                }
                b_ERR_DC_AcSwitch[group][3] = ERR_DC_AcSwitch[group][3];
            }
            else
            {
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoUpsMon_WithCommMon == 2))
                {
                    Delete_LinkList(UPS_ERR,group+1,18,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }
                else
                {
                    Delete_LinkList(DC_ERR,group+1,29,2);
                }
                b_ERR_DC_AcSwitch[group][3] = ERR_DC_AcSwitch[group][3];
            }
        }
        else if (b_ERR_DC_AcSwitch[group][3] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                if ((Special_35KV_flag == 2) 
                        || (Special_35KV_flag_NoUpsMon_WithCommMon == 2))
                {
                    Delete_LinkList(UPS_ERR,group+1,18,0); //��ר�õ�UPS��غ�ͨ�ŵ�Դ��أ�UPS���߿�����բ
                }
                else
                {
                    Delete_LinkList(DC_ERR,group+1,29,2);
                }
            }           
        }
#endif 

        if(ERR_DC_AcSwitch[group][4] != b_ERR_DC_AcSwitch[group][4]){			// �������ضϿ�����
            if(b_ERR_DC_AcSwitch[group][4] == 0){
                Add_LinkList(DC_ERR,group+1,29,1);
                b_ERR_DC_AcSwitch[group][4] = ERR_DC_AcSwitch[group][4];
            }else{
                Delete_LinkList(DC_ERR,group+1,29,1);
                b_ERR_DC_AcSwitch[group][4] = ERR_DC_AcSwitch[group][4];
            }
        }
        else if (b_ERR_DC_AcSwitch[group][4] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,29,1);
            }           
        }

        //		for(i=0;i<2;i++){
        if(ERR_DC_Battery_SW[group][0] != b_ERR_DC_Battery_SW[group][0]){		// ��ؿ��ضϿ�����
            if(b_ERR_DC_Battery_SW[group][0] == 0){
                Add_LinkList(DC_ERR,group+1,14,1);
                b_ERR_DC_Battery_SW[group][0] = ERR_DC_Battery_SW[group][0];
            }else{
                Delete_LinkList(DC_ERR,group+1,14,1);
                b_ERR_DC_Battery_SW[group][0] = ERR_DC_Battery_SW[group][0];
            }
        }
        else if (b_ERR_DC_Battery_SW[group][0] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,14,1);
            }           
        }

        if(ERR_DC_Battery_SW[group][1] != b_ERR_DC_Battery_SW[group][1]){		// �������ضϿ�����
            if(b_ERR_DC_Battery_SW[group][1] == 0){
                Add_LinkList(DC_ERR,group+1,37,0);
                b_ERR_DC_Battery_SW[group][1] = ERR_DC_Battery_SW[group][1];
            }else{
                Delete_LinkList(DC_ERR,group+1,37,0);
                b_ERR_DC_Battery_SW[group][1] = ERR_DC_Battery_SW[group][1];
            }
        }
        else if (b_ERR_DC_Battery_SW[group][1] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,37,0);
            }           
        }

        //		}
        for(i=0;i<2;i++){
            if(ERR_DC_External[group][i] != b_ERR_DC_External[group][i]){		//����豸����
                if(b_ERR_DC_External[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,15,i+1);
                    b_ERR_DC_External[group][i] = ERR_DC_External[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,15,i+1);
                    b_ERR_DC_External[group][i] = ERR_DC_External[group][i];
                }
            }
            else if (b_ERR_DC_External[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,15,i+1);
                }           
            }
        }

        for(i=0;i<2;i++){
            if(ERR_DC_BatteryFuse[group][i] != b_ERR_DC_BatteryFuse[group][i]){	// ����۶����Ͽ�����
                if(b_ERR_DC_BatteryFuse[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,16,i+1);
                    b_ERR_DC_BatteryFuse[group][i] = ERR_DC_BatteryFuse[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,16,i+1);
                    b_ERR_DC_BatteryFuse[group][i] = ERR_DC_BatteryFuse[group][i];
                }
            }
            else if (b_ERR_DC_BatteryFuse[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,16,i+1);
                }           
            }
        }

        //	for(i=0;i<2;i++){
        i=0;
        if (group == 0){
            if(ERR_DC_JY_detection[group][i] != b_ERR_DC_JY_detection[group][i]){ // ��Ե��ⵥԪ����(һ�ξ�Ե�ܹ���)
                if(b_ERR_DC_JY_detection[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,34,0);
                    b_ERR_DC_JY_detection[group][i] = ERR_DC_JY_detection[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,34,0);
                    b_ERR_DC_JY_detection[group][i] = ERR_DC_JY_detection[group][i];
                }
            }
            else if (b_ERR_DC_JY_detection[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,34,0);
                }           
            }
        }else if (group == 1){
            if(ERR_DC_JY_detection[group][i] != b_ERR_DC_JY_detection[group][i]){ // ��Ե��ⵥԪ����(���ξ�Ե�ܹ���)
                if(b_ERR_DC_JY_detection[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,35,0);
                    b_ERR_DC_JY_detection[group][i] = ERR_DC_JY_detection[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,35,0);
                    b_ERR_DC_JY_detection[group][i] = ERR_DC_JY_detection[group][i];
                }
            }
            else if (b_ERR_DC_JY_detection[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,35,0);
                }           
            }
        }

        //	}
        if(ERR_DC_Battery_EQ_timeout[group] != b_ERR_DC_Battery_EQ_timeout[group]){	//��ؾ��峬ʱ
            if(b_ERR_DC_Battery_EQ_timeout[group] == 0){
                Add_LinkList(DC_ERR,group+1,18,0);
                b_ERR_DC_Battery_EQ_timeout[group] = ERR_DC_Battery_EQ_timeout[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,18,0);
                b_ERR_DC_Battery_EQ_timeout[group] = ERR_DC_Battery_EQ_timeout[group];
            }
        }
        else if (b_ERR_DC_Battery_EQ_timeout[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,18,0);
            }           
        }

        if(ERR_DC_Battery_QY[group] != b_ERR_DC_Battery_QY[group]){		//�����Ƿѹ
            if(b_ERR_DC_Battery_QY[group] == 0){
                Add_LinkList(DC_ERR,group+1,19,0);
                b_ERR_DC_Battery_QY[group] = ERR_DC_Battery_QY[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,19,0);
                b_ERR_DC_Battery_QY[group] = ERR_DC_Battery_QY[group];
            }	
        }
        else if (b_ERR_DC_Battery_QY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,19,0);
            }           
        }

#if 1	
        if(ERR_DC_Battery_GY[group] != b_ERR_DC_Battery_GY[group]){		//������ѹ
            if(b_ERR_DC_Battery_GY[group] == 0){
                Add_LinkList(DC_ERR,group+1,40,0);
                b_ERR_DC_Battery_GY[group] = ERR_DC_Battery_GY[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,40,0);
                b_ERR_DC_Battery_GY[group] = ERR_DC_Battery_GY[group];
            }	
        }
        else if (b_ERR_DC_Battery_GY[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,40,0);
            }           
        }
#endif 

        for(i=0;i<2;i++){
            if(ERR_DC_BatteryPolling_comm[group][i] != b_ERR_DC_BatteryPolling_comm[group][i]){	// ��ؼ�ⵥԪͨѶ����
                if(b_ERR_DC_BatteryPolling_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,20,i+1);
                    b_ERR_DC_BatteryPolling_comm[group][i] = ERR_DC_BatteryPolling_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,20,i+1);
                    b_ERR_DC_BatteryPolling_comm[group][i] = ERR_DC_BatteryPolling_comm[group][i];
                }
            }
            else if (b_ERR_DC_BatteryPolling_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,20,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_DC_JY_Sample_comm[group][i] != b_ERR_DC_JY_Sample_comm[group][i]){			//��Ե������ԪͨѶ����
                if(b_ERR_DC_JY_Sample_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,21,i+1);
                    b_ERR_DC_JY_Sample_comm[group][i] = ERR_DC_JY_Sample_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,21,i+1);
                    b_ERR_DC_JY_Sample_comm[group][i] = ERR_DC_JY_Sample_comm[group][i];
                }
            }
            else if (b_ERR_DC_JY_Sample_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,21,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){		                                                                //�����߿�����բ
            for(j=0;j<30;j++){
                if(ERR_DC_SW_trip[group][i][j] != b_ERR_DC_SW_trip[group][i][j]){
                    if(b_ERR_DC_SW_trip[group][i][j] == 0){
                        num = (i<<8)+j+1;
                        Add_LinkList(DC_ERR,group+1,24,num);
                        b_ERR_DC_SW_trip[group][i][j] = ERR_DC_SW_trip[group][i][j];
                        Dc_info[group].ERR_DC_muxian = 2;
                        flag_sw = 1;
                    }else {
                        num = (i<<8)+j+1;
                        Delete_LinkList(DC_ERR,group+1,24,num);
                        b_ERR_DC_SW_trip[group][i][j] = ERR_DC_SW_trip[group][i][j];
                    }
                }
                else if (b_ERR_DC_SW_trip[group][i][j] == 0)
                {
                    if ((Light_DC > 0) && Fg_ClearCurrentError) 
                    {        
                        num = (i<<8)+j+1;
                        Delete_LinkList(DC_ERR,group+1,24,num);
                    }           
                }
            }
        }

        if(flag_sw == 0){
            if(Dc_info[group].ERR_DC_muxian == 2)
                Dc_info[group].ERR_DC_muxian = 0;
        }

        for(i=0;i<16;i++){
            if(ERR_DC_SW_Sample_comm[group][i] != b_ERR_DC_SW_Sample_comm[group][i]){			//��������ԪͨѶ����
                if(b_ERR_DC_SW_Sample_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,22,i+1);
                    b_ERR_DC_SW_Sample_comm[group][i] = ERR_DC_SW_Sample_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,22,i+1);
                    b_ERR_DC_SW_Sample_comm[group][i] = ERR_DC_SW_Sample_comm[group][i];
                }
            }
            else if (b_ERR_DC_SW_Sample_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,22,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_DC_St_Sample_comm[group][i] != b_ERR_DC_St_Sample_comm[group][i]){			//״̬����ԪͨѶ����
                if(b_ERR_DC_St_Sample_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,23,i+1);
                    b_ERR_DC_St_Sample_comm[group][i] = ERR_DC_St_Sample_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,23,i+1);
                    b_ERR_DC_St_Sample_comm[group][i] = ERR_DC_St_Sample_comm[group][i];
                }
            }
            else if (b_ERR_DC_St_Sample_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,23,i+1);
                }           
            }
        }

        for(i=0;i<127;i++){
            if(ERR_DC_BatterySingle_GY[group][i] != b_ERR_DC_BatterySingle_GY[group][i]){		//����鵥���ع�ѹ
                if(b_ERR_DC_BatterySingle_GY[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,25,i+1);
                    b_ERR_DC_BatterySingle_GY[group][i] = ERR_DC_BatterySingle_GY[group][i];	
                }else{
                    Delete_LinkList(DC_ERR,group+1,25,i+1);
                    b_ERR_DC_BatterySingle_GY[group][i] = ERR_DC_BatterySingle_GY[group][i];
                }	
            }
            else if (b_ERR_DC_BatterySingle_GY[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,25,i+1);
                }           
            }

            if(ERR_DC_BatterySingle_QY[group][i] != b_ERR_DC_BatterySingle_QY[group][i]){		//����鵥����Ƿѹ
                if(b_ERR_DC_BatterySingle_QY[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,26,i+1);
                    b_ERR_DC_BatterySingle_QY[group][i] = ERR_DC_BatterySingle_QY[group][i];	
                }else{
                    Delete_LinkList(DC_ERR,group+1,26,i+1);
                    b_ERR_DC_BatterySingle_QY[group][i] = ERR_DC_BatterySingle_QY[group][i];
                }	
            }
            else if (b_ERR_DC_BatterySingle_QY[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,26,i+1);
                }           
            }

#if 1  //����
            if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
            {
                //����鵥�������賬��
                if(ERR_DC_BatterySingle_ResOver[group][i] != b_ERR_DC_BatterySingle_ResOver[group][i])
                {		
                    if (b_ERR_DC_BatterySingle_ResOver[group][i] == 0)
                    {
                        Add_LinkList(DC_ERR, group + 1, 41, i + 1);
                    }
                    else
                    {
                        Delete_LinkList(DC_ERR, group + 1, 41, i + 1);
                    }
                    b_ERR_DC_BatterySingle_ResOver[group][i] = ERR_DC_BatterySingle_ResOver[group][i];	    
                }
                else if (b_ERR_DC_BatterySingle_ResOver[group][i] == 0)
                {
                    if ((Light_DC > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(DC_ERR, group + 1, 41, i + 1);
                    }           
                }
            }
#endif 
        }

#if 1   //����
        if (Fg_SysSet_BatteryCheckMode == 0) //�����ĵ��Ѳ��: PSMX-B
        {
            for (i = 0; i < 4; i ++)
            {
                //����������ͨѶ����
                if (ERR_DC_BatterySamplingBox_CommError[group][i] != b_ERR_DC_BatterySamplingBox_CommError[group][i])
                {		
                    if (b_ERR_DC_BatterySamplingBox_CommError[group][i] == 0)
                    {
                        Add_LinkList(DC_ERR, group + 1, 42, i + 1);
                    }
                    else
                    {
                        Delete_LinkList(DC_ERR, group + 1, 42, i + 1);
                    }
                    b_ERR_DC_BatterySamplingBox_CommError[group][i] = ERR_DC_BatterySamplingBox_CommError[group][i];    
                }
                else if (b_ERR_DC_BatterySamplingBox_CommError[group][i] == 0)
                {
                    if ((Light_DC > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(DC_ERR, group + 1, 42, i + 1);
                    }           
                }            
            }
        }
#endif 

        if (Sys_cfg_info.dc_set[group].state_monitor_num <= 16)  //С�ڵ���16��
        {
            for(i=0;i<Sys_cfg_info.dc_set[group].state_monitor_num ;i++)
            {		//��Ե�ӵ�
                for(j=0;j<32;j++)
                {
                    if(ERR_DC_SW_K_JY[group][i][j] != b_ERR_DC_SW_K_JY[group][i][j])
                    {
                        if(b_ERR_DC_SW_K_JY[group][i][j] == 0)
                        {
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Add_LinkList(DC_ERR,group+1,28,num);
                            else
                                Add_LinkList(DC_ERR,group+1,36,num);

                            b_ERR_DC_SW_K_JY[group][i][j] = ERR_DC_SW_K_JY[group][i][j];

                        }
                        else
                        {
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                            b_ERR_DC_SW_K_JY[group][i][j] = ERR_DC_SW_K_JY[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_K_JY[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                        }           
                    }

                    if(ERR_DC_SW_H_JY[group][i][j] != b_ERR_DC_SW_H_JY[group][i][j])
                    {
                        if(b_ERR_DC_SW_H_JY[group][i][j] == 0){
                            num = (i<<8)+j+1;
                            Add_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY[group][i][j] = ERR_DC_SW_H_JY[group][i][j];
                        }else{
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY[group][i][j] = ERR_DC_SW_H_JY[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_H_JY[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                        }           
                    }
                }
            }                
        }
        else if (Sys_cfg_info.dc_set[group].state_monitor_num <= 40) //17~40��
        {
            for(i=0;i<16;i++)
            {		//��Ե�ӵ�
                for(j=0;j<32;j++)
                {
                    if(ERR_DC_SW_K_JY[group][i][j] != b_ERR_DC_SW_K_JY[group][i][j])
                    {
                        if(b_ERR_DC_SW_K_JY[group][i][j] == 0)
                        {
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Add_LinkList(DC_ERR,group+1,28,num);
                            else
                                Add_LinkList(DC_ERR,group+1,36,num);

                            b_ERR_DC_SW_K_JY[group][i][j] = ERR_DC_SW_K_JY[group][i][j];

                        }
                        else
                        {
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                            b_ERR_DC_SW_K_JY[group][i][j] = ERR_DC_SW_K_JY[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_K_JY[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = (i<<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                        }           
                    }

                    if(ERR_DC_SW_H_JY[group][i][j] != b_ERR_DC_SW_H_JY[group][i][j])
                    {
                        if(b_ERR_DC_SW_H_JY[group][i][j] == 0){
                            num = (i<<8)+j+1;
                            Add_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY[group][i][j] = ERR_DC_SW_H_JY[group][i][j];
                        }else{
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY[group][i][j] = ERR_DC_SW_H_JY[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_H_JY[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                        }           
                    }
                }
            }   

            for(i=0;i<(Sys_cfg_info.dc_set[group].state_monitor_num - 16);i++)
            {		//��Ե�ӵ�
                for(j=0;j<32;j++)
                {
                    if(ERR_DC_SW_K_JY_Add[group][i][j] != b_ERR_DC_SW_K_JY_Add[group][i][j])
                    {
                        if(b_ERR_DC_SW_K_JY_Add[group][i][j] == 0)
                        {
                            num = ((i + 16) <<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Add_LinkList(DC_ERR,group+1,28,num);
                            else
                                Add_LinkList(DC_ERR,group+1,36,num);

                            b_ERR_DC_SW_K_JY_Add[group][i][j] = ERR_DC_SW_K_JY_Add[group][i][j];

                        }
                        else
                        {
                            num = ((i + 16) <<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                            b_ERR_DC_SW_K_JY_Add[group][i][j] = ERR_DC_SW_K_JY_Add[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_K_JY_Add[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = ((i + 16) <<8)+j+1;
                            if(SYS_JY_J3 == 2)
                                Delete_LinkList(DC_ERR,group+1,28,num);
                            else
                                Delete_LinkList(DC_ERR,group+1,36,num);
                        }           
                    }

                    if(ERR_DC_SW_H_JY_Add[group][i][j] != b_ERR_DC_SW_H_JY_Add[group][i][j])
                    {
                        if(b_ERR_DC_SW_H_JY_Add[group][i][j] == 0){
                            num = ((i + 16) <<8)+j+1;
                            Add_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY_Add[group][i][j] = ERR_DC_SW_H_JY_Add[group][i][j];
                        }else{
                            num = ((i + 16) <<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                            b_ERR_DC_SW_H_JY_Add[group][i][j] = ERR_DC_SW_H_JY_Add[group][i][j];
                        }
                    }
                    else if (b_ERR_DC_SW_H_JY_Add[group][i][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = ((i + 16) <<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,27,num);
                        }           
                    }
                }
            } 
        }

        for(i=0;i<30;i++){
            if(ERR_DC_Reserve[group][i] != b_ERR_DC_Reserve[group][i]){			//Ԥ��
                if(b_ERR_DC_Reserve[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,31,i+1);
                    b_ERR_DC_Reserve[group][i] = ERR_DC_Reserve[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,31,i+1);
                    b_ERR_DC_Reserve[group][i] = ERR_DC_Reserve[group][i];
                }
            }
            else if (b_ERR_DC_Reserve[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,31,i+1);
                }           
            }
        }

        if(ERR_DC_JY_VF[group] != b_ERR_DC_JY_VF[group]){		//��طŵ�
            if(b_ERR_DC_JY_VF[group] == 0){
                Add_LinkList(DC_ERR,group+1,32+group,0);
                b_ERR_DC_JY_VF[group] = ERR_DC_JY_VF[group];	
            }else{
                Delete_LinkList(DC_ERR,group+1,32+group,0);
                b_ERR_DC_AcSPD[group] = ERR_DC_JY_VF[group];
            }	
        }
        else if (b_ERR_DC_JY_VF[group] == 0)
        {
            if ((Light_DC > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(DC_ERR,group+1,32+group,0);
            }           
        }

        for(i = 0; i < 64; i++){  //����64�� 
            if(ERR_DC_standard_cell_comm[group][i] != b_ERR_DC_standard_cell_comm[group][i]){ //��׼��Ԫͨ�Ź���
                if(b_ERR_DC_standard_cell_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,38,i+1);
                    b_ERR_DC_standard_cell_comm[group][i] = ERR_DC_standard_cell_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,38,i+1);
                    b_ERR_DC_standard_cell_comm[group][i] = ERR_DC_standard_cell_comm[group][i];
                }
            }
            else if (b_ERR_DC_standard_cell_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,38,i+1);
                }           
            }
        }

        for(i=0;i<10;i++){
            if(ERR_DC_FG_comm[group][i] != b_ERR_DC_FG_comm[group][i]){			//�ֹ���ͨ�Ź���
                if(b_ERR_DC_FG_comm[group][i] == 0){
                    Add_LinkList(DC_ERR,group+1,39,i+1);
                    b_ERR_DC_FG_comm[group][i] = ERR_DC_FG_comm[group][i];
                }else{
                    Delete_LinkList(DC_ERR,group+1,39,i+1);
                    b_ERR_DC_FG_comm[group][i] = ERR_DC_FG_comm[group][i];
                }
            }
            else if (b_ERR_DC_FG_comm[group][i] == 0)
            {
                if ((Light_DC > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(DC_ERR,group+1,39,i+1);
                }           
            }
        }

        if ((Sys_cfg_info.dc_set[0].state_monitor_num > 16) || (Sys_cfg_info.dc_set[1].state_monitor_num > 16))
        {
            for(i = 16; i < 40; i++)
            {  //����40��
                for(j=0;j<16;j++)
                {
                    if(ERR_DC_FG_SW_trip[group][i-16][j] != b_ERR_DC_FG_SW_trip[group][i-16][j]){
                        if(b_ERR_DC_FG_SW_trip[group][i-16][j] == 0){
                            num = (i<<8)+j+1;
                            Add_LinkList(DC_ERR,group+1,24,num);
                            b_ERR_DC_FG_SW_trip[group][i-16][j] = ERR_DC_FG_SW_trip[group][i-16][j];
                        }else {
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,24,num);
                            b_ERR_DC_FG_SW_trip[group][i-16][j] = ERR_DC_FG_SW_trip[group][i-16][j];
                        }
                    }
                    else if (b_ERR_DC_FG_SW_trip[group][i-16][j] == 0)
                    {
                        if ((Light_DC > 0) && Fg_ClearCurrentError) 
                        {        
                            num = (i<<8)+j+1;
                            Delete_LinkList(DC_ERR,group+1,24,num);
                        }           
                    }
                }
            }
        }
    }
}

/**************************************************************************
 *������:			Fault_Comm_handling
 *��������:		����ͨ����ع��ϱ�־
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void Fault_Comm_handling()
{
    int group = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    for(group=0;group<4;group++){
        if (group == 0){
            for(i=0;i<12;i++){
                if(ERR_comm_BatterySingle_GY[group][i] != b_ERR_comm_BatterySingle_GY[group][i]){		//����鵥���ع�ѹ
                    if(b_ERR_comm_BatterySingle_GY[group][i] == 0){
                        Add_LinkList(Comm_ERR,group+1,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i] = ERR_comm_BatterySingle_GY[group][i];	
                    }else{
                        Delete_LinkList(Comm_ERR,group+1,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i] = ERR_comm_BatterySingle_GY[group][i];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_GY[group][i] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group+1,18,i+1);
                    }           
                }

                if(ERR_comm_BatterySingle_QY[group][i] != b_ERR_comm_BatterySingle_QY[group][i]){		//����鵥����Ƿѹ
                    if(b_ERR_comm_BatterySingle_QY[group][i] == 0){
                        Add_LinkList(Comm_ERR,group+1,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i] = ERR_comm_BatterySingle_QY[group][i];	
                    }else{
                        Delete_LinkList(Comm_ERR,group+1,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i] = ERR_comm_BatterySingle_QY[group][i];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_QY[group][i] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group+1,19,i+1);
                    }           
                }
            }
        }	
        else if (group == 1){
            for(i=12;i<24;i++){
                if(ERR_comm_BatterySingle_GY[group][i-12] != b_ERR_comm_BatterySingle_GY[group][i-12]){		//����鵥���ع�ѹ
                    if(b_ERR_comm_BatterySingle_GY[group][i-12] == 0){
                        Add_LinkList(Comm_ERR,group,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i-12] = ERR_comm_BatterySingle_GY[group][i-12];	
                    }else{
                        Delete_LinkList(Comm_ERR,group,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i-12] = ERR_comm_BatterySingle_GY[group][i-12];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_GY[group][i-12] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group,18,i+1);
                    }           
                }

                if(ERR_comm_BatterySingle_QY[group][i-12] != b_ERR_comm_BatterySingle_QY[group][i-12]){		//����鵥����Ƿѹ
                    if(b_ERR_comm_BatterySingle_QY[group][i-12] == 0){
                        Add_LinkList(Comm_ERR,group,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i-12] = ERR_comm_BatterySingle_QY[group][i-12];	
                    }else{
                        Delete_LinkList(Comm_ERR,group,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i-12] = ERR_comm_BatterySingle_QY[group][i-12];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_QY[group][i-12] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group,19,i+1);
                    }           
                }
            }
        }
        else if (group == 2){
            for(i=0;i<12;i++){
                if(ERR_comm_BatterySingle_GY[group][i] != b_ERR_comm_BatterySingle_GY[group][i]){		//����鵥���ع�ѹ
                    if(b_ERR_comm_BatterySingle_GY[group][i] == 0){
                        Add_LinkList(Comm_ERR,group,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i] = ERR_comm_BatterySingle_GY[group][i];	
                    }else{
                        Delete_LinkList(Comm_ERR,group,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i] = ERR_comm_BatterySingle_GY[group][i];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_GY[group][i] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group,18,i+1);
                    }           
                }

                if(ERR_comm_BatterySingle_QY[group][i] != b_ERR_comm_BatterySingle_QY[group][i]){		//����鵥����Ƿѹ
                    if(b_ERR_comm_BatterySingle_QY[group][i] == 0){
                        Add_LinkList(Comm_ERR,group,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i] = ERR_comm_BatterySingle_QY[group][i];	
                    }else{
                        Delete_LinkList(Comm_ERR,group,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i] = ERR_comm_BatterySingle_QY[group][i];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_QY[group][i] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group,19,i+1);
                    }           
                }
            }
        }
        else if (group == 3){
            for(i=12;i<24;i++){
                if(ERR_comm_BatterySingle_GY[group][i-12] != b_ERR_comm_BatterySingle_GY[group][i-12]){		//����鵥���ع�ѹ
                    if(b_ERR_comm_BatterySingle_GY[group][i-12] == 0){
                        Add_LinkList(Comm_ERR,group-1,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i-12] = ERR_comm_BatterySingle_GY[group][i-12];	
                    }else{
                        Delete_LinkList(Comm_ERR,group-1,18,i+1);
                        b_ERR_comm_BatterySingle_GY[group][i-12] = ERR_comm_BatterySingle_GY[group][i-12];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_GY[group][i-12] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group-1,18,i+1);
                    }           
                }                
                if(ERR_comm_BatterySingle_QY[group][i-12] != b_ERR_comm_BatterySingle_QY[group][i-12]){		//����鵥����Ƿѹ
                    if(b_ERR_comm_BatterySingle_QY[group][i-12] == 0){
                        Add_LinkList(Comm_ERR,group-1,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i-12] = ERR_comm_BatterySingle_QY[group][i-12];	
                    }else{
                        Delete_LinkList(Comm_ERR,group-1,19,i+1);
                        b_ERR_comm_BatterySingle_QY[group][i-12] = ERR_comm_BatterySingle_QY[group][i-12];
                    }	
                }
                else if (b_ERR_comm_BatterySingle_QY[group][i-12] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group-1,19,i+1);
                    }           
                } 
            }
        }
    }
    for(group=0;group<2;group++){
        for(i=0;i<16;i++){
            if(ERR_Comm_SW_specialty[group][i] != b_ERR_Comm_SW_specialty[group][i]){			//ͨ��ģ�����������բ
                if(b_ERR_Comm_SW_specialty[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,1,i+1);
                    b_ERR_Comm_SW_specialty[group][i] = ERR_Comm_SW_specialty[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,1,i+1);
                    b_ERR_Comm_SW_specialty[group][i] = ERR_Comm_SW_specialty[group][i];
                }
            }
            else if (b_ERR_Comm_SW_specialty[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,1,i+1);
                }           
            }
        }

        for(i=0;i<8;i++){
            for(j=0;j<32;j++){
                if(ERR_Comm_SW_trip[group][i][j] != b_ERR_Comm_SW_trip[group][i][j]){			// 1-4��ͨ�ŵ�Դ���߿�����բ
                    if(b_ERR_Comm_SW_trip[group][i][j] == 0){
                        Add_LinkList(Comm_ERR,group+1,0,(i<<8)+j+1);
                        b_ERR_Comm_SW_trip[group][i][j] = ERR_Comm_SW_trip[group][i][j];
                    }else{
                        Delete_LinkList(Comm_ERR,group+1,0,(i<<8)+j+1);
                        b_ERR_Comm_SW_trip[group][i][j] = ERR_Comm_SW_trip[group][i][j];
                    }
                }
                else if (b_ERR_Comm_SW_trip[group][i][j] == 0)
                {
                    if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                    {        
                        Delete_LinkList(Comm_ERR,group+1,0,(i<<8)+j+1);
                    }           
                }
            }
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_comm[group][i] != b_ERR_Comm_Module_comm[group][i]){			//ͨ��ģ��ͨѶ����
                if(b_ERR_Comm_Module_comm[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,2,i+1);
                    b_ERR_Comm_Module_comm[group][i] = ERR_Comm_Module_comm[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,2,i+1);
                    b_ERR_Comm_Module_comm[group][i] = ERR_Comm_Module_comm[group][i];
                }
            }
            else if (b_ERR_Comm_Module_comm[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,2,i+1);
                }           
            }
        }

        for(i=0;i<14;i++){
            if(ERR_Comm_Module[group][i] != b_ERR_Comm_Module[group][i]){			//ͨ��ģ�����
                if(b_ERR_Comm_Module[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,3,i+1);
                    b_ERR_Comm_Module[group][i] = ERR_Comm_Module[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,3,i+1);
                    b_ERR_Comm_Module[group][i] = ERR_Comm_Module[group][i];
                }
            }
            else if (b_ERR_Comm_Module[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,3,i+1);
                }           
            }
        }

        if(ERR_Comm_Module[group][14] != b_ERR_Comm_Module[group][14]){			//ͨ��ģ��ֱ������쳣
            if(b_ERR_Comm_Module[group][14] == 0){
                Add_LinkList(Comm_ERR,group+1,8,0);
                b_ERR_Comm_Module[group][14] = ERR_Comm_Module[group][14];
            }else{
                Delete_LinkList(Comm_ERR,group+1,8,0);
                b_ERR_Comm_Module[group][14] = ERR_Comm_Module[group][14];
            }
        }
        else if (b_ERR_Comm_Module[group][14] == 0)
        {
            if ((Light_Comm > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Comm_ERR,group+1,8,0);
            }           
        }


        if(ERR_Comm_Module[group][15] != b_ERR_Comm_Module[group][15]){			//ͨ��ģ��ֱ�������쳣
            if(b_ERR_Comm_Module[group][15] == 0){
                Add_LinkList(Comm_ERR,group+1,9,0);
                b_ERR_Comm_Module[group][15] = ERR_Comm_Module[group][15];
            }else{
                Delete_LinkList(Comm_ERR,group+1,9,0);
                b_ERR_Comm_Module[group][15] = ERR_Comm_Module[group][15];
            }
        }
        else if (b_ERR_Comm_Module[group][15] == 0)
        {
            if ((Light_Comm > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Comm_ERR,group+1,9,0);
            }           
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_output_GY[group][i] != b_ERR_Comm_Module_output_GY[group][i]){			//ͨ��ģ�������ѹ�ػ�
                if(b_ERR_Comm_Module_output_GY[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,11,i+1);
                    b_ERR_Comm_Module_output_GY[group][i] = ERR_Comm_Module_output_GY[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,11,i+1);
                    b_ERR_Comm_Module_output_GY[group][i] = ERR_Comm_Module_output_GY[group][i];
                }
            }
            else if (b_ERR_Comm_Module_output_GY[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,11,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_output_QY[group][i] != b_ERR_Comm_Module_output_QY[group][i]){			//ͨ��ģ�����Ƿѹ
                if(b_ERR_Comm_Module_output_QY[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,12,i+1);
                    b_ERR_Comm_Module_output_QY[group][i] = ERR_Comm_Module_output_QY[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,12,i+1);
                    b_ERR_Comm_Module_output_QY[group][i] = ERR_Comm_Module_output_QY[group][i];
                }
            }
            else if (b_ERR_Comm_Module_output_QY[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,12,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_AC_import_GY[group][i] != b_ERR_Comm_Module_AC_import_GY[group][i]){			//ͨ��ģ�齻�������ѹ
                if(b_ERR_Comm_Module_AC_import_GY[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,13,i+1);
                    b_ERR_Comm_Module_AC_import_GY[group][i] = ERR_Comm_Module_AC_import_GY[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,13,i+1);
                    b_ERR_Comm_Module_AC_import_GY[group][i] = ERR_Comm_Module_AC_import_GY[group][i];
                }
            }
            else if (b_ERR_Comm_Module_AC_import_GY[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,13,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_AC_import_QY[group][i] != b_ERR_Comm_Module_AC_import_QY[group][i]){			//ͨ��ģ�齻������Ƿѹ
                if(b_ERR_Comm_Module_AC_import_QY[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,14,i+1);
                    b_ERR_Comm_Module_AC_import_QY[group][i] = ERR_Comm_Module_AC_import_QY[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,14,i+1);
                    b_ERR_Comm_Module_AC_import_QY[group][i] = ERR_Comm_Module_AC_import_QY[group][i];
                }
            }
            else if (b_ERR_Comm_Module_AC_import_QY[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,14,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_Comm_Module_bu_advection[group][i] != b_ERR_Comm_Module_bu_advection[group][i]){			//ͨ��ģ�鲻����
                if(b_ERR_Comm_Module_bu_advection[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,15,i+1);
                    b_ERR_Comm_Module_bu_advection[group][i] = ERR_Comm_Module_bu_advection[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,15,i+1);
                    b_ERR_Comm_Module_bu_advection[group][i] = ERR_Comm_Module_bu_advection[group][i];
                }
            }
            else if (b_ERR_Comm_Module_bu_advection[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,15,i+1);
                }           
            }
        }

        for(i=0;i<16;i++){
            if(ERR_LN_Comm_Module[group][i] != b_ERR_LN_Comm_Module[group][i]){			//ͨ��ģ�����
                if(b_ERR_LN_Comm_Module[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,16,i+1);
                    b_ERR_LN_Comm_Module[group][i] = ERR_LN_Comm_Module[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,16,i+1);
                    b_ERR_LN_Comm_Module[group][i] = ERR_LN_Comm_Module[group][i];
                }
            }
            else if (b_ERR_LN_Comm_Module[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,16,i+1);
                }           
            }

        }

        for(i=0;i<16;i++){
            if(ERR_LN_Comm_Module_comm[group][i] != b_ERR_LN_Comm_Module_comm[group][i]){			//ͨ��ģ��ͨѶ����
                if(b_ERR_LN_Comm_Module_comm[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,17,i+1);
                    b_ERR_LN_Comm_Module_comm[group][i] = ERR_LN_Comm_Module_comm[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,17,i+1);
                    b_ERR_LN_Comm_Module_comm[group][i] = ERR_LN_Comm_Module_comm[group][i];
                }
            }
            else if (b_ERR_LN_Comm_Module_comm[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,17,i+1);
                }           
            }
        }

        for(i=0;i<8;i++){
            if(ERR_Comm_SW_comm[group][i] != b_ERR_Comm_SW_comm[group][i]){			//��������ԪͨѶ����
                if(b_ERR_Comm_SW_comm[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,4,i+1);
                    b_ERR_Comm_SW_comm[group][i] = ERR_Comm_SW_comm[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,4,i+1);
                    b_ERR_Comm_SW_comm[group][i] = ERR_Comm_SW_comm[group][i];
                }
            }
            else if (b_ERR_Comm_SW_comm[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,4,i+1);
                }           
            }
        }

        for(i=0;i<8;i++){
            if(ERR_Comm_St_comm[group][i] != b_ERR_Comm_St_comm[group][i]){			//״̬����ԪͨѶ����
                if(b_ERR_Comm_St_comm[group][i] == 0){
                    Add_LinkList(Comm_ERR,group+1,5,i+1);
                    b_ERR_Comm_St_comm[group][i] = ERR_Comm_St_comm[group][i];
                }else{
                    Delete_LinkList(Comm_ERR,group+1,5,i+1);
                    b_ERR_Comm_St_comm[group][i] = ERR_Comm_St_comm[group][i];
                }
            }
            else if (b_ERR_Comm_St_comm[group][i] == 0)
            {
                if ((Light_Comm > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(Comm_ERR,group+1,5,i+1);
                }           
            }
        }

        if(ERR_Comm_Feeder_sw[group] != b_ERR_Comm_Feeder_sw[group]){			//���߿��ضϿ�
            if(b_ERR_Comm_Feeder_sw[group] == 0){
                Add_LinkList(Comm_ERR,group+1,6,0);
                b_ERR_Comm_Feeder_sw[group] = ERR_Comm_Feeder_sw[group];
            }else{
                Delete_LinkList(Comm_ERR,group+1,6,0);
                b_ERR_Comm_Feeder_sw[group] = ERR_Comm_Feeder_sw[group];
            }
        }
        else if (b_ERR_Comm_Feeder_sw[group] == 0)
        {
            if ((Light_Comm > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Comm_ERR,group+1,6,0);
            }           
        }

        if(ERR_Comm_SPD[group] != b_ERR_Comm_SPD[group]){			//ͨ�ŵ�Դ����������
            if(b_ERR_Comm_SPD[group] == 0){
                Add_LinkList(Comm_ERR,group+1,7,0);
                b_ERR_Comm_SPD[group] = ERR_Comm_SPD[group];
            }else{
                Delete_LinkList(Comm_ERR,group+1,7,0);
                b_ERR_Comm_SPD[group] = ERR_Comm_SPD[group];
            }
        }
        else if (b_ERR_Comm_SPD[group] == 0)
        {
            if ((Light_Comm > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(Comm_ERR,group+1,7,0);
            }           
        }
    }
}
/**************************************************************************
 *������:			Fault_UPS_handling
 *��������:		����UPS��ع��ϱ�־
 *����:			void
 *����ֵ:			void
 ***************************************************************************/
void Fault_UPS_handling()
{
    //int group = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    for(i=0;i<5;i++)
    {
        if(ERR_UPS_Bypass[i] != b_ERR_UPS_Bypass[i]){
            if(b_ERR_UPS_Bypass[i] == 0){
                Add_LinkList(UPS_ERR,i+1,0,0);
                b_ERR_UPS_Bypass[i] = ERR_UPS_Bypass[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,0,0);
                b_ERR_UPS_Bypass[i] = ERR_UPS_Bypass[i];
            }
        }
        else if (b_ERR_UPS_Bypass[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,0,0);
            }           
        }

        if(ERR_UPS_Overload[i] != b_ERR_UPS_Overload[i]){
            if(b_ERR_UPS_Overload[i] == 0){
                Add_LinkList(UPS_ERR,i+1,1,0);
                b_ERR_UPS_Overload[i] = ERR_UPS_Overload[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,1,0);
                b_ERR_UPS_Overload[i] = ERR_UPS_Overload[i];
            }
        }
        else if (b_ERR_UPS_Overload[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,1,0);
            }           
        }

        if(ERR_UPS[i] != b_ERR_UPS[i]){
            if(b_ERR_UPS[i] == 0){
                Add_LinkList(UPS_ERR,i+1,2,0);
                b_ERR_UPS[i] = ERR_UPS[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,2,0);
                b_ERR_UPS[i] = ERR_UPS[i];
            }
        }
        else if (b_ERR_UPS[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,2,0);
            }           
        }

        if(ERR_UPS_Bypass_output[i] != b_ERR_UPS_Bypass_output[i]){
            if(b_ERR_UPS_Bypass_output[i] == 0){
                Add_LinkList(UPS_ERR,i+1,3,0);
                b_ERR_UPS_Bypass_output[i] = ERR_UPS_Bypass_output[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,3,0);
                b_ERR_UPS_Bypass_output[i] = ERR_UPS_Bypass_output[i];
            }
        }
        else if (b_ERR_UPS_Bypass_output[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,3,0);
            }           
        }

        if(ERR_UPS_DC[i] != b_ERR_UPS_DC[i]){
            if(b_ERR_UPS_DC[i] == 0){
                Add_LinkList(UPS_ERR,i+1,4,0);
                b_ERR_UPS_DC[i] = ERR_UPS_DC[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,4,0);
                b_ERR_UPS_DC[i] = ERR_UPS_DC[i];
            }
        }
        else if (b_ERR_UPS_DC[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,4,0);
            }           
        }

        if(ERR_UPS_MainsSupply[i] != b_ERR_UPS_MainsSupply[i]){
            if(b_ERR_UPS_MainsSupply[i] == 0){
                Add_LinkList(UPS_ERR,i+1,5,0);
                b_ERR_UPS_MainsSupply[i] = ERR_UPS_MainsSupply[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,5,0);
                b_ERR_UPS_MainsSupply[i] = ERR_UPS_MainsSupply[i];
            }
        }
        else if (b_ERR_UPS_MainsSupply[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,5,0);
            }           
        }

        if(ERR_UPS_Comm[i] != b_ERR_UPS_Comm[i]){
            if(b_ERR_UPS_Comm[i] == 0){
                Add_LinkList(UPS_ERR,i+1,6,0);
                b_ERR_UPS_Comm[i] = ERR_UPS_Comm[i];
            }else{
                Delete_LinkList(UPS_ERR,i+1,6,0);
                b_ERR_UPS_Comm[i] = ERR_UPS_Comm[i];
            }
        }
        else if (b_ERR_UPS_Comm[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,i+1,6,0);
            }           
        }
    }

    for(i=0;i<16;i++){    //1��16�ſ��أ�״̬���ͨѶ����
        if(ERR_UPS_SW_Comm[i] != b_ERR_UPS_SW_Comm[i]){
            if(b_ERR_UPS_SW_Comm[i] == 0){
                Add_LinkList(UPS_ERR,0,7,i+1);
                b_ERR_UPS_SW_Comm[i] = ERR_UPS_SW_Comm[i];
            }else{
                Delete_LinkList(UPS_ERR,0,7,i+1);
                b_ERR_UPS_SW_Comm[i] = ERR_UPS_SW_Comm[i];
            }
        }
        else if (b_ERR_UPS_SW_Comm[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,7,i+1);
            }           
        }

        if(ERR_UPS_State_Comm[i] != b_ERR_UPS_State_Comm[i]){
            if(b_ERR_UPS_State_Comm[i] == 0){
                Add_LinkList(UPS_ERR,0,8,i+1);
                b_ERR_UPS_State_Comm[i] = ERR_UPS_State_Comm[i];
            }else{
                Delete_LinkList(UPS_ERR,0,8,i+1);
                b_ERR_UPS_State_Comm[i] = ERR_UPS_State_Comm[i];
            }
        }
        else if (b_ERR_UPS_State_Comm[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,8,i+1);
            }           
        }
    }

    for(i=0;i<16;i++){
        for(j=0;j<32;j++){
            if(ERR_UPS_SW_trip[i][j] != b_ERR_UPS_SW_trip[i][j]){
                if(b_ERR_UPS_SW_trip[i][j] == 0){
                    Add_LinkList(UPS_ERR,0,9,(i<<8)+j+1);
                    b_ERR_UPS_SW_trip[i][j] = ERR_UPS_SW_trip[i][j];
                }else{
                    Delete_LinkList(UPS_ERR,0,9,(i<<8)+j+1);
                    b_ERR_UPS_SW_trip[i][j] = ERR_UPS_SW_trip[i][j];
                }
            }
            else if (b_ERR_UPS_SW_trip[i][j] == 0)
            {
                if ((Light_UPS > 0) && Fg_ClearCurrentError) 
                {        
                    Delete_LinkList(UPS_ERR,0,9,(i<<8)+j+1);
                }           
            }
        }
    }

    for(i=0;i<3;i++){
        if(ERR_UPS_AcInput_sw[i] != b_ERR_UPS_AcInput_sw[i]){
            if(b_ERR_UPS_AcInput_sw[i] == 0){
                Add_LinkList(UPS_ERR,0,10,i+1);
                b_ERR_UPS_AcInput_sw[i] = ERR_UPS_AcInput_sw[i];
            }else{
                Delete_LinkList(UPS_ERR,0,10,i+1);
                b_ERR_UPS_AcInput_sw[i] = ERR_UPS_AcInput_sw[i];
            }
        }
        else if (b_ERR_UPS_AcInput_sw[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,10,i+1);
            }           
        }

        if(ERR_UPS_DcInput_sw[i] != b_ERR_UPS_DcInput_sw[i]){
            if(b_ERR_UPS_DcInput_sw[i] == 0){
                Add_LinkList(UPS_ERR,0,11,i+1);
                b_ERR_UPS_DcInput_sw[i] = ERR_UPS_DcInput_sw[i];
            }else{
                Delete_LinkList(UPS_ERR,0,11,i+1);
                b_ERR_UPS_DcInput_sw[i] = ERR_UPS_DcInput_sw[i];
            }
        }
        else if (b_ERR_UPS_DcInput_sw[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,11,i+1);
            }           
        }

        if(ERR_UPS_BypassInput_sw[i] != b_ERR_UPS_BypassInput_sw[i]){
            if(b_ERR_UPS_BypassInput_sw[i] == 0){
                Add_LinkList(UPS_ERR,0,12,i+1);
                b_ERR_UPS_BypassInput_sw[i] = ERR_UPS_BypassInput_sw[i];
            }else{
                Delete_LinkList(UPS_ERR,0,12,i+1);
                b_ERR_UPS_BypassInput_sw[i] = ERR_UPS_BypassInput_sw[i];
            }
        }
        else if (b_ERR_UPS_BypassInput_sw[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,12,i+1);
            }           
        }

        if(ERR_UPS_AcOutput_sw[i] != b_ERR_UPS_AcOutput_sw[i]){
            if(b_ERR_UPS_AcOutput_sw[i] == 0){
                Add_LinkList(UPS_ERR,0,13,i+1);
                b_ERR_UPS_AcOutput_sw[i] = ERR_UPS_AcOutput_sw[i];
            }else{
                Delete_LinkList(UPS_ERR,0,13,i+1);
                b_ERR_UPS_AcOutput_sw[i] = ERR_UPS_AcOutput_sw[i];
            }
        }
        else if (b_ERR_UPS_AcOutput_sw[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,13,i+1);
            }           
        }

        if(ERR_UPS_Bypass_Overhaul_sw[i] != b_ERR_UPS_Bypass_Overhaul_sw[i]){
            if(b_ERR_UPS_Bypass_Overhaul_sw[i] == 0){
                Add_LinkList(UPS_ERR,0,14,i+1);
                b_ERR_UPS_Bypass_Overhaul_sw[i] = ERR_UPS_Bypass_Overhaul_sw[i];
            }else{
                Delete_LinkList(UPS_ERR,0,14,i+1);
                b_ERR_UPS_Bypass_Overhaul_sw[i] = ERR_UPS_Bypass_Overhaul_sw[i];
            }
        }
        else if (b_ERR_UPS_Bypass_Overhaul_sw[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR,0,14,i+1);
            }           
        }
    }

    if(ERR_UPS_Feeder_sw[0] != b_ERR_UPS_Feeder_sw[0]){  //ups���߿��ع���
        if(b_ERR_UPS_Feeder_sw[0] == 0){
            Add_LinkList(UPS_ERR,0,15,0);
            b_ERR_UPS_Feeder_sw[0] = ERR_UPS_Feeder_sw[0];
        }else{
            Delete_LinkList(UPS_ERR,0,15,0);
            b_ERR_UPS_Feeder_sw[0] = ERR_UPS_Feeder_sw[0];
        }
    }
    else if (b_ERR_UPS_Feeder_sw[0] == 0)
    {
        if ((Light_UPS > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(UPS_ERR,0,15,0);
        }           
    }

    if(ERR_UPS_SPD[0] != b_ERR_UPS_SPD[0]){    //ups����������
        if(b_ERR_UPS_SPD[0] == 0){
            Add_LinkList(UPS_ERR,0,16,0);
            b_ERR_UPS_SPD[0] = ERR_UPS_SPD[0];
        }else{
            Delete_LinkList(UPS_ERR,0,16,0);
            b_ERR_UPS_SPD[0] = ERR_UPS_SPD[0];
        }
    }
    else if (b_ERR_UPS_SPD[0] == 0)
    {
        if ((Light_UPS > 0) && Fg_ClearCurrentError) 
        {        
            Delete_LinkList(UPS_ERR,0,16,0);
        }           
    }

#if 1
    //ups����쳣
    for(i = 0; i < 2; i ++)
    {
        if(ERR_UPS_OutPut[i] != b_ERR_UPS_OutPut[i])
        {    
            if(b_ERR_UPS_OutPut[i] == 0)
            {
                Add_LinkList(UPS_ERR, 0, 19, i + 1);
                b_ERR_UPS_OutPut[i] = ERR_UPS_OutPut[i];
            } 
            else
            {
                Delete_LinkList(UPS_ERR, 0, 19, i + 1);
                b_ERR_UPS_OutPut[i] = ERR_UPS_OutPut[i];
            }
        }
        else if (b_ERR_UPS_OutPut[i] == 0)
        {
            if ((Light_UPS > 0) && Fg_ClearCurrentError) 
            {        
                Delete_LinkList(UPS_ERR, 0, 19, i + 1);
            }           
        }
    } 
#endif
}
