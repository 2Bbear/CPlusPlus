#pragma once
//#include "memlib.c"
namespace twobbearlib
{
#define WSIZE 4 // word 크기를 지정.
#define DSIZE 8 // wouble word의 크기 지정.
#define CHUNKSIZE (1<<12) // 초기 Heap의 크기를 설정해 준다.(4096)
#define OVERHEAD 8 // header + footer의 사이즈.
#define MAX(x, y) ((x) > (y)? (x) : (y))// x와 y를 비교하여 더 큰 값을 반환한다.
#define PACK(size, alloc) ((size) | (alloc))// size와 alloc(a)의 값을 한 word로 묶는다. 편리하게 header와 footer에 저장하기 위함.
#define GET(p) ( *(unsigned int *)(p) )//포인터 p가 가리키는 곳의 한 word의 값을 읽어온다.
#define PUT(p, val) ( *(unsigned int *)(p) = (val) )//포인터 p가 가르키는 곳의 한 word의 값에 val을 저장한다.
#define GET_SIZE(p) (GET(p) & ~0x7)//포인터 p가 가리키는 곳에서 한 word를 읽고 하위 3bit를 버린다. Header에서 block size를 읽기위함.

/*
포인터 p가 가리키는 곳에서 한 word를 읽고 최하위 1bit를 가져온다.
block의 할당여부 체크에 사용된다.
할당된 블록이라면 1, 아니라면 0.
*/
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - WSIZE)//주어진 포인터 bp의 header의 주소를 계산한다.
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)//주어진 포인터 bp의 footer의 주소를 계산한다.
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE( ((char *)(bp) - WSIZE)) )//주어진 포인터 bp를 이용하여 다음 블록의 주소를 얻어 온다.
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE( ((char *)(bp) - DSIZE)) )//주어진 포인터 bp를 이용하여 이전 블록의 주소를 얻어 온다.


	class ImplicitFreeList
	{
	private:
		// Implicit 힙 관리
		static char* heap_listp; //처음 first block의 포인터를 선언
		void* heap_cur; // 현재 block의 위치를 가리키는 포인터
		//Implicit 구현 함수
		int mm_int()
		{
			//if ((heap_listp = mem_sbrk[4 * WSIZE]) == NULL) return -1;
		}
		void* malloc(size_t size);
		void* realloc(void* oldptr, size_t size);
		void free(void* bp);
		static void *coalesce(void*);
		static void *find_fit(size_t asize);
		static void* extend_heap(size_t words);
		static void place(void* bp, size_t asize);


	};
}

