// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: FixedAudioModel.proto

#ifndef PROTOBUF_FixedAudioModel_2eproto__INCLUDED
#define PROTOBUF_FixedAudioModel_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3001000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3001000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace FixedAudioModel {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_FixedAudioModel_2eproto();
void protobuf_InitDefaults_FixedAudioModel_2eproto();
void protobuf_AssignDesc_FixedAudioModel_2eproto();
void protobuf_ShutdownFile_FixedAudioModel_2eproto();

class LoadResult;
class ModelInfo;
class ResultHeader;
class TaskHeader;

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

class TaskHeader : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:FixedAudioModel.TaskHeader) */ {
 public:
  TaskHeader();
  virtual ~TaskHeader();

  TaskHeader(const TaskHeader& from);

  inline TaskHeader& operator=(const TaskHeader& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TaskHeader& default_instance();

  static const TaskHeader* internal_default_instance();

  void Swap(TaskHeader* other);

  // implements Message ----------------------------------------------

  inline TaskHeader* New() const { return New(NULL); }

  TaskHeader* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const TaskHeader& from);
  void MergeFrom(const TaskHeader& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(TaskHeader* other);
  void UnsafeMergeFrom(const TaskHeader& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required fixed32 taskLength = 1;
  bool has_tasklength() const;
  void clear_tasklength();
  static const int kTaskLengthFieldNumber = 1;
  ::google::protobuf::uint32 tasklength() const;
  void set_tasklength(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.TaskHeader)
 private:
  inline void set_has_tasklength();
  inline void clear_has_tasklength();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::uint32 tasklength_;
  friend void  protobuf_InitDefaults_FixedAudioModel_2eproto_impl();
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto_impl();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<TaskHeader> TaskHeader_default_instance_;

// -------------------------------------------------------------------

class ResultHeader : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:FixedAudioModel.ResultHeader) */ {
 public:
  ResultHeader();
  virtual ~ResultHeader();

  ResultHeader(const ResultHeader& from);

  inline ResultHeader& operator=(const ResultHeader& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ResultHeader& default_instance();

  static const ResultHeader* internal_default_instance();

  void Swap(ResultHeader* other);

  // implements Message ----------------------------------------------

  inline ResultHeader* New() const { return New(NULL); }

  ResultHeader* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ResultHeader& from);
  void MergeFrom(const ResultHeader& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ResultHeader* other);
  void UnsafeMergeFrom(const ResultHeader& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required fixed32 resultLength = 2;
  bool has_resultlength() const;
  void clear_resultlength();
  static const int kResultLengthFieldNumber = 2;
  ::google::protobuf::uint32 resultlength() const;
  void set_resultlength(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.ResultHeader)
 private:
  inline void set_has_resultlength();
  inline void clear_has_resultlength();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::uint32 resultlength_;
  friend void  protobuf_InitDefaults_FixedAudioModel_2eproto_impl();
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto_impl();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<ResultHeader> ResultHeader_default_instance_;

// -------------------------------------------------------------------

class ModelInfo : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:FixedAudioModel.ModelInfo) */ {
 public:
  ModelInfo();
  virtual ~ModelInfo();

  ModelInfo(const ModelInfo& from);

  inline ModelInfo& operator=(const ModelInfo& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ModelInfo& default_instance();

  static const ModelInfo* internal_default_instance();

  void Swap(ModelInfo* other);

  // implements Message ----------------------------------------------

  inline ModelInfo* New() const { return New(NULL); }

  ModelInfo* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ModelInfo& from);
  void MergeFrom(const ModelInfo& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ModelInfo* other);
  void UnsafeMergeFrom(const ModelInfo& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef ModelInfo_StatusCode StatusCode;
  static const StatusCode FALURE =
    ModelInfo_StatusCode_FALURE;
  static const StatusCode SUCCESS =
    ModelInfo_StatusCode_SUCCESS;
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
  bool has_taskid() const;
  void clear_taskid();
  static const int kTaskidFieldNumber = 1;
  const ::std::string& taskid() const;
  void set_taskid(const ::std::string& value);
  void set_taskid(const char* value);
  void set_taskid(const char* value, size_t size);
  ::std::string* mutable_taskid();
  ::std::string* release_taskid();
  void set_allocated_taskid(::std::string* taskid);

  // required .FixedAudioModel.ModelInfo.StatusCode status = 2;
  bool has_status() const;
  void clear_status();
  static const int kStatusFieldNumber = 2;
  ::FixedAudioModel::ModelInfo_StatusCode status() const;
  void set_status(::FixedAudioModel::ModelInfo_StatusCode value);

  // required bytes modelUrl = 3;
  bool has_modelurl() const;
  void clear_modelurl();
  static const int kModelUrlFieldNumber = 3;
  const ::std::string& modelurl() const;
  void set_modelurl(const ::std::string& value);
  void set_modelurl(const char* value);
  void set_modelurl(const void* value, size_t size);
  ::std::string* mutable_modelurl();
  ::std::string* release_modelurl();
  void set_allocated_modelurl(::std::string* modelurl);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.ModelInfo)
 private:
  inline void set_has_taskid();
  inline void clear_has_taskid();
  inline void set_has_status();
  inline void clear_has_status();
  inline void set_has_modelurl();
  inline void clear_has_modelurl();

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr taskid_;
  ::google::protobuf::internal::ArenaStringPtr modelurl_;
  int status_;
  friend void  protobuf_InitDefaults_FixedAudioModel_2eproto_impl();
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto_impl();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<ModelInfo> ModelInfo_default_instance_;

// -------------------------------------------------------------------

class LoadResult : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:FixedAudioModel.LoadResult) */ {
 public:
  LoadResult();
  virtual ~LoadResult();

  LoadResult(const LoadResult& from);

  inline LoadResult& operator=(const LoadResult& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LoadResult& default_instance();

  static const LoadResult* internal_default_instance();

  void Swap(LoadResult* other);

  // implements Message ----------------------------------------------

  inline LoadResult* New() const { return New(NULL); }

  LoadResult* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const LoadResult& from);
  void MergeFrom(const LoadResult& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(LoadResult* other);
  void UnsafeMergeFrom(const LoadResult& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef LoadResult_StatusCode StatusCode;
  static const StatusCode FALURE =
    LoadResult_StatusCode_FALURE;
  static const StatusCode SUCCESS =
    LoadResult_StatusCode_SUCCESS;
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
  bool has_taskid() const;
  void clear_taskid();
  static const int kTaskidFieldNumber = 1;
  const ::std::string& taskid() const;
  void set_taskid(const ::std::string& value);
  void set_taskid(const char* value);
  void set_taskid(const char* value, size_t size);
  ::std::string* mutable_taskid();
  ::std::string* release_taskid();
  void set_allocated_taskid(::std::string* taskid);

  // required .FixedAudioModel.LoadResult.StatusCode status = 2;
  bool has_status() const;
  void clear_status();
  static const int kStatusFieldNumber = 2;
  ::FixedAudioModel::LoadResult_StatusCode status() const;
  void set_status(::FixedAudioModel::LoadResult_StatusCode value);

  // @@protoc_insertion_point(class_scope:FixedAudioModel.LoadResult)
 private:
  inline void set_has_taskid();
  inline void clear_has_taskid();
  inline void set_has_status();
  inline void clear_has_status();

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr taskid_;
  int status_;
  friend void  protobuf_InitDefaults_FixedAudioModel_2eproto_impl();
  friend void  protobuf_AddDesc_FixedAudioModel_2eproto_impl();
  friend void protobuf_AssignDesc_FixedAudioModel_2eproto();
  friend void protobuf_ShutdownFile_FixedAudioModel_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<LoadResult> LoadResult_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
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

inline const TaskHeader* TaskHeader::internal_default_instance() {
  return &TaskHeader_default_instance_.get();
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

inline const ResultHeader* ResultHeader::internal_default_instance() {
  return &ResultHeader_default_instance_.get();
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
  taskid_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_taskid();
}
inline const ::std::string& ModelInfo::taskid() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ModelInfo.taskid)
  return taskid_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ModelInfo::set_taskid(const ::std::string& value) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.ModelInfo.taskid)
}
inline void ModelInfo::set_taskid(const char* value) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.ModelInfo.taskid)
}
inline void ModelInfo::set_taskid(const char* value, size_t size) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.ModelInfo.taskid)
}
inline ::std::string* ModelInfo::mutable_taskid() {
  set_has_taskid();
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.ModelInfo.taskid)
  return taskid_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ModelInfo::release_taskid() {
  // @@protoc_insertion_point(field_release:FixedAudioModel.ModelInfo.taskid)
  clear_has_taskid();
  return taskid_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ModelInfo::set_allocated_taskid(::std::string* taskid) {
  if (taskid != NULL) {
    set_has_taskid();
  } else {
    clear_has_taskid();
  }
  taskid_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), taskid);
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
  modelurl_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_modelurl();
}
inline const ::std::string& ModelInfo::modelurl() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.ModelInfo.modelUrl)
  return modelurl_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ModelInfo::set_modelurl(const ::std::string& value) {
  set_has_modelurl();
  modelurl_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.ModelInfo.modelUrl)
}
inline void ModelInfo::set_modelurl(const char* value) {
  set_has_modelurl();
  modelurl_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.ModelInfo.modelUrl)
}
inline void ModelInfo::set_modelurl(const void* value, size_t size) {
  set_has_modelurl();
  modelurl_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.ModelInfo.modelUrl)
}
inline ::std::string* ModelInfo::mutable_modelurl() {
  set_has_modelurl();
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.ModelInfo.modelUrl)
  return modelurl_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ModelInfo::release_modelurl() {
  // @@protoc_insertion_point(field_release:FixedAudioModel.ModelInfo.modelUrl)
  clear_has_modelurl();
  return modelurl_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ModelInfo::set_allocated_modelurl(::std::string* modelurl) {
  if (modelurl != NULL) {
    set_has_modelurl();
  } else {
    clear_has_modelurl();
  }
  modelurl_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), modelurl);
  // @@protoc_insertion_point(field_set_allocated:FixedAudioModel.ModelInfo.modelUrl)
}

