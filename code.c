void compare_float(double f1, double f2)
{
 if (f1 < f2)
	 assert 0;
}

//TODO: == < > and other in case of float
//3 < 2 when you do this you compare then cast to float then back to int
//not sure 100%
int main()
{
	compare_float(1, 2);
	print ((double)2);
}
