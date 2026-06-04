#include "ST7789.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "oledfont.h"
#include "DMA.h"

uint16_t BACK_COLOR = WHITE;
uint16_t POINT_COLOR = BLACK;   //����ɫ������ɫ

void ReadWrite(uint8_t ByteSend){
	#if SoftWare_SPI// ����SPI, SSPI
		uint8_t i;
		for (i = 0; i < 8; i ++){
			// Mode 0: SCL ����Ϊ��, ��������д�� SDA �� ���� SCL ������ (������)
			OLED_SDA(ByteSend & (0x80 >> i));  // �� SDA
			OLED_SCL(1);                       // SCL ������, �豸�� SDA ������
			OLED_SCL(0);                       // SCL ����, ׼����һλ
		}
	#else			// Ӳ��SPI, HSPI
		#if	Fast_Mode
			SPI_I2S_SendData(ST7789_HSPI, ByteSend);
		#else
			uint8_t retrun_time=0;
			while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_TXE) != SET&&retrun_time<=2000)retrun_time++;
			SPI_I2S_SendData(ST7789_HSPI, ByteSend);
		#endif
	#endif
}

#if !SoftWare_SPI
static void SPI_WaitIdle(void) {
	while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_TXE) == RESET);
	while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_BSY) == SET);
}
#endif

static void ST7789_WR_DATA8(uint16_t da){ // 8-bit data
	#if ENABLE_CS
		OLED_CS(0);
	#endif
	#if !SoftWare_SPI
		SPI_WaitIdle();
	#endif
	#if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);  // ֱ�Ӳ����Ĵ���, ���� DC=1 (data)
	#else
		OLED_DC(1);
	#endif
		ReadWrite(da);
	#if ENABLE_CS
		OLED_CS(1);  // ���˷ֺ�
	#endif
}
static void ST7789_WR_DATA(int da){
	#if ENABLE_CS
		OLED_CS(0);
	#endif
	#if !SoftWare_SPI
		SPI_WaitIdle();
	#endif
    #if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);//Ϊ�˼���ļӿ��ٶȣ�������ֱ��ʹ�üĴ�������,��������ָPB10(1)
	#else
		OLED_DC(1);
	#endif
	ReadWrite(da>>8);
	ReadWrite(da);
	#if ENABLE_CS
		OLED_CS(1);
	#endif
}
static void ST7789_WR_REG(uint16_t da)	 {
	#if ENABLE_CS
		OLED_CS(0);
	#endif
	#if !SoftWare_SPI
		SPI_WaitIdle();
	#endif
    #if Fast_Mode
		GPIOB->BRR = DC_Pin(10);//Ϊ�˼���ļӿ��ٶȣ�������ֱ��ʹ�üĴ�������,��������ָPB10(0)
	#else
		OLED_DC(0);
	#endif
	ReadWrite(da);
	#if ENABLE_CS
		OLED_CS(1);
	#endif
}
//Ӳ��SPI��ʼ��
void ST7789_HSPI_Init(SPI_TypeDef* SPIx)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  	SPI_InitTypeDef  SPI_InitStructure;

	if(SPIx==SPI1){RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,  ENABLE );RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);}
	else if(SPIx==SPI2){RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE );RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);}
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if(SPIx==SPI1){
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
 		GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);//init gpio pin status
	}
	else if(SPIx==SPI2){
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
 		GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);//init gpio pin status
	}

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	// ST7789 ��� ST7735 ���ģ��ʹ�� SPI Mode 0 (CPOL=Low, CPHA=1Edge)
	// ԭ CPOL=High/CPHA=2Edge (Mode 3) ��һЩģ���ϲ�����
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIx, &SPI_InitStructure);
 
	SPI_Cmd(SPIx, ENABLE);
	ReadWrite(0x0000);
}   

void ST7789_Cursor(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2){
   ST7789_WR_REG(0x2a);
   ST7789_WR_DATA(x1);
   ST7789_WR_DATA(x2);
  
   ST7789_WR_REG(0x2b);
   ST7789_WR_DATA(y1);
   ST7789_WR_DATA(y2);

   ST7789_WR_REG(0x2C);					 						 
}

