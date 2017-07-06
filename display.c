#include <stdio.h>

void writeTextOnScreen(int row, int column, char* value)
{
	printf("\x1B[%d;%df", row, column);
	fflush(stdout);
	printf(value);
	fflush(stdout);
}

void initializeDisplay(void)
{
	writeTextOnScreen(10, 26, "--------------------");
	writeTextOnScreen(11, 25, "|");
	writeTextOnScreen(12, 25, "|");
	writeTextOnScreen(13, 25, "|");
	writeTextOnScreen(14, 25, "|");
	
	writeTextOnScreen(15, 26, "--------------------");
	writeTextOnScreen(11, 46, "|");
	writeTextOnScreen(12, 46, "|");
	writeTextOnScreen(13, 46, "|");
	writeTextOnScreen(14, 46, "|");
}

void displayMessage(int row, int column, char* value)
{
	writeTextOnScreen(10 + row, 26 + column, value);
}
