#ifndef _MY_UI_H_
#define _MY_UI_H_

#include "main.h"
#include "oled.h"
#include "base_timer.h"
#include "bsp_task.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
typedef enum{ //����������״̬
    NONE=0,				//�޲���
    CLICKED,			//�̰�
    LONG_PRESSED,	//����
    PRESSING			//���ڰ���
}KeyState;
typedef void (*ExecuteFunc)(void *);//��ص����������Ͷ���
typedef struct Item{//һ��Pageҳ���µ�Ԫ�ؽṹ�����Ͷ���
    uint8_t id;			//��ţ����ڼ�¼Ԫ����page�е�λ��
    char *desc;			//������Ϣ��������ʾԪ�ر���
    ExecuteFunc func; //��ص�����ָ�룬ÿ��item����һ��
    void *userdata;		//ָ���û����ݣ��ǻ�ص���������
    struct Item *next_item;//ָ����һ��Ԫ�أ���������ṹ
}Item_Typedef;
typedef struct Page{//ҳ��Page�ṹ�嶨��
    char *title;		//����⣬ͬʱҲ��ui��������
    struct Item *first_item;//ָ��ҳ���µĵ�һ��Ԫ��
    struct Page *last_page;//ָ����һ��page������uiҳ�淵��
}Page_Typedef;


void MenuInit(void);//�˵���س�ʼ��������ͬʱ������OLED��ʼ��
void MenuKeyHandler(void);//����״̬�����������ڲ��񰴼�״̬
void MenuProcessHandler(void);//�����߼������Լ�ҳ���л��������ǹؼ�

#endif