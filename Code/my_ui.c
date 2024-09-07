#include "my_ui.h"
#include "codetab.h"
#include "string.h"
#include "bsp_uart.h"


sbit KEY_LEFT = P2 ^ 4;
sbit KEY_RIGHT = P2 ^ 5;
sbit KEY_ENTER = P0 ^ 0;
sbit KEY_LEFT_LOW = P2^2;
sbit KEY_RIGHT_LOW = P2^7;
sbit KEY_ENTER_LOW = P0^2;

static idata Page_Typedef page_Main;
static idata Item_Typedef item_ShowRMS, item_ChangeGain, item_About;

static idata Page_Typedef *cur_page = NULL;
static idata Item_Typedef *cur_item = NULL;

static idata uint8_t index_y = 0;//�α�ָ��Ķ��壬��ֵΪ0-7
static idata ExecuteFunc running_func = NULL;//��¼�µ�ǰ��Ծ�Ļ�ص�������ÿһ���˵����ڶ�ִ��һ��

idata struct {
    uint8_t left: 2;//ʹ��λ�򣬽�Լ�ڴ�
    uint8_t enter: 2;
    uint8_t right: 2;
    uint32_t enter_last_tick;
}mykeys = {0};

static bdata bit app_init_flag=0;//��¼��ui��ʼ����ǣ����ڳ�ʼ���ص������ִ��һ��
static bdata bit app_delete_flag=0;//��¼uiɾ���ı�ǣ�����ɾ��ǰ�����ض������ص�


static void ShowRMSFunc(void *_data);//��ص���������

static void ChangeGainFunc(void *_data);

static void AboutFunc(void *_data);

static void itemInit(Item_Typedef *item, char *desc, ExecuteFunc func, void *userdata) {
  item->desc = desc;
  item->func = func;//����Ԫ�ػص�����ָ��
  item->userdata = userdata;
  item->next_item = NULL;//�ǵ�Ҫ�ÿգ����������Ҫ�пգ�
  item->id = 1;//Ĭ�ϵ�idΪ1
  if (cur_item) {//�������page�ĵ�һ��Ԫ��
    cur_item->next_item = item;//���Լ��ĵ�ַ������һ��Ԫ�ص�next_item
    item->id = cur_item->id + 1;//����һ��Ԫ�ص�id�ϼ�һ��Ϊ�Լ���id
  }
  cur_item = item;//��Ҫ��һ����ɵ����Ĺ�ϵ����
}

static void pageInit(Page_Typedef *page, char *title, Item_Typedef *first_item) {
  page->title = title;
  page->first_item = first_item;
  page->last_page = NULL;
  cur_item = NULL;//�µ�һҳ��ͷ�ڵ��ÿգ��Ա������׸�item
}
static void OLED_ShowStrCenterAligned(const char *str,uint8_t y){
  uint8_t x_padding=0,len;
  len = strlen(str)*6;//�����㷨
  if(SCREEN_WIDTH>=len)
    x_padding = (SCREEN_WIDTH - len)/2;
  OLED_ShowStr(x_padding,y,str);
}
static void pageJumper(void *tar_page) {//���ں�һ��page�������߻ָ�ǰһ��page�Ļָ�
  Page_Typedef *new_page = (Page_Typedef *) tar_page;//ǿת�����ת��Ŀ��page
  if (tar_page != NULL) {
		Item_Typedef * pt;
    uint8_t i;
    if(cur_page->last_page==new_page)//��Ϊpage�����򽫵�ǰ��last_page�ÿ�
      cur_page->last_page = NULL;
    else //��Ϊpage�����򽫵�ǰpage���浽�µ�page��last_page
      new_page->last_page = cur_page;//�洢��һ��page�Ա�֮��ҳ�淵��
    cur_page = new_page;
    cur_item = cur_page->first_item;
    running_func = NULL;//��ת���µ�ҳ�棬��ִ���κλ�ص�������
    OLED_Fill(0x00,0,7);//����
    OLED_ShowStrCenterAligned(cur_page->title,0);//�ڵ�һ�о�����ʾ����
    pt = cur_page->first_item;
    for(i =1;i<8;i++){//һ��page��������ʾ7��item
      if(pt==NULL)
        break;
      OLED_ShowStr(8,i,pt->desc);//������Ԫ�صı���
			pt = pt->next_item;
    }
    index_y = 0;//�α�Ĭ��ָ�����
  }
}

