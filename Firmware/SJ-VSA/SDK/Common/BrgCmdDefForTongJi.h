/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: BrgCmdDef.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��12��18��
**����޸�����: 2008��12��18��
**��        ��: ����ͨ�ŵ�������
*****************************************************************************************************************/
//
#define DYNC_FIRST_DUMP				40


//Ƶ�ʷ���=======================================================================
#define FREQ_HOST_DEFAULT			4330						//�ϲ�Ĭ��Ƶ��
#define FREQ_DEVICE_DEFAULT			4347						//�²�Ĭ��Ƶ��

//�����ʲ���
#define SAMPLE_RATE_SYNC			20							//ͬ��������
#define SAMPLE_SLIP_SYNC_RT			100							//ʵʱͬ��������(100΢��)

//ͨ��DNS����====================================================================
#define BRG_DNS_HOST				0x55555551				
#define BRG_DNS_RELAY				0x55555552
#define BRG_DNS_RELAYLINE			0xAAAAAAA5
#define BRG_DNS_COLLECTOR			0xAAAAAAA6

//ͨ�ŵ�ַ����===================================================================
#define BRG_DERECTION_DOWN			0x8000						//ͨ�����б�־
#define RELAY_ROL_BITS				10							//��ת��ID����λ��			
#define	RELAY_NET_ID				0x7C00						//��ת���㲥ID
#define	COLLECTOR_NET_ID			0x03FF						//�ɼ����㲥ID
#define	BRG_CHANNEL_OFFSET			2							//ͨ����ƫ����
#define COLLECTOR_CHANNEL_MASK		0xFF						//�ɼ���ͨ������
#define UPDATA_AREA_BYTES			24							//�����������ֽ���
#define NRF_UP_LEN					32							//���������ֽ���
#define NRF_DOWN_LEN				16							//���������ֽ���
#define BRG_ADDR_LEN				3							//��ַ���ֽ���
#define BRG_CRC_LEN					2							//CRCУ����ֽ���
#define BRG_ADRC_LEN				(BRG_ADDR_LEN+BRG_CRC_LEN)	//��ַ��CRC�����ֽ���
#define BRG_CMD_OFFSET				BRG_ADDR_LEN				//ָ���ֽ�ƫ����
#define BRG_SYNC_MASK				0xFC00						//ͬ�������������

//���ȶ���=======================================================================
//#define DYNAMIC_FRAME_COUNT			12		//��̬����ÿ֡����
#define RELAY_BUSY_WAIT_MAX			20		//��ת���ȴ��ɼ���æ��������	
#define RELAY_DYNC_DATA_FRAMES		100		//��ת����̬���ݻ��������ŵ�����֡��

//������������===================================================================
//����
#define BRG_CMD_MODE_AREA		0xE0		//����ģʽ��
//������
#define BRG_CMD_MODE_LR			0x20		//���ػظ�
#define BRG_CMD_MODE_TR			0x40		//������պ�ظ�
#define BRG_CMD_MODE_TR2		0x60		//������պ�ظ�
#define BRG_CMD_MODE_TA			0xA0		//������պ�ظ����ԼӴ���
#define BRG_CMD_MODE_TO			0xC0		//ֻ�´����ϴ�
#define BRG_CMD_MODE_RP			0xE0		//�ϴ�������Ϣ

//Э���ֹ��ܻ���=================================================================
//1�࣬�ڲ�ͨ�ţ����������������ڶ�����豸�Ĳ�������ת��
//ǰ��λ������Ϊ001
#define BRG_ATTACH_DEVICE		0x20		//�����¼��豸
#define BRG_HOST_BATTERY		0x21		//������ص���
#define BRG_REPORT_SET			0x22		//�����������
#define BRG_HOST_FREG			0x23		//�������豸Ƶ��

