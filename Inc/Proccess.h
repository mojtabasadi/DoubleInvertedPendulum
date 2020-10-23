#include "main.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim7;


void StartPeripherals();
void ProcessData();
uint8_t Comp1B(uint8_t *buff,uint8_t zero, uint8_t CSNumber);
uint8_t Calc1B(uint8_t* buff, uint8_t zero, uint8_t CSNumber);
void SetSpeed();
void SetPosition();
void SendData();


void SendData()
{
  Matlab.SendBuffer[0]='I';
  Matlab.SendBuffer[1]='P';
  Matlab.SendBuffer[2]=IP.Position & 0xff;
  Matlab.SendBuffer[3]=(IP.Position>>8) & 0xff;
  IP.Angle1=TIM4->CNT;
  Matlab.SendBuffer[4]=IP.Angle1 & 0xff;
  Matlab.SendBuffer[5]=(IP.Angle1>>8) & 0xff;
  IP.Angle2=TIM3->CNT;
  Matlab.SendBuffer[6]=IP.Angle2 & 0xff;
  Matlab.SendBuffer[7]=(IP.Angle2>>8) & 0xff;
  Matlab.SendBuffer[8]=Calc1B(Matlab.SendBuffer,0,8);
  Matlab.SendBuffer[9]='\r';
  Matlab.SendBuffer[10]='\n';
  HAL_UART_Transmit(&huart2, (uint8_t*)&Matlab.SendBuffer, 11,11);
}
void SetPosition()
{
  if(IP.DesirePosition > IP.Position)
  { 
    IP.Direction=Left;
    HAL_GPIO_WritePin(GPIOB, Direction_Pin, GPIO_PIN_RESET);
    TIM1->CR1 |= 1;//Resume
  }
  else if(IP.DesirePosition < IP.Position)
  { 
    IP.Direction=Right;
    HAL_GPIO_WritePin(GPIOB, Direction_Pin, GPIO_PIN_SET);
    TIM1->CR1 |= 1;//Resume
  }
  else
  {
    //TIM1->CR1 &= 0xfe;//Stop Pulse
  }
}
void SetSpeed()
{
  if(IP.DesireSpeed == 'W')//Weak speed
  {
    TIM1->PSC = 2500;
    TIM1->ARR = 19;
  }
  else if(IP.DesireSpeed == 'L')//Low speed
  {
    TIM1->PSC = 1199;
    TIM1->ARR = 19;
  }
  else if(IP.DesireSpeed == 'M')//Medium speed
  {
    TIM1->PSC = 899;
    TIM1->ARR = 19;
  }
  else if(IP.DesireSpeed == 'H')//High speed
  {
    TIM1->PSC = 799;
    TIM1->ARR = 19;
  }
  else if(IP.DesireSpeed == 'V')//Very High speed
  {
    TIM1->PSC = 699;
    TIM1->ARR = 19;
  }
}
void StartPeripherals()
{
  //aSSIGN tEMP bUFFER
  Matlab.Temp=Matlab.buffer[0];
  //sET fIRST pARAMETERS
  IP.Direction=Left;
  IP.DesireSpeed='M';
  SetSpeed(); 
  //sTART pERIPHERAL
  HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
  HAL_TIM_OC_Start_IT(&htim1,TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim7);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
}
void ProcessData()
{
   if(Matlab.buffer[Matlab.BufferProc][0]== 'P' && 
      Matlab.buffer[Matlab.BufferProc][1]== 'C' && 
      Matlab.buffer[Matlab.BufferProc][6]== '\r'&& 
      Matlab.buffer[Matlab.BufferProc][7]== '\n')
    {
        /*  @Calculate CheckSum
         *  @Matlab.buffer[0][0]
         *  @P,C,<3BYTE Data>,CS,\r,\n
         *  @Checksum number:5
         *  @Checksum with P,C(so zero number is 0)       
         */
        if(Comp1B(&Matlab.buffer[Matlab.BufferProc][0],0,5)==1)
        {  
          IP.DesirePosition=Matlab.buffer[Matlab.BufferProc][2]+(Matlab.buffer[0][3]<<8);
          IP.DesireSpeed=Matlab.buffer[Matlab.BufferProc][4];
          if(IP.DesirePosition>10000)IP.DesirePosition=10000;
          else if(IP.DesirePosition<150)IP.DesirePosition=150;
          SetPosition();
          SetSpeed();
        }
        //cLEAR bUFFER
        Matlab.buffer[Matlab.BufferProc][ 0]=0;
        Matlab.buffer[Matlab.BufferProc][ 1]=0;
        Matlab.buffer[Matlab.BufferProc][ 6]=0;
        Matlab.buffer[Matlab.BufferProc][ 7]=0;
    }
   Matlab.BufferProc++;
   if(Matlab.BufferProc>3)Matlab.BufferProc=0;
}

uint8_t Comp1B(uint8_t *buff,uint8_t zero, uint8_t CSNumber)
{
    uint8_t  CheckSum1B=0;
    uint8_t cunter=zero;
    while(cunter<CSNumber)
    {
        CheckSum1B += buff[cunter];
        cunter++;
    }
    if(CheckSum1B == buff[CSNumber])return 1;
    else return 0;
}
uint8_t Calc1B(uint8_t* buff, uint8_t zero, uint8_t CSNumber)
{
    uint8_t  CheckSum1B=0;
    uint8_t cunter=zero;
    while(cunter<CSNumber)
    {
        CheckSum1B += buff[cunter];
        cunter++;
    }
    return CheckSum1B;
}
