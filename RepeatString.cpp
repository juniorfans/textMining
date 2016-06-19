#include <cassert>
#include <vector>
using std::vector;
#include "RepeatString.h"
#include <cmath>

RepeatString::RepeatString(const wchar_t *str)
:theStr(str)
{
	assert(NULL != str);
	while (*str)
	{
		++ str;
		++ theStrLen;
	}
	
	init();
}


void RepeatString::init()
{
	assert(NULL != theStr);
	const wchar_t *dupStr = theStr;
	//先判断是否是质数，若是，则肯定不是重复结构
	Prime *pt = Prime::getPrimeTable();
	if(pt->isPrime(theStrLen))
	{
		repeatPoses.clear();
		return;
	}
	//分解因式 theStr = a * b
	int a = 0;
	int b = 0;
	int q = sqrt((double)theStrLen);

	const vector<int> *primes = pt->getPrimes();
	vector<int>::const_iterator qit = primes->begin();

	for (;qit != primes->end() && *qit <= q;++ qit);	//首个大于 q 的质数
	assert(qit != primes->end() && *qit > q);

	vector<int>::const_iterator lit = primes->begin();
	for (;lit != qit;++ lit)
	{
		if(0 == theStrLen % *lit)
		{
			a = *lit;
			b = theStrLen / *lit;
			break;
		}
	}
	assert(0 != a && 0 != b);
	assert(a * b == theStrLen);
	//每 
}

bool RepeatString::isRepeat()
{
	return 0 != repeatPoses.size();
}

vector<int> & RepeatString::getRepeatPoses()
{
	return repeatPoses;
}

RepeatString::~RepeatString(void)
{
	repeatPoses.clear();
}

Prime* Prime::thePrime = NULL;

void testPrime()
{
	Prime *p = Prime::getPrimeTable();
//	p->initTable(29);
	printf("128 is prime : %s\r\n",p->isPrime(128) ? "yes" : "no");
	printf("569 is prime : %s\r\n",p->isPrime(569) ? "yes" : "no");
	printf("673 is prime : %s\r\n",p->isPrime(673) ? "yes" : "no");
	printf("1279 is prime : %s\r\n",p->isPrime(1279) ? "yes" : "no");
	printf("67409 is prime : %s\r\n",p->isPrime(67409) ? "yes" : "no");
	printf("44159 is prime : %s\r\n",p->isPrime(44159) ? "yes" : "no");
	printf("12 is prime : %s\r\n",p->isPrime(12) ? "yes" : "no");
	printf("44259 is prime : %s\r\n",p->isPrime(44259) ? "yes" : "no");

}