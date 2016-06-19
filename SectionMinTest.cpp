#include <stdio.h>
#include <math.h>
#include <string>
using std::string;
#include <ctime>
#include "SectionM.h"

int *randomInts(int len)
{
	int *ret = new int[len];
	srand(clock());
	for(int i = 0;i < len;++ i)
	{
		ret[i] = abs(rand());
	}
	return ret;
}

int *randomInts1_1(int len)
{
	if(0 == len) return NULL;
	int *ret = new int[len];
	srand(clock());
	ret[0] = rand();
	for(int i = 1;i < len;++ i)
	{
		ret[i] = (1 == (1 & abs(rand()))) ? ret[i-1] + 1 : ret[i-1] - 1;
	}
	return ret;
}



int minFromArray(int *a,int start,int end)
{
	int min = a[start];
	for(int i = start + 1;i <= end;++ i)
	{
		if(min > a[i]) min = a[i];
	}
	return min;
}

int maxFromArray(int *a,int start,int end)
{
	int max = a[start];
	for(int i = start + 1;i <= end;++ i)
	{
		if(max < a[i]) max = a[i];
	}
	return max;
}

void testSectionMin()
{
	while (true)
	{
		int len = 100000;
		int *height = randomInts(len);
	//	int len = 9;
	//	int height[9] = {0,3,2,1,8,1,4,7,9,};
		int support = 3;

		int minIndex = 0;
		int maxQuerySection = 4;
		SectionM<int> sm(height,len,maxQuerySection,1);

		for(int start = 0;start < len;++ start)
			for(int end = start + 1;end < start + 3 * maxQuerySection && end < len; ++ end)
			{
			//	int end = start + maxQuerySection;
			//	int minNum = sm.query(start,end);
				int dupMinNum = maxFromArray(height,start,end);
				int minNumEx = sm.queryNoLimit(start,end);
				if(dupMinNum == minNumEx)
				{
					printf("success\r\n");
				}
				else
				{
					printf("error\r\n");
				}

			}
	}
}

#include "RMQEffective.h"
void testEffectiveRMQ()
{
	while (true)
	{
		int len = 100000;
		int *height = randomInts(len);
		//	int len = 9;
		//	int height[9] = {0,3,2,1,8,1,4,7,9,};

		int minIndex = 0;
		int maxQuerySection = 4;
		RMQEffective<int> rmq(height,len,1);
	//	SectionM sm(height,len,maxQuerySection,1);

		for(int start = 0;start < len;++ start)
			for(int end = start + 1;end < start + 3 * maxQuerySection && end < len; ++ end)
			{
				//	int end = start + maxQuerySection;
				//	int minNum = sm.query(start,end);
				int dupMinNum = maxFromArray(height,start,end);
				int minNumEx = rmq.query(start,end,NULL); //sm.queryNoLimit(start,end);
				if(dupMinNum == minNumEx)
				{
					printf("success\r\n");
				}
				else
				{
					printf("error\r\n");
				}

			}
	}
}