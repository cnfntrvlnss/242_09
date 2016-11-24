// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: FixedAudioModel.proto

#ifndef PROTOBUF_FixedAudioModel_2eproto__INCLUDED
#define PROTOBUF_FixedAudioModel_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace FixedAudioModel {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_FixedAudioModel_2eproto();
void protobuf_AssignDesc_FixedAudioModel_2eproto();
void protobuf_ShutdownFile_FixedAudioModel_2eproto();

class TaskHeader;
class ResultHeader;
class ModelInfo;
class LoadResult;

enum ModelInfo_StatusCode {
  ModelInfo_StatusCode_FALURE = 0,
  ModelInfo_StatusCode_SUCCESS = 1
};
bool ModelInfo_StatusCode_IsValid(int value);
const ModelInfo_StatusCode ModelInfo_StatusCode_StatusCode_MIN = ModelInfo_StatusCode_FALURE;
const ModelInfo_StatusCode ModelInfo_StatusCode_StatusCode_MAX = ModelInfo_StatusCode_SUCCESS;
const int ModelInfo_StatusCode_StatusCode_ARRAYSIZE = ModelInfo_StatusCode_StatusCode_MAX + 1;

const ::google::protobuf::EnumDescriptor* ModelInfo_StatusCode_descriptor();
inline const ::std::string& ModelInfo_StatusCode_Name(ModelInfo_StatusCode value) {
  return ::google::protobuf::internal::NameOfEnum(
    ModelInfo_StatusCode_descriptor(), value);
}
inline bool ModelInfo_StatusCode_Parse(
    const ::std::string& name, ModelInfo_StatusCode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ModelInfo_StatusCode>(
    ModelInfo_StatusCode_descriptor(), name, value);
}
enum LoadResult_StatusCode {
  LoadResult_StatusCode_FALURE = 0,
  LoadResult_StatusCode_SUCCESS = 1
};
bool LoadResult_StatusCode_IsValid(int value);
const LoadResult_StatusCode LoadResult_StatusCode_StatusCode_MIN = LoadResult_StatusCode_FALURE;
const LoadResult_StatusCode LoadResult_StatusCode_StatusCode_MAX = LoadResult_StatusCode_SUCCESS;
const int LoadResult_StatusCode_StatusCode_ARRAYSIZE = LoadResult_StatusCode_StatusCode_MAX + 1;

const ::google::protobuf::EnumDescriptor* LoadResult_StatusCode_descriptor();
inline const ::std::string& LoadResult_StatusCode_Name(LoadResult_StatusCode value) {
  return ::google::protobuf::internal::NameOfEnum(
    LoadResult_StatusCode_descriptor(), value);
}
inline bool LoadResult_StatusCode_Parse(
    const ::std::string& name, LoadResult_StatusCode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<LoadResult_StatusCode>(
    LoadResult_StatusCode_descriptor(), name, value);
}
// ===================================================================

class TaskHeader : public ::google::protobuf::Message {
 public:
  TaskHeader();
  virtual ~TaskHeader();

  TaskHeader(const TaskHeader& from);

  inline TaskHeader& operator=(const TaskHeader& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TaskHeader& default_instance();

  void Swap(TaskHeader* other);

  // implements Message ----------------------------------------------

  TaskHeader* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const TaskHeader& from);
  void MergeFrom(const TaskHeader& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required fixed32 taskLength = 1;
  inline bool has_tasklength() const;
  inline void clear_tasklength();
  static const int kTaskLengthFieldNumber = 1;
  inline ::google::protobuf::uint32 tasklength() const;
  inline void set_tasklength(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.TaskHeader)
 private:
  inline void set_has_tasklength();
  inline void clear_has_tasklength();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint32 tasklength_;
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
  static TaskHeader* default_instance_;
};
// -------------------------------------------------------------------

class ResultHeader : public ::google::protobuf::Message {
 public:
  ResultHeader();
  virtual ~ResultHeader();

  ResultHeader(const ResultHeader& from);

  inline ResultHeader& operator=(const ResultHeader& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ResultHeader& default_instance();

  void Swap(ResultHeader* other);

  // implements Message ----------------------------------------------

  ResultHeader* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ResultHeader& from);
  void MergeFrom(const ResultHeader& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required fixed32 resultLength = 2;
  inline bool has_resultlength() const;
  inline void clear_resultlength();
  static const int kResultLengthFieldNumber = 2;
  inline ::google::protobuf::uint32 resultlength() const;
  inline void set_resultlength(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.ResultHeader)
 private:
  inline void set_has_resultlength();
  inline void clear_has_resultlength();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint32 resultlength_;
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
  static ResultHeader* default_instance_;
};
// -------------------------------------------------------------------

class ModelInfo : public ::google::protobuf::Message {
 public:
  ModelInfo();
  virtual ~ModelInfo();

