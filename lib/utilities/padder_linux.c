#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	unsigned long total;
	unsigned int i,filesize,headsize;
	unsigned char *pad;
	char *str;
	FILE *fp,*fpAux;
        if ((argc != 2)) {
		if ((str = strrchr(argv[0],'\\'))) str++; //filter directori
		else str = argv[0];
		printf("Usage: %s <infile> ",str);
		return 1;							        }

	if (!(fp = fopen(argv[1],"a+b"))) {
		printf("Cannot open \"%s\"\n",argv[1]);
		return 1;
	}
	//put pointer to the end of ROM
	fseek(fp,0,SEEK_END);
	//create an auxiliary fp
	fpAux=fopen(argv[1],"a+b");
	//move auxFp to the end of ROM
	fseek(fpAux,0,SEEK_END);
	//move auxFp up 5to the end of ROM
	fseek(fpAux,-544,SEEK_CUR);
	filesize = ftell(fp);
	if(filesize >  1048032){
		total= 1048032 - filesize;
	}
	else{
		total= 1048032 * 2 - filesize;
	}
	if (!(pad = (unsigned char*)malloc(total))) {
		printf("Cannot allocate memory!\n");
		return 1;
	}
	memset(pad,0xFF,total);
	fwrite(pad,1,total,fp);
	{
	int count=0;
	unsigned short buffer=0;
	do{
		 fread(&buffer, sizeof(buffer), 1,fpAux);
		 fwrite(&buffer,sizeof(buffer), 1,fp);
	}while(count++ < 543/2);


	}

	fclose(fp);
	fclose(fpAux);
	printf("Completed padding to %d bytes, %d bytes added\n",total+filesize,total);
	return 0;
}
