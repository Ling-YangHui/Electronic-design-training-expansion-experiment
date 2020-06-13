/*
    Authored by YangHui on 2020.06.09
    Program for STC89C52 on 11.0592MHz
*/
#include <STC89C5xRC.H>
#include <string.h>
#include <ctype.h>
#define int8 unsigned char
#define int16 unsigned int
#define HIGH 1
#define LOW 0

sbit Trig = P1^6;
sbit Echo = P1^7;
sbit Beep = P2^3;
int8 get_cache[48];
int8 top;
int8 order_state;
int8 dot_mode;
int8 dot_count;
int8 dot_limit;
int8 Limit;
int8 TimeCount;
int8 Return_Int[4];
int8 Return_Float[3];

void SerialSet(int16);
void SendChar(int8);
void SendString(int8 *);
void SendStringLn(int8 *);
void SerialStop();
void SerialBegin();
void DelayMs(int16);
void TimerSet(int16);
void BeginTimer();
void StopTimer();
void show();
void num_init();
void get_init();
void get_char(int8 *);
void get_string();
void test();
void open();

void open()
{
	while(order_state != 3)
	{
		SendStringLn("AT+RST");
		TR2 = 1;
		while(TimeCount < 150);
		TR2 = 0;
		TimeCount = 0;
	}
	order_state = 0;
	while(order_state != 2)
	{
		dot_mode = 1;
		dot_limit = 2;
		SendStringLn("AT+CIPMODE=1");
		TR2 = 1;
		while(TimeCount < 50);
		TR2 = 0;
		TimeCount = 0;
	}
	order_state = 0;
	//ATsend();
	while(order_state != 2)
	{
		dot_limit = 2;
		SendStringLn("AT+CWMODE=1");
		TR2 = 1;
		while(TimeCount < 50);
		TR2 = 0;
		TimeCount = 0;
	}
	//ATsend();
	order_state = 0;
	while(order_state != 2)
	{
		dot_limit = 3;
		SendStringLn("AT+CIPSTART=\"TCP\",\"49.235.143.220\",7000");
		TR2 = 1;
		while(TimeCount < 50);
		TR2 = 0;
		TimeCount = 0;
	}
	dot_mode = 0;
	order_state = 0;
	//ATsend();
	SendStringLn("AT+CIPSEND");
    top = 0;
	DelayMs(3000);
    SendStringLn("Sakura");
    top = 0;
}
void test()
{
	int16 num = 0;
    int8 top_p;
	if (strcmp(get_cache,"OK\r\n") == 0)
		order_state = 2;
	else if (strcmp(get_cache,"WIFI GOT IP\r\n") == 0)
		order_state = 3;
    else if (strcmp(get_cache,"PW P SW\r\n") == 0)
		order_state = 1;
    else if (strstr(get_cache,"PW PL RST ") != NULL)
	{
        top_p = 10;
		while(isdigit(get_cache[top_p]))
		{
			num *= 10;
			num += get_cache[top_p] - '0';
			top_p ++;
		}
		if (top_p != 10 && num > 3)
			Limit = num;
	}
	return;
}
void get_char(int8 *c)
{
	if (SBUF == 0xFF)
	{
		top --;
		RI = 0;
		return;
	}
	*c = SBUF;
	RI = 0;
	return;
}
void get_string() interrupt 4
{
	get_char(&get_cache[top ++]);
	if (get_cache[top - 1] == '\n' && dot_mode == 0)
	{
		get_cache[top] = 0;
		test();
		get_init();
		return;
	}
	else if (get_cache[top - 1] == '\n' && dot_mode == 1 && dot_count < dot_limit)
	{
		dot_count ++;
		top = 0;
	}
	else if (get_cache[top - 1] == '\n' && dot_mode == 1 && dot_count == dot_limit)
	{
		dot_count = 0;
		get_cache[top] = 0;
		test();
		top = 0;
	}
	if (top == 48)
		get_init();
	return;
}
void get_init()
{
	top = 0;
	memset(get_cache,0,48);
}
void num_init()
{
	dot_limit = 0;
	dot_count = 0;
	dot_mode = 0;
	order_state = 0;
    Limit = 5;
	top = 0;
}
void Delay12Us()
{
    int8 i = 3;
    while(i --);
    return;
}
void DelayMs(int16 Delay) //Problem
{
    int16 i;
    int8 j;
    for (i = Delay;i > 0;i --)
        for (j = 250;j > 0;j --);
}

