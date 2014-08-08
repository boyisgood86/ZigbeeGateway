#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

extern void rt_hw_lcd_init(void);
extern void rt_hw_key_init(void);
extern void workbench_init(void);
extern void panel_init(void);

/* GUI�����ʾ��ڣ�������߳��н��г�ʼ�� */
void rtgui_startup()
{
	/* GUIϵͳ��ʼ�� */
    rtgui_system_server_init();

	/* ������ʼ�� */
	rt_hw_key_init();
	/* LCD������ʼ�� */
	rt_hw_lcd_init();

	/* ��������ʼ�� */
	panel_init();

	/* ����workbench */
	workbench_init();
}
