#include "buzzwindow.h"
#include <Alert.h>
#include <Entry.h>
#include <Path.h>
#include <stdlib.h>
#include "enum.h"
#include <Application.h>
#include "main.h"
#include <LayoutBuilder.h>

//-------------------------------------------------------------------
BuzzWindow::BuzzWindow()
	:
	BWindow(BRect(30, 30, 0, 0), "WakeUp", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	TestButton = new BButton("", "Test", new BMessage(TEST));

	SoundName = new BStringView("", "Drop sound file!");

	StartButton = new BButton("", "Start", new BMessage(START));

	IntervalControl = new BTextControl("", "", "5", NULL, B_WILL_DRAW);
	IntervalControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_RIGHT);

	Seconds = new BStringView("", "min");

	TimeElapsed = new BStringView("", "0:00:00");
	TimeElapsed->SetFont(be_bold_font);
	TimeElapsed->SetFontSize(22);
	TimeElapsed->SetAlignment(B_ALIGN_CENTER);
	TimeElapsed->Hide();
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_ITEM_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(TestButton)
			.Add(SoundName)
			.AddGlue()
			.Add(TimeElapsed)
			.AddGlue()
			.End()
		.AddGroup(B_HORIZONTAL)
			.Add(StartButton)
			.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
				.Add(IntervalControl)
				.Add(Seconds)
				.End()
			.End();
	
	MyClock = new AClock();
	Playing = false; //do we want the thread to run
}
//-------------------------------------------------------------------
bool BuzzWindow::QuitRequested()
{
	Playing = false;
	status_t* exitvalue=NULL;
	wait_for_thread(ClockId, exitvalue); //wait for the clock thread to finish
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
			if (message->FindRef("refs", &ref) != B_OK) {
				BWindow::MessageReceived(message);
				break;
			}

			BFileGameSound* soundFile = new BFileGameSound(&ref, false);
			if (soundFile->InitCheck() != B_OK) {
				delete soundFile;
				BAlert* not_supported = new BAlert("Error",
					"This is not a supported sound file.", "OK");
				not_supported->SetType(B_WARNING_ALERT);
				not_supported->Go();
				break;
			}

			MyClock->SetSound(soundFile);
			SoundName->SetText(ref.name);
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
				SetTitle("WakeUp");
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
   		
		default:
			BWindow::MessageReceived(message);
			break;
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
		//h:mm:ss format
		TimeString << T->GetHour() << ":";
		int t = T->GetMinute();
		if (t < 10)
			TimeString << "0";
		TimeString << t << ":";
		t = T->GetSecond();
		if (t < 10)
			TimeString << "0";
		TimeString << t;
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

