
/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 ���������жϺͲ�ѯ��ʽ���вɼ������浱ǰ����ͨ������״̬��������ʼ���Ͱ�����������ʾ����
*******************************************************************************/
#include "CH554.H"
#include "Debug.H"
#include "TouchKey.H"
#include <stdio.h>

#pragma  NOAREGS

#define	LED_PIN 4 
sbit	LED=P1^4;
main( )
{

	  
    CfgFsys( );                                                                //CH554ʱ��ѡ������
    mDelaymS(5);
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED_PIN);
  //  TouchKeyQueryCyl2ms();                                                     //TouchKey��ѯ����2ms
  //  GetTouckKeyFreeBuf();                                                      //��ȡ������׼ֵ

   UART1Setup();
     EA = 1;
 

	P3_DIR_PU=0xFF;
    while(1)
    {
        mDelaymS(500);
        LED=!LED;

	 
        CH554UART1SendByte('R');

 
    }

}