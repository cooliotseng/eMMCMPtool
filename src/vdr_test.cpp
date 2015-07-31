//============================================================================
// Name        : vdr_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/types.h>	//lseek, open
#include <sys/stat.h>	//read, write, open
#include <fcntl.h>	//open
#include <unistd.h>	//close




typedef unsigned char u8;


using namespace std;

void OpeneMMCTest(){
	int fd_0, fd_1;
		char Shellbuf[4096];
		string tShellbuf ;
		string mCmdIndexString;
		stringstream streamIndex;
		char *majornum;
		FILE *pp;

		if((pp=popen("cat /proc/devices | grep vdr_test","r")) == NULL){
			std::cout << "Popen() error: " << std::endl;
		}
		majornum = strtok(fgets(Shellbuf,sizeof(Shellbuf),pp)," ");

	    if(majornum == NULL){
	    	system("echo vli | sudo -S insmod mmc_test.ko");
	    }

	    if((pp=popen("ls /dev/vdr_test*","r")) == NULL){
	        std::cout << "Popen() error: " << std::endl;
	    }

	    if(fgets(Shellbuf,sizeof(Shellbuf),pp)==NULL){

	    	if((pp=popen("cat /proc/devices | grep vdr_test","r")) == NULL){
	    	    			std::cout << "Popen() error: " << std::endl;
	    	    }

	    	    majornum = strtok(fgets(Shellbuf,sizeof(Shellbuf),pp)," ");

	    	    for(int i=0;i<2;i++){
	    	    	streamIndex.str("");
	    	    	streamIndex << i;
	    	    	tShellbuf.assign("echo vli | sudo -S mknod /dev/vdr_test");
	    	    	tShellbuf.append(streamIndex.str()).append(" ")
	    	    			.append("c ")
	    					.append(majornum).append(" ")
	    					.append(streamIndex.str());
	    	        system(tShellbuf.c_str());
	    	    }
	    }

	    pclose(pp);
}

void CloseMMCTest(){
	system("ls /dev/vdr_test*");
	system("echo vli | sudo -S rm -f  /dev/vdr_test*");
	system("echo vli | sudo -S rmmod mmc_test");
}

static void set_VDR_packet_checksum (u8 *buf)
{
	int i,
	    j;
	for (i = 0; i < 4; i++) {
	    u8 temp = 0x00;
	    for (j = 0; j < 4; j++) {
		temp ^= buf[16 + i + j * 4];
	    }
	    buf[32 + i] = temp;
	}
}

static void set_VDR_cmd_packet_writeblock(u8 *buf)
{
    char VDR_serial[17] = "SBAL-AIVTCEJP394";
    memcpy (buf, VDR_serial, sizeof (VDR_serial));

    buf[16] = 0xEE;

    //Write Block
    buf[17] = 0x95;

    buf[18] = 0x00;
    buf[19] = 0x00;
    buf[20] = 0x0A;
    buf[21] = 0x00;

    buf[22] = 0x8B;

    buf[23] = 0x20;
    buf[24] = 0x20;

    buf[25] = 0x80;
}

const int vdr_hdr_size = 512;

int main() {

	OpeneMMCTest();

	int fd_0, fd_1;

	u8 *vdr_hdr;

	char *s = "This is the write function test\n";

	fd_0 = open ("/dev/vdr_test0", O_RDWR);
	if (fd_0 < 0/* || fd_1 < 0*/) {
		perror ("Open /dev/vdr_test error! QQ!");
	exit (1);
	}
	vdr_hdr = (u8 *)malloc (vdr_hdr_size * sizeof(char));
	memset(vdr_hdr, 0, vdr_hdr_size);

	set_VDR_cmd_packet_writeblock(vdr_hdr);
	set_VDR_packet_checksum (vdr_hdr);

	write (fd_0, vdr_hdr, vdr_hdr_size);

	close (fd_0);

	free(vdr_hdr);

	CloseMMCTest();
	return 0;
}


