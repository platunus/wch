/********************************** (C) COPYRIGHT *******************************
* File Name          : ROOT_UDISKIF.C
* Author             : WCH
* Version            : V1.1
* Date               : 2015/10/13
* Description        : CH559 U�̶�д Interface                                  				   
*******************************************************************************/
#include "../../CH559.H"
//����Ҫ����LIB���ļ�
#include "../../USB_LIB/CH559UFI.H"
#include "../../USB_LIB/CH559UFI.C"

// ��ȡ�豸������
UINT8C  SetupGetDevDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof( USB_DEV_DESCR ), 0x00 };
// ��ȡ����������
UINT8C  SetupGetCfgDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00 };
// ����USB��ַ
UINT8C  SetupSetUsbAddr[] = { USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00 };
// ����USB����
UINT8C  SetupSetUsbConfig[] = { USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// ����˵�STALL
UINT8C  SetupClrEndpStall[] = { USB_REQ_TYP_OUT | USB_REQ_RECIP_ENDP, USB_CLEAR_FEATURE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
UINT8X  RxBuffer[ MAX_PACKET_SIZE ] _at_ 0x0000 ;  // IN, must even address
UINT8X  TxBuffer[ MAX_PACKET_SIZE ] _at_ 0x0040 ;  // OUT, must even address


/*******************************************************************************
* Function Name  : DisableRootHubPort( void )
* Description    : �ر�Roothub�˿�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void   DisableRootHubPort( void )      // �رն˿�,ʵ����Ӳ���Ѿ��Զ��ر�,�˴�ֻ�����һЩ�ṹ״̬
{
    CH559DiskStatus = DISK_DISCONNECT;
    UHUB0_CTRL = 0x00;                 // ����й�HUB0�Ŀ�������,ʵ���ϲ���Ҫ���
}

/*******************************************************************************
* Function Name  : AnalyzeRootHub( void )
* Description    : �����˿�״̬
* Input          : None
* Output         : None
* Return         : UINT8 s
*******************************************************************************/
UINT8   AnalyzeRootHub( void )         // �����˿�״̬,�����˿ڵ��豸����¼�
// ����ERR_SUCCESSΪû�����,����ERR_USB_CONNECTΪ��⵽������,����ERR_USB_DISCONΪ��⵽�Ͽ�
{
//�����˿ڵĲ���¼�,����豸�γ�,�����е���DisableRootHubPort()����,���˿ڹر�,�����¼�,����Ӧ�˿ڵ�״̬λ
    UINT8   s;
    s = ERR_SUCCESS;
    if ( USB_HUB_ST & bUHS_H0_ATTACH ) // �豸����
    {
        if ( CH559DiskStatus == DISK_DISCONNECT || ( UHUB0_CTRL & bUH_PORT_EN ) == 0x00 )    // ��⵽���豸����,����δ����,˵���Ǹղ���
        {
            DisableRootHubPort( );     // �رն˿�
            CH559DiskStatus = DISK_CONNECT;
            s = ERR_USB_CONNECT;
        }
    }
    else if ( CH559DiskStatus >= DISK_CONNECT )
    {
        DisableRootHubPort( );         // �رն˿�
        if ( s == ERR_SUCCESS )
        {
            s = ERR_USB_DISCON;
        }
    }
    return( s );
}

/*******************************************************************************
* Function Name  : SetHostUsbAddr( UINT8 addr )
* Description    : ����USB������ǰ������USB�豸��ַ
* Input          : UINT8 addr
* Output         : None
* Return         : None
*******************************************************************************/
void   SetHostUsbAddr( UINT8 addr )  
{
    USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | addr & 0x7F;
}

/*******************************************************************************
* Function Name  : ResetRootHubPort( void )
* Description    : ��⵽�豸��,��λ����,Ϊö���豸׼��,����ΪĬ��Ϊȫ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void   ResetRootHubPort( void )        
{
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;  /* USB�豸�Ķ˵�0�������ߴ� */
    SetHostUsbAddr( 0x00 );
    SetUsbSpeed( 1 );                   // Ĭ��Ϊȫ��
    UHUB0_CTRL = UHUB0_CTRL & ~ bUH_LOW_SPEED | bUH_BUS_RESET;  // Ĭ��Ϊȫ��,��ʼ��λ
    mDelaymS( 15 );                     // ��λʱ��10mS��20mS
    UHUB0_CTRL = UHUB0_CTRL & ~ bUH_BUS_RESET;  // ������λ
    mDelayuS( 250 );
    UIF_DETECT = 0;                     // ���жϱ�־
}

/*******************************************************************************
* Function Name  : EnableRootHubPort( void )
* Description    : ʹ�ܶ˿�,��Ӧ��bUH_PORT_EN��1�����˿�,�豸�Ͽ����ܵ��·���ʧ��
* Input          : None
* Output         : None
* Return         : ERR_USB_DISCON
*******************************************************************************/
UINT8   EnableRootHubPort( void )  
{
    if ( CH559DiskStatus < DISK_CONNECT )
    {
        CH559DiskStatus = DISK_CONNECT;
    }
    if ( USB_HUB_ST & bUHS_H0_ATTACH )    // ���豸
    {
        UHUB0_CTRL |= bUH_PORT_EN;  //ʹ��HUB�˿�
        return( ERR_SUCCESS );
    }
    return( ERR_USB_DISCON );
}

/*******************************************************************************
* Function Name  : WaitUSB_Interrupt( void )
* Description    : �ȴ�USB�ж�
* Input          : None
* Output         : None
* Return         : UIF_TRANSFER ? ERR_SUCCESS : ERR_USB_UNKNOWN
*******************************************************************************/
UINT8   WaitUSB_Interrupt( void )  
{
    UINT16  i;
    for ( i = WAIT_USB_TOUT_200US; i != 0 && UIF_TRANSFER == 0; i -- )
    {
        ;
    }
    return( UIF_TRANSFER ? ERR_SUCCESS : ERR_USB_UNKNOWN );
}

/*******************************************************************************
* Function Name  : USBHostTransact( UINT8 endp_pid, UINT8 tog, UINT16 timeout )
* Description    : CH559��������,����Ŀ�Ķ˵��ַ/PID����,ͬ����־,��20uSΪ��λ��NAK������ʱ��(0������,0xFFFF��������),����0�ɹ�,��ʱ/��������
* Input          : UINT8 endp_pid, UINT8 tog, UINT16 timeout
* Output         : None
* Return         : some
*******************************************************************************/ 
UINT8   USBHostTransact( UINT8 endp_pid, UINT8 tog, UINT16 timeout )  // endp_pid: ��4λ��token_pid����, ��4λ�Ƕ˵��ַ
{
 // ���ӳ���������������,����ʵ��Ӧ����,Ϊ���ṩ�����ٶ�,Ӧ�öԱ��ӳ����������Ż�
    #define TransRetry  UEP0_T_LEN  // ��Լ�ڴ�
    UINT8   s, r;
    UINT16  i;
    UH_RX_CTRL = UH_TX_CTRL = tog;
    TransRetry = 0;
    do
    {
        UH_EP_PID = endp_pid;  // ָ������PID��Ŀ�Ķ˵��
        UIF_TRANSFER = 0;  // ��������
        for ( i = WAIT_USB_TOUT_200US; i != 0 && UIF_TRANSFER == 0; i -- )
        {
            ;
        }
        UH_EP_PID = 0x00;  // ֹͣUSB����
        if ( UIF_TRANSFER == 0 )
        {
            return( ERR_USB_UNKNOWN );
        }
        if ( UIF_DETECT )    // USB�豸����¼�
        {
            UIF_DETECT = 0;  // ���жϱ�־
            s = AnalyzeRootHub( );   // ����ROOT-HUB״̬
            if ( s == ERR_USB_CONNECT )
            {
                FoundNewDev = 1;
            }
            if ( CH559DiskStatus == DISK_DISCONNECT )
            {
                return( ERR_USB_DISCON );    // USB�豸�Ͽ��¼�
            }
            if ( CH559DiskStatus == DISK_CONNECT )
            {
                return( ERR_USB_CONNECT );    // USB�豸�����¼�
            }
            mDelayuS( 200 );  // �ȴ��������
        }
        if ( UIF_TRANSFER )    // �������
        {
            if ( U_TOG_OK )
            {
                return( ERR_SUCCESS );
            }
#ifdef DEBUG_NOW
            printf("endp_pid=%02X\n",(UINT16)endp_pid);
            printf("USB_INT_FG=%02X\n",(UINT16)USB_INT_FG);
            printf("USB_INT_ST=%02X\n",(UINT16)USB_INT_ST);
            printf("USB_MIS_ST=%02X\n",(UINT16)USB_MIS_ST);
            printf("USB_RX_LEN=%02X\n",(UINT16)USB_RX_LEN);
            printf("UH_TX_LEN=%02X\n",(UINT16)UH_TX_LEN);
            printf("UH_RX_CTRL=%02X\n",(UINT16)UH_RX_CTRL);
            printf("UH_TX_CTRL=%02X\n",(UINT16)UH_TX_CTRL);
            printf("UHUB0_CTRL=%02X\n",(UINT16)UHUB0_CTRL);
            printf("UHUB1_CTRL=%02X\n",(UINT16)UHUB1_CTRL);
#endif
            r = USB_INT_ST & MASK_UIS_H_RES;  // USB�豸Ӧ��״̬
            if ( r == USB_PID_STALL )
            {
                return( r | ERR_USB_TRANSFER );
            }
            if ( r == USB_PID_NAK )
            {
                if ( timeout == 0 )
                {
                    return( r | ERR_USB_TRANSFER );
                }
                if ( timeout < 0xFFFF )
                {
                    timeout --;
                }
                -- TransRetry;
            }
            else switch ( endp_pid >> 4 )
                {
                case USB_PID_SETUP:
                case USB_PID_OUT:
                    if ( r )
                    {
                        return( r | ERR_USB_TRANSFER );    // ���ǳ�ʱ/����,����Ӧ��
                    }
                    break;  // ��ʱ����
                case USB_PID_IN:
                    if ( r == USB_PID_DATA0 && r == USB_PID_DATA1 )    // ��ͬ�����趪��������
                    {
                    }  // ��ͬ������
                    else if ( r )
                    {
                        return( r | ERR_USB_TRANSFER );    // ���ǳ�ʱ/����,����Ӧ��
                    }
                    break;  // ��ʱ����
                default:
                    return( ERR_USB_UNKNOWN );  // �����ܵ����
                    break;
                }
        }
        else    // �����ж�,��Ӧ�÷��������
        {
            USB_INT_FG = 0xFF;  /* ���жϱ�־ */
        }
        mDelayuS( 15 );
    }
    while ( ++ TransRetry < 3 );
    return( ERR_USB_TRANSFER );  // Ӧ��ʱ
}

/*******************************************************************************
* Function Name  : HostCtrlTransfer( PUINT8X DataBuf, PUINT8I RetLen )
* Description    : ִ�п��ƴ���,8�ֽ���������pSetupReq��,DataBufΪ��ѡ���շ�������
* Input          : PUINT8X DataBuf, PUINT8I RetLen
* Output         : None
* Return         : some
*******************************************************************************/
UINT8   HostCtrlTransfer( PUINT8X DataBuf, PUINT8I RetLen )  
// �����Ҫ���պͷ�������,��ôDataBuf��ָ����Ч���������ڴ�ź�������,ʵ�ʳɹ��շ����ܳ��ȱ�����ReqLenָ����ֽڱ�����
{
    UINT8   s, RemLen, RxLen, RxCnt, TxCnt;
    PUINT8X xdata   pBuf;
    PUINT8I xdata   pLen;
    pBuf = DataBuf;
    pLen = RetLen;
    mDelayuS( 200 );
    if ( pLen )
    {
        *pLen = 0;    // ʵ�ʳɹ��շ����ܳ���
    }
    UH_TX_LEN = sizeof( USB_SETUP_REQ );
    s = USBHostTransact( USB_PID_SETUP << 4 | 0x00, 0x00, 200000/20 );  // SETUP�׶�,200mS��ʱ
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    UH_RX_CTRL = UH_TX_CTRL = bUH_R_TOG | bUH_R_AUTO_TOG | bUH_T_TOG | bUH_T_AUTO_TOG;  // Ĭ��DATA1
    UH_TX_LEN = 0x01;  // Ĭ�������ݹ�״̬�׶�ΪIN
    RemLen = pSetupReq -> wLengthH ? 0xFF : pSetupReq -> wLengthL;
    if ( RemLen && pBuf )    // ��Ҫ�շ�����
    {
        if ( pSetupReq -> bRequestType & USB_REQ_TYP_IN )    // ��
        {
            while ( RemLen )
            {
                mDelayuS( 200 );
                s = USBHostTransact( USB_PID_IN << 4 | 0x00, UH_RX_CTRL, 200000/20 );  // IN����
                if ( s != ERR_SUCCESS )
                {
                    return( s );
                }
                RxLen = USB_RX_LEN < RemLen ? USB_RX_LEN : RemLen;
                RemLen -= RxLen;
                if ( pLen )
                {
                    *pLen += RxLen;    // ʵ�ʳɹ��շ����ܳ���
                }
                for ( RxCnt = 0; RxCnt != RxLen; RxCnt ++ )
                {
                    *pBuf = RxBuffer[ RxCnt ];
                    pBuf ++;
                }
                if ( USB_RX_LEN == 0 || ( USB_RX_LEN & ( UsbDevEndp0Size - 1 ) ) )
                {
                    break;    // �̰�
                }
            }
            UH_TX_LEN = 0x00;  // ״̬�׶�ΪOUT
        }
        else    // ��
        {
            while ( RemLen )
            {
                mDelayuS( 200 );
                UH_TX_LEN = RemLen >= UsbDevEndp0Size ? UsbDevEndp0Size : RemLen;
                for ( TxCnt = 0; TxCnt != UH_TX_LEN; TxCnt ++ )
                {
                    TxBuffer[ TxCnt ] = *pBuf;
                    pBuf ++;
                }
                s = USBHostTransact( USB_PID_OUT << 4 | 0x00, UH_TX_CTRL, 200000/20 );  // OUT����
                if ( s != ERR_SUCCESS )
                {
                    return( s );
                }
                RemLen -= UH_TX_LEN;
                if ( pLen )
                {
                    *pLen += UH_TX_LEN;    // ʵ�ʳɹ��շ����ܳ���
                }
            }
        }
    }
    mDelayuS( 200 );
    s = USBHostTransact( ( UH_TX_LEN ? USB_PID_IN << 4 | 0x00: USB_PID_OUT << 4 | 0x00 ), bUH_R_TOG | bUH_T_TOG, 200000/20 );  // STATUS�׶�
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( UH_TX_LEN == 0 )
    {
        return( ERR_SUCCESS );    // ״̬OUT
    }
    if ( USB_RX_LEN == 0 )
    {
        return( ERR_SUCCESS );    // ״̬IN,���IN״̬�������ݳ���
    }
    return( ERR_USB_BUF_OVER );  // IN״̬�׶δ���
}

/*******************************************************************************
* Function Name  : CopySetupReqPkg( PUINT8C pReqPkt )
* Description    : ���ƿ��ƴ���������
* Input          : PUINT8C pReqPkt
* Output         : None
* Return         : None
*******************************************************************************/
void   CopySetupReqPkg( PUINT8C pReqPkt )  
{
    UINT8   i;
    for ( i = 0; i != sizeof( USB_SETUP_REQ ); i ++ )
    {
        ((PUINT8X)pSetupReq)[ i ] = *pReqPkt;
        pReqPkt ++;
    }
}

/*******************************************************************************
* Function Name  : CtrlGetDeviceDescr( void )
* Description    : ��ȡ�豸������,������TxBuffer��
* Input          : None
* Output         : None
* Return         : some
*******************************************************************************/
UINT8   CtrlGetDeviceDescr( void )  
{
    UINT8   s;
    UINT8D  len;
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;
    CopySetupReqPkg( SetupGetDevDescr );
    s = HostCtrlTransfer( TxBuffer, &len );  // ִ�п��ƴ���
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    UsbDevEndp0Size = ( (PXUSB_DEV_DESCR)TxBuffer ) -> bMaxPacketSize0;  // �˵�0��������,���Ǽ򻯴���,����Ӧ���Ȼ�ȡǰ8�ֽں���������UsbDevEndp0Size�ټ���
    if ( len < ( (PUSB_SETUP_REQ)SetupGetDevDescr ) -> wLengthL )
    {
        return( ERR_USB_BUF_OVER );    // ���������ȴ���
    }
    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlGetConfigDescr( void )
* Description    : ��ȡ����������,������TxBuffer��
* Input          : None
* Output         : None
* Return         : some
*******************************************************************************/
UINT8   CtrlGetConfigDescr( void ) 
{
    UINT8   s;
    UINT8D  len;
    CopySetupReqPkg( SetupGetCfgDescr );
    s = HostCtrlTransfer( TxBuffer, &len );  // ִ�п��ƴ���
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( len < ( (PUSB_SETUP_REQ)SetupGetCfgDescr ) -> wLengthL )
    {
        return( ERR_USB_BUF_OVER );    // ���س��ȴ���
    }
    len = ( (PXUSB_CFG_DESCR)TxBuffer ) -> wTotalLengthL;
    if ( len > MAX_PACKET_SIZE )
    {
        return( ERR_USB_BUF_OVER );    // ���س��ȴ���
    }
    CopySetupReqPkg( SetupGetCfgDescr );
    pSetupReq -> wLengthL = len;  // �����������������ܳ���
    s = HostCtrlTransfer( TxBuffer, &len );  // ִ�п��ƴ���
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( len < ( (PUSB_SETUP_REQ)SetupGetCfgDescr ) -> wLengthL || len < ( (PXUSB_CFG_DESCR)TxBuffer ) -> wTotalLengthL )
    {
        return( ERR_USB_BUF_OVER );    // ���������ȴ���
    }
    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlSetUsbAddress( UINT8 addr )
* Description    : ����USB�豸��ַ
* Input          : UINT8 addr
* Output         : None
* Return         : some
*******************************************************************************/
UINT8   CtrlSetUsbAddress( UINT8 addr )  // ����USB�豸��ַ
{
    UINT8   s;
    CopySetupReqPkg( SetupSetUsbAddr );
    pSetupReq -> wValueL = addr;  // USB�豸��ַ
    s = HostCtrlTransfer( NULL, NULL );  // ִ�п��ƴ���
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    SetHostUsbAddr( addr );  // ����USB������ǰ������USB�豸��ַ
    mDelaymS( 10 );  // �ȴ�USB�豸��ɲ���
    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlSetUsbConfig( UINT8 cfg )
* Description    : ����USB�豸����
* Input          : UINT8 cfg
* Output         : None
* Return         : HostCtrlTransfer( NULL, NULL )
*******************************************************************************/
UINT8   CtrlSetUsbConfig( UINT8 cfg )  // ����USB�豸����
{
    CopySetupReqPkg( SetupSetUsbConfig );
    pSetupReq -> wValueL = cfg;  // USB�豸����
    return( HostCtrlTransfer( NULL, NULL ) );  // ִ�п��ƴ���
}

/*******************************************************************************
* Function Name  : CtrlClearEndpStall( UINT8 endp )
* Description    : ����˵�STALL
* Input          : UINT8 endp
* Output         : None
* Return         : HostCtrlTransfer( NULL, NULL )
*******************************************************************************/
UINT8   CtrlClearEndpStall( UINT8 endp )  // ����˵�STALL
{
    CopySetupReqPkg( SetupClrEndpStall );  // ����˵�Ĵ���
    pSetupReq -> wIndexL = endp;  // �˵��ַ
    return( HostCtrlTransfer( NULL, NULL ) );  /* ִ�п��ƴ��� */
}

/*******************************************************************************
* Function Name  : AnalyzeHidIntEndp( PUINT8X buf )
* Description    : ���������з�����HID�ж϶˵�ĵ�ַ
* Input          : PUINT8X buf
* Output         : None
* Return         : UINT8 s
*******************************************************************************/
UINT8   AnalyzeHidIntEndp( PUINT8X buf )  // ���������з�����HID�ж϶˵�ĵ�ַ
{
    UINT8   i, s, l;
    s = 0;
    for ( i = 0; i < ( (PXUSB_CFG_DESCR)buf ) -> wTotalLengthL; i += l )    // �����ж϶˵�������,���������������ͽӿ�������
    {
        if ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bDescriptorType == USB_DESCR_TYP_ENDP  // �Ƕ˵�������
                && ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bmAttributes & USB_ENDP_TYPE_MASK ) == USB_ENDP_TYPE_INTER  // ���ж϶˵�
                && ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_DIR_MASK ) )    // ��IN�˵�
        {
            s = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;  // �ж϶˵�ĵ�ַ
            break;  // ���Ը�����Ҫ����wMaxPacketSize��bInterval
        }
        l = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bLength;  // ��ǰ����������,����
        if ( l > 16 )
        {
            break;
        }
    }
    return( s );
}

/*******************************************************************************
* Function Name  : InitRootDevice( void )
* Description    : ��ʼ��USB�豸
* Input          : None
* Output         : None
* Return         : some
*******************************************************************************/
UINT8   InitRootDevice( void )  // ��ʼ��USB�豸
{
    UINT8   i, s, cfg, dv_cls, if_cls;
    ResetRootHubPort( );  // ��⵽�豸��,��λ��Ӧ�˿ڵ�USB����
    for ( i = 0, s = 0; i < 100; i ++ )    // �ȴ�USB�豸��λ����������,100mS��ʱ
    {
        mDelaymS( 1 );
        if ( EnableRootHubPort( ) == ERR_SUCCESS )    // ʹ�ܶ˿�
        {
            i = 0;
            s ++;  // ��ʱ�ȴ�USB�豸���Ӻ��ȶ�
            if ( s > 100 )
            {
                break;    // �Ѿ��ȶ�����100mS
            }
        }
    }
    if ( i )    // ��λ���豸û������
    {
        DisableRootHubPort( );
        return( ERR_USB_DISCON );
    }
    SetUsbSpeed( ThisUsbDev.DeviceSpeed );  // ���õ�ǰUSB�ٶ�
    s = CtrlGetDeviceDescr( );  // ��ȡ�豸������
    if ( s == ERR_SUCCESS )
    {
        for ( i = 0; i < ( (PUSB_SETUP_REQ)SetupGetDevDescr ) -> wLengthL; i ++ )
        {
           ;
        }
        dv_cls = ( (PXUSB_DEV_DESCR)TxBuffer ) -> bDeviceClass;  // �豸�����
        s = CtrlSetUsbAddress( ( (PUSB_SETUP_REQ)SetupSetUsbAddr ) -> wValueL );  // ����USB�豸��ַ
        if ( s == ERR_SUCCESS )
        {
            s = CtrlGetConfigDescr( );  // ��ȡ����������
            if ( s == ERR_SUCCESS )
            {
                cfg = ( (PXUSB_CFG_DESCR)TxBuffer ) -> bConfigurationValue;
                for ( i = 0; i < ( (PXUSB_CFG_DESCR)TxBuffer ) -> wTotalLengthL; i ++ )
                {
                  ; 
                }
                /* ��������������,��ȡ�˵�����/���˵��ַ/���˵��С��,���±���endp_addr��endp_size�� */
                if_cls = ( (PXUSB_CFG_DESCR_LONG)TxBuffer ) -> itf_descr.bInterfaceClass;  // �ӿ������
                if ( dv_cls == 0x00 && if_cls == USB_DEV_CLASS_STORAGE )    // ��USB�洢���豸,������ȷ����U��
                {
                    CH559DiskStatus = DISK_USB_ADDR;
                    return( ERR_SUCCESS );
                }
                else
                {
                    return( ERR_USB_UNSUPPORT );
                }
            }
        }
    }
    CH559DiskStatus = DISK_CONNECT;
    SetUsbSpeed( 1 );  // Ĭ��Ϊȫ��
    return( s );
}

/*******************************************************************************
* Function Name  : InitUSB_Host( void )
* Description    : ��ʼ��USB����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void    InitUSB_Host( void )  // ��ʼ��USB����
{
    IE_USB = 0;
    USB_CTRL = bUC_HOST_MODE;  // ���趨ģʽ
    USB_DEV_AD = 0x00;
    UH_EP_MOD = bUH_EP_TX_EN | bUH_EP_RX_EN ;
    UH_RX_DMA = RxBuffer;
    UH_TX_DMA = TxBuffer;
    UH_RX_CTRL = 0x00;
    UH_TX_CTRL = 0x00;
    USB_CTRL = bUC_HOST_MODE | bUC_INT_BUSY | bUC_DMA_EN;  // ����USB������DMA,���жϱ�־δ���ǰ�Զ���ͣ
    UH_SETUP = bUH_SOF_EN;
    USB_INT_FG |= 0xFF;  // ���жϱ�־
    DisableRootHubPort( );  // ���
    USB_INT_EN = bUIE_TRANSFER | bUIE_DETECT;
}

/*******************************************************************************
* Function Name  : mStopIfError( UINT8 iError )
* Description    : ������״̬,�����������ʾ������벢ͣ��
* Input          : UINT8 iError
* Output         : None
* Return         : None
*******************************************************************************/
void    mStopIfError( UINT8 iError )
{
    if ( iError == ERR_SUCCESS )
    {
        return;    /* �����ɹ� */
    }
    printf( "Error: %02X\n", (UINT16)iError );  /* ��ʾ���� */
    /* ���������,Ӧ�÷����������Լ�CH559DiskStatus״̬,�������CH559DiskReady��ѯ��ǰU���Ƿ�����,���U���ѶϿ���ô�����µȴ�U�̲����ٲ���,
       ���������Ĵ�������:
       1������һ��CH559DiskReady,�ɹ����������,����Open,Read/Write��
       2�����CH559DiskReady���ɹ�,��ôǿ�н���ͷ��ʼ����(�ȴ�U�����ӣ�CH559DiskReady��) */
    while ( 1 )
    {
//      LED_TMP=0;  /* LED��˸ */
//      mDelaymS( 100 );
//      LED_TMP=1;
//      mDelaymS( 100 );
    }
}