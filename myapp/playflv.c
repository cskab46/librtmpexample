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

#include "myflv.h"

void PlayFlv(const char*rtmpurl,const char*flvfilename,int bLive);

int MYINIT();//��ʼ�����
void MYCLEAR();//������
int main(){
	const char* rtmpurl="rtmp://192.168.1.100:1935/live/test";//���ӵ�URL
	const char* flvfilename="test2.flv";//�����flv�ļ�
	RTMP_LogLevel loglevel=RTMP_LOGINFO;//����RTMP��Ϣ�ȼ�
	RTMP_LogSetLevel(loglevel);//������Ϣ�ȼ�
//	RTMP_LogSetOutput(FILE*fp);//������Ϣ����ļ�
	MYINIT();
	PlayFlv(rtmpurl,flvfilename,0);
	MYCLEAR();
	return 0;
}

void PlayFlv(const char*rtmpurl,const char*flvfilename,int bLive){
	RTMP*rtmp=NULL;//rtmpӦ��ָ��
	RTMPPacket*packet=NULL;//rtmp���ṹ
	char url[256]={0};
	int buffsize=1024;
	char*buff=(char*)malloc(buffsize);
	double duration=-1;
	int nRead=0;
	FILE*fp=fopen(flvfilename,"wb");
	long 	countbuffsize=0;
	rtmp=RTMP_Alloc();//����rtmp�ռ�
	RTMP_Init(rtmp);//��ʼ��rtmp����
	rtmp->Link.timeout=25;//��ʱ����
	//����crtmpserver��ÿ��һ��ʱ��(Ĭ��8s)�������ݰ�,����ڷ��ͼ������
	strcpy(url,rtmpurl);
	RTMP_SetupURL(rtmp,url);
	if (bLive){
		//����ֱ����־
		rtmp->Link.lFlags|=RTMP_LF_LIVE;
	}
	RTMP_SetBufferMS(rtmp,100*1000);//10s
	if(!RTMP_Connect(rtmp,NULL)){
		printf("Connect Server Err\n");
		goto end;
	}
	if(!RTMP_ConnectStream(rtmp,0)){
		printf("Connect stream Err\n");
		goto end;
	}
#if 1
	packet=(RTMPPacket*)malloc(sizeof(RTMPPacket));//������
	memset(packet,0,sizeof(RTMPPacket));	
	RTMPPacket_Reset(packet);//����packet״̬
	while (RTMP_GetNextMediaPacket(rtmp,packet)){
		if(packet->m_packetType==0x09&&packet->m_body[0]==0x17)
			printf("TimeStamp:%u\n",packet->m_nTimeStamp);
		RTMPPacket_Free(packet);
		RTMPPacket_Reset(packet);//����packet״̬
	}
#else
	//��ֱ������ľ���FLV�ļ�,����FLVͷ,�ɶ�������flv��ʽ�����Ϳ���ǰ��Ƶ,��Ƶ����
	while(nRead=RTMP_Read(rtmp,buff,buffsize)){
		fwrite(buff,1,nRead,fp);
		countbuffsize+=nRead;
		printf("DownLand...:%0.2fkB",countbuffsize*1.0/1024);
	}
#endif
end:
	if(fp){
		fclose(fp);
		fp=NULL;
	}
	if(buff){
		free(buff);
		buff=NULL;
	}
	if(rtmp!=NULL){
		RTMP_Close(rtmp);//�Ͽ�����
		RTMP_Free(rtmp);//�ͷ��ڴ�
		rtmp=NULL;
	}
}

int MYINIT(){
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
void MYCLEAR(){
#ifdef WIN32
	WSACleanup();
#endif
}