//
//  MailMessage.cpp
//  PMM Sucker
//
//  Created by Juan Guerrero on 10/14/11.
//  Copyright (c) 2011 fn(x) Software. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#ifdef __linux__
#include <string.h>
#endif
#include "MTLogger.h"
#include "libetpan/libetpan.h"
#include "MailMessage.h"
#include "UtilityFunctions.h"
#include <iconv.h>

namespace pmm {
	namespace MIMEParameter {
		static const char *charset = "charset";
	}
	namespace MIMEContentSubtype {
		static const char *plain = "plain";
		static const char *html = "html";
	}
	namespace TextEncoding {
		static const char *utf8 = "UTF-8";
	}
	
	static void getMIMEData(struct mailmime_data * data, std::stringstream &outputStream, const std::string &charset)
	{
		if (data->dt_type == MAILMIME_DATA_TEXT) {
			switch (data->dt_encoding) {
				case MAILMIME_MECHANISM_BASE64:
				{
					//Decode base64 encoding
					char *decodedMsg;
					size_t decodedSize = 0, indx = 0;
					int r = mailmime_base64_body_parse(data->dt_data.dt_text.dt_data, data->dt_data.dt_text.dt_length, &indx, &decodedMsg, &decodedSize);
					if (r == MAILIMF_NO_ERROR) {
						if(charset.size() > 0 && charset.compare(TextEncoding::utf8) != 0){
							size_t inleft = decodedSize, outleft = decodedSize * 2;
							char *utf8d = (char *)malloc(outleft);
							bzero(utf8d, outleft);
							char *ptr_in = decodedMsg, *ptr_out = utf8d;
							iconv_t cd = iconv_open(TextEncoding::utf8, charset.c_str());
							iconv(cd, &ptr_in, &inleft, &ptr_out, &outleft);
							outputStream.write(utf8d, strlen(utf8d));
							mailmime_decoded_part_free(decodedMsg);
							free(utf8d);
							iconv_close(cd);
						}
						else outputStream.write(decodedMsg, decodedSize);
						mailmime_decoded_part_free(decodedMsg);
					}
				}
				break;
				case MAILMIME_MECHANISM_QUOTED_PRINTABLE:
				{
					char *decodedMsg;
					size_t indx = 0, decodedSize = 0;
					int r = mailmime_quoted_printable_body_parse(data->dt_data.dt_text.dt_data, data->dt_data.dt_text.dt_length, &indx, &decodedMsg, &decodedSize, 0);
					if (r == MAILIMF_NO_ERROR) {
						/*size_t indx2 = 0;
						char *newText = 0;
						mailmime_encoded_phrase_parse(charset.c_str(), decodedMsg, decodedSize, &indx2, TextEncoding::utf8, &newText);
						if(newText != 0){
							outputStream << newText;
							free(newText);
						}
						else*/ outputStream.write(decodedMsg, decodedSize);
						//mailmime_decoded_part_free(decodedMsg);
					}
				}
				break;
				case MAILMIME_MECHANISM_8BIT:
				case MAILMIME_MECHANISM_BINARY:
				case MAILMIME_MECHANISM_7BIT:
				{
					size_t indx = 0, decodedSize = 0;
					char *decodedMsg;
					//mailmime_part_parse(data->dt_data.dt_text.dt_data, data->dt_data.dt_text.dt_length, &indx, data->dt_encoding, &decodedMsg, &decodedSize);
					mailmime_binary_body_parse(data->dt_data.dt_text.dt_data, data->dt_data.dt_text.dt_length, &indx, &decodedMsg, &decodedSize);
					if(decodedSize != 0){
						size_t inleft = decodedSize, outleft = decodedSize * 2;
						char *utf8d = (char *)malloc(outleft);
						bzero(utf8d, outleft);
						char *ptr_in = decodedMsg, *ptr_out = utf8d;
						iconv_t cd = iconv_open(TextEncoding::utf8, charset.c_str());
						iconv(cd, &ptr_in, &inleft, &ptr_out, &outleft);
						outputStream.write(utf8d, strlen(utf8d));
						mailmime_decoded_part_free(decodedMsg);
						free(utf8d);
						iconv_close(cd);
					}
				}
				break;
				default:
					//Consider an encoding conversion here!!!!
					if(charset.size() > 0 && charset.compare(TextEncoding::utf8) != 0){
						size_t inleft = data->dt_data.dt_text.dt_length, outleft = data->dt_data.dt_text.dt_length * 2;
						char *utf8d = (char *)malloc(outleft);
						bzero(utf8d, outleft);
						char *ptr_in = (char *)data->dt_data.dt_text.dt_data, *ptr_out = utf8d;
						iconv_t cd = iconv_open(TextEncoding::utf8, charset.c_str());
						iconv(cd, &ptr_in, &inleft, &ptr_out, &outleft);
						outputStream.write(utf8d, strlen(utf8d));
						free(utf8d);
						iconv_close(cd);
					}
					else outputStream.write(data->dt_data.dt_text.dt_data, data->dt_data.dt_text.dt_length);
					break;
			}
		}
	}