  ModelInfo(const ModelInfo& from);

  inline ModelInfo& operator=(const ModelInfo& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ModelInfo& default_instance();

  void Swap(ModelInfo* other);

  // implements Message ----------------------------------------------

  ModelInfo* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ModelInfo& from);
  void MergeFrom(const ModelInfo& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef ModelInfo_StatusCode StatusCode;
  static const StatusCode FALURE = ModelInfo_StatusCode_FALURE;
  static const StatusCode SUCCESS = ModelInfo_StatusCode_SUCCESS;
  static inline bool StatusCode_IsValid(int value) {
    return ModelInfo_StatusCode_IsValid(value);
  }
  static const StatusCode StatusCode_MIN =
    ModelInfo_StatusCode_StatusCode_MIN;
  static const StatusCode StatusCode_MAX =
    ModelInfo_StatusCode_StatusCode_MAX;
  static const int StatusCode_ARRAYSIZE =
    ModelInfo_StatusCode_StatusCode_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  StatusCode_descriptor() {
    return ModelInfo_StatusCode_descriptor();
  }
  static inline const ::std::string& StatusCode_Name(StatusCode value) {
    return ModelInfo_StatusCode_Name(value);
  }
  static inline bool StatusCode_Parse(const ::std::string& name,
      StatusCode* value) {
    return ModelInfo_StatusCode_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required string taskid = 1;
  inline bool has_taskid() const;
  inline void clear_taskid();
  static const int kTaskidFieldNumber = 1;
  inline const ::std::string& taskid() const;
  inline void set_taskid(const ::std::string& value);
  inline void set_taskid(const char* value);
  inline void set_taskid(const char* value, size_t size);
  inline ::std::string* mutable_taskid();
  inline ::std::string* release_taskid();
  inline void set_allocated_taskid(::std::string* taskid);

  // required .FixedAudioModel.ModelInfo.StatusCode status = 2;
  inline bool has_status() const;
  inline void clear_status();
  static const int kStatusFieldNumber = 2;
  inline ::FixedAudioModel::ModelInfo_StatusCode status() const;
  inline void set_status(::FixedAudioModel::ModelInfo_StatusCode value);

  // required bytes modelUrl = 3;
  inline bool has_modelurl() const;
  inline void clear_modelurl();
  static const int kModelUrlFieldNumber = 3;
  inline const ::std::string& modelurl() const;
  inline void set_modelurl(const ::std::string& value);
  inline void set_modelurl(const char* value);
  inline void set_modelurl(const void* value, size_t size);
  inline ::std::string* mutable_modelurl();
  inline ::std::string* release_modelurl();
  inline void set_allocated_modelurl(::std::string* modelurl);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.ModelInfo)
 private:
  inline void set_has_taskid();
  inline void clear_has_taskid();
  inline void set_has_status();
  inline void clear_has_status();
  inline void set_has_modelurl();
  inline void clear_has_modelurl();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* taskid_;
  ::std::string* modelurl_;
  int status_;
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
  static ModelInfo* default_instance_;
};
// -------------------------------------------------------------------

class LoadResult : public ::google::protobuf::Message {
 public:
  LoadResult();
  virtual ~LoadResult();

  LoadResult(const LoadResult& from);

