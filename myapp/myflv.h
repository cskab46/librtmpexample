#ifndef MYFLV_H
#define MYFLV_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef MAX_FILE_PATH
#define MAX_FILE_PATH 260
#endif

//#define MYFLVMALLOC(a) (void*b=malloc(a);memset(b,0,a);b)
//#define MYFLVFREE(a)   do{free(a);a=NULL;}while(0)


#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

int ReadU8(uint32_t *u8,FILE*fp);
int ReadU16(uint32_t *u16,FILE*fp);
int ReadU24(uint32_t *u24,FILE*fp);
int ReadU32(uint32_t *u32,FILE*fp);
int PeekU8(uint32_t *u8,FILE*fp);
int ReadUTime(uint32_t *utime,FILE*fp);


int WriteU8(uint32_t u8,FILE*fp);
int WriteU16(uint32_t u16,FILE*fp);
int WriteU24(uint32_t u24,FILE*fp);
int WriteU32(uint32_t u32,FILE*fp);
int WriteUTime(uint32_t utime,FILE*fp);

typedef struct _MyFrame{
uint32_t type;//���ͣ�1�ֽ�0x08��Ƶ0x09��Ƶ
uint32_t datalength;//���ݳ��ȣ�3�ֽ�
uint32_t timestamp;//ʱ�����4�ֽ�
uint32_t streamid;//��ID��3�ֽ�

uint32_t bkeyframe;//�ؼ�֡0x17

char*	 buffer;//�洢����Ƶ������Ϣ
uint8_t  breadbuf;//��ȡ����
uint32_t alldatalength;//��֡�ܳ��ȣ�4�ֽ�
}MyFrame;

typedef struct _MyFLV{
FILE*fp;//�ļ�ָ��
char filename[MAX_FILE_PATH];
MyFrame*vi;//��Ƶ��Ϣ
MyFrame*ai;//��Ƶ��Ϣ
uint32_t startpos;//��ʼλ�ã�����Ƶ��Ϣ��
uint32_t totalsize;//�ļ��ܴ�С
uint32_t pos;//��ǰλ��
uint32_t currenttime;//��ǰʱ���ms
uint32_t duration;//��ʱ��
uint32_t looptimes;//ѭ������
uint8_t bloop;//�Ƿ�ѭ��
uint8_t beof;//�ļ���ȡ����
}MyFLV;
//��FLV�ļ�������
MyFLV*MyFlvOpen(const char*filename);
//��ȡ֡��Ϣ
//buf��Ϊ�ղ���len�㹻���������ݲ�������һ֡
//�����ڵ�ǰ֡
MyFrame MyFlvGetFrameInfo(MyFLV*myflv,char*buf,uint32_t len);


MyFLV*MyFlvCreate(const char*filename);
int MyFlvWriteFrame(MyFLV*myflv,MyFrame myframe,char*buf,uint32_t len);

MyFLV*MyFlvClose(MyFLV*myflv);
#endif