//2�࣬ÿ�ζ���Ҫ������ת����ͨ�ţ�����Ҫ�ظ����յ�������
//ǰ��λ������Ϊ010, 011
#define BRG_SET_REG      		0x40        //��ת���Ĵ�������
#define BRG_CONNECT_DEVICE		0x41        //�����豸
#define BRG_READ_STATE   		0x42        //��״ָ̬��
#define BRG_SET_FREQ     		0x43        //����Ƶ������
#define BRG_SET_ZERO     		0x44        //�ɼ���У��
#define BRG_KEEP_ALIVE   		0x45        //���ֻ���
#define BRG_STATIC_OPERATION	0x46		//��̬����
#define BRG_DYNC_OPERATION		0x47		//��̬����
#define BRG_ERASE_FLASH			0x48        //�����ɼ���FLASH
#define BRG_CLR_LINK_BIT		0x49		//������ӱ�־
#define BRG_RELAY_RAIN			0x4A		//����ת�������ز�
#define	BRG_CALI_VALUE			0x4B		//У׼�ɼ���
#define BRG_SEND_CARRY			0x4C		//�����ز�
#define BRG_GET_POWER			0x4D		//��ȡ��Դ��ֵ(0-100)
#define BRG_RELAY_RAIN_EX		0x4F		//��չ�����ź�

//3�࣬ÿ�ζ���Ҫת����ͨ�ţ����һظ�����ת�����Զ�������һ֡���ݣ�
//ǰ��λ������λ101
#define BRG_SYNC_DYNAMIC 		0xA0        //��̬�ɼ�ͬ���ź�
#define BRG_READ_DYNAMIC 		0xA1        //����̬����

//4�ֻ࣬�������У�������
//ǰ��λ������Ϊ110
#define BRG_WAKE_UP      		0xC0        //����ָ��

//5�࣬һ��ֻ�������еı���ָ��
//ǰ��λ������Ϊ111
#define BRG_ERR_REPORT      	0xE0        //���󱨸�

//�������ݵ�״̬�ֲ���===========================================================
#define	DEVICE_DATA_VALID		0x01
#define	DEVICE_STATE_BUSY		0x02
#define	DEVICE_DYNC_OVER		0x04

//��̬��������ʱ״̬�ֶ���=======================================================
#define	STATIC_DATA_OK			0x01

//�ɼ��������ֶ���===============================================================
#define LOCAL_ZEROLIZED_BIT		0x01		//���ص���
#define STATIC_PREPARE_BIT		0x02		//��̬Ԥ�ɼ�

//�ɼ�������ʱ״̬��=============================================================
#define COLLECTOR_STATE_MASK	0xFFFFFF
#define COLLECTOR_OK			0x80		//�ɼ����Ƿ�������
#define COLLECTOR_ZERO			0x40		//�ɼ����Ƿ��Ѿ�У��
#define	COLLECTOR_STATIC		0x20		//�ɼ����Ƿ��ھ�̬�ɼ�
#define	COLLECTOR_DYNAMIC		0x10		//�ɼ����Ƿ��ڶ�̬�ɼ�
#define	COLLECTOR_BATTLOW		0x08		//�ɼ����Ƿ��ڵ͵�ѹ״̬
#define	COLLECTOR_FLASH_ERASED	0x04		//Flash�Ƿ��Ѿ�����
#define	COLLECTOR_FREG_CONFIGED	0x02		//�ɼ���Ƶ���Ƿ�������
#define COLLECTOR_VALUE_CALI	0x01		//У��λ
#define COLLECTOR_DATA_PREP		0x0100		//Ԥ�ɼ�λ
#define COLLECTOR_DATA_OK		0x0400		//�ɼ�������׼��ȡ���λ
#define COLLECTOR_ZERO_PREP		0x0200		//Ԥ����λ

//��������������=================================================================
//ͨ��֡����
#define BRG_FRAME_COUNT		6			//����֡����
#define BUFF_HOST_DOWN		0			//��PC���յ���
#define BUFF_DOWN_POOL		1 			//���ͻ���
#define BUFF_DEVICE_TRCV	2 			//�豸�շ���
#define BUFF_DEVICE_POOL	3			//�豸������
#define BUFF_UP_POOL		4			//���ջ���
#define BUFF_HOST_UP		5			//��PC��������

