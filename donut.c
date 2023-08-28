
#include <math.h>
#include <stdio.h>
#include <string.h>

#define M_PI_M_2 M_PI_2
#define M_PI (3.1415927f)
#define M_PI_2 (M_PI/2.0f)
#define M_PI_M_2 (M_PI*2.0f)

int compare_float(double f1, double f2)
{
 double precision = 0.00000000000000000001;
 if ((f1 - precision) < f2)
  {
 return -1;
  }
 else if ((f1 + precision) > f2)
 {
  return 1;
 }
 else
  {
 return 0;
  }
}

double my_cos(double x){
 if( x < 0.0f ) 
  x = -x;

  if (0 <= compare_float(x,M_PI_M_2)) 
 {
 do {
  x -= M_PI_M_2;
  }while(0 <= compare_float(x,M_PI_M_2));

  }

  if ((0 <= compare_float(x, M_PI)) && (-1 == compare_float(x, M_PI_M_2)))
  {
   x -= M_PI;
   return ((-1)*(1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))))));
  } 
 return 1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))));
}

double my_sin(double x){return my_cos(x-M_PI_2);}


void fmemset(char *ptr, char c, int size)
{
	for (int i = 0; i < size; i = i + 1)
		ptr[i] = c;
}

// TODO: for (;;) + for (j = 0 ;;)
int main()
{
	printf("%.20lf\n", M_PI_M_2);
	printf("%.20lf\n", M_PI);
	printf("%.20lf\n", M_PI_2);
	return 0;
	char ascii[13];

	ascii[0] = '.';
	ascii[1] = ',';
	ascii[2] = '-';
	ascii[3] = '~';
	ascii[4] = ':';
	ascii[5] = ';';
	ascii[6] = '=';
	ascii[7] = '!';
	ascii[8] = '*';
	ascii[9] = '#';
	ascii[10] = '$';
	ascii[11] = '@';
	ascii[12] = 0;
//".,-~:;=!*#$@"

	int k;
    float A=0, B=0, i, j, z[1760];
    char b[1760];
	while (1)
	{
        fmemset(b,32,1760);
        fmemset(z,0,7040);
		j = 0;
        for(; 6.28>j; j=j+0.07) {
			i = 0;
            for(; 6.28 >i; i=i+0.02) {
                float my_sini=my_sin(i),
                      my_cosj=my_cos(j),
                      my_sinA=my_sin(A),
                      my_sinj=my_sin(j),
                      my_cosA=my_cos(A),
                      my_cosj2=my_cosj+2,
                      mess=1/(my_sini*my_cosj2*my_sinA+my_sinj*my_cosA+5),
                      my_cosi=my_cos(i),
                      my_cosB=my_cos(B),
                      my_sinB=my_sin(B),
                      t=my_sini*my_cosj2*my_cosA-my_sinj* my_sinA;
                int x=40+30*mess*(my_cosi*my_cosj2*my_cosB-t*my_sinB),
                    y= 12+15*mess*(my_cosi*my_cosj2*my_sinB +t*my_cosB),
                    o=x+80*y,
                    N=8*((my_sinj*my_sinA-my_sini*my_cosj*my_cosA)*my_cosB-my_sini*my_cosj*my_sinA-my_sinj*my_cosA-my_cosi *my_cosj*my_sinB);
                if(22>y&&y>0&&x>0&&80>x&&mess>z[o]){
                    z[o]=mess;
					int index = 0;
					if (N > 0)
						index = N;
                    b[o]=ascii[index];
                }
            }
        }
 //       printf("\x1b[d");

		k = 0;
		for(; 1761>k; k = k + 1)
		{
			char c = 10;
			if (k % 80)
				c = b[k];
			printf("%c", c);
		}
        A = A + 0.04;
        B = B + 0.02;
    }
}

