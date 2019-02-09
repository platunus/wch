
/********************************** (C) COPYRIGHT *******************************
* File Name          : DataFlash.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/6/24
* Description        : ��дCH559��DataFlash,CH55X���ڲ�Flash��˫�ֽ�д����ˣ������ֽڶ�
*******************************************************************************/
#include "..\DEBUG.C"                                                          //������Ϣ��ӡ
#include "..\DEBUG.H"
#include <intrins.h>

#pragma  NOAREGS

/*******************************************************************************
* Function Name  : EraseBlock(UINT16 Addr)
* Description    : CodeFlash�����(1KB)��ȫ������λд1
* Input          : UINT16 Addr
* Output         : None
* Return         : None
*******************************************************************************/
UINT8	EraseBlock( UINT16 Addr )
{
	ROM_ADDR = Addr;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                          // ������ַ��Ч
		ROM_CTRL = ROM_CMD_ERASE;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                           // ����״̬,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : ProgWord( UINT16 Addr, UINT16 Data )
* Description    : дEEPROM��˫�ֽ�д
* Input          : UNIT16 Addr,д��ַ
                   UINT16 Data,����
* Output         : None
* Return         : SUCESS 
*******************************************************************************/
UINT8	ProgWord( UINT16 Addr, UINT16 Data )
{
	ROM_ADDR = Addr;
	ROM_DATA = Data;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                           // ������ַ��Ч
		ROM_CTRL = ROM_CMD_PROG;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                            // ����״̬,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : EraseDataFlash(UINT16 Addr)
* Description    : DataFlash�����(1KB)��ȫ������λд1
* Input          : UINT16 Addr
* Output         : None
* Return         : UINT8 status
*******************************************************************************/
UINT8 EraseDataFlash(UINT16 Addr)
{
    UINT8 status;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG |= bDATA_WE;                                                    //ʹ��DataFlashд
    SAFE_MOD = 0;	
    status = EraseBlock(Addr);	
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG &= ~bDATA_WE;                                                   //����DataFlashд����
    SAFE_MOD = 0;	
    return status;
}

/*******************************************************************************
* Function Name  : WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
* Description    : DataFlashд
* Input          : UINT16 Addr��PUINT16 buf,UINT8 len
* Output         : None
* Return         : 
*******************************************************************************/
void WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
{
    UINT8 j;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG |= bDATA_WE;                                                    //ʹ��DataFlashд
    SAFE_MOD = 0;
    for(j=0;j<len;j++)
    {
        ProgWord(Addr,buf[j]);
        Addr = Addr + 2;
    }
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG &= ~bDATA_WE;                                                   //����DataFlashд����
    SAFE_MOD = 0;
}

void main()
{
    UINT8X buf[50];
    UINT8 ret;
    UINT16 j,i;
    i = 0;

//  CfgFsys( );                                                                //CH559ʱ��ѡ������  
//  mDelaymS(5);                                                               //�ȴ��ⲿ�����ȶ� 	 
    mInitSTDIO( );                                                             //����0,�������ڵ���
//  CH559UART0Alter();                                                         //����0ӳ�䵽P02 P03��Ĭ����P30 P31  
while(1){

    ret = EraseDataFlash(0xF000);
    if(ret == 0){
        printf("Erase DataFlash SUCCESS...\n");
    }
    else{
        printf("ERR %02X\n",(UINT16)ret);    
    }
		
    printf("Write DataFlash ...\n");                                              //дDataFlash
    for(j=0;j<512;j++){
        buf[0] = j;
        WriteDataFlash(0xF000+2*j,buf,1);			
    }

    for(j=0;j<1024;j++){
        printf("%02X  ",(UINT16)*((PUINT8C)(0xF000+j)));                        //��DataFlash
    }
    printf("\nRead DataFlash SUCCESS...\n");
//     while(1);
    mDelaymS(100);	
    printf("######################## %02d &&&&&&&\n",i++);		
 }
}