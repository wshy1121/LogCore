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
//实际分配/释放函数声明。 
//************************************* 
void* __real_malloc(size_t); 
void __real_free(void*); 
void* __real_memalign(size_t, size_t); 

//************************************* 
//这两个符号为链接器定义的， 
//代码里可以直接引用。 
//__executable_start表示程序首地址 
//__etext表示代码段结束的地址 
//************************************** 
extern char __executable_start[]; 
extern char __etext[]; 


//************************************* 
//头信息： 
//目前大小为32个字节。 
//结构体形式如下: 
//struct 
//{ 
// unsigned char headFlag[4] 
// unsigned int instructionAddr[4] 
// unsigned char allocateInfoSize[4] //allocateInfoSize[0]表示分配方式，allocateInfoSize[1]表示对齐方式。 
//} 
//************************************** 
const unsigned int headFlagSize = 4; 
const unsigned int instructionAddrSize = 24; 
const unsigned int allocateInfoSize = 4; 

//************************************* 
//尾信息： 
//目前大小写死为8个字节， 
//但只会填充前4个字节， 
//如果为0表示没有尾信息。 
//************************************** 
const unsigned int tailFlagSize = 0;// 8 or 0 
const unsigned int fillTailFlagSize = 0;// 4 or 0 

//************************************* 
//栈搜索区域大小，目前为4K。 
//************************************** 
const unsigned int searchSize = 1024; 

//************************************* 
//标记信。 
//************************************** 
enum 
{ 
 mallocHeadFlag = 0xaa,  //已分配头标记 
 mallocTailFlag = 0xbb,  //已分配尾标记 
 freeHeadFlag = 0xcc,  //已释放头标记 
 freeTailFlag = 0xdd,  //已释放尾标记 
 mallocUserDataFlag = 0xee, //已分配用户区域标记 
 freeUserDataFlag = 0xff  //已释放用户区域标记 
}; 

//************************************* 
//分配方式。目前只用了mallocType 
//和memAlignType这两种分配方式。 
//************************************** 
enum 
{ 
 mallocType = 1, //通过malloc分配出来的内存 
 reallocType, //通过realloc分配出来的内存   
 callocType,  //通过calloc分配出来的内存 
 vallocType,  //通过valloc分配出来的内存 
 memAlignType //通过memalign分配出来的内存 
}; 

//************************************************* 
//__wrap_malloc里每个节点会多分配40个字节， 
//来记录内存块的额外的信息。 
//****************************************************** 
void* __wrap_malloc(size_t c) 
{ 
 if(c == 0) 
 { 
  return 0; 
 } 
 //计算正真需要分配的内存大小，并通过__real_malloc分配内存。 
 const unsigned int totalSize = c + headFlagSize + instructionAddrSize + allocateInfoSize + tailFlagSize; 
 unsigned char* addr = (unsigned char*)__real_malloc(totalSize); 

 //将分配区域初始化为mallocUserDataFlag，但是复用下一块的4个字节并没有初始化。 
 const unsigned int chunkSize = *(unsigned int*)(addr - 4) & (~7); 
 memset(addr, mallocUserDataFlag, chunkSize - 8); 

 //填充头标志 
 unsigned char* headPtr = addr; 
 memset(headPtr, mallocHeadFlag, headFlagSize); 

 //搜索调用指令地址(其实是调用指令的下一条指令的地址)， 
 //并将该地址记录在instructionAddrPtr中，最多记录6个地址。 
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

 //填写分配信息 
 unsigned char* allocateInfoPtr = addr + headFlagSize + instructionAddrSize; 
 memset(allocateInfoPtr, 0x00, allocateInfoSize); 
 *allocateInfoPtr = mallocType; 

 //填充尾信息 
 if(fillTailFlagSize != 0) 
 { 
  unsigned char* tailPtr = addr - 8 + chunkSize - fillTailFlagSize; 
  memset(tailPtr, mallocTailFlag, fillTailFlagSize); 
 } 

 return addr + headFlagSize + instructionAddrSize + allocateInfoSize; 
} 

