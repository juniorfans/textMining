#pragma once

/**
 * Constructs the suffix array of a given string 
 * using the improved two-stage sort algorithm.
 * @param T[0..n-1] The input string.
 * @param SA[0..n] The output array of suffixes.
 * @param n The length of the given string.
 */

	int itssort(const wchar_t *T, int *SA, int n);

	void testIttsort();

	void initIttSortConstants(int alphabetSize);

	static int ALPHABET_SIZE;
	static int BUCKET_A_SIZE;
	static int BUCKET_B_SIZE;
