/* Header for testing procedures */

#ifndef _INCLUDED_TEST_H_
#define _INCLUDED_TEST_H_

#ifdef __STDC__
void Test_Init(char *, int);
void Test_Printf(char *, ...);
void Test_Message(char *);
void Test_Failed(char *);
void Test_Passed(char *);
int Summarize_Test_Results(void);
void Test_Finalize(void);
#else
void Test_Init();
void Test_Message();
void Test_Printf();
void Test_Failed();
void Test_Passed();
int Summarize_Test_Results();
void Test_Finalize();
#endif

#endif
