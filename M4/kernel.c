void printString(char*);
void copyMessageToStdout(char*);
void printNumber(int);
void readString(char*);
void readSector(char*, int);
void writeSector(char*, int);
void readFile(char* , char*);
void writeFile(char*, char*, int);
void deleteFile(char*);
void executeProgram(char*, int);
void handleInterrupt21(int, int, int, int);
void terminateProgram();
void println();
int div(int, int);
int mod(int, int);

char line[82];
char stdout[81];
char buffer[13312]; /*this is the maximum size of a file*/
char sector[512]; // temp array to read sectors on
char directory[512];
char map[512];
char detectedFreeSectors[512];

main( void )
{
	/*
	readString(line);
	printString(line);
	interrupt(0x10, 0xE*256+0xd, 0, 0, 0); //carriage return
	*/

	// makeInterrupt21();
	// interrupt(0x21, 3, "messag\0", buffer, 0); /*read the file into buffer*/
	// interrupt(0x21, 0, buffer, 0, 0); /*print out the file*/

	/*printString("Enter a line:\n\0");
	interrupt(0x10, 0xE*256+0xd, 0, 0, 0); //carriage return

	makeInterrupt21();

	interrupt(0x21,1,line,0,0);
	interrupt(0x21,0,line,0,0);
	interrupt(0x10, 0xE*256+0xd, 0, 0, 0); //carriage return
	*/

	// makeInterrupt21();
	// interrupt(0x21, 4, "tstpr2\0", 0x2000, 0);
	// while(1);

	makeInterrupt21();
	interrupt(0x21,4,"shell\0",0x2000,0);

	// makeInterrupt21();
	// interrupt(0x21, 7, "messag\0", 0, 0); //delete messag
	// interrupt(0x21, 3, "messag\0", buffer, 0); // try to read messag
	// interrupt(0x21, 0, buffer, 0, 0); //print out the contents of buffer
	// while(1);
	//
	// int i=0;
	// //FIXME hey local
	// char buffer1[13312];
	// char buffer2[13312];
	// buffer2[0]='h'; buffer2[1]='e'; buffer2[2]='l'; buffer2[3]='l';
	// buffer2[4]='o';
	// for(i=5; i<13312; i++) buffer2[i]=0x0;
	//
	// makeInterrupt21();
	// interrupt(0x21,8, "testW\0", buffer2, 1); //write file testW
	// interrupt(0x21,3, "testW\0", buffer1, 0); //read file testW
	// interrupt(0x21,0, buffer1, 0, 0); // print out contents of testW
	// while(1);
}

void printString(char* chars)
{
	int i;
	for (i = 0; chars[i] != '\0'; i++){
		int currChar = chars[i];
		interrupt(0x10, 0xE*256+currChar, 0, 0, 0);
	};
}

void println()
{
	interrupt(0x10, 0xE*256+0xa, 0, 0, 0); //line feed "new line"
	interrupt(0x10, 0xE*256+0xd, 0, 0, 0); //carriage return
}

void printNumber(int num)
{
	int currDigit,mask,i;
	do{
		i = num;
		mask = 1;
		while(i>9){
			i = div(i,10);
			mask = mask*10;
		}
		currDigit = div(num,mask);
		num = mod(num,mask);
		interrupt(0x10, 0xE*256+currDigit+48, 0, 0, 0);
	}while(num!=0);
}


void readString(char* chars)
{
	int index;
	char x,enter,backspace;
	backspace = 0x8;
	enter = 0xd;
	index = 0;
   	while(1){
		x = interrupt(0x16, 0, 0, 0, 0);
		if(x == enter){
			interrupt(0x10, 0xE*256+0xa, 0, 0, 0); //line feed "new line"
			interrupt(0x10, 0xE*256+0xd, 0, 0, 0); //carriage return
			chars[index] = 0x0;                    //end of string
			break;
		}else if(x == backspace && index!=0){
			interrupt(0x10, 0xE*256+x, 0, 0, 0);     // print backspace
			interrupt(0x10, 0xE*256+0x20, 0, 0, 0);  // print space
			interrupt(0x10, 0xE*256+x, 0, 0, 0);     // print backspace again
		    index--;
		}else if (x != backspace && index < 80) {
			interrupt(0x10, 0xE*256+x, 0, 0, 0);
			chars[index++] = x;
		}
	}
}


void readSector(char* buffer, int sector)
{
	/*
	• AH = 2 (this number tells the BIOS to read a sector as opposed to write)
	• AL = number of sectors to read (use 1)
	• BX = address where the data should be stored to (pass your char* array here)
	• CH = track number
	• CL = relative sector number
	• DH = head number
	• DL = device number (for the floppy disk, use 0)
	relative sector = ( sector MOD 18 ) + 1
	head = ( sector / 18 ) MOD 2
	track = ( sector / 36 )
	*/

	int CX,DX,CH,CL,DH,DL;
	CH = div(sector , 36);
    CL = mod(sector , 18) + 1;
    DH = div(sector , 18);
    DH = mod(DH ,2);
    DL = 0;
    CX = CH*256 + CL;
    DX = DH*256 + DL;
	interrupt(0x13, 0x201, buffer, CX, DX);
}