void SerialSet(int16 Rate) //Using Timer 1
{
    
    TMOD |= 0x20;
    TH1 = 256 - 57600 / Rate;
    TL1 = TH1;
    PCON |= 0x80;
    SCON = 0x50;
    TR1 = 0;
    ES = 1;
    EA = 1;
}
void SendChar(int8 send)
{
    ES = 0;
    SBUF = send;
    while(TI == 0);
    TI = 0;
    ES = 1;
}
void SendString(int8 *str)
{
    while (*str)
        SendChar(*str ++);
    return;
}
void SendStringLn(int8 *str)
{
    SendString(str);
    SendChar('\r');
    SendChar('\n');
}
void SerialStop()
{
    TR1 = 0;
}
void SerialBegin()
{
    TR1 = 1;
}

void TimerSet(int16 Time) //Using Timer 2
{
    EA = 1;
    ET2 = 1;
    RCAP2H = (0xFFFF - Time * 1000) / 256;
    RCAP2L = (0xFFFF - Time * 1000) % 256;
    T2CON = 0;
    T2MOD = 0;
    TR2 = 0;
}
void BeginTimer()
{
    TR2 = 1;
}
void StopTimer()
{
    TR2 = 0;
}

void show()
{
	const int8 sel[4] = {0x10,0x20,0x40,0x80};
	const int8 num[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
    const int8 dot = 0x80;
    int8 i = 0,j;
    int8 ss[4];
    ss[3] = Return_Int[0] - '0';
    ss[2] = Return_Int[1] - '0';
    ss[1] = Return_Int[2] - '0';
    ss[0] = Return_Float[0] - '0';
    for (j = 0;j < 4;j ++)
    {
        if (j != 1)
            P0 = num[ss[j]];
        else
            P0 = num[ss[j]] | dot;
        P2 |= sel[3 - j];
        DelayMs(1);
        P0 = 0x00;
        P2 &= 0X0F;
    }
}
void Count() interrupt 5
{
    TimeCount ++;
	TF2 = 0;
}
void SetCounter()
{
    TL0 = 0;
    TH0 = 0;
}

void main()
{
    int16 DelayTime = 0;
    float Distance;
    int8 flag,beep_c,done;
    int8 open_close = 1;
    TimerSet(50);
    SerialSet(9600);
    SerialBegin();
    num_init();
    Trig = LOW;
    TMOD |= 0x01;
    open();
    BeginTimer();
    while(1)
    {
        if (order_state == 1)
        {
            if (open_close == 1)
                open_close = 0;
            else
                open_close = 1;
            order_state = 0;
        }
        if (open_close == 1)
        {
            if (TimeCount == 30)
            {
                StopTimer();
                SetCounter();
                DelayTime = 0;
                Trig = HIGH;
                Delay12Us();
                Trig = LOW;
                while(Echo == LOW);
                TR0 = 1;
                while(Echo == HIGH);
                TR0 = 0;
                DelayTime = TH0 * 256 + TL0;
                Distance = DelayTime * 1.085 / 58.8;
                while(Distance >= 1000)
                    Distance -= 1000;
                TimeCount = 0;
                Return_Int[0] = (int)Distance / 100;
                Return_Int[1] = ((int)Distance - Return_Int[0] * 100) / 10;
                Return_Int[2] = ((int)Distance - Return_Int[0] * 100 - Return_Int[1] * 10);
                Return_Int[0] += '0';
                Return_Int[1] += '0';
                Return_Int[2] += '0';
                Return_Float[0] = (int)((Distance - (int)Distance) * 10);
                Return_Float[1] = (int)((Distance - (int)Distance) * 100) / 10;
                Return_Float[0] += '0';
                Return_Float[1] += '0';
                //SendString("Distance:");
                SendString(Return_Int);
                SendChar('.');
                SendString(Return_Float);
                SendStringLn("cm");
                BeginTimer();
            }
            show();
            if (Distance < Limit)
            {
                Beep = LOW;
            }
            else if (Distance > 150)
            {
                if (flag == 0)
                {
                    flag = 1;
                    beep_c = 0;
                }
                if ((TimeCount % 7 == 0) && done == 0)
                {
                    if (beep_c == 0)
                    {
                        Beep = LOW;
                        beep_c = 1;
                    }
                    else if (beep_c == 1)
                    {
                        Beep = HIGH;
                        beep_c = 0;
                    }
                    done = 1;
                }
                else
                {
                    done = 0;
                }
            }
            else
            {
                flag = 0;
                Beep = HIGH;
            }
        }
        else
        {
            Return_Float[0] = '0';
            Return_Int[0] = '0';
            Return_Int[1] = '0';
            Return_Int[2] = '0';
            if (TimeCount == 30)
            {
                StopTimer();
                show();
                SendStringLn("OFF");
                TimeCount = 0;
                BeginTimer();
            }
        }
    }
}