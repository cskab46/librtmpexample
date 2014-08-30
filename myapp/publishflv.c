#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifndef RTMPDUMP_VERSION
#define RTMPDUMP_VERSION "v2.4"
#endif 

#include "librtmp/rtmp_sys.h"
#include "librtmp/log.h"
/*
build in unix
gcc -Wall -o sendflvrtmp  sendflvrtmp.c -lpthread -Llibrtmp -lrtmp -lssl -lcrypto -lz
small head
*/
/*
#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"lib/librtmp.lib")
#endif
*/

#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

int ReadU8(uint32_t *u8,FILE*fp);
int ReadU16(uint32_t *u16,FILE*fp);
int ReadU24(uint32_t *u24,FILE*fp);
int ReadU32(uint32_t *u32,FILE*fp);
int PeekU8(uint32_t *u8,FILE*fp);
int ReadTime(uint32_t *utime,FILE*fp);

//RTMP_XXX()����0��ʾʧ�ܣ�1��ʾ�ɹ�

RTMP*rtmp=NULL;//rtmpӦ��ָ��
RTMPPacket*packet=NULL;//rtmp���ṹ
char* rtmpurl="rtmp://192.168.1.101:1935/live/test";//���ӵ�URL
char* flvfilename="test.flv";//��ȡ��flv�ļ�



int ZINIT();//��ʼ�����
void ZCLEAR();//������

int main(){
	long start=0;
	long perframetime=0;
	long lasttime=0;
	int bNextIsKey=1;
	RTMP_LogLevel lvl=RTMP_LOGINFO;
	FILE*fp=NULL;	
	if (!ZINIT()){
		printf("Init Socket Err\n");
		return -1;
	}
/////////////////////////////////��ʼ��//////////////////////	
//	RTMP_debuglevel=RTMP_LOGINFO;//��Ϣ�ȼ�(0-6)
	RTMP_LogSetLevel(lvl);//������Ϣ�ȼ�
//	RTMP_LogSetOutput(FILE*fp);//������Ϣ����ļ�

	rtmp=RTMP_Alloc();//����rtmp�ռ�
	RTMP_Init(rtmp);//��ʼ��rtmp����
	rtmp->Link.timeout=5;//�������ӳ�ʱ����λ�룬Ĭ��30��
	packet=(RTMPPacket*)malloc(sizeof(RTMPPacket));//������
	memset(packet,0,sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet,1024*64);//��packet�������ݿռ�
	RTMPPacket_Reset(packet);//����packet״̬
////////////////////////////////����//////////////////
	RTMP_SetupURL(rtmp,rtmpurl);//����url
	RTMP_EnableWrite(rtmp);//���ÿ�д״̬
	//���ӷ�����
	if (!RTMP_Connect(rtmp,NULL)){
		printf("Connect Err\n");
		ZCLEAR();
		return -1;
	}
	//������������(ȡ����rtmp->Link.lFlags)
	if (!RTMP_ConnectStream(rtmp,0)){
		printf("ConnectStream Err\n");
		ZCLEAR();
		return -1;
	}
	packet->m_hasAbsTimestamp = 0; //����ʱ���
	packet->m_nChannel = 0x04; //ͨ��
	packet->m_nInfoField2 = rtmp->m_stream_id;

	fp=fopen(flvfilename,"rb");
	if (fp==NULL){
		printf("Open File:%s Err\n",flvfilename);
		ZCLEAR();
		return -1;
	}

	printf("rtmpurl:%s\nflvfile:%s\nsend data ...\n",rtmpurl,flvfilename);
////////////////////////////////////////��������//////////////////////
	fseek(fp,9,SEEK_SET);//����ǰ9���ֽ�
	fseek(fp,4,SEEK_CUR);//����4�ֽڳ���
	start=time(NULL)-1;
	perframetime=0;//��һ֡ʱ���
	while(TRUE){
		uint32_t type=0;//����
		uint32_t datalength=0;//���ݳ���
		uint32_t timestamp=0;//ʱ���
		uint32_t streamid=0;//��ID
		uint32_t alldatalength=0;//��֡�ܳ���
	
		if(((time(NULL)-start)<(perframetime/1000))&&bNextIsKey){	
			//����̫��͵�һ��
			if(perframetime>lasttime){
				printf("TimeStamp:%8lu ms\n",perframetime);
				lasttime=perframetime;
			}
#ifdef WIN32
			Sleep(1000);
#else			
			sleep(1);
#endif
			continue;
		}	
		if(!ReadU8(&type,fp))
			break;
		if(!ReadU24(&datalength,fp))
			break;
		if(!ReadTime(&timestamp,fp))
			break;
		if(!ReadU24(&streamid,fp))
			break;
		if (type!=0x08&&type!=0x09){
			//����������Ƶ��
			fseek(fp,datalength+4,SEEK_CUR);
			continue;
		}
		if(fread(packet->m_body,1,datalength,fp)!=datalength)
			break;
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM; 
		packet->m_nTimeStamp = timestamp; 
		packet->m_packetType=type;
		packet->m_nBodySize=datalength;

		if (!RTMP_IsConnected(rtmp)){
			printf("rtmp is not connect\n");
			break;
		}
		if (!RTMP_SendPacket(rtmp,packet,0)){
			printf("Send Err\n");
			break;
		}
		if(!ReadU32(&alldatalength,fp))
			break;
		perframetime=timestamp;
///////////////�ж���һ֡�Ƿ�ؼ�֡////////////////		
		bNextIsKey=0;
		if(!PeekU8(&type,fp))
			break;
		if(type==0x09){
			if(fseek(fp,11,SEEK_CUR)!=0)
				break;
			if(!PeekU8(&type,fp)){
				break;
			}
			if(type==0x17){
				bNextIsKey=1;
			}
			fseek(fp,-11,SEEK_CUR);
		}
////////////////////////////////////		
	}
	printf("\nSend Data Over\n");
	fclose(fp);
	ZCLEAR();
	return 0;
}

