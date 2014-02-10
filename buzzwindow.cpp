#include "buzzwindow.h"
#include <Path.h>
#include <stdlib.h>
#include "enum.h"
#include <Application.h>
#include "main.h"
//-------------------------------------------------------------------
BuzzWindow::BuzzWindow()
	: BWindow(BRect(30, 30, 150, 80), "WakeUp v1.0", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	TestButton = new BButton(BRect(1, 1, 35, 1), "", "Test", new BMessage(TEST));
	AddChild(TestButton);

	SoundName = new BStringView(BRect(35,10,120,20), "", "Drop sound file!");
	AddChild(SoundName);

	StartButton = new BButton(BRect(1, 25, 35, 25), "", "Start", new BMessage(START));
	AddChild(StartButton);

	IntervalControl = new BTextControl(BRect(35, 30, 90, 40), "", "", "5", 
										NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	IntervalControl->SetDivider(0);
	AddChild(IntervalControl);

	Seconds = new BStringView(BRect(90,35,130,45), "", "min");
	AddChild(Seconds);

	TimeElapsed = new BStringView(BRect(1,5,130,25), "", "0:0:0");
	TimeElapsed->SetFont(be_bold_font);
	TimeElapsed->SetFontSize(20);
	TimeElapsed->SetAlignment(B_ALIGN_CENTER);
	TimeElapsed->Hide();
	AddChild(TimeElapsed);
	
	MyClock = new AClock();
	Playing = false; //do we want the thread to run
}
//-------------------------------------------------------------------
bool BuzzWindow::QuitRequested()
{
	Playing = false;
	status_t* exitvalue=NULL;
	wait_for_thread(ClockId, exitvalue); //wait for the clock thread to finishes
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
//-------------------------------------------------------------------
void BuzzWindow::MessageReceived(BMessage* message)
{
 	switch (message->what)
 	{
   		case B_SIMPLE_DATA:
   		{// Look for a ref in the message
		   	entry_ref ref;
			BPath path;
   			if(message->FindRef("refs", &ref) == B_OK)
   			{
				BEntry Entry(&ref);
				Entry.GetPath(&path);
				if((Entry.GetRef(&ref) == B_OK))
				{
					MyClock->SetSound(new BFileGameSound(&ref, false));
					SoundName->SetText(ref.name);
				}
			}
   		
   			else BWindow::MessageReceived(message);break;
   		}break;
   		
   		case TEST:
   		{
	   		MyClock->PlaySound();
   		}break;
   		
   		case START:
   		{
			if(Playing)
			{
				Playing = false;
				StartButton->SetLabel("Start");
				TimeElapsed->Hide();
				SoundName->Show();
				TestButton->Show();
			}
			
			else
			{
				if(atoi(IntervalControl->Text()) < 1) 
					return;

				Playing = true;
				MyClock->SetInterval(atoi(IntervalControl->Text()) * 60000000); //min->ticks
				BString TitleString = "WakeUp (";
				TitleString << IntervalControl->Text() << ")";
				SetTitle(TitleString.String());
				StartButton->SetLabel("Stop");
				TimeElapsed->Show();
				SoundName->Hide();
				TestButton->Hide();
   				Start();
   			}
   		}
   		
   		default:BWindow::MessageReceived(message);break;
	}
}
//-------------------------------------------------------------------
void BuzzWindow::Start()
{//Start a new thread
	ClockId = spawn_thread(TimerThread, "TimerThread", B_NORMAL_PRIORITY, (void *) MyClock);
	resume_thread(ClockId);
}
//-------------------------------------------------------------------
int32 TimerThread(void *data)
{
	AClock* T = (AClock*)data;
	T->Restart();
	BString TimeString;
	int loop = 0;

	while(Playing)
	{
		snooze(1000000);
		T->AddTime(1000000); //+1 sec
		TimeString = "";
		TimeString << ((uint32)T->GetHour()) << ":" << ((uint32)T->GetMinute()) << ":" << ((uint32)T->GetSecond());
		((Buzzer*)be_app)->MainWindow->Lock();
		((Buzzer*)be_app)->MainWindow->TimeElapsed->SetText(TimeString.String());
		((Buzzer*)be_app)->MainWindow->Sync();
		((Buzzer*)be_app)->MainWindow->Unlock();

		loop += 1000000; //+1 sec
		if(loop == T->GetInterval())
		{
			T->PlaySound();
			loop = 0;
		}
	}

	return 0;
}
//-------------------------------------------------------------------

