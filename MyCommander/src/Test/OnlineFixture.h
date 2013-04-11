#ifndef ONLINE_FIXTURE_H
#define ONLINE_FIXTURE_H

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* OfflineFixture
* Provides variables and functions needed to test the commander in an
* online setting.
*/

struct OnlineFixture
{
	static const double MAX_TIME_TICK;
};

const double OnlineFixture::MAX_TIME_TICK = 80.0;

#endif // ONLINE_FIXTURE_H