	static void getMIMEMsgBody(struct mailmime * mime, std::stringstream &outputStream, std::stringstream &htmlOutputStream)
	{
		clistiter * cur;		
		switch (mime->mm_type) {
			case MAILMIME_SINGLE:
			{
				if (strcasecmp(mime->mm_content_type->ct_subtype, MIMEContentSubtype::plain) == 0) {
					//Now we guess the charset
					clist *params = mime->mm_content_type->ct_parameters;
					std::string charset = TextEncoding::utf8;
					for (clistiter *param = clist_begin(params); param != NULL; param = clist_next(param)) {
						struct mailmime_parameter *theParam = (struct mailmime_parameter *)clist_content(param);
						//std::cout << theParam->pa_name << "=" << theParam->pa_value << std::endl;
						if (strcasecmp(theParam->pa_name, MIMEParameter::charset) == 0) {
							charset = theParam->pa_value;
						}
					}
					getMIMEData(mime->mm_data.mm_single, outputStream, charset);
				}
				else if (strcasecmp(mime->mm_content_type->ct_subtype, MIMEContentSubtype::html) == 0) {
					clist *params = mime->mm_content_type->ct_parameters;
					std::string charset = TextEncoding::utf8;
					for (clistiter *param = clist_begin(params); param != NULL; param = clist_next(param)) {
						struct mailmime_parameter *theParam = (struct mailmime_parameter *)clist_content(param);
						//std::cout << theParam->pa_name << "=" << theParam->pa_value << std::endl;
						if (strcasecmp(theParam->pa_name, MIMEParameter::charset) == 0) {
							charset = theParam->pa_value;
						}
					}
					getMIMEData(mime->mm_data.mm_single, htmlOutputStream, charset);
				}
			}
				break;
			case MAILMIME_MULTIPLE:
				for(cur = clist_begin(mime->mm_data.mm_multipart.mm_mp_list) ; cur != NULL ; cur = clist_next(cur)) {
					getMIMEMsgBody((struct mailmime *)clist_content(cur), outputStream, htmlOutputStream);
				}
				break;
				
			case MAILMIME_MESSAGE:
				if (mime->mm_data.mm_message.mm_fields) {					
					if (mime->mm_data.mm_message.mm_msg_mime != NULL) {
						getMIMEMsgBody(mime->mm_data.mm_message.mm_msg_mime, outputStream, htmlOutputStream);
					}
					break;
				}
		}
	}

	
	MailMessage::MailMessage(){
		serverDate = 0;
		tzone = 0;
	}
	
	MailMessage::MailMessage(const std::string &_from, const std::string &_subject){
		from = _from;
		subject = _subject;
		tzone = 0;
		serverDate = 0;
	}
	
	MailMessage::MailMessage(const MailMessage &m){
		from = m.from;
		subject = m.subject;
		to = m.to;
		dateOfArrival = m.dateOfArrival;
		msgUid = m.msgUid;
		tzone = m.tzone;
		serverDate = m.serverDate;
		fromEmail = m.fromEmail;
		htmlMsg = m.htmlMsg;
	}
	
