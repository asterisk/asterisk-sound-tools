/* Copyright (C) 2009 Jean-Marc Valin
   File: audiofilter.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define MAX_FILTER_SIZE 100
#define FRAME_SIZE 256

void filter(short *x, float *num, float *den, int ord, short *y, int N, float *mem)
{
	int i,j;
	float xi,yi,nyi;
	for (i=0;i<N;i++)
	{
		xi= x[i];
		yi = xi*num[0] + mem[0];
		nyi = -yi;
		for (j=0;j<ord-1;j++)
			mem[j] = mem[j+1] + num[j+1]*xi + den[j+1]*nyi;
		mem[ord-1] = num[ord]*xi + den[ord]*nyi;
		if (yi < -32768)
			yi = -32768;
		if (yi > 32767)
			yi = 32767;
		y[i] = floor(.5+yi);
	}
}


int main(int argc, char **argv)
{
	int i, ret;
	float num[MAX_FILTER_SIZE], den[MAX_FILTER_SIZE], mem[MAX_FILTER_SIZE];
	short pcm[FRAME_SIZE];
	int num_size, den_size;
	float *fill=num;
	int *size = &num_size;
	FILE *fin, *fout;
	num_size = den_size = 0;
	
	for (i=0;i<MAX_FILTER_SIZE;i++)
		num[i]=den[i]=mem[i] = 0;
	
	for (i=1;i<argc-2;i++)
	{
		if (strcmp(argv[i],"-n")==0)
		{
			fill = num;
			num_size = 0;
			size = &num_size;
		} else if (strcmp(argv[i],"-d")==0)
		{
			fill = den;
			den_size = 0;
			size = &den_size;
		} else {
			fill[(*size)++] = atof(argv[i]);
		}
	}
	if (num_size<den_size)
		num_size = den_size;
	if (den_size<num_size)
		den_size = num_size;

	if (strcmp(argv[argc-2],"-")==0)
		fin = stdin;
	else
		fin = fopen(argv[argc-2],"rb");
	if (strcmp(argv[argc-1],"-")==0)
		fout = stdout;
	else
		fout = fopen(argv[argc-1],"wb");
	
	while ((ret = fread(pcm, 2, FRAME_SIZE, fin)) > 0)
	{
		filter(pcm, num, den, den_size, pcm, ret, mem);
		fwrite(pcm, 2, ret, fout);
	}
	return 0;
}