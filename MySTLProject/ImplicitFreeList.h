#pragma once
//#include "memlib.c"
namespace twobbearlib
{
#define WSIZE 4 // word ũ�⸦ ����.
#define DSIZE 8 // wouble word�� ũ�� ����.
#define CHUNKSIZE (1<<12) // �ʱ� Heap�� ũ�⸦ ������ �ش�.(4096)
#define OVERHEAD 8 // header + footer�� ������.
#define MAX(x, y) ((x) > (y)? (x) : (y))// x�� y�� ���Ͽ� �� ū ���� ��ȯ�Ѵ�.
#define PACK(size, alloc) ((size) | (alloc))// size�� alloc(a)�� ���� �� word�� ���´�. ���ϰ� header�� footer�� �����ϱ� ����.
#define GET(p) ( *(unsigned int *)(p) )//������ p�� ����Ű�� ���� �� word�� ���� �о�´�.
#define PUT(p, val) ( *(unsigned int *)(p) = (val) )//������ p�� ����Ű�� ���� �� word�� ���� val�� �����Ѵ�.
#define GET_SIZE(p) (GET(p) & ~0x7)//������ p�� ����Ű�� ������ �� word�� �а� ���� 3bit�� ������. Header���� block size�� �б�����.

/*
������ p�� ����Ű�� ������ �� word�� �а� ������ 1bit�� �����´�.
block�� �Ҵ翩�� üũ�� ���ȴ�.
�Ҵ�� ����̶�� 1, �ƴ϶�� 0.
*/
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - WSIZE)//�־��� ������ bp�� header�� �ּҸ� ����Ѵ�.
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)//�־��� ������ bp�� footer�� �ּҸ� ����Ѵ�.
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE( ((char *)(bp) - WSIZE)) )//�־��� ������ bp�� �̿��Ͽ� ���� ����� �ּҸ� ��� �´�.
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE( ((char *)(bp) - DSIZE)) )//�־��� ������ bp�� �̿��Ͽ� ���� ����� �ּҸ� ��� �´�.


	class ImplicitFreeList
	{
	private:
		// Implicit �� ����
		static char* heap_listp; //ó�� first block�� �����͸� ����
		void* heap_cur; // ���� block�� ��ġ�� ����Ű�� ������
		//Implicit ���� �Լ�
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

