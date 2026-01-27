#ifndef __TIMER_IT_H
#define __TIMER_IT_H

//int8_t if_update_screen(void);
//int8_t if_change_menu(void);

void timer_it_init(void);
void TIM1_UP_IRQHandler(void);
void TIM2_IRQHandler(void);
//void TIM16_IRQHandler(void);
//void TIM17_IRQHandler(void);
//void RTC_Alarm_IRQHandler(void);

#endif /* __TIMER_IT_H */
