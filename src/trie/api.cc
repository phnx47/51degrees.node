/*

This Source Code Form is copyright of Yorkshire, Inc.
Copyright © 2014 Yorkshire, Inc,
Guiyang, Guizhou, China

This Source Code Form is copyright of 51Degrees Mobile Experts Limited.
Copyright © 2014 51Degrees Mobile Experts Limited, 5 Charlotte Close,
Caversham, Reading, Berkshire, United Kingdom RG4 7BY

This Source Code Form is the subject of the following patent
applications, owned by 51Degrees Mobile Experts Limited of 5 Charlotte
Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
European Patent Application No. 13192291.6; and
United States Patent Application Nos. 14/085,223 and 14/085,301.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0.

If a copy of the MPL was not distributed with this file, You can obtain
one at http://mozilla.org/MPL/2.0/.

This Source Code Form is “Incompatible With Secondary Licenses”, as
defined by the Mozilla Public License, v. 2.0.

*/

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <v8.h>
#include "api.h"

using namespace v8;

TrieParser::TrieParser(char * filename, char * required_properties) {
  init_result = fiftyoneDegreesInitWithPropertyString(filename, required_properties);
}

TrieParser::~TrieParser() {
}

void TrieParser::Init(Handle<Object> target) {
  Nan::HandleScope scope;
  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  // TODO(Yorkie): will remove
  t->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(t, "parse", Parse);
  Nan::Set(target, 
    Nan::New<String>("TrieParser").ToLocalChecked(), 
    t->GetFunction());
}

NAN_METHOD(TrieParser::New) {
  Nan::HandleScope scope;
  char *filename;
  char *required_properties;

  // convert v8 objects to c/c++ types
  v8::String::Utf8Value v8_filename(info[0]->ToString());
  v8::String::Utf8Value v8_properties(info[1]->ToString());
  filename = *v8_filename;
  required_properties = *v8_properties;

  // create new instance of C++ class TrieParser
  TrieParser *parser = new TrieParser(filename, required_properties);
  parser->Wrap(info.This());

  // valid the database file content
  switch(parser->init_result) {
    case DATA_SET_INIT_STATUS_INSUFFICIENT_MEMORY:
      return Nan::ThrowError("Insufficient memory");
    case DATA_SET_INIT_STATUS_CORRUPT_DATA:
      return Nan::ThrowError("Device data file is corrupted");
    case DATA_SET_INIT_STATUS_INCORRECT_VERSION:
      return Nan::ThrowError("Device data file is not correct");
    case DATA_SET_INIT_STATUS_FILE_NOT_FOUND:
      return Nan::ThrowError("Device data file not found");
    default:
      info.GetReturnValue().Set(info.This());
  }
}

NAN_METHOD(TrieParser::Parse) {
  Nan::HandleScope scope;

  // convert v8 objects to c/c++ types
  char *input = NULL;
  Local<Object> result = Nan::New<Object>();
  v8::String::Utf8Value v8_input(info[0]->ToString());
  input = *v8_input;
  
  int device = fiftyoneDegreesGetDeviceOffset(input);
  int index;
  int propCount = fiftyoneDegreesGetRequiredPropertiesCount();
  char **propNames = fiftyoneDegreesGetRequiredPropertiesNames();

  for (index = 0; index < propCount; index++) {
    char *key = *(propNames + index);
    Local <v8::Value> keyVal = Nan::New<v8::String>(key).ToLocalChecked();

    int propIndex = fiftyoneDegreesGetPropertyIndex(key);
    char *val = fiftyoneDegreesGetValue(device, propIndex);
    if (strcmp(val, "True") == 0)
      Nan::Set(result, keyVal, Nan::True());
    else if (strcmp(val, "False") == 0)
      Nan::Set(result, keyVal, Nan::False());
    else
      Nan::Set(result, keyVal, Nan::New<v8::String>(val).ToLocalChecked());
  }
  info.GetReturnValue().Set(result);
}

NODE_MODULE(trie, TrieParser::Init)