void ST7789_Init(uint16_t Back_color,uint16_t Pen_color){
	BACK_COLOR = Back_color;
	POINT_COLOR = Pen_color;
	//---------------------------------- DC,CS,RST------------------------------------
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11);

	#if SoftWare_SPI
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_15);
	//------------------------------------SCL|SDA-----------------------------------------
	#else
		ST7789_HSPI_Init(ST7789_HSPI);// Ӳ��SPI��ʼ��
	#endif

	#if !SoftWare_SPI
	if(ST7789_HSPI == SPI1)
		MYDMA_Init(DMA1_Channel3,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,480);// ��ʼ�� DMA ͨ��
	else
		MYDMA_Init(DMA1_Channel5,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,480);// SPI2 ʹ�� Channel5

	SPI_I2S_DMACmd(ST7789_HSPI,SPI_I2S_DMAReq_Tx,ENABLE);// allow SPI connect to DMA
	#endif
	#if ENABLE_CS
		OLED_CS(0);// Ƭѡʹ��
	#endif
	// ��λʱ�伴ʹ SysTick δ����, ��Ȼֱ�� __NOP ��ʱ (72MHz �� 2000 NOP �� 28us ���ȶ�)
	OLED_RST(0);
	Delay_ms(10);
	OLED_RST(1);
	Delay_ms(120);

	ST7789_WR_REG(0x36);
	ST7789_WR_DATA8(0x60);  // MV=1, MX=1

	ST7789_WR_REG(0x3A); 
	ST7789_WR_DATA8(0x05);

	ST7789_WR_REG(0xB2);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x00);
	ST7789_WR_DATA8(0x33);
	ST7789_WR_DATA8(0x33);

	ST7789_WR_REG(0xB7); 
	ST7789_WR_DATA8(0x35);  

	ST7789_WR_REG(0xBB);
	ST7789_WR_DATA8(0x19);

	ST7789_WR_REG(0xC0);
	ST7789_WR_DATA8(0x2C);

	ST7789_WR_REG(0xC2);
	ST7789_WR_DATA8(0x01);

	ST7789_WR_REG(0xC3);
	ST7789_WR_DATA8(0x12);   

	ST7789_WR_REG(0xC4);
	ST7789_WR_DATA8(0x20);  

	ST7789_WR_REG(0xC6);//ˢ����
	ST7789_WR_DATA8(0x0F);    

	ST7789_WR_REG(0xD0); 
	ST7789_WR_DATA8(0xA4);
	ST7789_WR_DATA8(0xA1);

	ST7789_WR_REG(0xE0);
	ST7789_WR_DATA8(0xD0);
	ST7789_WR_DATA8(0x04);
	ST7789_WR_DATA8(0x0D);
	ST7789_WR_DATA8(0x11);
	ST7789_WR_DATA8(0x13);
	ST7789_WR_DATA8(0x2B);
	ST7789_WR_DATA8(0x3F);
	ST7789_WR_DATA8(0x54);
	ST7789_WR_DATA8(0x4C);
	ST7789_WR_DATA8(0x18);
	ST7789_WR_DATA8(0x0D);
	ST7789_WR_DATA8(0x0B);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x23);

	ST7789_WR_REG(0xE1);
	ST7789_WR_DATA8(0xD0);
	ST7789_WR_DATA8(0x04);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x11);
	ST7789_WR_DATA8(0x13);
	ST7789_WR_DATA8(0x2C);
	ST7789_WR_DATA8(0x3F);
	ST7789_WR_DATA8(0x44);
	ST7789_WR_DATA8(0x51);
	ST7789_WR_DATA8(0x2F);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x20);
	ST7789_WR_DATA8(0x23);

	ST7789_WR_REG(0x21);

	ST7789_WR_REG(0x11);  // �˳�˯��
	Delay_ms(120);          // ST7789 Ҫ�� 0x11 ���� 120ms ����ܷ��� 0x29

	ST7789_WR_REG(0x29);  // ����ʾ

	#if SoftWare_SPI
		OLED_SCL(1);  // ���� SPI ���еĿ���ʱ�� SCL Ϊ�� (CPOL=High ƥ��)
	#endif
}
//������ת����
void ST7789_SetRotation(uint8_t Diraction){
	ST7789_WR_REG(0x36);	// MADCTL
	switch (Diraction) {
	case 0:ST7789_WR_DATA8(0x00);break;//0000 0000
	case 1:ST7789_WR_DATA8(0xA0);break;//1010 0000
	case 2:ST7789_WR_DATA8(0xC0);break;//1100 0000
	case 3:ST7789_WR_DATA8(0x60);break;//0110 0000 
	default://���������⣬��Ļ�Ƿ�������ת������������������⣬Ӧ����Ҫ��ת�������������꣬��������(���޸�...........)
		break;
	}
}

