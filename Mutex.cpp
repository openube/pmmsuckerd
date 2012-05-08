//
//  Mutex.cpp
//  PMM Sucker
//
//  Created by Juan V. Guerrero on 9/25/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//

#include <iostream>
#include <cerrno>
#include <pthread.h>
#include "Mutex.h"

namespace pmm {
	
	Mutex::Mutex(){
		pthread_mutexattr_init(&attrs);
#ifdef __linux__
		pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK_NP);
#else
		pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK);
#endif
		pthread_mutex_init(&theMutex, &attrs);
		lFile = NULL;
		lLine = -1;
	}
	
	Mutex::~Mutex() {
		pthread_mutex_destroy(&theMutex);
	}
	
	void Mutex::lock() {
#ifdef USE_MANAGED_MUTEXES
		switch(pthread_mutex_trylock(&theMutex)){
			case 0:
				//Mutex locked succesfully
				break;
			case EBUSY:
				if(lFile != NULL && lLine > 0) throw MutexLockException("Can't lock an already locked mutex!!!", lFile, lLine);
				else throw MutexLockException("Can't lock an already locked mutex!!!");
				break;
			case EINVAL:
				throw MutexLockException("Unable to lock a non-properly-initialized mutex!!!");
				break;
			default:
				throw MutexLockException("Can't lock this mutex!!!");
		}
#else
		switch(pthread_mutex_lock(&theMutex)){
			case 0:
				//Mutex locked succesfully
				break;
			case EBUSY:
				if(lFile != NULL && lLine > 0) throw MutexLockException("Can't lock an already locked mutex!!!", lFile, lLine);
				else throw MutexLockException("Can't lock an already locked mutex!!!");
				break;
			case EINVAL:
				throw MutexLockException("Unable to lock a non-properly-initialized mutex!!!");
				break;
			default:
				throw MutexLockException("Can't lock this mutex!!!");
		}
#endif
	}
	
	void Mutex::lockWithInfo(const char *_srcFile, int _srcLine){
		lock();
		lFile = (char *)_srcFile;
		lLine = _srcLine;
	}
	
	void Mutex::unlock() {
		lFile = NULL;
		lLine = -1;
		pthread_mutex_unlock(&theMutex);
	}
	
	MutexLockException::MutexLockException(){
		
	}
	
	MutexLockException::MutexLockException(const std::string &_errmsg) : pmm::GenericException(_errmsg){
		
	}
	
	MutexLockException::MutexLockException(const std::string &_errmsg, const char *_srcFile, int _srcLine) : pmm::GenericException(_errmsg){
		srcFile = _srcFile;
		srcLine = _srcLine;
	}

	AtomicFlag::AtomicFlag() : Mutex(){
		theFlag = false;
	}
	
	AtomicFlag::AtomicFlag(bool initialValue) : Mutex() {
		theFlag = initialValue;
	}
	void AtomicFlag::set(bool newValue){
		lock();
		theFlag = newValue;
		unlock();
	}
	
	bool AtomicFlag::get(){
		bool ret = false;
		lock();
		ret = theFlag;
		unlock();
		return ret;
	}
	
	void AtomicFlag::toggle(){
		lock();
		if (theFlag) {
			theFlag = false;
		}
		else {
			theFlag = true;
		}
		unlock();
	}
	
	bool AtomicFlag::operator=(bool newValue){
		lock();
		theFlag = newValue;
		unlock();		
		return newValue;
	}
	
	bool AtomicFlag::operator==(bool anotherValue){
		bool ret = false;
		lock();
		if (theFlag == anotherValue) ret = true;
		unlock();
		return ret;
	}

	bool AtomicFlag::operator==(AtomicFlag &another) {
		bool ret = false;
		lock();
		if (theFlag == another.get()) {
			ret = true;
		}
		unlock();
		return ret;
	}
		
	bool AtomicFlag::operator!=(bool anotherValue){
		bool ret = true;
		lock();
		if (theFlag == anotherValue) ret = false;
		unlock();
		return ret;
	}
	
	bool AtomicFlag::operator!=(AtomicFlag &another) {
		bool ret = true;
		lock();
		if (theFlag == another.get()) {
			ret = false;
		}
		unlock();
		return ret;
	}

}