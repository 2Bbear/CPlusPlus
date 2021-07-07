#pragma once
#include <math.h>
#include <functional>
namespace twobbearlib
{
# define ABS(N) ((N < 0) ? (-N) : (N))

	using namespace std;
	struct Position
	{
		Position(){}
		Position(int _x, int _y) { x = _x; y = _y; }
		int x = 0;
		int y = 0;
	};
	class Bresenham2D
	{
	private:
	public:
		Bresenham2D() {}
		void GetNext(Position* start, Position* end,function<void(Position*)> _func)
		{
			int dx = abs(end->x - start->x); //x축의 변화량이 많은지
			int dy = abs(end->y - start->y); // y축의 변화량이 많은지
			Position cur = *start;
			int xdirection = end->x > start->x ? 1 : -1;
			int ydirection = end->y > start->y ? 1 : -1;
			int sum=0;

			if (dy <= dx) //x변화량이 많을때
			{
				for (; cur.x!= end->x+ xdirection; cur.x +=xdirection)
				{
					
					if (sum>=dx)
					{
						sum -= dx*2;
						cur.y += ydirection;
					}
					sum += dy * 2;
					_func(&cur);
				}
			}
			else // y 변화량이 많을때
			{
				for (; cur.y != end->y+ ydirection; cur.y += ydirection)
				{
					if (sum >=dy)
					{
						sum -= dy * 2;
						cur.x += xdirection;
					}
					sum += dx * 2;
					_func(&cur);
				}
			}
		}
	};


}


