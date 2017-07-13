#include <stdio.h>

void writeTextOnScreen(int row, int column, char* value)
{
	return;
	printf("\x1B[%d;%df", row, column);
	fflush(stdout);
	printf(value);
	fflush(stdout);
}

void initializeDisplay(void)
{
	return;
	printf("\033[2J"); 
	
	writeTextOnScreen(10, 26, "------------------------");
	writeTextOnScreen(11, 25, "|");
	writeTextOnScreen(12, 25, "|");
	writeTextOnScreen(13, 25, "|");
	writeTextOnScreen(14, 25, "|");
	
	writeTextOnScreen(15, 26, "------------------------");
	writeTextOnScreen(11, 50, "|");
	writeTextOnScreen(12, 50, "|");
	writeTextOnScreen(13, 50, "|");
	writeTextOnScreen(14, 50, "|");
	
	
}

void displayMessage(int row, int column, char* value)
{
	writeTextOnScreen(11 + row, 26 + column, value);
}
