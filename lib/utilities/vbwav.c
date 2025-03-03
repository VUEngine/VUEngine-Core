/* vbwav.c

   WAV to Virtual Boy Sample Converter
   version 1.00 (30-July-2003)
   by frostgiant
   
   Simply converts a 8 bit mono PCM wave file into 6 bit sound data and places 
   it in a header file. This is intented for use on programs designed for the 
   Virtual Boy, but there may be other applications.
   
   Copyright (c) 2003 frostgiant

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"), 
   to deal in the Software without restriction, including without limitation 
   the rights to use, copy, modify, merge, publish, distribute, sublicense, 
   and/or sell copies of the Software, and to permit persons to whom the 
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in 
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
   ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
   OTHER DEALINGS IN THE SOFTWARE.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *fptr;
unsigned int length;

//Handy functions for reading from the file
unsigned int read_uint32(void)
{
	unsigned int ret;
	ret = fgetc(fptr) | (fgetc(fptr)<<8) | (fgetc(fptr)<<16) | (fgetc(fptr)<<24);
	return ret;
}

unsigned short read_uint16(void)
{
	unsigned int ret;
	ret = fgetc(fptr) | (fgetc(fptr)<<8);
	return ret;
}

unsigned short read_uint8(void)
{
	unsigned int ret;
	ret = fgetc(fptr);
	return ret;
}

//Saves some repetition
void show_requirements(void)
{
	printf("File must be an uncompressed PCM wave file at 8 bits per sample in mono!\n\n");
	printf("Please correct the file to meet these specifications and try again.\n");
}

void show_syntax(void)
{
	printf("Correct syntax is vbwav <file.wav> [8/15/32/45]\n");
	printf("Output file will be file.h\n");
}

//Make sure it is a WAV file in the correct format
int check_file(void)
{
	if(read_uint8() != 'R')
	{
		printf("Fatal Error: File does not appear to be a WAV file!\n");
		show_requirements();
		return 1;
	}
	fseek(fptr, 3, SEEK_CUR);
	length = read_uint32();

	fseek(fptr, 22, SEEK_SET);
	if(read_uint16() != 0x01)
	{
		show_requirements();
		return 1;
	}
	printf("Sampling rate: %d Hz\n", read_uint32());
	fseek(fptr, 6, SEEK_CUR);
	if(read_uint16() != 8)
	{
		show_requirements();
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	unsigned char *wave;
	unsigned int i;
	unsigned int linecount=0;
	unsigned char max_size = 0;
	unsigned char name[256];
	
	printf("WAV to Virtual Boy Sample Converter\n");
	printf("version 1.00 (30-July-2003)\n");
	printf("(c) 2003 frostgiant\n\n");
	
	if(argc!=2 && argc!=3)
	{
		show_syntax();
		return 1;
	}
	
	fptr = fopen(argv[1], "rb");

	int levelAmplitude = 45;

	if(argv[2])
	{
		levelAmplitude = atoi(argv[2]);
	}

	printf("Amplitude: %d\n", levelAmplitude);

	if(!fptr)
	{
		printf("Fatal Error: File not found!\n");
		return 1;
	}

	if(check_file())
		return 1;

	//cut out header from size
	length -= 28;

	//get enough memory for the entire file
	wave = calloc(length, 1);
	
	if(!wave)
	{
		printf("Fatal Error: Could not allocate memory!\n");
		return 1;
	}

	//copy file to memory, also track how big the biggest byte is
	for(i = 0; i < length; i++)
	{
		wave[i] = read_uint8();
		
		if(wave[i] > max_size)
		{
			max_size = wave[i];
		}
	}
	fclose(fptr);

	float scale = (float)levelAmplitude / 0x3F;

	printf("Scale: %f\n", scale);

/*
	//is the sample already 6 bit?
	if(max_size>0x3F)
	{
		//if 8 bit, shift away lowest 2
		if(max_size>0x1F)
		{
			for(i=0;i<length;i++)
				wave[i] = (wave[i]>>4); 
		}
		
		//if 7 bit, shift away lowest 1
		else
		{
			for(i=0;i<length;i++)
				wave[i] = (wave[i]>>2);
		}
	}
*/
	for(i=0;i<length;i++) {

		wave[i] = (float)(wave[i] >> 2) * scale;
	}
/*
	for(i=0;i<length;i++) {

		wave[i] >>= 4; 
		wave[i] = (wave[i] << 4) | wave[i];
	}
*/
//		wave[i] = (float)wave[i] * scale + 0.5f;
	/*

	int temp, min=65536, max=0;

	for (int i=0; i<length; i++) //find min and max in data
	{
			temp=wave[i];
			if (temp>max)
				max=temp;
			if (temp<min)
				min=temp;
	}

	double scale_factor = 1.0;

	//don't do anything if max = min
	if (min!=max)
	{
		if ((128-min)>=(max-127)) //mag of min larger than max
			scale_factor=(128.0/((double)(128-min)));
		else
			scale_factor=(128.0/((double)(max-127)));
	}
	
	//cout << scale_factor << " " << max << " " << min << endl; //debugging
	
	for (int i=0; i<length; i++) //scale data by scale factor
		wave[i]=(unsigned char)(((double)(wave[i]-128))*scale_factor)+128;

int which_half = 0;
unsigned char outdata = 0;

	for(i=0;i<length;i++){
		outdata |= (wave[i]>>4) << ((which_half^1)*4);

		if (which_half==1)
		{
			wave[i] = outdata;
			outdata=0;
		}
		which_half^=1;
	}
*/

	//find the '.' in the file argument and make the string end there
	i=0;
	while(argv[1][i])
	{
		if(argv[1][i]=='.')
		{
			argv[1][i] = '\0';
			break;
		}
		
		i++;
	}
	
	//form the header file name
	strcpy(name, argv[1]);
	strcat(name, ".h");

	fptr = fopen(name, "w");
	if(!fptr)
	{
		printf("Fatal Error: Could not open output header!\n");
		return 1;
	}

	printf("Output file: %s\n", name);
	
	fprintf(fptr, "/* This file was generated by vbwav. The following array holds\n");
	fprintf(fptr, "   uncompressed 6 bit PCM data. */\n\n");
	fprintf(fptr, "#define SAMPLE_%s_LEN\t%d\n\n", argv[1], length);
	fprintf(fptr, "static const unsigned char sample_%s[] =\n{\n\t", argv[1]);
	
	//output the main array
	for(i=0;i<length;i++)
	{
		linecount++;
		fprintf(fptr, "0x%02X, ", wave[i]);
		if(linecount>10)
		{
			linecount=0;
			fprintf(fptr, "\n\t");
		}
	}
	fprintf(fptr, "\n};\n");
	
	fclose(fptr);
	free(wave);
	
	return 0;
}