void ST7789_Clear(uint16_t Color){
	uint16_t i,j;  	
	ST7789_Cursor(0,0,ST7789_W-1,ST7789_H-1);
    for(i=0;i<ST7789_W;i++){
	  for (j=0;j<ST7789_H;j++){
        	ST7789_WR_DATA(Color);	 			 
	    }
	}
}
void ST7789_Clear_DMA(uint16_t Color){
	uint16_t i;
	if(ST7789_HSPI == SPI1)
		MYDMA_Init(DMA1_Channel3,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,sizeof(SendBuff));
	else
		MYDMA_Init(DMA1_Channel5,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,sizeof(SendBuff));

	ST7789_Cursor(0,0,ST7789_W-1,ST7789_H-1);
	for (i=0;i<480;i+=2){//ST7789һ��Ҫ240*2���ֽ�,one line in ST7789 needs one 480 byte
		SendBuff[i] = Color>>8;
		SendBuff[i+1] = Color;
	}
	#if !SoftWare_SPI
	SPI_WaitIdle();
	#endif
	#if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);
	#else
		OLED_DC(1);
	#endif
	for(i = 0;i<240;i++){
		if (ST7789_HSPI == SPI1)
			DMA_Transfer(DMA1_Channel3,480);
		else
			DMA_Transfer(DMA1_Channel5,480);
	}
}

void ST7789_PrintChinese(unsigned int x,unsigned int y,unsigned char index)	
{  
	unsigned char i,j;
	unsigned char *temp=hanzi;//��������
    ST7789_Cursor(x,y,x+31,y+31);  
	temp+=index*128;	
	for(j=0;j<128;j++){
		for(i=0;i<8;i++){	     
		 	if((*temp&(1<<i))!=0){
				ST7789_WR_DATA(POINT_COLOR);
			}
			else{
				ST7789_WR_DATA(BACK_COLOR);
			}
		}
		temp++;
	 }
}

void ST7789_DrawPoint(uint16_t x,uint16_t y){
	ST7789_Cursor(x,y,x,y);//���ù��λ�� 
	ST7789_WR_DATA(POINT_COLOR);
}

void ST7789_DrawPoint_big(uint16_t x,uint16_t y)
{
	ST7789_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
} 

void ST7789_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color){
	uint16_t i;
	uint16_t width = xend - xsta + 1;
	uint16_t height = yend - ysta + 1;
	uint16_t line_bytes = width * 2;

	ST7789_Cursor(xsta,ysta,xend,yend);

	for (i=0;i<line_bytes && i<480;i+=2){
		SendBuff[i] = color>>8;
		SendBuff[i+1] = color;
	}

	#if !SoftWare_SPI
	SPI_WaitIdle();
	#endif
	OLED_DC(1);

	for(i = 0;i<height;i++){
		if(ST7789_HSPI == SPI1)
			DMA_Transfer(DMA1_Channel3,line_bytes);
		else
			DMA_Transfer(DMA1_Channel5,line_bytes);
	}
}  
 
void ST7789_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ ){//�������
		ST7789_DrawPoint(uRow,uCol);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    

void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	ST7789_DrawLine(x1,y1,x2,y1);
	ST7789_DrawLine(x1,y1,x1,y2);
	ST7789_DrawLine(x1,y2,x2,y2);
	ST7789_DrawLine(x2,y1,x2,y2);
}

void ST7789_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b){
		ST7789_DrawPoint(x0-b,y0-a);             //3           
		ST7789_DrawPoint(x0+b,y0-a);             //0           
		ST7789_DrawPoint(x0-a,y0+b);             //1       
		ST7789_DrawPoint(x0-b,y0-a);             //7           
		ST7789_DrawPoint(x0-a,y0-b);             //2             
		ST7789_DrawPoint(x0+b,y0+a);             //4               
		ST7789_DrawPoint(x0+a,y0-b);             //5
		ST7789_DrawPoint(x0+a,y0+b);             //6 
		ST7789_DrawPoint(x0-b,y0+a);             
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		ST7789_DrawPoint(x0+a,y0+b);
	}
} 

