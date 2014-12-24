#include "RTC.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_conf.h"

void initialize_RTC(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); /* Enable the PWR clock */
	PWR_BackupAccessCmd(ENABLE); /* Allow access to RTC */
	RCC_LSICmd(ENABLE); /* Enable the LSI OSC */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); /* Wait till LSI is ready */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); /* Select the RTC Clock Source */
	RCC_RTCCLKCmd(ENABLE); /* Enable the RTC Clock */
	RTC_WaitForSynchro(); /* Wait for RTC APB registers synchronisation */
	/* Configure the RTC data register and RTC prescaler
	   Make ck_spre 1Hz
	 */
	RTC_InitStructure.RTC_AsynchPrediv = 127;
	RTC_InitStructure.RTC_SynchPrediv = 249;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
}
void reset_alarm()
{
	/* set 8:29:55 */
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_TimeStruct.RTC_Hours = 0x00;//BCD
	RTC_TimeStruct.RTC_Minutes = 0x00;//BCD
	RTC_TimeStruct.RTC_Seconds = 0x00;
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStruct);

	RTC_DateTypeDef RTC_DateStruct;
	RTC_DateStruct.RTC_WeekDay = 0x03;
	RTC_DateStruct.RTC_Month = 0x12;
	RTC_DateStruct.RTC_Date = 0x24;
	RTC_DateStruct.RTC_Year = 0x14;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStruct);
}


void set_alarm_time(uint8_t min, uint8_t sec, char AmOrPm)
{
	RTC_AlarmTypeDef RTC_AlarmStructure;
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE); /* disable before setting or cann't write */
	/* set alarm time 8:30:0 everyday */
	if(AmOrPm == 'a')
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	else
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = 0x00;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = min;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = sec;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31; // Nonspecific
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // Everyday
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	/* Enable Alarm */
	//RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	RTC_ClearFlag(RTC_FLAG_ALRAF);
}
