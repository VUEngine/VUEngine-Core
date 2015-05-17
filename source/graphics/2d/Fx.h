/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
		while (i<rows-1);
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