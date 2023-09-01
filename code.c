int main()
{
	char buf[4];
	buf[0] = 'a';
	buf[1] = 'b';
	buf[2] = 'c';
	write(buf, 3);
}
