
#include <e32test.h>
#include <e32std.h>

RTest test(_L("AMD Pinball"));

extern "C"
{
	TInt SymbianMain();
	
	TInt getCurrentTime()
	{
	    return User::NTickCount();
	}
	
	void symbianPrintf(char* string)
	{
		TBufC8<256> str((TText8*)string);
		TBuf16<256> str16;
		str16.Copy(str);
		test.Printf( str16 );
		test.Printf( _L("\n") );
	}
}

TInt E32Main()
{
	CTrapCleanup* trapHandler=CTrapCleanup::New();
	test(trapHandler!=NULL);

	test.Title();
	test.Start(_L("AMD Pinball\n"));
	TInt err = SymbianMain();

	test.Printf(_L("Finished...\n"));

	test.End();
	TKeyCode tmp = test.Getch();

	delete trapHandler;
	return(KErrNone);
}
