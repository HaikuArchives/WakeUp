#ifndef MAIN_H
#define MAIN_H

#include <Application.h>
#include "buzzwindow.h"

class Buzzer : public BApplication {
public:
	BuzzWindow* MainWindow;
	Buzzer();
};

#endif