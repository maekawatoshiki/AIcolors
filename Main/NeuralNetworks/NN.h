#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <string>
#include <sstream>
#include <omp.h>
#include <windows.h>
using namespace std;
#define LEARNING_NUM 300000 //äwèKâÒêî
#define SAMPLE_NUM 100	//ÉTÉìÉvÉãêî
#define INPUT_NUM 3	//ì¸óÕëwêî
#define HIDDEN_NUM 10	//íÜä‘ëwêî
#define OUTPUT_NUM 1	//èoóÕëwêî
#define EPSILON 0.01	//èdÇ›èCê≥íËêî
#define PATTERNS 3		//ÉpÉ^Å[ÉìÇÃêî ì¸óÕëwêî
#define rand() xor128()
#define RAND_MAX 0xFFFFFFFF

int mouseXY[INPUT_NUM] = { 0 };
extern HWND hWnd;

unsigned int w = 0;
unsigned long xor128() 
{
	static unsigned long x = 123456789, y = 362436069, z = 521288629;
	unsigned long t = (x^(x << 11));
	x = y; y = z; z = w;
	return (w = (w^(w >> 19))^(t^(t >> 8)));
}

double sigmoid(double x)
{
	return 1.0 / (1.0 + exp(-x));
}

struct NN
{
	double x[INPUT_NUM];
	double y[OUTPUT_NUM];
};

NN txy[SAMPLE_NUM] = { 0 }, txy2[SAMPLE_NUM] = { 0 };
double x[INPUT_NUM + 1], h[HIDDEN_NUM + 1], h2[HIDDEN_NUM + 1], y[OUTPUT_NUM];
double w_i[INPUT_NUM + 1][HIDDEN_NUM], w_h[HIDDEN_NUM + 1][HIDDEN_NUM],
		w_o[HIDDEN_NUM + 1][OUTPUT_NUM];
double h_back[HIDDEN_NUM + 1], h2_back[HIDDEN_NUM + 1], y_back[OUTPUT_NUM];

int NNLearning(int learning)
{
	int learn, sample, i, j, k;
	double net_input;
	double e = 0.0;
	char s[12800] = { 0 };
	FILE *fp = fopen("w_10.fc", "r+b");
	if (fp == NULL)
	{
		fp = fopen("w_10.fc", "wb");
	}
	else
	{
		fread(w_i, sizeof(double), (INPUT_NUM + 1)*HIDDEN_NUM, fp);
		fread(w_o, sizeof(double), (HIDDEN_NUM + 1)*OUTPUT_NUM, fp);
		goto BREAK;
	}
	srand((unsigned)time(NULL));
	w = time(0);

	for (i = 0; i < INPUT_NUM + 1; i++)
		for (j = 0; j < HIDDEN_NUM; j++)
			w_i[i][j] = (double)rand() / RAND_MAX;

	for (i = 0; i < HIDDEN_NUM + 1; i++)
		for (j = 0; j < OUTPUT_NUM; j++)
			w_o[i][j] = (double)rand() / RAND_MAX;

	for (learn = 0; learn < LEARNING_NUM; learn++)
	{
		e = 0;
		for (sample = 0; sample < learning; sample++)
		{
			for (i = 0; i < PATTERNS; i++)
				x[i] = (double)txy[sample].x[i];

			x[PATTERNS] = h[HIDDEN_NUM] =  1.0;

			for (j = 0; j < HIDDEN_NUM; j++)
			{
				net_input = 0;
				for (i = 0; i < PATTERNS + 1; i++)
					net_input += w_i[i][j] * x[i];

				h[j] = tanh(net_input);
			}

			for (j = 0; j < OUTPUT_NUM; j++)
			{
				net_input = 0;
				for (i = 0; i < HIDDEN_NUM + 1; i++)
					net_input += w_o[i][j] * h[i];

				y[j] = sigmoid(net_input);
				e += 0.5 * pow(txy[sample].y[j] - y[j], 2.0);
			}
			
			//Ç±Ç±Ç©ÇÁãtì`îd
			for (j = 0; j < OUTPUT_NUM; j++)
				y_back[j] = (y[j] - txy[sample].y[j]) * (1.0 - y[j]) * y[j];

			for (i = 0; i < HIDDEN_NUM; i++)
			{
				net_input = 0;

				for (j = 0; j < OUTPUT_NUM; j++)
					net_input += w_o[i][j] * y_back[j];

				h_back[i] = net_input * ((double)1.0 - h[i]) * h[i];
			}

			for (i = 0; i < PATTERNS + 1; i++)
				for (j = 0; j < HIDDEN_NUM; j++)
					w_i[i][j] = w_i[i][j] - (EPSILON * x[i] * h_back[j]);
			for (i = 0; i < HIDDEN_NUM + 1; i++)
				for (j = 0; j < OUTPUT_NUM; j++)
					w_o[i][j] = w_o[i][j] - (EPSILON * h[i] * y_back[j]);
		}
		if (e < 0.01)
			break;
	}
	fwrite(w_i, sizeof(double), (INPUT_NUM + 1)*HIDDEN_NUM, fp);
	fwrite(w_o, sizeof(double), (HIDDEN_NUM + 1)*OUTPUT_NUM, fp);
BREAK:

	sprintf(s, "%lf", e);
	SetWindowText(hWnd, s);
	fclose(fp);
	return 0;
}

