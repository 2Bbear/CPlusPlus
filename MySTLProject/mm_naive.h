#pragma once
namespace twobbearlib
{
/*
word size 4�� WSIZE�� ��ũ�� �������ְ�, double word size 8�� DSIZE�� ��ũ�� ���� ���ְ�, ����� Ǫ�͸� ������ overhead�� 8�� OVERHEAD�� ��ũ������
*/
#define ALIGNMENT 8
/*
����� 8�� ����� �Ҵ�ǵ��� �����Ѵ�
������ ���� short ��������,  size�� 1�̸� (1+8-1) & ~0x07 �̰� , 00000000 00001000 & 11111111 11111000 = 1000 �̴�
���� size�� ������ ������ 8�� ����� ��ȯ�Ѵ�.
*/
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7) 

/*
sizeof ����� ���� ������ Ÿ���� ũ�⸦ ��ȯ�Ѵ�. int ���� ��� 4 double 8��
size_t�� int���� ���� 4�� ��ȯ�Ѵ�
���� ����� �ּ� ������ 8�� �����ؾ� �ϱ� ������ 
�� ��ũ�δ� ����� ����� ����ϸ� ��������� ���� 8�� ��Ÿ����.
*/
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//����� ����� ����Ű�� �ִ� �����Ϳ� �����Ѵ�
#define SIZE_PTR(p) ((size_t*)(((char*)(p)) - SIZE_T_SIZE))

#define WSIZE 4
#define DSIZE 8
#define OVERHEAD 8

/*
GET(p)�� p�� ����Ű�� ����� ���� size_t ������ �о� ����
PUT(p,val)�� p�� ����Ű�� ����� ��(word) �� val ���� �־��ִ� ������ ���ش�
*/
#define GET(p) (*(size_t*)(p)) // p�� ����Ű�� ���� �о� �´�
#define PUT(p,val) (*(size_t*)(p) = (size_t)(val))// p�� ����Ű�� ���� �ִ´�.
/*
p�� ����Ű�� ����� ���� �о�� AND������ ���ؼ� ���� GET_SIZE�� size�� �о����, GET_ALLOC�� allocated bit�� �о�� �Ҵ翩�θ� �˷��ش�.
*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/*
���� block�� ����� ����Ű�� �����Ϳ� block�� Ǫ�͸� ����Ű�� �����͸� �������ش�.
*/
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp))-DSIZE)

/*
size�� allocated ���θ� 4����Ʈ (1word)�� �����ش�.
*/
#define PACK(size, alloc) ((size) | (alloc))


#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))) // bp ���� ����� ����Ű�� ������
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE)) // bp ���� ����� ����Ű�� ������

/*
bp�� ���� �׸��� ���� ��������� ����Ű�� �������� ��ġ,
�� ��ũ�ο� put ��ũ�θ� �̿��ϸ� bp�� ���� �׸��� ���� ��������� ������ �� �ִ�.
*/
#define NEXT_FREEP(bp) ((char *)(bp))
#define PREV_FREEP(bp) ((char *)(bp) + WSIZE)

/*
bp�� ���� �׸��� ���� ��������� ����Ű�� ������,
�� ��ũ�θ� �̿��ؼ� bp�� ���� �׸��� ���� ��������� ���� �� �� �ִ�.
*/
#define NEXT_FREE_BLKP(bp) GET((char*)(bp))
#define PRECV_FREE_BLKP(bp) GET((char*)(bp)+WSIZE)
}