//ͨ��֡״̬��
#define ST_FRAME_EMPTY		0x00		//Ϊ��
#define ST_FRAME_READY		0x01		//׼����
#define ST_FRAME_TRANCE		0x02		//����ͨ����

//ͨ��֡�����ֶ���
#define OP_NONE				0x00		//�ղ���
#define OP_DOWN_BUFF		0x01		//����������
#define OP_UP_BUFF			0x02		//����������	
#define OP_SRC_SET			0x04		//����Դ��־
#define OP_DES_SET			0x10		//����Ŀ������־
#define	OP_SHIFT_CMD		OP_DOWN_BUFF|OP_SRC_SET|OP_DES_SET
#define OP_CPY_CMD			OP_DOWN_BUFF|OP_DES_SET	
#define OP_SHIFT_ALL		OP_DOWN_BUFF|OP_UP_BUFF|OP_SRC_SET|OP_DES_SET

//ָ�����======================================================================
//BRG_SET_REG 
#define RELAY_REG0				0x00
#define RELAY_REG1				0x01
#define RELAY_REG2				0x02

//BRG_REPORT_SET
#define ERR_REPORT_SET			0x01
#define ERR_REPORT_OPEN			0x01
#define ERR_REPORT_COLOSE		0x00

//BRG_STATIC_OPERATION
#define BRG_START_STATIC 		0x01        //��ʼ��̬�ɼ�
#define BRG_END_STATIC   		0x02        //������̬�ɼ�
#define BRG_READ_STATIC  		0x03        //����̬����
#define BRG_GATHER_STATIC		0x04        //�ռ���̬�ɼ�����
#define BRG_PREPARE_STATIC		0x05		//Ԥ�Ȳɼ�

//BRG_DYNC_OPERATION
#define	BRG_DISABLE_DYNC		0x01		//��ֹ��̬�ɼ�
#define	BRG_ENABLE_DYNC 		0x02		//ʹ�ܶ�̬�ɼ�
#define BRG_START_DYNC			0x03		//��ʼ��̬�ɼ�
#define BRG_GATHER_DYNC			0x04        //��ת���ռ���̬����
#define BRG_END_DYNC 			0x05        //������̬�ɼ�
#define BRG_READ_DYNC 			0x06        //������̬�ɼ�

//BRG_RELAY_RAIN��BRG_WAKE_UP
#define WAKEUP_ONLY				0x01		//��������
#define WAKEUP_RESET_FREQ		0x02		//���Ѳ���λƵ��
#define WAKEUP_SET_FREQ			0x03		//���Ѳ�����Ƶ��

//BRG_ERASE_FLASH
#define BRG_ERASE_FLASH_KEY		0x35

//BRG_ERR_REPORT
#define BRG_ERR_DOWN			0x01		//�������ݳ���			
#define BRG_ERR_UP				0x02		//�������ݳ���	

//BRG_CALI_VALUE
#define CALI_MODE_SELF			0x01
#define CALI_MODE_MUL			0x02
#define CALI_MODE_SET			0x03
#define CALI_MODE_FACTORY		0x04
#define CALI_MODE_GET			0x06

//BRG_SYNC_DYNAMIC
#define SYNC_CALI_TIME			0x0400		//У׼ʱ��
#define SYNC_SAMPLE_REF			0x0800		//ʹ���˲ο�Ƶ��
#define SYNC_READ_DATA			0x1000		//������

//BRG_END_DYNC
#define END_DYNC_SELF			0x01
#define END_DYNC_REF			0x02

//BRG_GET_POWER
#define BRG_POWER_RELAY			0x01		//��ת����ѹ
#define BRG_POWER_CLT			0x02		//�ɼ�����ѹ




