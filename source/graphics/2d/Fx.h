/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FX_H_
#define FX_H_

float disp111;

void initializeGeneralWave(u32 param,int rows,int totalDisplacement)
{
	int amplitude=totalDisplacement;
	float displacement=0;
	int i=0;
	int sign=1;
	float period=1;
	float var;
	var=rows/(period*4);
	var=(totalDisplacement)/var;
	disp111=var;
	for(;i<rows;i++)
	{
		PARAM[(int)param++]=(int)FIX7_9TOI(FTOFIX7_9(displacement));
		PARAM[(int)param++]=(int)FIX7_9TOI(FTOFIX7_9(displacement));

		displacement-=var*sign;
		if(displacement<totalDisplacement*(-1))
		{
			sign=-1;
		}
		if(displacement>totalDisplacement)
		{
			sign=1;
		}
	}
}

void generalWave(u32 param,int rows)
{
	int paramPointer=(int)param;
	int i;
	int difference=PARAM[paramPointer+(int)rows*2-2]-PARAM[paramPointer];
	int static sign=1;
	PARAM[paramPointer+(int)rows*2-2]=PARAM[paramPointer];//-(PARAM[paramPointer+(int)rows*2-2]-PARAM[paramPointer]);
	PARAM[paramPointer+(int)rows*2-1]=PARAM[paramPointer+1];//-(PARAM[paramPointer+(int)rows*2-1]-PARAM[paramPointer+1]);

	for(i=0;i<rows-1;i++)
	{
		PARAM[paramPointer]=PARAM[paramPointer+2];
		PARAM[paramPointer+1]=PARAM[paramPointer+3];

		paramPointer+=2;
	}
	//delay(50);
}

void generalWave2(u32 param1,int rows)
{
	static int initial=0;
	int displacement=10;
	int i=0;
	int difference;
	static int sign=1;
	int param=(int) param1;
	int leftDisp;
	int rightDisp;
	int prevLeftDisp;
	int prevRightDisp;
	int limit=7;
	if(!initial)
	{
		for(;i<rows;i++)
		{
			PARAM[param++]=displacement;
			PARAM[param++]=displacement;
			displacement-=sign;
			if(displacement<limit*(-1))
			{
				sign=-1;
			}
			if(displacement>limit)
			{
				sign=1;
			}
		}
		initial=1;
		displacement=limit;
		sign=1;
	}
	else
	{
		i=0;
		do
		{
			if(i<rows-2)
			{
				PARAM[param]=PARAM[param+2];
				PARAM[param+1]=PARAM[param+3];
			}
			else
			{
				PARAM[param]=PARAM[(int) param1];
				PARAM[param+1]=PARAM[(int) param1+1];
			}
			param++;
			param++;
			i++;
		}
		while(i<rows-1);
	}
	//vbPrint(_textBgMap, 44,27, itoa((int)prevDisp,10,4), 0);
	delay(50);
}

void generalWave1(u32 param1,int rows)
{
	int i=0;
	int param=(int) param1;
	int displacement;
	static int prevDisp=10;
	static int sign=-1;
	int sign2=1;
	int disp2;
	disp2=displacement=prevDisp;
	for(;i<rows;i++)
	{
		PARAM[param]=(int)displacement;
		PARAM[param+1]=(int)displacement;
		param++;
		param++;
		displacement+=sign2;
		if(displacement>10)
		{
			sign2=-1;
		}
		else if(displacement<-10)
		{
			sign2=1;
		}
	}
	prevDisp=disp2+sign;
	if(prevDisp<(-10))
	{
		//prevDisp=-10;
		sign=1;
	}
	if(prevDisp>10)
	{
		sign=-1;
		//prevDisp=10;
	}
	//vbPrint(_textBgMap, 44,27, itoa((int)prevDisp,10,4), 0);
	delay(50);
}

void underWater(u32 param,int rows)
{
}

void fire(u32 param,int rows)
{
}


#endif