  inline LoadResult& operator=(const LoadResult& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LoadResult& default_instance();

  void Swap(LoadResult* other);

  // implements Message ----------------------------------------------

  LoadResult* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const LoadResult& from);
  void MergeFrom(const LoadResult& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef LoadResult_StatusCode StatusCode;
  static const StatusCode FALURE = LoadResult_StatusCode_FALURE;
  static const StatusCode SUCCESS = LoadResult_StatusCode_SUCCESS;
  static inline bool StatusCode_IsValid(int value) {
    return LoadResult_StatusCode_IsValid(value);
  }
  static const StatusCode StatusCode_MIN =
    LoadResult_StatusCode_StatusCode_MIN;
  static const StatusCode StatusCode_MAX =
    LoadResult_StatusCode_StatusCode_MAX;
  static const int StatusCode_ARRAYSIZE =
    LoadResult_StatusCode_StatusCode_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  StatusCode_descriptor() {
    return LoadResult_StatusCode_descriptor();
  }
  static inline const ::std::string& StatusCode_Name(StatusCode value) {
    return LoadResult_StatusCode_Name(value);
  }
  static inline bool StatusCode_Parse(const ::std::string& name,
      StatusCode* value) {
    return LoadResult_StatusCode_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required string taskid = 1;
  inline bool has_taskid() const;
  inline void clear_taskid();
  static const int kTaskidFieldNumber = 1;
  inline const ::std::string& taskid() const;
  inline void set_taskid(const ::std::string& value);
  inline void set_taskid(const char* value);
  inline void set_taskid(const char* value, size_t size);
  inline ::std::string* mutable_taskid();
  inline ::std::string* release_taskid();
  inline void set_allocated_taskid(::std::string* taskid);

  // required .FixedAudioModel.LoadResult.StatusCode status = 2;
  inline bool has_status() const;
  inline void clear_status();
  static const int kStatusFieldNumber = 2;
  inline ::FixedAudioModel::LoadResult_StatusCode status() const;
  inline void set_status(::FixedAudioModel::LoadResult_StatusCode value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.LoadResult)
 private:
  inline void set_has_taskid();
  inline void clear_has_taskid();
  inline void set_has_status();
  inline void clear_has_status();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* taskid_;
  int status_;
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
  static LoadResult* default_instance_;
};
// ===================================================================


// ===================================================================

// TaskHeader

// required fixed32 taskLength = 1;
inline bool TaskHeader::has_tasklength() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void TaskHeader::set_has_tasklength() {
  _has_bits_[0] |= 0x00000001u;
}
inline void TaskHeader::clear_has_tasklength() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void TaskHeader::clear_tasklength() {
  tasklength_ = 0u;
  clear_has_tasklength();
}
inline ::google::protobuf::uint32 TaskHeader::tasklength() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.TaskHeader.taskLength)
  return tasklength_;
}
inline void TaskHeader::set_tasklength(::google::protobuf::uint32 value) {
  set_has_tasklength();
  tasklength_ = value;
  // @@protoc_insertion_point(field_set:FixedAudioModel.TaskHeader.taskLength)
}

// -------------------------------------------------------------------

// ResultHeader

// required fixed32 resultLength = 2;
inline bool ResultHeader::has_resultlength() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ResultHeader::set_has_resultlength() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ResultHeader::clear_has_resultlength() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ResultHeader::clear_resultlength() {
  resultlength_ = 0u;
  clear_has_resultlength();
}
inline ::google::protobuf::uint32 ResultHeader::resultlength() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ResultHeader.resultLength)
  return resultlength_;
}
inline void ResultHeader::set_resultlength(::google::protobuf::uint32 value) {
  set_has_resultlength();
  resultlength_ = value;
  // @@protoc_insertion_point(field_set:FixedAudioModel.ResultHeader.resultLength)
}

// -------------------------------------------------------------------

// ModelInfo

// required string taskid = 1;
inline bool ModelInfo::has_taskid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ModelInfo::set_has_taskid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ModelInfo::clear_has_taskid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ModelInfo::clear_taskid() {
  if (taskid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_->clear();
  }
  clear_has_taskid();
}
inline const ::std::string& ModelInfo::taskid() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ModelInfo.taskid)
  return *taskid_;
}
inline void ModelInfo::set_taskid(const ::std::string& value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.ModelInfo.taskid)
}
inline void ModelInfo::set_taskid(const char* value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.ModelInfo.taskid)
}
inline void ModelInfo::set_taskid(const char* value, size_t size) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.ModelInfo.taskid)
}
inline ::std::string* ModelInfo::mutable_taskid() {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.ModelInfo.taskid)
  return taskid_;
}
inline ::std::string* ModelInfo::release_taskid() {
  clear_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = taskid_;
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void ModelInfo::set_allocated_taskid(::std::string* taskid) {
  if (taskid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete taskid_;
  }
  if (taskid) {
    set_has_taskid();
    taskid_ = taskid;
  } else {
    clear_has_taskid();
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:FixedAudioModel.ModelInfo.taskid)
}

// required .FixedAudioModel.ModelInfo.StatusCode status = 2;
inline bool ModelInfo::has_status() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ModelInfo::set_has_status() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ModelInfo::clear_has_status() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ModelInfo::clear_status() {
  status_ = 0;
  clear_has_status();
}
inline ::FixedAudioModel::ModelInfo_StatusCode ModelInfo::status() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ModelInfo.status)
  return static_cast< ::FixedAudioModel::ModelInfo_StatusCode >(status_);
}
inline void ModelInfo::set_status(::FixedAudioModel::ModelInfo_StatusCode value) {
  assert(::FixedAudioModel::ModelInfo_StatusCode_IsValid(value));
  set_has_status();
  status_ = value;
  // @@protoc_insertion_point(field_set:FixedAudioModel.ModelInfo.status)
}