void MenuInit(void) {
  oledInit();//oled��ʼ������
  pageInit(&page_Main, "Main", &item_ShowRMS);//�ȳ�ʼ��page
  itemInit(&item_ShowRMS, "ShowRMS", ShowRMSFunc, NULL);//��һ��item
  itemInit(&item_ChangeGain, "ChangeGain", ChangeGainFunc, NULL);//�ڶ���
  itemInit(&item_About, "About", AboutFunc, NULL);//������
	/*��Ҫ�����µĽ��棬�����ȳ�ʼ��һ��page���Ժ��item����������Ԫ�ء�
	pageInit(...)
	itemInit(...)
	itemInit(...)
	...
	*/
  pageJumper(&page_Main);//��ת����ʼ���棬����˵�
}
void MenuKeyHandler() {
  KEY_LEFT = KEY_ENTER = KEY_RIGHT = 1;//�Ȱ�����˿����ߣ����͵�ƽ
	KEY_LEFT_LOW = KEY_RIGHT_LOW = KEY_ENTER_LOW =  0;//����������˿��ṩ�͵�ƽ
  switch (mykeys.left) {//��switch��乹�ɰ�����״̬��
    case NONE: {
      if (KEY_LEFT == 0)//���ɼ����͵�ƽ˵����������
        mykeys.left = PRESSING;//��������
      break;
    }
    case PRESSING: {
      if (KEY_LEFT == 1)//��pressing״̬�¼�⵽�����ƽΪ�߱������������һ�ζ̰�
        mykeys.left = CLICKED;
      break;
    }
  }
  switch (mykeys.right) {
    case NONE: {
      if (KEY_RIGHT == 0)
        mykeys.right = PRESSING;
      break;
    }
    case PRESSING: {
      if (KEY_RIGHT == 1)
        mykeys.right = CLICKED;
      break;
    }
  }
  switch (mykeys.enter) {
    case NONE: {
      if (KEY_ENTER == 0) {
        mykeys.enter = PRESSING;
        mykeys.enter_last_tick = getTick();//��ȷ�ϼ����£���Ҫ��¼��ʱ��ʱ��
      }
      break;
    }
    case PRESSING: {
      if (KEY_ENTER == 1) {//�����ɿ���
				//getTick()��ȡϵͳ������ʱ�̣�����ǰ�ڸձ�����ʱ��900ms
        if (getTick() - mykeys.enter_last_tick > 900){
					mykeys.enter = LONG_PRESSED;//��������
				}
        else{
          mykeys.enter = CLICKED;//������ʱ��С��900ms�򴥷��̰�
        }
      }
      break;
    }
  }
}

