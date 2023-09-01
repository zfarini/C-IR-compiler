
int strlen(char *s)
{
	int i = 0;

	while (s[i])
		i = i + 1;
	return i;
}

void putchar(char c)
{
	write(&c, 1);
}

void putstr(char *s)
{
	write(s, strlen(s));
}

void	putnbr(int n)
{
	int	p;
	int	x;

	p = 1;
	x = n;
	while (x >= 10 || x <= -10)
	{
		x /= 10;
		p *= 10;
	}
	if (n < 0)
		putchar('-');
	while (p > 0)
	{
		if (n < 0)
			putchar(-((n / p) % 10) + '0');
		else
			putchar((n / p) % 10 + '0');
		p /= 10;
	}
}

int a, b;

int main()
{
	a = 5;
	putnbr(a);
}