// required bytes modelUrl = 3;
inline bool ModelInfo::has_modelurl() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void ModelInfo::set_has_modelurl() {
  _has_bits_[0] |= 0x00000004u;
}
inline void ModelInfo::clear_has_modelurl() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void ModelInfo::clear_modelurl() {
  if (modelurl_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    modelurl_->clear();
  }
  clear_has_modelurl();
}
inline const ::std::string& ModelInfo::modelurl() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ModelInfo.modelUrl)
  return *modelurl_;
}
inline void ModelInfo::set_modelurl(const ::std::string& value) {
  set_has_modelurl();
  if (modelurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    modelurl_ = new ::std::string;
  }
  modelurl_->assign(value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.ModelInfo.modelUrl)
}
inline void ModelInfo::set_modelurl(const char* value) {
  set_has_modelurl();
  if (modelurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    modelurl_ = new ::std::string;
  }
  modelurl_->assign(value);
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.ModelInfo.modelUrl)
}
inline void ModelInfo::set_modelurl(const void* value, size_t size) {
  set_has_modelurl();
  if (modelurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    modelurl_ = new ::std::string;
  }
  modelurl_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.ModelInfo.modelUrl)
}
inline ::std::string* ModelInfo::mutable_modelurl() {
  set_has_modelurl();
  if (modelurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    modelurl_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.ModelInfo.modelUrl)
  return modelurl_;
}
inline ::std::string* ModelInfo::release_modelurl() {
  clear_has_modelurl();
  if (modelurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = modelurl_;
    modelurl_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void ModelInfo::set_allocated_modelurl(::std::string* modelurl) {
  if (modelurl_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete modelurl_;
  }
  if (modelurl) {
    set_has_modelurl();
    modelurl_ = modelurl;
  } else {
    clear_has_modelurl();
    modelurl_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:FixedAudioModel.ModelInfo.modelUrl)
}

// -------------------------------------------------------------------

// LoadResult

// required string taskid = 1;
inline bool LoadResult::has_taskid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void LoadResult::set_has_taskid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void LoadResult::clear_has_taskid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void LoadResult::clear_taskid() {
  if (taskid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_->clear();
  }
  clear_has_taskid();
}
inline const ::std::string& LoadResult::taskid() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.LoadResult.taskid)
  return *taskid_;
}
inline void LoadResult::set_taskid(const ::std::string& value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.LoadResult.taskid)
}
inline void LoadResult::set_taskid(const char* value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.LoadResult.taskid)
}
inline void LoadResult::set_taskid(const char* value, size_t size) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.LoadResult.taskid)
}
inline ::std::string* LoadResult::mutable_taskid() {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    taskid_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.LoadResult.taskid)
  return taskid_;
}
inline ::std::string* LoadResult::release_taskid() {
  clear_has_taskid();
  if (taskid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = taskid_;
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void LoadResult::set_allocated_taskid(::std::string* taskid) {
  if (taskid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete taskid_;
  }
  if (taskid) {
    set_has_taskid();
    taskid_ = taskid;
  } else {
    clear_has_taskid();
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:FixedAudioModel.LoadResult.taskid)
}

// required .FixedAudioModel.LoadResult.StatusCode status = 2;
inline bool LoadResult::has_status() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void LoadResult::set_has_status() {
  _has_bits_[0] |= 0x00000002u;
}
inline void LoadResult::clear_has_status() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void LoadResult::clear_status() {
  status_ = 0;
  clear_has_status();
}
inline ::FixedAudioModel::LoadResult_StatusCode LoadResult::status() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.LoadResult.status)
  return static_cast< ::FixedAudioModel::LoadResult_StatusCode >(status_);
}
inline void LoadResult::set_status(::FixedAudioModel::LoadResult_StatusCode value) {
  assert(::FixedAudioModel::LoadResult_StatusCode_IsValid(value));
  set_has_status();
  status_ = value;
  // @@protoc_insertion_point(field_set:FixedAudioModel.LoadResult.status)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace FixedAudioModel

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::FixedAudioModel::ModelInfo_StatusCode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::FixedAudioModel::ModelInfo_StatusCode>() {
  return ::FixedAudioModel::ModelInfo_StatusCode_descriptor();
}
template <> struct is_proto_enum< ::FixedAudioModel::LoadResult_StatusCode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::FixedAudioModel::LoadResult_StatusCode>() {
  return ::FixedAudioModel::LoadResult_StatusCode_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_FixedAudioModel_2eproto__INCLUDED