inline const ModelInfo* ModelInfo::internal_default_instance() {
  return &ModelInfo_default_instance_.get();
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
  taskid_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_taskid();
}
inline const ::std::string& LoadResult::taskid() const {
  // @@protoc_insertion_point(field_get:FixedAudioModel.LoadResult.taskid)
  return taskid_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void LoadResult::set_taskid(const ::std::string& value) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:FixedAudioModel.LoadResult.taskid)
}
inline void LoadResult::set_taskid(const char* value) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:FixedAudioModel.LoadResult.taskid)
}
inline void LoadResult::set_taskid(const char* value, size_t size) {
  set_has_taskid();
  taskid_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:FixedAudioModel.LoadResult.taskid)
}
inline ::std::string* LoadResult::mutable_taskid() {
  set_has_taskid();
  // @@protoc_insertion_point(field_mutable:FixedAudioModel.LoadResult.taskid)
  return taskid_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* LoadResult::release_taskid() {
  // @@protoc_insertion_point(field_release:FixedAudioModel.LoadResult.taskid)
  clear_has_taskid();
  return taskid_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void LoadResult::set_allocated_taskid(::std::string* taskid) {
  if (taskid != NULL) {
    set_has_taskid();
  } else {
    clear_has_taskid();
  }
  taskid_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), taskid);
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

inline const LoadResult* LoadResult::internal_default_instance() {
  return &LoadResult_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


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

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_FixedAudioModel_2eproto__INCLUDED