const int a=+10,b=-2;
const char c='2',d='+';
int e,f;
int g[32],h[23];
int refunc( int i,int j,char k)
{
	const int refunc4=5;
	int refunc1,refunc2,refunc3;
	refunc1=0;
	refunc2=+1;
	refunc3=-1;
	g[5]=2;
	refunc4=refunc2*refunc3;
	refunc4=refunc4/refunc2;
	refunc1=refunc2-refunc3;
	return (refunc2+refunc3);
}
void norefunc( )
{
	a=12345;
	b=67890;
}
void main()
{
	int input1;
	int input2;
	int i;
	scanf( input1 );
	scanf( input2 );
	switch ( input1 )
	{
		case 0 : printf (+1+2-3) ;
		case 1 : printf ("abc") ;
		default: printf ("abc",1+2+3);
	}
	i=0;
	while (i<input1){
		i=i+1;
	}
	if(input2<0)
	{
		if(input2<=-3)
		{
			printf("input2<=-3");
		}
		else if( input2==-2)
		{
			printf("input2==-2");
		}
		else
		{
			printf("input2==-1");
		}
	}
	else if(input2>0)
	{
		if(input2>=3)
		{
			printf("input2>=3");
		}
		else if(input2!=1)
		{
			printf("input2==2");
		}
		else
		{
			printf("input2==1");
		}
	}
	else
	{
		printf("input2==0");
	}
	g[5]=input+g[5]-2+'c'+refunc(1,2,3)+(4-3);
	norefunc();
}