//*********************************************************************************** 
//__wrap_free里会判断尾信息是否有被篡改，如果被篡改 
//则assert（如果没有尾信息则不会进行此检测）。除此之外， 
//free掉的内存块中也会记录free的调用链，可以方便以后内存定位。 
//************************************************************************************ 
void __wrap_free(void* addr) 
{ 
 if(addr == 0) 
 { 
  return; 
 } 
 //通过allocateInfoPtr得到将要释放内存的分配方式（目前只有mallocType和memAlignType两种）。 
 unsigned char* freeAddr = (unsigned char*)addr - allocateInfoSize - instructionAddrSize - headFlagSize; 
 unsigned char* allocateInfoPtr = freeAddr + headFlagSize + instructionAddrSize; 
 if(*allocateInfoPtr == memAlignType) 
 { 
  //当为memAlignType分配方式时，*(allocateInfoPtr+1)记录的是 
  //内存的对齐方式（这里是以2为底的对数形式记录的）。 
  const unsigned int offset = *(allocateInfoPtr+1); 
  freeAddr = (unsigned char*)addr - (1 << offset); 
  printf("memalign boundary(%u) offset(%u)\n", 1 << offset, offset); 
 } 
 else if(*allocateInfoPtr == mallocType) 
 { 
  //内存越界检测，当没有尾时则不检测。 
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
 //将要释放的内存块内容填充为freeUserDataFlag。 
 const unsigned int chunkSize = *(unsigned int*)(freeAddr - 4) & (~7); 
 memset(freeAddr, freeUserDataFlag, chunkSize - 8); 

 //找到并保存free的调用链。在ptmalloc中，当一个内存块为空闲块时，ptmalloc最多会用16个字节 
 //来存储自己的信息（当空闲块在large bins中，就用了16个字节，在其他bins中则没有这么多）， 
 //所以这里的头信息从freeAddr + 16开始存储。 
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
  //填充尾信息 
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
  //填充尾信息 
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

 //将内存归还给ptmalloc 
 return __real_free(freeAddr); 
} 

//**************************************************************** 
//__wrap_calloc的就是简单的调用__wrap_malloc并初始化为0。 
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
//__wrap_realloc调用__wrap_malloc和__wrap_free来实现。 
//********************************************************* 
void* __wrap_realloc(void *ptr, size_t size) 
{ 
 //判断是否真的需要malloc。 
 //ptr为NULL则直接调用__wrap_malloc。 
 //size为0则直接调用__wrap_free。 
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
 //通过*allocateInfoPtr的分配类型，计算出真正的首地址，并顺带做一下内存越界检测。 
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
 //计算当前内存块里用户数据大小。 
 //当有尾时，不会有用户数据在一下块内存头里（尾有8个字节），此时不用考虑块是否为MMAP方式分配。 
 //用户数据大小可以简单的为：块大小 － GLIBC头大小 － 头大小 － 填充尾大小。 
 //当没有尾时，用户数据可能会存在一下块内存头里。 
 //此时用户数据大小为：块大小 － GLIBC头大小 － 头大小 + （MMAP方式分配 ？ 0 ：4）。 
 //当ptmalloc用MMAP方式分配内存时，因为没有下一块，当前内存块不会借用下一个内存块的头4个字节。 
 const unsigned int chunkSize = *(unsigned int*)((unsigned char*)realAddr - 4) & (~7); 
 const bool isMemMap = *(unsigned int*)((unsigned char*)realAddr - 4) & (2); 
 const unsigned int reusedSize = fillTailFlagSize == 0 &&  !isMemMap ? 4 : 0;  
 const unsigned int userDataSize = chunkSize - 8 - headFlagSize - instructionAddrSize - allocateInfoSize - fillTailFlagSize + reusedSize; 

 //如果用户要求减少内存则简单的返回ptr 
 if(userDataSize >= size) 
 { 
  return ptr; 
 } 

 //如果用户要求增加内存则调__wrap_malloc重新分配内存，做拷贝，然后free掉老的内存。 
 void* addr = __wrap_malloc(size); 
 memcpy(addr, ptr, userDataSize); 
 __wrap_free(ptr); 

 return addr; 
} 

//********************************************************************************************* 
//__wrap_memalign调用__real_memalign实现。 
//为了写代码简单目前实现要求boundary >= headTotalSize。 
//此外，在实现中为了内存对齐，多分配了boundary个字节， 
//并且用这多出来的boundary个字节来记录头信息，为了节省内存 
//就省去了尾信息。因为memalign不跨平台，不建议使用，所以 
//第三方代码不会怎么用到，使用主要集中在公司代码中。而公司 
//代码中调用memalign的地方很少，如果有问题可以直接通过搜索源码定位。 
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

 //在allocateInfoPtr里记录分配方式和对齐方式（为了节约内存，采用取对数的方式记录的）。 
 //这些信息在free的时候会用到。 
 unsigned char* allocateInfoPtr = headPtr + headFlagSize + instructionAddrSize; 
 memset(allocateInfoPtr, 0x00, allocateInfoSize); 
 *allocateInfoPtr = memAlignType; 
 *(allocateInfoPtr + 1) = log2(boundary); 

 return addr + boundary; 
} 

//********************************************** 
//__wrap_valloc调用__wrap_memalign实现。目前 
//页大小写死为4K。 
//*********************************************** 
void* __wrap_valloc(size_t size) 
{ 
 printf("__wrap_valloc\n"); 
 return __wrap_memalign(1024*4, size); 
} 
} 
#endif 

