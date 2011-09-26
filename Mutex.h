//
//  Mutex.h
//  PMM Sucker
//
//  Created by Juan V. Guerrero on 9/25/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//

#ifndef PMM_Sucker_Mutex_h
#define PMM_Sucker_Mutex_h
#include<pthread.h>
namespace pmm {
	//Encapsulates a POSIX threads mutual exclusion device
	class Mutex {
	private:
		pthread_mutex_t theMutex;
	public:
		Mutex();
		~Mutex();
		void lock();
		void unlock();
	};
}

#endif