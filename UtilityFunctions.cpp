//
//  UtilityFunctions.cpp
//  PMM Sucker
//
//  Created by Juan V. Guerrero on 9/29/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <strings.h>
#include "UtilityFunctions.h"

namespace pmm {
	static inline bool isURLEncodable(char c){
		static const char *urlEncodableCharacters = "$&+,/:;=?@<>#%{}|\\^~[]` ";		
		if (c <= 0x1F || c >= 0x7f) {
			return true;
		}
		if (index(urlEncodableCharacters, c) != NULL) {
			return true;
		}
		return false;
	}
	
	void url_encode(std::string &theString){
		std::stringstream urlEncodedString;
		bool replaceHappened = false;
		for (size_t i = 0; i < theString.size(); i++) {
			if (isURLEncodable(theString[i])) {
				urlEncodedString << "%" << std::hex << (int)theString[i];
				replaceHappened = true;
			}
			else {
				char tbuf[2] = { theString[i], 0x00 };
				urlEncodedString << tbuf;
			}
		}
		if(replaceHappened) theString.assign(urlEncodedString.str());
	}
	
	void devToken2Binary(std::string devTokenString, std::string &binaryDevToken){
		std::stringstream binbuilder;
		for (size_t i = 0; i < 64; i+=2) {
			std::string hex_s = devTokenString.substr(i, 2);
			int unit = 0;
			sscanf(hex_s.c_str(), "%x", &unit);
			binbuilder << std::hex << (char)unit;
			//binaryDevToken.append(sizeof(char), (char)unit);
		}
		binaryDevToken = binbuilder.str();q
	}

	void splitEmailAccounts(std::vector<MailAccountInfo> &mailAccounts, std::vector<MailAccountInfo> &imapAccounts, std::vector<MailAccountInfo> &pop3Accounts){
		for (size_t i = 0; i < mailAccounts.size(); i++) {
			if (mailAccounts[i].mailboxType().compare("IMAP") == 0) {
				imapAccounts.push_back(mailAccounts[i]);
			}
			else {
				pop3Accounts.push_back(mailAccounts[i]);				
			}
		}
	}

}