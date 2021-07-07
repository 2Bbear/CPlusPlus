#pragma once
namespace twobbearlib
{
/*
word size 4를 WSIZE로 매크로 정의해주고, double word size 8을 DSIZE로 매크로 정의 해주고, 헤더와 푸터를 포함한 overhead인 8을 OVERHEAD로 매크로정의
*/
#define ALIGNMENT 8
/*
블록이 8의 배수로 할당되도록 연산한다
연산의 예로 short 기준으로,  size가 1이면 (1+8-1) & ~0x07 이고 , 00000000 00001000 & 11111111 11111000 = 1000 이다
따라서 size가 무엇이 들어오든 8의 배수를 반환한다.
*/
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7) 

/*
sizeof 명령은 안의 데이터 타입의 크기를 반환한다. int 형일 경우 4 double 8등
size_t는 int형과 같이 4를 반환한다
따라서 블록이 최소 단위로 8을 리턴해야 하기 때문에 
이 매크로는 블록의 사이즈를 명시하며 결과적으로 값은 8을 나타낸다.
*/
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//블록의 사이즈를 가르키고 있는 포인터에 접근한다
#define SIZE_PTR(p) ((size_t*)(((char*)(p)) - SIZE_T_SIZE))

#define WSIZE 4
#define DSIZE 8
#define OVERHEAD 8

/*
GET(p)는 p가 가르키는 블록의 값을 size_t 형으로 읽어 오고
PUT(p,val)은 p가 가리키는 블록의 값(word) 에 val 값을 넣어주는 역할을 해준다
*/
#define GET(p) (*(size_t*)(p)) // p가 가르키는 값을 읽어 온다
#define PUT(p,val) (*(size_t*)(p) = (size_t)(val))// p가 가르키는 곳에 넣는다.
/*
p가 가르키는 블록의 값을 읽어와 AND연산을 통해서 각각 GET_SIZE는 size를 읽어오고, GET_ALLOC은 allocated bit를 읽어와 할당여부를 알려준다.
*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/*
각각 block의 헤더를 가리키는 포인터와 block의 푸터를 가리키는 포인터를 정의해준다.
*/
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp))-DSIZE)

/*
size와 allocated 여부를 4바이트 (1word)로 묶어준다.
*/
#define PACK(size, alloc) ((size) | (alloc))


#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))) // bp 다음 블록을 가리키는 포인터
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE)) // bp 이전 블록을 가리키는 포인터

/*
bp의 다음 그리고 이전 프리블록을 가리키는 포인터의 위치,
이 매크로와 put 매크로를 이용하면 bp의 다음 그리고 이전 프리블록을 설정할 수 있다.
*/
#define NEXT_FREEP(bp) ((char *)(bp))
#define PREV_FREEP(bp) ((char *)(bp) + WSIZE)

/*
bp의 다음 그리고 이전 프리블록을 가리키는 포인터,
이 매크로를 이용해서 bp의 다음 그리고 이전 프리블록을 접근 할 수 있다.
*/
#define NEXT_FREE_BLKP(bp) GET((char*)(bp))
#define PRECV_FREE_BLKP(bp) GET((char*)(bp)+WSIZE)
}