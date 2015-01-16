// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/ctocpp/stream_reader_ctocpp.h"
#include "libcef_dll/ctocpp/zip_reader_ctocpp.h"


// STATIC METHODS - Body may be edited by hand.

CefRefPtr<CefZipReader> CefZipReader::Create(
    CefRefPtr<CefStreamReader> stream) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: stream; type: refptr_same
  DCHECK(stream.get());
  if (!stream.get())
    return NULL;

  // Execute
  cef_zip_reader_t* _retval = cef_zip_reader_create(
      CefStreamReaderCToCpp::Unwrap(stream));

  // Return type: refptr_same
  return CefZipReaderCToCpp::Wrap(_retval);
}


// VIRTUAL METHODS - Body may be edited by hand.

bool CefZipReaderCToCpp::MoveToFirstFile() {
  if (CEF_MEMBER_MISSING(struct_, move_to_first_file))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->move_to_first_file(struct_);

  // Return type: bool
  return _retval?true:false;
}

bool CefZipReaderCToCpp::MoveToNextFile() {
  if (CEF_MEMBER_MISSING(struct_, move_to_next_file))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->move_to_next_file(struct_);

  // Return type: bool
  return _retval?true:false;
}

bool CefZipReaderCToCpp::MoveToFile(const CefString& fileName,
    bool caseSensitive) {
  if (CEF_MEMBER_MISSING(struct_, move_to_file))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: fileName; type: string_byref_const
  DCHECK(!fileName.empty());
  if (fileName.empty())
    return false;

  // Execute
  int _retval = struct_->move_to_file(struct_,
      fileName.GetStruct(),
      caseSensitive);

  // Return type: bool
  return _retval?true:false;
}

bool CefZipReaderCToCpp::Close() {
  if (CEF_MEMBER_MISSING(struct_, close))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->close(struct_);

  // Return type: bool
  return _retval?true:false;
}

CefString CefZipReaderCToCpp::GetFileName() {
  if (CEF_MEMBER_MISSING(struct_, get_file_name))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = struct_->get_file_name(struct_);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

int64 CefZipReaderCToCpp::GetFileSize() {
  if (CEF_MEMBER_MISSING(struct_, get_file_size))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int64 _retval = struct_->get_file_size(struct_);

  // Return type: simple
  return _retval;
}

time_t CefZipReaderCToCpp::GetFileLastModified() {
  if (CEF_MEMBER_MISSING(struct_, get_file_last_modified))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  time_t _retval = struct_->get_file_last_modified(struct_);

  // Return type: simple
  return _retval;
}

bool CefZipReaderCToCpp::OpenFile(const CefString& password) {
  if (CEF_MEMBER_MISSING(struct_, open_file))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: password

  // Execute
  int _retval = struct_->open_file(struct_,
      password.GetStruct());

  // Return type: bool
  return _retval?true:false;
}

bool CefZipReaderCToCpp::CloseFile() {
  if (CEF_MEMBER_MISSING(struct_, close_file))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->close_file(struct_);

  // Return type: bool
  return _retval?true:false;
}

int CefZipReaderCToCpp::ReadFile(void* buffer, size_t bufferSize) {
  if (CEF_MEMBER_MISSING(struct_, read_file))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: buffer; type: simple_byaddr
  DCHECK(buffer);
  if (!buffer)
    return 0;

  // Execute
  int _retval = struct_->read_file(struct_,
      buffer,
      bufferSize);

  // Return type: simple
  return _retval;
}

int64 CefZipReaderCToCpp::Tell() {
  if (CEF_MEMBER_MISSING(struct_, tell))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int64 _retval = struct_->tell(struct_);

  // Return type: simple
  return _retval;
}

bool CefZipReaderCToCpp::Eof() {
  if (CEF_MEMBER_MISSING(struct_, eof))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->eof(struct_);

  // Return type: bool
  return _retval?true:false;
}


#ifndef NDEBUG
template<> long CefCToCpp<CefZipReaderCToCpp, CefZipReader,
    cef_zip_reader_t>::DebugObjCt = 0;
#endif