//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void ST7789_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
    uint8_t temp;
    uint8_t pos,t;
	uint16_t x0=x;
	uint16_t colortemp=POINT_COLOR;      
    if(x>ST7789_W-16||y>ST7789_H-16)return;	    
	//���ô���		   
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	ST7789_Cursor(x,y,x+8-1,y+16-1);      //���ù��λ�� 
	if(!mode) //�ǵ��ӷ�ʽ
	{
		for(pos=0;pos<16;pos++)
		{ 
			temp=asc2_1608[(uint16_t)num*16+pos];		 //����1608����
			for(t=0;t<8;t++){
		        if(temp&0x01)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				ST7789_WR_DATA(POINT_COLOR);
				temp>>=1;
				x++;
		    }
			x=x0;
			y++;
		}	
	}else{//���ӷ�ʽ
		for(pos=0;pos<16;pos++)
		{
		    temp=asc2_1608[(uint16_t)num*16+pos];		 //����1608����
			for(t=0;t<8;t++){        
		        if(temp&0x01)ST7789_DrawPoint(x+t,y+pos);//��һ����     
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   

uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

void ST7789_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len){ 
	uint8_t t,temp;
	uint8_t enshow=0;
	num=(uint16_t)num;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1)){
			if(temp==0){
				ST7789_ShowChar(x+8*t,y,' ',0);
				continue;
			}else enshow=1;  	 
		}
	 	ST7789_ShowChar(x+8*t,y,temp+48,0); 
	}
} 

void ST7789_ShowString(uint16_t x,uint16_t y,char *p)
{         
    while(*p!='\0'){      
        if(x>ST7789_W-16){x=0;y+=16;}
        if(y>ST7789_H-16){y=x=0;ST7789_Clear(RED);}
        ST7789_ShowChar(x,y,*p,0);
        x+=8;
        p++;
    }  
}

void ST7789_Printf(uint16_t X, uint16_t Y,const char* format, ...){
	char String[256];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	ST7789_ShowString(X, Y, String);
}

void ST7789_SlowPrint(uint16_t x,uint16_t y,const char* string){
  while (*string!='\0'){
    ST7789_ShowChar(x,y,*string,1);
    x+=8;
    string++;
    Delay_ms(100);
  }
}

//��Ƭ����flash����
void ST7789_ShowImage_Flash(uint16_t startX, uint16_t startY,const uint8_t *Picture_ptr){
	uint16_t x,y;
	uint16_t pic_Hight = Picture_ptr[3];
	uint16_t pic_Width = Picture_ptr[5];
	ST7789_Cursor(startX, startY, startX + pic_Width, startY +pic_Hight);
	for(y = 0; y < pic_Hight; y++){
		for(x = 0; x <= pic_Width; x++){
			ST7789_WR_DATA(((uint16_t)Picture_ptr[(y * pic_Width + x) * 2]<< 8 | Picture_ptr[(y * pic_Width + x) * 2 + 1]) );
			//!!!use MSB,��λ��ǰ
		}
	}
}

void ST7789_ShowImage_DMA(uint16_t startX, uint16_t startY,const uint8_t *Picture_ptr,uint32_t sizof_pic){
	uint16_t pic_Hight = Picture_ptr[5];
	uint16_t pic_Width = Picture_ptr[3];
	ST7789_Cursor(startX, startY, startX + pic_Width-1, startY +pic_Hight);
	#if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);
	#else
		OLED_DC(1);
	#endif
	if(ST7789_HSPI == SPI1){
		MYDMA_Init(DMA1_Channel3,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)Picture_ptr,sizof_pic);
		DMA_Transfer(DMA1_Channel3,sizof_pic);
	}
	else{
		MYDMA_Init(DMA1_Channel5,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)Picture_ptr,sizof_pic);
		DMA_Transfer(DMA1_Channel5,sizof_pic);
	}
}

void ST7789_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ST7789_Cursor(x1, y1, x2, y2);
}

void ST7789_WriteData16(uint16_t da)
{
    ST7789_WR_DATA(da);
}

void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Color)
{
    uint16_t colortemp = POINT_COLOR;
    POINT_COLOR = Color;
    ST7789_DrawLine(x,       y,       x + w,   y);
    ST7789_DrawLine(x,       y + h,   x + w,   y + h);
    ST7789_DrawLine(x,       y,       x,       y + h);
    ST7789_DrawLine(x + w,   y,       x + w,   y + h);
    POINT_COLOR = colortemp;
}
