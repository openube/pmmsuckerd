//
//  FetchedMailsCache.h
//  PMM Sucker
//
//  Created by Juan V. Guerrero on 10/9/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//

#ifndef PMM_Sucker_FetchedMailsCache_h
#define PMM_Sucker_FetchedMailsCache_h
#include "GenericException.h"
#include "Mutex.h"
#include "MTLogger.h"
#include "libetpan/libetpan.h"
#include "SharedQueue.h"
#include "pmmrpc_types.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#ifdef __linux__
#include <inttypes.h>
#endif

namespace pmm {
	class FetchedMailsCache {
	private:
		std::string datafile;
		sqlite3 *dbConn;
		void verifyTables();
		void verifyTables(sqlite3 *conn, const std::string &email);
	protected:
		sqlite3 *openDatabase();
		//sqlite3 *openDatabase(const std::string &email);
		sqlite3 *openDatabase(const std::string &email, bool &createTable);
		void closeDatabase(sqlite3 *db);
	public:
		FetchedMailsCache();
		FetchedMailsCache(const std::string &_datafile);
		~FetchedMailsCache();
#ifdef OLD_CACHE_INTERFACE
		void addEntry(const std::string &email, const std::string &uid);
		void addEntry(const std::string &email, uint32_t uid);
		bool entryExists(const std::string &email, const std::string &uid);
		bool entryExists(const std::string &email, uint32_t uid);
		void expireOldEntries();		
		bool hasAllThesePOP3Entries(const std::string &email, carray *msgList);
		void removeMultipleEntries(const std::string &email, const std::vector<uint32_t> &uidList);
		void removeEntriesNotInSet(const std::string &email, const std::vector<uint32_t> &uidSet);
		void removeAllEntriesOfEmail(const std::string &email);
#endif
		bool addEntry2(const std::string &email, const std::string &uid);
		bool addEntry2(const std::string &email, uint32_t &uid);


		

		bool entryExists2(const std::string &email, const std::string &uid, bool &tableCreated);
		bool entryExists2(const std::string &email, uint32_t uid);
		bool hasAllTheseEntries(const std::string &email, carray *msgList);
		
		void expireOldEntries(const std::string &email);
		
		
		void removeMultipleEntries2(const std::string &email, const std::vector<uint32_t> &uidList);
		void removeEntriesNotInSet2(const std::string &email, const std::vector<uint32_t> &uidSet);
		void removeAllEntriesOfEmail2(const std::string &email);

		void closeConnection(const std::string &email);
	};
	
	extern MTLogger CacheLog;
	extern pmm::SharedQueue<pmmrpc::FetchDBItem> fetchDBItems2SaveQ;
	extern const char *DefaultFetchDBTableName;
}


#endif