void MenuProcessHandler() {
  if (mykeys.enter == CLICKED) {//ȷ�ϼ����̰���
    if(cur_item){
      if(running_func == NULL){//��ǰû�л�ص�������ֻ�����Ǵ�������ɾ��ui����
        OLED_Fill(0x00,0,7);//����
        running_func = cur_item->func;
        app_init_flag = 1;//����app����ʱ��ʼ�������
				OLED_ShowStrCenterAligned(cur_item->desc,0);
				running_func(cur_item->userdata);//�����ص�app��ִ��app��ʼ�������
				app_init_flag = 0;//��λapp��ʼ����־
      }
    }
  } else if (mykeys.enter == LONG_PRESSED) {//ȷ�ϼ�����ֻ�����ǻ���ui����
    if(cur_page){
      if(running_func == NULL)//������ǰuiͣ��pageҳ��
        pageJumper(cur_page->last_page);//ֱ��ת����һ��ҳ��
      else {//���ui��ǰͣ��Ԫ�صĻ�ص��׶Σ�����˵�Ԫ���Լ���page��
				app_delete_flag = 1;//����appɾ����Ļص������
				running_func(cur_item->userdata);//�����ص�app��ִ��appɾ���ص������
				app_delete_flag = 0;
        pageJumper(cur_page);
      }
    }
  }
  if(running_func == NULL){//����Ϊ��ʱ�������α�����������ڻ�ص���̰����Ҽ������α�
    if (mykeys.left == CLICKED) {
      Item_Typedef * pt = cur_page->first_item;
			/*���������Ѱ����һ��Ԫ�أ�
				1.��ǰԪ�ز�����Ԫ�أ���ֻ��Ҫ�ҵ������Լ�ָ���Ԫ�ؼ���
				2.��ǰԪ������Ԫ�أ���һ��Ԫ�ؾ���ĩβԪ���ˣ���ֻ��Ҫ�ҵ���һ��Ԫ��ָ��Ϊ�յ�Ԫ��
			*/
      while (pt) {
        if ((cur_item != cur_page->first_item && pt->next_item == cur_item) ||
            (cur_item == cur_page->first_item && pt->next_item == NULL))
          break;
        pt = pt->next_item;
      }
      cur_item = pt;//���ҵ���Ԫ����Ϊ��ǰԪ��
    }
    if (mykeys.right == CLICKED) {//����һ��Ԫ��
      if (cur_item) {
        if (cur_item->next_item == NULL)//���һ��Ԫ������һ��Ԫ��ΪͷԪ��
          cur_item = cur_page->first_item;
        else
          cur_item = cur_item->next_item;
      }
    }
  }
  if (running_func) {//��ΪpageJumper�ص�������ֻ����ִ��һ�Σ���Ϊ�����running_func�ÿ���
    running_func(cur_item->userdata);
  }
  if (cur_item) {//��pageûԪ�أ����α겻�����
    if (index_y != cur_item->id) {//ʱ�̼�⵱ǰ�α�λ�ã��Ը���
      if(index_y!=0)//��ˢ�±��⣬û��Ҫ
        OLED_DrawBMP(0, index_y, 8, index_y+1, NULL);//���ԭ�����α�
      OLED_DrawBMP(0,cur_item->id, 8,cur_item->id+1,BMP_INDEX_8X8);//���µ�λ���ϻ��α�
      index_y = cur_item->id;//���µ�id��Ϊ��ǰ�α�λ��
    }
  }
	//�����ǰ���״̬������ú����νӵĹؼ��������갴����������pressing״̬������״̬�°�����Ҫ��λ
  if(mykeys.enter!=PRESSING)mykeys.enter = NONE;
  if(mykeys.right!=PRESSING)mykeys.right = NONE;
  if(mykeys.left!=PRESSING)mykeys.left = NONE;
}

static idata uint8_t PGA_Gain=1;
//Appliance��
static void ShowRMSFunc(void *_data) {
  static uint8_t ticks=0;
  float RMS;
  if(app_init_flag==1){
		PGA_Gain = getGain();
		return;
  }
  if(++ticks*TASK_MENU_CORE_MS_PER_TICK>=200){
		uint16_t adc;
		float DC_volt;
		adc = TM7705_ReadAdc();
    RMS = getRMS(256,PGA_Gain,&DC_volt);
		OLED_ShowStr(24,2, "DC: %6.1fmV",DC_volt);
    OLED_ShowStr(18,4, "RMS: %6.1fmV",RMS);
		UART_SendBuf((uint8_t *)&RMS,4);
    ticks = 0;
  }
}

static void ChangeGainFunc(void *_data) {
  if(app_init_flag==1){
    PGA_Gain = getGain();
		return;
  }
  if (mykeys.left == CLICKED) {
		PGA_Gain = PGA_Gain==0?7:PGA_Gain-1;
  }
  if (mykeys.right == CLICKED) {
		PGA_Gain = PGA_Gain==7?0:PGA_Gain+1;
  }
	OLED_ShowStr(42,3, "X %3bu",(uint8_t)(0x01<<PGA_Gain));
	if(app_delete_flag==1){
		setGain(PGA_Gain);
	}

}

static void AboutFunc(void *_data) {
	if(app_init_flag==1){
    OLED_DrawBMP(49,2,79,6,BMP_AVATAR_32X30);
		OLED_ShowStrCenterAligned("Author:JasomWu",7);
		return;
  }
}
