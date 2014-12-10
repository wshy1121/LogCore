#ifdef MEM_EXTRA_INFO 
/* 
********************************************************************************** 
**                                   MemExtraInfo.cpp 
**                          Digital Video Recoder xp 
** 
**   (c) Copyright 1992-2020, ZheJiang Dahua Information Technology Stock CO.LTD. 
**                            All Rights Reserved 
** 
** File   :MemExtraInfo.cpp 
** Description :  
** Create  : 2014/12/8      jingtaiyang  Create the file 
** Modify  : 2014/12/8      jingtaiyang  Modify the file 
********************************************************************************** 
*/ 

#include <stdio.h> 
#include <stdlib.h> 
#include <malloc.h> 
#include <string.h> 
#include <assert.h> 
#include <math.h> 

extern "C"{ 

//************************************* 
//ʵ�ʷ���/�ͷź��������� 
//************************************* 
void* __real_malloc(size_t); 
void __real_free(void*); 
void* __real_memalign(size_t, size_t); 

//************************************* 
//����������Ϊ����������ģ� 
//���������ֱ�����á� 
//__executable_start��ʾ�����׵�ַ 
//__etext��ʾ����ν����ĵ�ַ 
//************************************** 
extern char __executable_start[]; 
extern char __etext[]; 


//************************************* 
//ͷ��Ϣ�� 
//Ŀǰ��СΪ32���ֽڡ� 
//�ṹ����ʽ����: 
//struct 
//{ 
// unsigned char headFlag[4] 
// unsigned int instructionAddr[4] 
// unsigned char allocateInfoSize[4] //allocateInfoSize[0]��ʾ���䷽ʽ��allocateInfoSize[1]��ʾ���뷽ʽ�� 
//} 
//************************************** 
const unsigned int headFlagSize = 4; 
const unsigned int instructionAddrSize = 24; 
const unsigned int allocateInfoSize = 4; 

//************************************* 
//β��Ϣ�� 
//Ŀǰ��Сд��Ϊ8���ֽڣ� 
//��ֻ�����ǰ4���ֽڣ� 
//���Ϊ0��ʾû��β��Ϣ�� 
//************************************** 
const unsigned int tailFlagSize = 0;// 8 or 0 
const unsigned int fillTailFlagSize = 0;// 4 or 0 

//************************************* 
//ջ���������С��ĿǰΪ4K�� 
//************************************** 
const unsigned int searchSize = 1024; 

//************************************* 
//����š� 
//************************************** 
enum 
{ 
 mallocHeadFlag = 0xaa,  //�ѷ���ͷ��� 
 mallocTailFlag = 0xbb,  //�ѷ���β��� 
 freeHeadFlag = 0xcc,  //���ͷ�ͷ��� 
 freeTailFlag = 0xdd,  //���ͷ�β��� 
 mallocUserDataFlag = 0xee, //�ѷ����û������� 
 freeUserDataFlag = 0xff  //���ͷ��û������� 
}; 

//************************************* 
//���䷽ʽ��Ŀǰֻ����mallocType 
//��memAlignType�����ַ��䷽ʽ�� 
//************************************** 
enum 
{ 
 mallocType = 1, //ͨ��malloc����������ڴ� 
 reallocType, //ͨ��realloc����������ڴ�   
 callocType,  //ͨ��calloc����������ڴ� 
 vallocType,  //ͨ��valloc����������ڴ� 
 memAlignType //ͨ��memalign����������ڴ� 
}; 

//************************************************* 
//__wrap_malloc��ÿ���ڵ������40���ֽڣ� 
//����¼�ڴ��Ķ������Ϣ�� 
//****************************************************** 
void* __wrap_malloc(size_t c) 
{ 
 if(c == 0) 
 { 
  return 0; 
 } 
 //����������Ҫ������ڴ��С����ͨ��__real_malloc�����ڴ档 
 const unsigned int totalSize = c + headFlagSize + instructionAddrSize + allocateInfoSize + tailFlagSize; 
 unsigned char* addr = (unsigned char*)__real_malloc(totalSize); 

 //�����������ʼ��ΪmallocUserDataFlag�����Ǹ�����һ���4���ֽڲ�û�г�ʼ���� 
 const unsigned int chunkSize = *(unsigned int*)(addr - 4) & (~7); 
 memset(addr, mallocUserDataFlag, chunkSize - 8); 

 //���ͷ��־ 
 unsigned char* headPtr = addr; 
 memset(headPtr, mallocHeadFlag, headFlagSize); 

 //��������ָ���ַ(��ʵ�ǵ���ָ�����һ��ָ��ĵ�ַ)�� 
 //�����õ�ַ��¼��instructionAddrPtr�У�����¼6����ַ�� 
 unsigned char* instructionAddrPtr = addr + headFlagSize; 
 const unsigned int* sp = &totalSize; 
 for(size_t i = 0, j = 0; i < searchSize && j < (instructionAddrSize / 4); ++i, ++sp) 
 { 
  if(*sp >= (unsigned int)__executable_start && *sp <= (unsigned int)__etext) 
  { 
   *(unsigned int*)(instructionAddrPtr + 4*j) = *sp; 
   ++j; 
  } 
 } 

 //��д������Ϣ 
 unsigned char* allocateInfoPtr = addr + headFlagSize + instructionAddrSize; 
 memset(allocateInfoPtr, 0x00, allocateInfoSize); 
 *allocateInfoPtr = mallocType; 

 //���β��Ϣ 
 if(fillTailFlagSize != 0) 
 { 
  unsigned char* tailPtr = addr - 8 + chunkSize - fillTailFlagSize; 
  memset(tailPtr, mallocTailFlag, fillTailFlagSize); 
 } 

 return addr + headFlagSize + instructionAddrSize + allocateInfoSize; 
} 

//*********************************************************************************** 
//__wrap_free����ж�β��Ϣ�Ƿ��б��۸ģ�������۸� 
//��assert�����û��β��Ϣ�򲻻���д˼�⣩������֮�⣬ 
//free�����ڴ����Ҳ���¼free�ĵ����������Է����Ժ��ڴ涨λ�� 
//************************************************************************************ 
void __wrap_free(void* addr) 
{ 
 if(addr == 0) 
 { 
  return; 
 } 
 //ͨ��allocateInfoPtr�õ���Ҫ�ͷ��ڴ�ķ��䷽ʽ��Ŀǰֻ��mallocType��memAlignType���֣��� 
 unsigned char* freeAddr = (unsigned char*)addr - allocateInfoSize - instructionAddrSize - headFlagSize; 
 unsigned char* allocateInfoPtr = freeAddr + headFlagSize + instructionAddrSize; 
 if(*allocateInfoPtr == memAlignType) 
 { 
  //��ΪmemAlignType���䷽ʽʱ��*(allocateInfoPtr+1)��¼���� 
  //�ڴ�Ķ��뷽ʽ����������2Ϊ�׵Ķ�����ʽ��¼�ģ��� 
  const unsigned int offset = *(allocateInfoPtr+1); 
  freeAddr = (unsigned char*)addr - (1 << offset); 
  printf("memalign boundary(%u) offset(%u)\n", 1 << offset, offset); 
 } 
 else if(*allocateInfoPtr == mallocType) 
 { 
  //�ڴ�Խ���⣬��û��βʱ�򲻼�⡣ 
  const unsigned int chunkSize = *(unsigned int*)(freeAddr - 4) & (~7); 
  unsigned char* tailPtr = freeAddr - 8 + chunkSize - fillTailFlagSize; 
  if(fillTailFlagSize != 0 &&  
   !(tailPtr[0] == mallocTailFlag && tailPtr[1] == mallocTailFlag && 
    tailPtr[2] == mallocTailFlag && tailPtr[3] == mallocTailFlag)) 
  { 
   printf("invalid addr(%p)\n", addr); 
   assert(0); 
  } 
 } 
 else 
 { 
  printf("invalid addr(%p)\n", addr); 
  assert(0); 
 } 
 //��Ҫ�ͷŵ��ڴ���������ΪfreeUserDataFlag�� 
 const unsigned int chunkSize = *(unsigned int*)(freeAddr - 4) & (~7); 
 memset(freeAddr, freeUserDataFlag, chunkSize - 8); 

 //�ҵ�������free�ĵ���������ptmalloc�У���һ���ڴ��Ϊ���п�ʱ��ptmalloc������16���ֽ� 
 //���洢�Լ�����Ϣ�������п���large bins�У�������16���ֽڣ�������bins����û����ô�ࣩ�� 
 //���������ͷ��Ϣ��freeAddr + 16��ʼ�洢�� 
 const unsigned int totalSize = headFlagSize + instructionAddrSize + fillTailFlagSize; 
 if(chunkSize - 24 >= totalSize) 
 { 
  unsigned char* headPtr = freeAddr + 16; 
  memset(headPtr, freeHeadFlag, headFlagSize); 
  unsigned char* instructionAddrPtr = freeAddr + 16 + headFlagSize; 
  const unsigned int* sp = &totalSize; 
  for(size_t i = 0, j = 0; i < searchSize && j < (instructionAddrSize / 4); ++i, ++sp) 
  { 
   if(*sp >= (unsigned int)__executable_start && *sp <= (unsigned int)__etext) 
   { 
    assert((instructionAddrPtr + 4*j) <= (freeAddr - 8 + chunkSize - 4)); 
    *(unsigned int*)(instructionAddrPtr + 4*j) = *sp; 
    ++j; 
   } 
  } 
  //���β��Ϣ 
  if(fillTailFlagSize != 0) 
  { 
   unsigned char* tailPtr = freeAddr - 8 + chunkSize - fillTailFlagSize; 
   memset(tailPtr, freeTailFlag, fillTailFlagSize); 
  } 

 } 
 else if(chunkSize - 24 > fillTailFlagSize) 
 { 
  const int instructionSize = chunkSize - 24 - fillTailFlagSize; 
  unsigned char* instructionAddrPtr = freeAddr + 16; 
  const unsigned int* sp = &totalSize; 
  for(size_t i = 0, j = 0; i < searchSize && j < (instructionSize / 4); ++i, ++sp) 
  { 
   if(*sp >= (unsigned int)__executable_start && *sp <= (unsigned int)__etext) 
   { 
    assert((instructionAddrPtr + 4*j) <= (freeAddr - 8 + chunkSize - 4)); 
    *(unsigned int*)(instructionAddrPtr + 4*j) = *sp; 
    ++j; 
   } 
  } 
  //���β��Ϣ 
  if(fillTailFlagSize != 0) 
  { 
   unsigned char* tailPtr = freeAddr - 8 + chunkSize - fillTailFlagSize; 
   memset(tailPtr, freeTailFlag, fillTailFlagSize); 
  } 
 } 
 else if(chunkSize - 24 == fillTailFlagSize) 
 { 
  printf("invalid addr(%p) should not reach here\n", addr); 
  if(fillTailFlagSize != 0) 
  { 
   unsigned char* tailPtr = freeAddr - 8 + chunkSize - fillTailFlagSize; 
   memset(tailPtr, freeTailFlag, fillTailFlagSize); 
  } 
 } 

 //���ڴ�黹��ptmalloc 
 return __real_free(freeAddr); 
} 

//**************************************************************** 
//__wrap_calloc�ľ��Ǽ򵥵ĵ���__wrap_malloc����ʼ��Ϊ0�� 
//**************************************************************** 
void* __wrap_calloc(size_t nmemb, size_t size) 
{ 
 if(nmemb == 0 || size == 0) 
 { 
  return 0; 
 } 
 void* addr = __wrap_malloc(nmemb*size); 
 memset(addr, 0x00, nmemb*size); 

 return addr; 
} 

//********************************************************* 
//__wrap_realloc����__wrap_malloc��__wrap_free��ʵ�֡� 
//********************************************************* 
void* __wrap_realloc(void *ptr, size_t size) 
{ 
 //�ж��Ƿ������Ҫmalloc�� 
 //ptrΪNULL��ֱ�ӵ���__wrap_malloc�� 
 //sizeΪ0��ֱ�ӵ���__wrap_free�� 
 if(ptr == 0 && size == 0) 
 { 
  return 0; 
 } 
 else if(ptr == 0 && size != 0) 
 { 
  return __wrap_malloc(size); 
 } 
 else if(ptr != 0 && size == 0) 
 { 
  __wrap_free(ptr); 
  return 0; 
 } 
 //ͨ��*allocateInfoPtr�ķ������ͣ�������������׵�ַ����˳����һ���ڴ�Խ���⡣ 
 unsigned char* realAddr = (unsigned char*)ptr - allocateInfoSize - instructionAddrSize - headFlagSize; 
 unsigned char* allocateInfoPtr = realAddr + headFlagSize + instructionAddrSize; 
 if(*allocateInfoPtr == memAlignType) 
 { 
  const unsigned int offset = *(allocateInfoPtr+1); 
  realAddr = (unsigned char*)ptr - (1 << offset); 
  printf("memalign boundary(%u) offset(%u)\n", 1 << offset, offset); 
 } 
 else if(*allocateInfoPtr == mallocType) 
 { 
  const unsigned int chunkSize = *(unsigned int*)(realAddr - 4) & (~7); 
  unsigned char* tailPtr = realAddr - 8 + chunkSize - fillTailFlagSize; 
  if(fillTailFlagSize != 0 &&  
   !(tailPtr[0] == mallocTailFlag && tailPtr[1] == mallocTailFlag && 
    tailPtr[2] == mallocTailFlag && tailPtr[3] == mallocTailFlag)) 
  { 
   printf("invalid addr(%p)\n", ptr); 
   assert(0); 
  } 
 } 
 else 
 { 
  printf("invalid addr(%p)\n", ptr); 
  assert(0); 
 } 
 //���㵱ǰ�ڴ�����û����ݴ�С�� 
 //����βʱ���������û�������һ�¿��ڴ�ͷ�β��8���ֽڣ�����ʱ���ÿ��ǿ��Ƿ�ΪMMAP��ʽ���䡣 
 //�û����ݴ�С���Լ򵥵�Ϊ�����С �� GLIBCͷ��С �� ͷ��С �� ���β��С�� 
 //��û��βʱ���û����ݿ��ܻ����һ�¿��ڴ�ͷ� 
 //��ʱ�û����ݴ�СΪ�����С �� GLIBCͷ��С �� ͷ��С + ��MMAP��ʽ���� �� 0 ��4���� 
 //��ptmalloc��MMAP��ʽ�����ڴ�ʱ����Ϊû����һ�飬��ǰ�ڴ�鲻�������һ���ڴ���ͷ4���ֽڡ� 
 const unsigned int chunkSize = *(unsigned int*)((unsigned char*)realAddr - 4) & (~7); 
 const bool isMemMap = *(unsigned int*)((unsigned char*)realAddr - 4) & (2); 
 const unsigned int reusedSize = fillTailFlagSize == 0 &&  !isMemMap ? 4 : 0;  
 const unsigned int userDataSize = chunkSize - 8 - headFlagSize - instructionAddrSize - allocateInfoSize - fillTailFlagSize + reusedSize; 

 //����û�Ҫ������ڴ���򵥵ķ���ptr 
 if(userDataSize >= size) 
 { 
  return ptr; 
 } 

 //����û�Ҫ�������ڴ����__wrap_malloc���·����ڴ棬��������Ȼ��free���ϵ��ڴ档 
 void* addr = __wrap_malloc(size); 
 memcpy(addr, ptr, userDataSize); 
 __wrap_free(ptr); 

 return addr; 
} 

//********************************************************************************************* 
//__wrap_memalign����__real_memalignʵ�֡� 
//Ϊ��д�����Ŀǰʵ��Ҫ��boundary >= headTotalSize�� 
//���⣬��ʵ����Ϊ���ڴ���룬�������boundary���ֽڣ� 
//��������������boundary���ֽ�����¼ͷ��Ϣ��Ϊ�˽�ʡ�ڴ� 
//��ʡȥ��β��Ϣ����Ϊmemalign����ƽ̨��������ʹ�ã����� 
//���������벻����ô�õ���ʹ����Ҫ�����ڹ�˾�����С�����˾ 
//�����е���memalign�ĵط����٣�������������ֱ��ͨ������Դ�붨λ�� 
//*********************************************************************************************** 
void* __wrap_memalign(size_t boundary, size_t size) 
{ 
 printf("__wrap_memalign\n"); 
 const unsigned int headTotalSize = headFlagSize + instructionAddrSize + allocateInfoSize; 
 assert(boundary >= headTotalSize); 
 unsigned char* addr = (unsigned char*)__real_memalign(boundary, size + boundary); 
 const unsigned int chunkSize = *(unsigned int*)(addr - 4) & (~7); 
 memset(addr, mallocUserDataFlag, chunkSize - 8); 

 unsigned char* headPtr = addr + boundary - headTotalSize; 
 memset(headPtr, mallocHeadFlag, headFlagSize); 

 unsigned char* instructionAddrPtr = headPtr + headFlagSize; 
 const unsigned int* sp = &headTotalSize; 
 for(size_t i = 0, j = 0; i < searchSize && j < (instructionAddrSize / 4); ++i, ++sp) 
 { 
  if(*sp >= (unsigned int)__executable_start && *sp <= (unsigned int)__etext) 
  { 
   *(unsigned int*)(instructionAddrPtr + 4*j) = *sp; 
   ++j; 
  } 
 } 

 //��allocateInfoPtr���¼���䷽ʽ�Ͷ��뷽ʽ��Ϊ�˽�Լ�ڴ棬����ȡ�����ķ�ʽ��¼�ģ��� 
 //��Щ��Ϣ��free��ʱ����õ��� 
 unsigned char* allocateInfoPtr = headPtr + headFlagSize + instructionAddrSize; 
 memset(allocateInfoPtr, 0x00, allocateInfoSize); 
 *allocateInfoPtr = memAlignType; 
 *(allocateInfoPtr + 1) = log2(boundary); 

 return addr + boundary; 
} 

//********************************************** 
//__wrap_valloc����__wrap_memalignʵ�֡�Ŀǰ 
//ҳ��Сд��Ϊ4K�� 
//*********************************************** 
void* __wrap_valloc(size_t size) 
{ 
 printf("__wrap_valloc\n"); 
 return __wrap_memalign(1024*4, size); 
} 
} 
#endif 

