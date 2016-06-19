#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "itssort.h"


/*- Macros -*/
#define BUCKET_A(c0) (bucket_A[(c0)])
#define BUCKET_B(c0, c1) (bucket_B[(c1) * ALPHABET_SIZE + (c0)])
#define BUCKET_BSTAR(c0, c1) (bucket_B[(c0) * ALPHABET_SIZE + (c1)])


/*-- Improved Two-Stage Sort --*/

/* Simple Multikey Quicksort. */
static
void
mkqsort(const wchar_t*Td, const wchar_t*Tn, int *first, int *last) 
{
#define Ti(_a) (((Td + (_a)) < Tn) ? Td[(_a)] : -1)
  const wchar_t *U1, *U2;
  int *a, *b, *c, *d, t;
  int v, x;
  for(; 8 < (last - first);) {
    a = first, b = first + (last - first) / 2, c = last - 1;
    if(Ti(*a) > Ti(*b)) { d = a, a = b, b = d; }
    if(Ti(*b) > Ti(*c)) { b = (Ti(*a) > Ti(*c)) ? a : c; }
    v = Ti(t = *b), *b = *first, *first = t;
    for(a = first, b = first + 1, c = d = last - 1;;) {
      for(; (b <= c) && ((x = Ti(*b)) <= v); ++b) { if(x != v) { t = *b, *b = *a, *a++ = t; } }
      for(; (b < c) && ((x = Ti(*c)) >= v); --c) { if(x != v) { t = *c, *c = *d, *d-- = t; } }
      if(c <= b) { break; }
      t = *a, *a++ = *c, *c-- = *d, *d-- = *b, *b++ = t;
    }
    if(b == c) { t = *b, *b = *d, *d = t; } else { d += 1; }
    if(((b = d) - a) < (last - first)) {
      if((a - first) <= (last - b)) {
        if((last - b) <= (b - a)) {
          mkqsort(Td, Tn, first, a);
          mkqsort(Td, Tn, b, last);
          first = a, last = b, Td += 1;
        } else if((a - first) <= (b - a)) {
          mkqsort(Td, Tn, first, a);
          mkqsort(Td + 1, Tn, a, b);
          first = b;
        } else {
          mkqsort(Td + 1, Tn, a, b);
          mkqsort(Td, Tn, first, a);
          first = b;
        }
      } else {
        if((a - first) <= (b - a)) {
          mkqsort(Td, Tn, b, last);
          mkqsort(Td, Tn, first, a);
          first = a, last = b, Td += 1;
        } else if((last - b) <= (b - a)) {
          mkqsort(Td, Tn, b, last);
          mkqsort(Td + 1, Tn, a, b);
          last = a;
        } else {
          mkqsort(Td + 1, Tn, a, b);
          mkqsort(Td, Tn, b, last);
          last = a;
        }
      }
    } else {
      Td += 1;
    }
  }

  for(a = last - 2; first <= a; --a) {
    for(t = *a, b = a + 1; b < last; ++b) {
      for(U1 = Td + t, U2 = Td + *b; (U1 < Tn) && (U2 < Tn) && (*U1 == *U2); ++U1, ++U2) { }
      if((Tn <= U1) || ((U2 < Tn) && (*U1 < *U2))) { break; }
      *(b - 1) = *b;
    }
    *(b - 1) = t;
  }
#undef Ti
}



/*---------------------------------------------------------------------------*/