void writeSector(char* buffer, int sector)
{
	int CX,DX,CH,CL,DH,DL;
	CH = div(sector , 36);
    CL = mod(sector , 18) + 1;
    DH = div(sector , 18);
    DH = mod(DH ,2);
    DL = 0;
    CX = CH*256 + CL;
    DX = DH*256 + DL;
	interrupt(0x13, 0x301, buffer, CX, DX);
}

void readFile(char* buffer, char* fileName)
{
	int i,j,entry,sectorNum,count;
	int flag = 0;
	readSector(directory,2);

	for (i = 0; i < 16; i++){
		for (j = 0; j < 6; j++){
			if(directory[i*32 + j]!=fileName[j]){
				break;
			}
		};
		if(j == 6){
			flag = 1;
			entry = i;
			break;
		}
	};

	if(flag==0){
		printString(stdout);
		println();
		return;
	}
	count = 0;
	for (j = 6; j < 32; j++){
		sectorNum = directory[entry*32+j];
		if(sectorNum == 0)
			break;
		readSector(buffer+count,sectorNum);
		count = count + 512;
	}
}

void writeFile(char* name, char* buffer, int secNum)
{
	int i , j , entry ;
	readSector(map,1);
	readSector(directory,2);
	for (i = 0; i < 16; i++){
		if(directory[i*32] == 0x00)
			{
				entry = i ;
				for(j = 0 ; name[j] != 0x00 ; j++){
					directory[(entry * 32 )+j] = name[j];
				}
				for(;j<6;j++){
					directory[(entry * 32 )+j] = 0x00;
				}
				break;
			}
	};
	j = 0;

	//Looping through map
	for(i = 0;i<512 && j < secNum;i++){
		if(map[i] == 0x00) {
			detectedFreeSectors[j++] = i;
		}
	}
	// If there is no sectors free for our file
	if(j < secNum){
		printString(name);
		println();
		return;
	}

	j = 0 ;
	for(i=0;i<secNum;i++){
		map[detectedFreeSectors[i]] = 0xFF;
		directory[entry*32 + i + 6] = detectedFreeSectors[i]-1;
		writeSector(buffer + j,detectedFreeSectors[i]-1);
		j+=512;
	}

	for(;i<512;i++){
		directory[entry*32 + i + 6] = 0x00 ;
	}
	writeSector(map,1);
	writeSector(directory,2);
}

void deleteFile(char* name)
{
	int i,j,entry,sectorNum;
	int flag = 0;
	readSector(directory,2);
	for (i = 0; i < 16; i++){
		for (j = 0; j < 6; j++){
			if(directory[i*32 + j]!=name[j]){
				break;
			}
		};
		if(j == 6){
			flag = 1;
			entry = i;
			break;
		}
	};
	if(flag==0){
		printString(stdout);
		println();
		return;
	}
	//the file is found , set the first byte to 0x00
	directory[32*entry] = 0x00;

	//set the correspoding sectors to 0x00 in the map
	readSector(map,1);
	for (j = 6; j < 32; j++){
		sectorNum = directory[entry*32+j];
		if(sectorNum == 0)
			break;
		map[sectorNum+1] = 0x00;
	}
	writeSector(map,1);
	writeSector(directory,2);
}

void executeProgram(char* name,int segment) {

	int offset = 0x0000;
	int base = segment*10;
	int i = 0;

	readFile(buffer,name);

	while(i < 13312) {
		putInMemory(segment,offset,buffer[i++]);
		offset++;
	}

	launchProgram(segment);
}

void terminateProgram()
{
	char name[6];
	name[0] = 's';
	name[1] = 'h';
	name[2] = 'e';
	name[3] = 'l';
	name[4] = 'l';
	name[5] = '\0';
	interrupt(0x21,4,name,0x2000,0);
}

void handleInterrupt21(int ax, int bx, int cx, int dx)
{
	if(ax == 0){
		printString(bx);
	}else if(ax == 1){
		readString(bx);
	}else if(ax == 2){
		readSector(bx,cx);
	}else if(ax == 3){
		copyMessageToStdout(dx);
		readFile(cx,bx);
	}else if(ax == 4){
		executeProgram(bx,cx);
	}else if(ax == 5){
		terminateProgram();
	}else if(ax == 6){
		writeSector(bx,cx);
	}else if(ax == 7){
		copyMessageToStdout(dx);
		deleteFile(bx);
	}else if(ax == 8){
		writeFile(bx,cx,dx);
	}else if (ax == 10) {
		println();
	}else if (ax == 11) {
		printNumber(bx);
	}else{
		printString("Error AX value should be < 12\0");
	}
}

void copyMessageToStdout(char* message){
  int i;
  for (i = 0; message[i] != '\0'; i++){
    stdout[i] = message[i];
  };
}


int div(int a, int b)
{
    int res;
    for (res = 0; a >= b; a -= b, res++);

    return res;
}

int mod(int a, int b)
{
	int res = div(a, b);
	res = res*b;
	res = a - res;
	return res;
}