int NNoutput(int learning)
{
	int i, j, k, num;
	int r, g, b;
	double net_input, tmp;
	char s[0xFF] = { 0 };

	for (i = 0; i < 1000; i++)
	{
		for (i = 0; i < PATTERNS; i++)
			x[i] = (double)(rand() % 0xFF) / 0xFF;

		h[HIDDEN_NUM] = x[PATTERNS] = (double)1.0;

		for (j = 0; j < HIDDEN_NUM; j++)
		{
			net_input = 0;
			for (i = 0; i < PATTERNS + 1; i++)
			{
				net_input = net_input + w_i[i][j] * x[i];
			}
			h[j] = tanh(net_input);
		}

		for (j = 0; j < OUTPUT_NUM; j++)
		{
			net_input = 0;

			for (i = 0; i < HIDDEN_NUM + 1; i++)
				net_input += w_o[i][j] * h[i];

			y[j] = sigmoid(net_input);
		}

		/*for (j = 0; j < PATTERNS; j++)
		{
			if (x[j] && j >= 0 && j < 256)
				r = j;
			if (x[j] && j >= 256 && j < 512)
				g = j - 256;
			if (x[j] && j >= 512 && j < 768)
				b = j - 512;
		}*/
		r = x[0] * 0xFF;
		g = x[1] * 0xFF;
		b = x[2] * 0xFF;
		if (y[0] > 0.95) break;
	}
	sprintf(s, "%lf", y[0]);
	SetWindowText(hWnd, s);
	
	return RGB(r, g, b);
}

/*
class GeneticColor
{
public:
const int szRGB;
const int geneLen;
int gene[12][300];
int child1[300];
int child2[300];
int maxes[12];
int NNoutput[12];

GeneticColor();
int NNoutputuation();
};

GeneticColor::GeneticColor() :szRGB(3*10*10), geneLen(12)
{
srand(time(0));
for (int i = 0; i<geneLen; i++)
for (int k = 0; k<szRGB; k++)
gene[i][k] = rand() % 0xFF;
memset(child1, 0, sizeof(int)  * szRGB);
memset(child2, 0, sizeof(int) * szRGB);
memset(maxes, 0, sizeof(int) * geneLen);
memset(NNoutput, 0, sizeof(int) * geneLen);
}

int GeneticColor::NNoutputuation()
{
int i, k, j;
int mn1 = 0, mn2 = 0, min1 = 0, min2 = 0,
mx1 = 0, mx2 = 0, max1 = 0, max2 = 0, minNumber = 0, min = 0;

memset(maxes, 0, sizeof(int) * geneLen);
for (i = 0; i < geneLen; i++)
maxes[i] = NNoutput[i];

mn1 = mx1 = maxes[0];
max2 = max1 = min2 = min1 = 0;

for (i = 0; i < geneLen; i++)
{
min2 = min = maxes[i];
for (k = i; k < geneLen; k++)
{
min2 = maxes[k];
if (min2 < min)
{
minNumber = k;
min = min2;
}
}
memcpy(&gene[i], &gene[minNumber], sizeof(int) * szRGB);
}
// àÍólåç≥
for (i = 0; i < szRGB; i++)
{
if (rand() % 2) child1[i] = gene[0][i];
else child1[i] = gene[1][i];

if (rand() % 2) child2[i] = gene[1][i];
else child2[i] = gene[0][i];
}

// éqë∑
memcpy(&gene[max1], child1, sizeof(int) * szRGB);
memcpy(&gene[max2], child2, sizeof(int) * szRGB);

// ìÀëRïœàŸ
for (i = 0; i < geneLen  ; i++)
gene[rand() % geneLen][rand() % szRGB] += rand() % 2 ? -1 : 1;


return min1;
}
*/