#ifndef RESUMABLE_H
#define RESUMABLE_H

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

template<class Itt, class Op, class Cond>
void ResumableForEach(Itt & in_Begin, const Itt & in_End, Op in_Func, Cond in_Cond)
{
	for(; in_Cond() && in_Begin != in_End; ++in_Begin)
	{
		in_Func();
	}
}

template<class Itt, class Op, class Cond>
void ResumableEmbeddedLoop(Itt & in_Begin1, Itt & in_Begin2, const Itt & in_End, Op in_Func, Cond in_Cond)
{
	while(in_Cond() && in_Begin1 != in_End)
	{
		while(in_Cond() && in_Begin2 != in_End)
		{
			in_Func();
			++in_Begin2;
		}
		if(in_Cond())
		{
			++in_Begin1;
			in_Begin2 = in_Begin1 + 1;
		}
	}
}

#endif // RESUMABLE_H
