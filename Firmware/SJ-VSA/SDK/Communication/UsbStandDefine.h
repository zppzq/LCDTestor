/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: UsbInclude.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��04��17��
**����޸�����: 2008��04��17��
**��        ��: USB��׼������
*****************************************************************************************************************/
#ifndef _USB_STAND_DEFINE_H_
#define _USB_STAND_DEFINE_H_


//���������Ͷ���(��USB��׼���涨)===================================================================================
#define  DSC_DEVICE              0x01        //�豸������
#define  DSC_CONFIG              0x02        //����������
#define  DSC_STRING              0x03        //�ַ�������
#define  DSC_INTERFACE           0x04        //�ӿ�������
#define  DSC_ENDPOINT            0x05        //�˵�������

//HID����������=====================================================================================================
#define DSC_HID					0x21		// HID Class Descriptor
#define DSC_HID_REPORT			0x22		// HID Report Descriptor

// HID Request Codes================================================================================================
#define GET_REPORT 				0x01		// Code for Get Report
#define GET_IDLE				0x02		// Code for Get Idle
#define GET_PROTOCOL			0x03		// Code for Get Protocol
#define SET_REPORT				0x09		// Code for Set Report
#define SET_IDLE				0x0A		// Code for Set Idle
#define SET_PROTOCOL			0x0B		// Code for Set Protocol

//�����豸��״̬====================================================================================================
#define  DEV_ATTACHED           0x00       	//����̬���豸��������������ͨ������ź����ϵĵ�ƽ�仯�������豸�Ľ���
#define  DEV_POWERED            0x01   		//����̬�����Ǹ��豸���磬��Ϊ�豸����ʱ��Ĭ�Ϲ���ֵ�����ý׶κ�Ĺ���ֵ
#define  DEV_DEFAULT            0x02      	//ȱʡ̬��USB�ڱ�����֮ǰ��ͨ��ȱʡ��ַ0����������ͨ��
#define  DEV_ADDRESS            0x03     	//��ַ̬�����������ã�USB�豸����λ�󣬾Ϳ��԰��������������Ψһ��ַ��������ͨ�ţ�����״̬���ǵ�ַ̬
#define  DEV_CONFIGURED         0x04       	//����̬��ͨ�����ֱ�׼��USB������������ȡ�豸�ĸ�����Ϣ�������豸��ĳ����Ϣ���иı������
#define  DEV_SUSPENDED          0x05       	//����̬�����߹����豸��3ms��û�����߶�������USB���ߴ��ڿ���״̬�Ļ������豸��Ҫ�Զ��������״̬

//����˵�״̬======================================================================================================
#define  EP_IDLE                0x00        //����״̬
#define  EP_TX                  0x01        //����״̬
#define  EP_RX                  0x02        //����״̬
#define  EP_HALT                0x03        //ֹͣ״̬
#define  EP_STALL               0x04        //����ֹͣ
#define  EP_ADDRESS             0x05        //�����ı��ַ

//Setup��������������Ĵ���(bRequest�ֶ�)===========================================================================
#define  GET_STATUS             0x00        //�����ض������ߵ�״̬
#define  CLEAR_FEATURE          0x01        //������ֹ�����ߵ�ĳЩ����
#define  SET_FEATURE            0x03        //���û򼤻���������ߵ�ĳЩ����
#define  SET_ADDRESS            0x05        //���豸�����ַ
#define  GET_DESCRIPTOR         0x06        //����������ȡ�豸���ض�������
#define  SET_DESCRIPTOR         0x07        //�޸��豸���йص������������������µ�������
#define  GET_CONFIGURATION      0x08        //������ȡ�豸��ǰ�豸������ֵ
#define  SET_CONFIGURATION      0x09        //����ָʾ�豸���õ�Ҫ�������
#define  GET_INTERFACE          0x0A        //��ȡ��ǰĳ���ӿ����������
#define  SET_INTERFACE          0x0B        //����Ҫ���豸��ĳ���������������ӿ�
#define  SYNCH_FRAME            0x0C        //�����豸���úͱ���һ���˵��ͬ��֡

//Setup��������������Ľ����ߺͷ���Ķ���(bmRequestType�ֶ�)========================================================
#define  IN_DEVICE               0x00        //������Ϊ�豸, ������IN 
#define  OUT_DEVICE              0x80        //������Ϊ�豸, ������OUT
#define  IN_INTERFACE            0x01        //������Ϊ�ӿ�, ������IN
#define  OUT_INTERFACE           0x81        //������Ϊ�ӿ�, ������OUT
#define  IN_ENDPOINT             0x02        //������Ϊ�˵�, ������IN
#define  OUT_ENDPOINT            0x82        //������Ϊ�˵�, ������OUT

//Setup������wIndex����ȡֵ����(wIndex�ֶ�)=========================================================================
#define  IN_EP1                  0x81        
#define  OUT_EP1                 0x01        
#define  IN_EP2                  0x82
#define  OUT_EP2				 0x01		

//Setup������wValue����ȡֵ����(wValue�ֶ�)=========================================================================
#define  DEVICE_REMOTE_WAKEUP    0x01       
#define  ENDPOINT_HALT           0x00      


#endif	//_USB_STAND_DEFINE_H_