/* Sorts type B* suffixes using qsort. */
static
int
sort_typeBstar(const wchar_t*T, int *SA,
               int *bucket_A, int *bucket_B, int n) {
  int *PAb,*ISAb,*buf,bufsize;
  int i, j, k, t, m;
  int c0, c1;

  /* Initialize bucket arrays. */
  for(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  for(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  /* Count the number of occurrences of the first one or two characters of each
     type A, B and B* suffix. Moreover, store the beginning position of all
     type B* suffixes into the array SA. */
  for(i = n - 1, m = n; 0 <= i;) {
    /* type A suffix. */
    do { ++BUCKET_A(T[i]); } while((0 <= --i) && (T[i] >= T[i + 1]));
    if(0 <= i) {
      /* type B* suffix. */
      ++BUCKET_BSTAR(T[i], T[i + 1]);
      SA[--m] = i;
      /* type B suffix. */
      for(--i; (0 <= i) && (T[i] <= T[i + 1]); --i) {
        ++BUCKET_B(T[i], T[i + 1]);
      }
    }
  }
  m = n - m;
/*
note:
  A type B* suffix is lexicographically smaller than a type B suffix that
  begins with the same first two characters.
*/

  /* Calculate the index of start/end point of each bucket. */
  for(c0 = 0, i = 0, j = 0; c0 < ALPHABET_SIZE; ++c0) 
  {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; /* start point */
    i = t + BUCKET_B(c0, c0);
    for(c1 = c0 + 1; c1 < ALPHABET_SIZE; ++c1) 
	{
      j += BUCKET_BSTAR(c0, c1);
      BUCKET_BSTAR(c0, c1) = j; /* end point */
      i += BUCKET_B(c0, c1);
    }
  }

  if(0 < m) 
  {
    /* Sort the type B* suffixes by their first two characters. */
    PAb = SA + n - m;ISAb = SA + m;

    for(i = m - 1; 0 <= i; --i) 
	{
      t = PAb[i];
      c0 = T[t]; c1 = T[t + 1];
      SA[--BUCKET_BSTAR(c0, c1)] = t;
    }

    /* Sort the type B* suffixes using qsort. */
    for(c0 = ALPHABET_SIZE - 2, j = m; 0 < j; --c0) 
	{
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; j = i, --c1) 
	  {
        i = BUCKET_BSTAR(c0, c1);	// 同一个桶内有多个后缀则要排序
        if(1 < (j - i)) 
		{
          mkqsort(T + 2, T + n, SA + i, SA + j);
        }
      }
    }
    /* Calculate the index of start/end point of each bucket. */
    BUCKET_B(ALPHABET_SIZE - 1, ALPHABET_SIZE - 1) = n; /* end point */
    for(c0 = ALPHABET_SIZE - 2, k = m - 1; 0 <= c0; --c0) {
      i = BUCKET_A(c0 + 1) - 1;
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i; /* end point */
  
        /* Move all type B* suffixes to the correct position. */
        for(i = t, j = BUCKET_BSTAR(c0, c1);
            j <= k;
            --i, --k) {
          t = SA[k];
          SA[i] = ((0 < t) && (T[t - 1] > c0)) ? ~t : t;
        }
      }
      BUCKET_BSTAR(c0, c0 + 1) = i - BUCKET_B(c0, c0) + 1; /* start point */
      BUCKET_B(c0, c0) = i; /* end point */
    }
  }

  return m;
}

/* Constructs the suffix array by using the sorted order of type B* suffixes. */
static
void
construct_SA(const wchar_t*T, int *SA,
             int *bucket_A, int *bucket_B,
             int n, int m) {
  int *i, *j, *k;
  int s;
  int c0, c1, c2;

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 < (s = *j)) {
          assert(T[s] == c1);
          assert(((s + 1) < n) && (T[s] <= T[s + 1]));
          assert(T[s - 1] <= T[s]);
          *j = ~s;
          c0 = T[--s];
          if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          if(c0 != c2) {
            if(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
          assert(k < j);
          *k-- = s;
        } else {
          assert(((s == 0) && (T[s] == c1)) || (s < 0));
          *j = ~s;
        }
      }
    }
  }

  /* Construct the suffix array by using
     the sorted order of type B suffixes. */
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~(n - 1) : (n - 1);
  /* Scan the suffix array from left to right. */
  for(i = SA, j = SA + n; i < j; ++i) {
    if(0 < (s = *i)) {
      assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      if((s == 0) || (T[s - 1] < c0)) { s = ~s; }
      if(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
      assert(i < k);
      *k++ = s;
    } else {
      assert(s < 0);
      *i = ~s;
    }
  }
}


int itssort(const wchar_t *T, int *SA, int n) {
  int *bucket_A, *bucket_B;
  int m;
  int err = 0;

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  else if(n == 0) { return 0; }
  else if(n == 1) { SA[0] = 0; return 0; }
  else if(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; return 0; }

  bucket_A = (int*)malloc(BUCKET_A_SIZE * sizeof(int));
  bucket_B = (int*)malloc(BUCKET_B_SIZE * sizeof(int));

  /* Suffixsort. */
  if((bucket_A != NULL) && (bucket_B != NULL)) 
  {
		m = sort_typeBstar(T, SA, bucket_A, bucket_B, n);
		construct_SA(T, SA, bucket_A, bucket_B, n, m);
  } 
  else 
  {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);

  return err;
}

void initIttSortConstants(int alphabetSize)
{
	ALPHABET_SIZE = alphabetSize;
	BUCKET_A_SIZE = ALPHABET_SIZE;
	BUCKET_B_SIZE = ALPHABET_SIZE * ALPHABET_SIZE;
}

void testIttsort()
{
	initIttSortConstants(10000);
	wchar_t height[15] = {857,9567,7567,6567,3,4788,8,5679,1560,151,12,14,6,9,1000};
	int len = 15;
	int SA[15] = {0};
	int err = itssort(height,SA,len);
	int i =0;
	if(0 != err)printf("error code : %d\r\n",err);
	else
	{
		for(i = 0;i < 15;i ++)
		{
			printf("%d ",height[SA[i]]);
		}
		printf("\r\n");
	}
}


