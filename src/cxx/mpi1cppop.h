typedef void User_function(const void *invec, void* inoutvec, int len, 
			   const Datatype& datatype);
void Init(User_function* function, bool commute);
void Free();
