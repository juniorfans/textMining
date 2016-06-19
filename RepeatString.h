#pragma once

class Prime
{
private:
	int curMaxPrime;
	vector<int> primeNums;
	static Prime *thePrime;
public:
	static Prime* getPrimeTable()
	{
		if(NULL == thePrime)
		{
			thePrime = new Prime();
		}
		return thePrime;
	}
	bool isPrime(int num)
	{
		if(0 == (num & 1))	return false;	//偶数肯定不是质数
		if(0 == (num % 3))	return false;	//整除 3 就不是质数
		if(curMaxPrime < num)
		{
			initTable(num);
		}
		//...
		exsit(num);
	}
	const vector<int> *getPrimes()
	{
		return NULL;//thePrime;
	}
	
private :
	Prime()
	{
		primeNums.push_back(2);
		primeNums.push_back(3);
		curMaxPrime = (3);
	}
	
	void initTable(int noMorethan)
	{
		assert(2 != curMaxPrime);
		for (int i = curMaxPrime + 2;i <= noMorethan;i += 2)	//curMaxprime 必定是个奇数
		{
			bool isPrime = true;
			vector<int>::iterator it = primeNums.begin();
			for (;it != primeNums.end();++ it)
			{
				if(0 == i % *it)
				{
					isPrime = false;
					break;
				}
			}
			if(isPrime)
			{
				if(curMaxPrime == i)	break;
				curMaxPrime = i;
				primeNums.push_back(i);
			}
		}/*
		 for (int i = 0;i < primeNums.size();++ i)
		 {
		 if(i > 0 && i % 10 == 0)
			{
			printf("\r\n");
			}
			else
			{
			printf("%d\t",primeNums[i]);
			}
			}
			printf("[init for %d]\r\n",noMorethan);
			*/
	}


	bool exsit(int prime)
	{
		int start = 0,end = primeNums.size() - 1;
		int mid;
		while (true)
		{
			if(start < end)	return false;
			mid = (start + end) >> 1;
			if(primeNums[mid] > prime)
			{
				end = mid - 1;
			}
			else if(primeNums[mid] < prime)
			{
				start = mid + 1;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
};




class RepeatString
{
private:
	const wchar_t *theStr;
	int theStrLen;
	
	vector<int> repeatPoses;
	
	
	void init();
public:
	void initPrimes(int noMorethan);
	RepeatString(const wchar_t *str);
	bool isRepeat();
	vector<int> &getRepeatPoses();
	~RepeatString(void);
};

void testPrime();