	bool MailMessage::parse(MailMessage &m, const std::string &rawMessage){ 
		return MailMessage::parse(m, rawMessage.c_str(), rawMessage.size());
	}
	
	bool MailMessage::parse(MailMessage &m, const char *msgBuffer, size_t msgSize){
		size_t indx = 0;
		struct mailimf_fields *fields;
		struct mailmime *result;
		int retCode = mailmime_parse(msgBuffer, msgSize, &indx, &result);
		if(retCode != MAILIMF_NO_ERROR){
			pmm::Log << "Unable to properly parse message";
			if(msgBuffer == 0 || msgSize == 0){
				pmm::Log << " because the given e-mail message is either NULL or empty!!!";
			}
			else {
				pmm::Log << ": " << msgBuffer;
			}
			pmm::Log << pmm::NL;
			return false;
		}
		fields = result->mm_data.mm_message.mm_fields;
		time_t now = time(0);
		m.from = "";
		m.subject = "";
		m.dateOfArrival = now;
		m.tzone = 0;
		bool gotTime = false;
		for (clistiter *iter = clist_begin(fields->fld_list); iter != clist_end(fields->fld_list); iter = iter->next) {
			struct mailimf_field *field = (struct mailimf_field *)clist_content(iter);
			switch (field->fld_type) {
#ifdef DEBUG_FROM_FIELD
				case MAILIMF_FIELD_OPTIONAL_FIELD:
				{
					pmm::Log << field->fld_data.fld_optional_field->fld_name << " = " << field->fld_data.fld_optional_field->fld_value << pmm::NL;
				}
					break;
#endif
				case MAILIMF_FIELD_FROM:
				{
					clistiter *iter2 = clist_begin(field->fld_data.fld_from->frm_mb_list->mb_list);
					struct mailimf_mailbox *mbox = (struct mailimf_mailbox *)clist_content(iter2);
					if(mbox->mb_display_name == NULL) m.from = mbox->mb_addr_spec;
					else{
						m.from = mbox->mb_display_name;
					}
					size_t s1pos; 
					if ((s1pos = m.from.find("=?")) != m.from.npos) {
						//Encoded with RFC 2047, let's decode this damn thing!!!
						size_t indx2 = 0;
						char *newFrom;
						int r = mailmime_encoded_phrase_parse(TextEncoding::utf8, m.from.c_str(), m.from.size(), &indx2, TextEncoding::utf8, &newFrom);
						if(r == MAILIMF_NO_ERROR){
							m.from = newFrom;
							free(newFrom);
						}
					}
					if(mbox->mb_addr_spec != 0) m.fromEmail = mbox->mb_addr_spec;
#ifdef DEBUG_FROM_FIELD
					pmm::Log << "DEBUG: From=\"" << m.from << "\" fromEmail=\"" << m.fromEmail << "\"" << pmm::NL;
#endif
					if (m.from.size() > 0 && m.subject.size() > 0 && gotTime) break;
				}
					break;
				case MAILIMF_FIELD_SUBJECT:
				{
					m.subject = field->fld_data.fld_subject->sbj_value;
					size_t s1pos; 
					if ((s1pos = m.subject.find("=?")) != m.subject.npos) {
						size_t indx2 = 0;
						char *newSubject = 0;
						mailmime_encoded_phrase_parse(TextEncoding::utf8, m.subject.c_str(), m.subject.size(), &indx2, TextEncoding::utf8, &newSubject);
						if(newSubject != 0){
							m.subject = newSubject;
							free(newSubject);
						}
						else {
							pmm::Log << "Unable to decode subject from subject field: " << m.subject << pmm::NL;
							m.subject = "";
						}
					}
#ifdef DEBUG_SUBJECT_DATA
					pmm::Log << "DEBUG: Subject=\"" << m.subject << "\"" << pmm::NL;
#endif
					if (m.from.size() > 0 && m.subject.size() > 0 && gotTime) break;
				}
					break;
				case MAILIMF_FIELD_ORIG_DATE:
				{
					struct mailimf_date_time *origDate = field->fld_data.fld_orig_date->dt_date_time;
					struct tm tDate = { 0 };
					memset(&tDate, 0, sizeof(tm));
					tDate.tm_year = origDate->dt_year - 1900;
					tDate.tm_mon = origDate->dt_month - 1;
					tDate.tm_mday = origDate->dt_day;
					tDate.tm_hour = origDate->dt_hour;
					tDate.tm_min = origDate->dt_min;
					tDate.tm_sec = origDate->dt_sec;
					tDate.tm_gmtoff = origDate->dt_zone;
					m.serverDate = mktime(&tDate);
					m.dateOfArrival = now;
					m.tzone = 0;
#ifdef DEBUG_MSG_TIME_FIELD
					pmm::Log << "DEBUG: Computed dateOfArrival=" << m.dateOfArrival << " remote time="<< m.serverDate << " currTstamp=" << now << " diff=" << (m.dateOfArrival - m.serverDate) << pmm::NL;
#endif
					gotTime = true;
					if (m.from.size() > 0 && m.subject.size() > 0 && gotTime) break;
				}
					break;
				default:
					break;
			}
		}
		if (m.subject.size() <= 384) {
			std::stringstream msgBody;
			std::stringstream htmlBody;
			getMIMEMsgBody(result, msgBody, htmlBody);
			std::string theBody = msgBody.str();
			std::string theHtmlBody = htmlBody.str();
			if(theBody.size() == 0 &&  theHtmlBody.size() > 0){
				std::map<std::string, std::string> htmlProps;
				stripHTMLTags(htmlBody.str(), theHtmlBody, htmlProps, 4096);
				if (!(htmlProps["charset"].compare(TextEncoding::utf8) == 0 || htmlProps["charset"].compare("utf-8") == 0)) {
					//Encoding is different convert it to utf8
					std::string html_s = theHtmlBody;
					size_t inleft = html_s.size(), outleft = html_s.size() * 2;
					char *utf8d = (char *)malloc(outleft);
					bzero(utf8d, outleft);
					char *ptr_in = (char *)html_s.c_str(), *ptr_out = utf8d;
					iconv_t cd = iconv_open(TextEncoding::utf8, htmlProps["charset"].c_str());
					iconv(cd, &ptr_in, &inleft, &ptr_out, &outleft);
					theHtmlBody = utf8d;
					free(utf8d);
					iconv_close(cd);
				}
				theBody = theHtmlBody;
			}
			std::string tmpBody = theBody;
			stripBlankLines(tmpBody, theBody);
			/*while (theBody[0] == '\r' || theBody[0] == '\n') {
				theBody = theBody.substr(1);
			}*/
			if (theBody.size() > 0) {
				if(m.subject.size() > 0) m.subject.append("\n");
				if(theBody.size() < 512){
					m.subject.append(theBody.c_str(), theBody.size());
				}
				else {
					m.subject.append(theBody.c_str(), 512);
				}
				while (m.subject[0] == '\r' || m.subject[0] == '\n') {
					m.subject = m.subject.substr(1);
				}
			}
			else {
				m.subject.append("\n------------------\nMessage does not have a plaintext body.");
			}
		}
		mailmime_free(result);
		return true;
	}
	
	void MailMessage::toJson(std::string &_result, const std::string &sound){
		std::stringstream json;
		if(to.size() == 0) throw GenericException("Unable to create ");
		json << "{\"emailAccount\":\"" << to << "\",";
		json << "\"tStamp\":\"" << dateOfArrival << "\",";
		json << "\"from\":\"" << from << "\",";
		if(fromEmail.size() > 0) json << "\"fromEmail\":\"" << fromEmail << "\",";
		json << "\"sound\":\"" << sound << "\",";
		json << "\"msgUID\":\"" << msgUid << "\",";
		json << "\"timezone\":\"" << tzone << "\",";
		
		std::string encodedSubject = subject;
		jsonTextEncode(encodedSubject);
		json << "\"subject\":\"" << encodedSubject << "\"";
		json << "}";
		_result = json.str();
	}
}
