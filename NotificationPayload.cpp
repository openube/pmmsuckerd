//
//  NotificationPayload.cpp
//  PMM Sucker
//
//  Created by Juan V. Guerrero on 9/29/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//
#include "NotificationPayload.h"
#include "UtilityFunctions.h"
#include <iostream>
#include <sstream>
#ifndef MAXPAYLOAD_SIZE
#define MAXPAYLOAD_SIZE 256
#endif
#ifndef DEFAULT_SILENT_SOUND
#define DEFAULT_SILENT_SOUND ""
#endif

namespace pmm {
	
	static void msg_encode(std::string &theMsg){
		std::string newString;
		for (size_t i = 0; i < theMsg.size(); i++) {
			if (theMsg[i] == '\n') {
				newString.append("\\n");
			}
			else if (theMsg[i] == '\r') {
				newString.append("\\r");
			}
			else if (theMsg[i] == '\t') {
				newString.append("\\t");
			}
			else if (theMsg[i] == '\"') {
				newString.append("\\\"");
			}
			else if (theMsg[i] == '\\') {
				newString.append("\\\\");
			}
			else if (theMsg[i] == '\'') {
				newString.append("\\'");
			}
			/*else if (((unsigned char)theMsg[i]) >= 0x7f) {
				newString.append("\\u");
				std::stringstream hconv;
				hconv.fill('0');
				hconv.width(2);
				while ((unsigned char)theMsg[i] >= 0x7f) {
					hconv << std::right << std::hex << (int)((unsigned char)theMsg[i]);
					i++;
				}
				i--;
				newString.append(hconv.str());
			}*/
			else {
				char tbuf[2] = { theMsg[i], 0x00};
				newString.append(tbuf);
			}
		}
		//if (newString.size() > theMsg.size()) {
			theMsg = newString;
		//}
	}

	NotificationPayload::NotificationPayload(){
		_badgeNumber = 1;
		isSystemNotification = false;
		attempts = 0;
	}
	
	NotificationPayload::NotificationPayload(const std::string &devToken_, const std::string &_message, int badgeNumber, const std::string &sndName){
		msg = _message;
		_soundName = sndName;
		devToken = devToken_;
		_badgeNumber = badgeNumber;
		isSystemNotification = false;
		attempts = 0;
		build();
	}
	
	NotificationPayload::NotificationPayload(const NotificationPayload &n){
		msg = n.msg;
		_soundName = n._soundName;
		devToken = n.devToken;
		_badgeNumber = n._badgeNumber;
		build();
		origMailMessage = n.origMailMessage;
		isSystemNotification = n.isSystemNotification;
		attempts = n.attempts;
		customParams = n.customParams;
	}
	
	NotificationPayload::~NotificationPayload(){
		
	}
	
	void NotificationPayload::build(){
		std::stringstream jsonbuilder;
		size_t l = msg.size();
		std::string encodedMsg = msg;
		bool addDots = false;
		bool useSteps = false;
		do{
			if(addDots == true){
				if(!useSteps){
					l = l - (MAXPAYLOAD_SIZE - jsonbuilder.str().size()) - 3;
					useSteps = true;
				}
				else {
					l--;
				}
			}
			jsonbuilder.str(std::string());
			encodedMsg = msg.substr(0, l);
			if(addDots) encodedMsg.append("...");
			msg_encode(encodedMsg);
			if(encodedMsg.size() > 1 && encodedMsg[encodedMsg.size() - 1] == '\\') encodedMsg[encodedMsg.size() - 1] = ' ';
			jsonbuilder << "{";
			jsonbuilder << "\"aps\":";
			jsonbuilder << "{";
			jsonbuilder << "\"alert\":\"" << encodedMsg << "\"";
			if(_soundName.size() > 0) jsonbuilder << ",\"sound\":\"" << _soundName << "\"";
			if(_badgeNumber > 0) jsonbuilder << ",\"badge\":" << _badgeNumber;
			jsonbuilder << "}";
			if(customParams.size() > 0){
				for (std::map<std::string, std::string>::iterator iter = customParams.begin(); iter != customParams.end(); iter++) {
					jsonbuilder << ",\"" << iter->first << "\":\"" << iter->second << "\"";
				}
			}
			jsonbuilder << "}";
			addDots = true;
		}while (jsonbuilder.str().size() > MAXPAYLOAD_SIZE);
		jsonRepresentation = jsonbuilder.str();
	}
	
	const std::string &NotificationPayload::toJSON() {
		build();
		return jsonRepresentation;
	}
	
	std::string &NotificationPayload::soundName(){
		return _soundName;
	}

	const std::string &NotificationPayload::soundName() const {
		return _soundName;
	}
	
	std::string &NotificationPayload::message(){
		return msg;
	}

	std::string &NotificationPayload::deviceToken(){
		return devToken;
	}
	
	void NotificationPayload::useSilentSound(){
		_soundName = DEFAULT_SILENT_SOUND;
		build();
	}
	
	int NotificationPayload::badge(){
		return _badgeNumber;
	}
	
	NoQuotaNotificationPayload::NoQuotaNotificationPayload() : pmm::NotificationPayload(){
		
	}
	
	NoQuotaNotificationPayload::NoQuotaNotificationPayload(const std::string &devToken_, const std::string &_email, const std::string &sndName, int badgeNumber) : pmm::NotificationPayload(devToken_, "", badgeNumber, sndName){
		std::stringstream msg_s;
		msg_s << "You have ran out of quota on: " << _email;
		msg = msg_s.str();
		customParams["e"] = _email;
		isSystemNotification = true;
	}
	
	NoQuotaNotificationPayload::NoQuotaNotificationPayload(const NoQuotaNotificationPayload &n) : pmm::NotificationPayload(n){
	}
	
	NoQuotaNotificationPayload::~NoQuotaNotificationPayload(){
		
	}

}
