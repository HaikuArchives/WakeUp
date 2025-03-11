#ifndef BUZZWINDOW_H
#define BUZZWINDOW_H

#include <Button.h>
#include <FileGameSound.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>
//------------------------------------------------------------------------
int32 TimerThread(void* data);
//------------------------------------------------------------------------
class AClock {
private:
	BFileGameSound* Sound;
	bigtime_t Interval;	 // every 'Interval' ticks, we beep
	int Second;			 // seconds elapsed
	int Minute;			 // minutes elapsed
	int Hour;			 // hour elapsed
	bigtime_t Temps;	 // number of ticks elapsed

public:
	AClock()
	{
		Sound = NULL;
		Interval = 0;
		Second = 0;
		Minute = 0;
		Hour = 0;
		Temps = 0;
	}
	void Restart() { Temps = 0; }
	void SetSound(BFileGameSound* s)
	{
		if (Sound != NULL)
			delete Sound;
		Sound = s;
	}
	bigtime_t GetInterval() { return Interval; }
	void SetInterval(bigtime_t t) { Interval = t; }
	void PlaySound()
	{
		if (Sound != NULL)
			Sound->StartPlaying();
	}
	void AddTime(bigtime_t t) { Temps += t; }
	short int GetHour() { return Temps / 3600000000UL; }
	short int GetMinute() { return (Temps / 60000000) % 60; }
	short int GetSecond() { return (Temps / 1000000) % 60; }
};
//------------------------------------------------------------------------
class BuzzWindow : public BWindow {
	friend int32 TimerThread(void* data);

private:
	BStringView* SoundName;
	BStringView* Seconds;
	BStringView* TimeElapsed;
	BButton* TestButton;
	BButton* StartButton;
	BTextControl* IntervalControl;
	AClock* MyClock;
	thread_id ClockId;
	void Start();

public:
	BuzzWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage* message);
};

#endif
