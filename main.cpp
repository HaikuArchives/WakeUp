#include "main.h"
//--------------------------------------------------------------
int main()
{
	Buzzer* Prog;
	Prog = new Buzzer;
	Prog->Run();
	return 0;
}
//--------------------------------------------------------------
Buzzer::Buzzer() : BApplication("application/x-vnd-WakeUp")
{
	MainWindow = new BuzzWindow();
	MainWindow->Show();
}
//--------------------------------------------------------------
