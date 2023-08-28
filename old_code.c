



int compare_float(double f1, double f2)
{
 double precision = 0.00000000000000000001;

 if (f1 < f2)
 {
	 assert 0;
	 return -1;
}
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

double cos(double x)
{
	double M_PI_M_2 = 6.28318548202514648438;
	double M_PI = 3.14159274101257324219;
	double M_PI_2 = 1.57079637050628662109;

 if( x < 0.0f ) 
  	x = -x;
  //if (0 <= compare_float(x,M_PI_M_2)) 
 	{
  	//	x = x - M_PI_M_2;
		while (x > M_PI_M_2)//0 <= compare_float(x, M_PI_M_2))
	{
  			x = x - M_PI_M_2;
		//	print(x, (char)'\n');
		//	for (int i = 0; i < 1000000; i = i + 1)
		//	{
		//	}
		}
  	}

  if ((0 <= compare_float(x, M_PI)) && (-1 == compare_float(x, M_PI_M_2)))
  {
   x = x - M_PI;
   return ((-1)*(1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))))));
  } 
 return 1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))));
}

float sin(float x)
{
	float M_PI_2 = 1.57079637050628662109;
	return cos(x - M_PI_2);
}

void memset(char *ptr, char c, int size)
{
	for (int i = 0; i < size; i = i + 1)
		ptr[i] = c;
}


// TODO: for (;;) + for (j = 0 ;;)
int main()
{
	print(compare_float(1, 2), (char)'\n');
	print(compare_float(1, 1), (char)'\n');
	print(compare_float(1, 0), (char)'\n');
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
        memset(b,32,1760);
        memset(z,0,7040);
		j = 0;
        for(; 6.28>j; j=j+0.07) {
			i = 0;
            for(; 6.28 >i; i=i+0.02) {
                float sini=sin(i),
                      cosj=cos(j),
                      sinA=sin(A),
                      sinj=sin(j),
                      cosA=cos(A),
                      cosj2=cosj+2,
                      mess=1/(sini*cosj2*sinA+sinj*cosA+5),
                      cosi=cos(i),
                      cosB=cos(B),
                      sinB=sin(B),
                      t=sini*cosj2*cosA-sinj* sinA;
                int x=40+30*mess*(cosi*cosj2*cosB-t*sinB),
                    y= 12+15*mess*(cosi*cosj2*sinB +t*cosB),
                    o=x+80*y,
                    N=8*((sinj*sinA-sini*cosj*cosA)*cosB-sini*cosj*sinA-sinj*cosA-cosi *cosj*sinB);
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
			print(c);
		}
        A = A + 0.04;
        B = B + 0.02;
    }
}