int ZINIT(){
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version=MAKEWORD(2,2);
	if(WSAStartup(version,&wsaData)!=0){
		return 0;
	}
#endif
	return 1;
}
void ZCLEAR(){
	//////////////////////////////////////////�ͷ�/////////////////////
	if (rtmp!=NULL){
		RTMP_Close(rtmp);//�Ͽ�����
		RTMP_Free(rtmp);//�ͷ��ڴ�
		rtmp=NULL;
	}
	if (packet!=NULL){
		RTMPPacket_Free(packet);//�ͷ��ڴ�
		free(packet);
		packet=NULL;
	}
	///////////////////////////////////////////////////
#ifdef WIN32
	WSACleanup();
#endif
}

int ReadU8(uint32_t *u8,FILE*fp){
	if(fread(u8,1,1,fp)!=1)
		return 0;
	return 1;
}
int ReadU16(uint32_t *u16,FILE*fp){
	if(fread(u16,2,1,fp)!=1)
		return 0;
	*u16=HTON16(*u16);
	return 1;
}
int ReadU24(uint32_t *u24,FILE*fp){
	if(fread(u24,3,1,fp)!=1)
		return 0;
	*u24=HTON24(*u24);
	return 1;
}
int ReadU32(uint32_t *u32,FILE*fp){
	if(fread(u32,4,1,fp)!=1)
		return 0;
	*u32=HTON32(*u32);
	return 1;
}
int PeekU8(uint32_t *u8,FILE*fp){
	if(fread(u8,1,1,fp)!=1)
		return 0;
	fseek(fp,-1,SEEK_CUR);
	return 1;
}
int ReadTime(uint32_t *utime,FILE*fp){
	if(fread(utime,4,1,fp)!=1)
		return 0;
	*utime=HTONTIME(*utime);
	return 1;
}
