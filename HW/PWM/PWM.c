/*********************************************************************************************************
* 模块名称：PWM.c
* 摘    要：PWM模块
* 当前版本：1.0.0
* 作    者：YYW
* 完成日期：2026年05月11日 
* 内    容：
* 注    意：                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "PWM.h"
#include "stm32f10x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  i16 s_iDutyCycle = 0;  //用于存放占空比

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTimerForPWM(u16 arr, u16 psc);  //配置PWM

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTimerForPWM
* 函数功能：配置TIM3与TIM4
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年05月12日
* 注    意：
*********************************************************************************************************/
static void ConfigTimerForPWM(u16 arr, u16 psc)
{
  GPIO_InitTypeDef GPIO_InitStructure;            //GPIO_InitStructure用于存放GPIO的参数
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure; //TIM_TimeBaseStructure用于存放定时器的基本参数
  TIM_OCInitTypeDef  TIM_OCInitStructure;         //TIM_OCInitStructure用于存放定时器的通道参数

  //使能RCC相关时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    //使能TIM3的时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);    //使能TIM4的时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //使能GPIOA的时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   //使能GPIOB的时钟

  //配置PA6对应TIM3的CH1; PA7对应TIM3的CH2
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_7;             //设置引脚  
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;        //设置模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //设置I/O输出速度
  GPIO_Init(GPIOA, &GPIO_InitStructure);                  //根据参数初始化GPIO

  //配置PB0对应TIM3的CH3; PB1对应TIM3的CH4; PB6对应TIM4的CH1  
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6;             //设置引脚  
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;        //设置模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //设置I/O输出速度
  GPIO_Init(GPIOB, &GPIO_InitStructure);                  //根据参数初始化GPIO
 
  //配置TIM3 TIM4时基单元
  TIM_TimeBaseStructure.TIM_Period        = arr;  //设置自动重装载值
  TIM_TimeBaseStructure.TIM_Prescaler     = psc;  //设置预分频器值
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;                  //设置时钟分割：tDTS = tCK_INT  
  TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; //设置向上计数模式  
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);               //根据参数初始化TIM3
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);               //根据参数初始化TIM4
  
  //配置TIM3为PWM2模式，TIM_CounterMode_Up模式下，TIMx_CNT < TIMx_CCRx时为无效电平（高电平）  
  TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM2;        //设置为PWM2模式
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //使能比较输出
  TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low;     //设置极性，OC2低电平有效 
  TIM_OCInitStructure.TIM_Pulse       = 0;
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);                      //根据参数初始化TIM3的CH1
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);                      //根据参数初始化TIM3的CH2
  TIM_OC3Init(TIM3, &TIM_OCInitStructure);                      //根据参数初始化TIM3的CH3
  TIM_OC4Init(TIM3, &TIM_OCInitStructure);                      //根据参数初始化TIM3的CH4
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);             //使能TIM3的CH1预装载
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);             //使能TIM3的CH2预装载
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);             //使能TIM3的CH3预装载
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);             //使能TIM3的CH4预装载
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);             //使能TIM4的CH1预装载
  
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  TIM_Cmd(TIM3, ENABLE);                                        //使能TIM3
  TIM_ARRPreloadConfig(TIM4, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
}


/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitPWM
* 函数功能：初始化PWM模块
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void  InitPWM(void)
{
  ConfigTimerForPWM(99, 71);  //配置TIM3，72000000/(99+1)/(71+1)=10kHz
  TIM_SetCompare1(TIM3, 0);         //设置初始值为0
  TIM_SetCompare2(TIM3, 0);         //设置初始值为0
  TIM_SetCompare3(TIM3, 0);         //设置初始值为0
  TIM_SetCompare4(TIM3, 0);         //设置初始值为0
  TIM_SetCompare1(TIM4, 0);         //设置初始值为0
}

/*********************************************************************************************************
* 函数名称：SetPWM
* 函数功能：设置占空比
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年05月11日
* 注    意：
*********************************************************************************************************/
void SetPWM(i16 val)
{
  s_iDutyCycle = val;                   //获取占空比的值
  TIM_SetCompare1(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare2(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare3(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare4(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare1(TIM4, s_iDutyCycle);  //设置占空比 
}

/*********************************************************************************************************
* 函数名称：IncPWMDutyCycle
* 函数功能：递增占空比，每次递增方波周期的1/100，直至高电平输出 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年05月11日
* 注    意：
*********************************************************************************************************/
void IncPWMDutyCycle(void)
{
  if(s_iDutyCycle >= 100)               //如果占空比不小于100
  {                                     
    s_iDutyCycle = 100;                 //保持占空比值为100
  }                                     
  else                                  
  {                                     
    s_iDutyCycle += 1;                 //占空比递增方波周期的1/10
  }
  TIM_SetCompare1(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare1(TIM4, s_iDutyCycle);  //设置占空比
  TIM_SetCompare2(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare3(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare4(TIM3, s_iDutyCycle);  //设置占空比
}

/*********************************************************************************************************
* 函数名称：DecPWMDutyCycle
* 函数功能：递减占空比，每次递减方波周期的1/100，直至低电平输出 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年05月11日
* 注    意：
*********************************************************************************************************/
void DecPWMDutyCycle(void)
{
  if(s_iDutyCycle <= 0)               //如果占空比不大于0
  {                                   
    s_iDutyCycle = 0;                 //保持占空比值为0
  }                                   
  else                                
  {                                   
    s_iDutyCycle -= 1;               //占空比递减方波周期的1/10
  }
  TIM_SetCompare1(TIM3, s_iDutyCycle);//设置占空比
  TIM_SetCompare2(TIM3, s_iDutyCycle);//设置占空比
  TIM_SetCompare3(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare4(TIM3, s_iDutyCycle);  //设置占空比
  TIM_SetCompare1(TIM4, s_iDutyCycle);  //设置占空比
}

/*********************************************************************************************************
* 函数名称：AlterPWMDutyCycle
* 函数功能：占空比增加到最大后逐渐减少，到最小后逐渐增加，实现渐强渐弱交替循环
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年05月11日
* 注    意：PWM占空比变化为1
*********************************************************************************************************/
void AlterPWMDutyCycle(void)
{
  static u8 state;
  if(state==DOWN){    
    DecPWMDutyCycle();                  //占空比增加
    if(s_iDutyCycle <= 10)               //如果占空比不大于10
    {                                   
      IncPWMDutyCycle();
      state = UP;
    }
  }
  else if(state==UP){
    IncPWMDutyCycle();
    if(s_iDutyCycle >= 100)                               
    {                                   
      DecPWMDutyCycle();               
      state = DOWN;
    }
  }  
}
