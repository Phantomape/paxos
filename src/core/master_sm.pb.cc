// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: master_sm.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "master_sm.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace paxos {
class MasterOperatorDefaultTypeInternal {
public:
 ::google::protobuf::internal::ExplicitlyConstructed<MasterOperator>
     _instance;
} _MasterOperator_default_instance_;

namespace protobuf_master_5fsm_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[1];

}  // namespace

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTableField
    const TableStruct::entries[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  {0, 0, 0, ::google::protobuf::internal::kInvalidMask, 0, 0},
};

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::AuxillaryParseTableField
    const TableStruct::aux[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  ::google::protobuf::internal::AuxillaryParseTableField(),
};
PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTable const
    TableStruct::schema[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { NULL, NULL, 0, -1, -1, -1, -1, NULL, false },
};

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, nodeid_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, version_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, timeout_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, operator__),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, sid_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MasterOperator, lastversion_),
  0,
  1,
  2,
  3,
  5,
  4,
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 11, sizeof(MasterOperator)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_MasterOperator_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "master_sm.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

}  // namespace
void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _MasterOperator_default_instance_._instance.DefaultConstruct();
  ::google::protobuf::internal::OnShutdownDestroyMessage(
      &_MasterOperator_default_instance_);}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
namespace {
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\017master_sm.proto\022\005paxos\"v\n\016MasterOperat"
      "or\022\016\n\006nodeid\030\001 \002(\004\022\017\n\007version\030\002 \002(\004\022\017\n\007t"
      "imeout\030\003 \002(\005\022\020\n\010operator\030\004 \002(\r\022\013\n\003sid\030\005 "
      "\002(\r\022\023\n\013lastversion\030\006 \001(\004"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 144);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "master_sm.proto", &protobuf_RegisterTypes);
}
} // anonymous namespace

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_master_5fsm_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int MasterOperator::kNodeidFieldNumber;
const int MasterOperator::kVersionFieldNumber;
const int MasterOperator::kTimeoutFieldNumber;
const int MasterOperator::kOperatorFieldNumber;
const int MasterOperator::kSidFieldNumber;
const int MasterOperator::kLastversionFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

MasterOperator::MasterOperator()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_master_5fsm_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:paxos.MasterOperator)
}
MasterOperator::MasterOperator(const MasterOperator& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&nodeid_, &from.nodeid_,
    static_cast<size_t>(reinterpret_cast<char*>(&sid_) -
    reinterpret_cast<char*>(&nodeid_)) + sizeof(sid_));
  // @@protoc_insertion_point(copy_constructor:paxos.MasterOperator)
}

void MasterOperator::SharedCtor() {
  _cached_size_ = 0;
  ::memset(&nodeid_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&sid_) -
      reinterpret_cast<char*>(&nodeid_)) + sizeof(sid_));
}

MasterOperator::~MasterOperator() {
  // @@protoc_insertion_point(destructor:paxos.MasterOperator)
  SharedDtor();
}

void MasterOperator::SharedDtor() {
}

void MasterOperator::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* MasterOperator::descriptor() {
  protobuf_master_5fsm_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_master_5fsm_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const MasterOperator& MasterOperator::default_instance() {
  protobuf_master_5fsm_2eproto::InitDefaults();
  return *internal_default_instance();
}

MasterOperator* MasterOperator::New(::google::protobuf::Arena* arena) const {
  MasterOperator* n = new MasterOperator;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void MasterOperator::Clear() {
// @@protoc_insertion_point(message_clear_start:paxos.MasterOperator)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 63u) {
    ::memset(&nodeid_, 0, static_cast<size_t>(
        reinterpret_cast<char*>(&sid_) -
        reinterpret_cast<char*>(&nodeid_)) + sizeof(sid_));
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool MasterOperator::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:paxos.MasterOperator)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint64 nodeid = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u /* 8 & 0xFF */)) {
          set_has_nodeid();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &nodeid_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint64 version = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u /* 16 & 0xFF */)) {
          set_has_version();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &version_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required int32 timeout = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(24u /* 24 & 0xFF */)) {
          set_has_timeout();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &timeout_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint32 operator = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(32u /* 32 & 0xFF */)) {
          set_has_operator_();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &operator__)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint32 sid = 5;
      case 5: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(40u /* 40 & 0xFF */)) {
          set_has_sid();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &sid_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional uint64 lastversion = 6;
      case 6: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(48u /* 48 & 0xFF */)) {
          set_has_lastversion();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &lastversion_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:paxos.MasterOperator)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:paxos.MasterOperator)
  return false;
#undef DO_
}

void MasterOperator::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:paxos.MasterOperator)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 nodeid = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->nodeid(), output);
  }

  // required uint64 version = 2;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(2, this->version(), output);
  }

  // required int32 timeout = 3;
  if (cached_has_bits & 0x00000004u) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->timeout(), output);
  }

  // required uint32 operator = 4;
  if (cached_has_bits & 0x00000008u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(4, this->operator_(), output);
  }

  // required uint32 sid = 5;
  if (cached_has_bits & 0x00000020u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(5, this->sid(), output);
  }

  // optional uint64 lastversion = 6;
  if (cached_has_bits & 0x00000010u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(6, this->lastversion(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:paxos.MasterOperator)
}

::google::protobuf::uint8* MasterOperator::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:paxos.MasterOperator)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 nodeid = 1;
  if (cached_has_bits & 0x00000001u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->nodeid(), target);
  }

  // required uint64 version = 2;
  if (cached_has_bits & 0x00000002u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(2, this->version(), target);
  }

  // required int32 timeout = 3;
  if (cached_has_bits & 0x00000004u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->timeout(), target);
  }

  // required uint32 operator = 4;
  if (cached_has_bits & 0x00000008u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(4, this->operator_(), target);
  }

  // required uint32 sid = 5;
  if (cached_has_bits & 0x00000020u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(5, this->sid(), target);
  }

  // optional uint64 lastversion = 6;
  if (cached_has_bits & 0x00000010u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(6, this->lastversion(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:paxos.MasterOperator)
  return target;
}

size_t MasterOperator::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:paxos.MasterOperator)
  size_t total_size = 0;

  if (has_nodeid()) {
    // required uint64 nodeid = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->nodeid());
  }

  if (has_version()) {
    // required uint64 version = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->version());
  }

  if (has_timeout()) {
    // required int32 timeout = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->timeout());
  }

  if (has_operator_()) {
    // required uint32 operator = 4;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->operator_());
  }

  if (has_sid()) {
    // required uint32 sid = 5;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->sid());
  }

  return total_size;
}
size_t MasterOperator::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:paxos.MasterOperator)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  if (((_has_bits_[0] & 0x0000002f) ^ 0x0000002f) == 0) {  // All required fields are present.
    // required uint64 nodeid = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->nodeid());

    // required uint64 version = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->version());

    // required int32 timeout = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->timeout());

    // required uint32 operator = 4;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->operator_());

    // required uint32 sid = 5;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->sid());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  // optional uint64 lastversion = 6;
  if (has_lastversion()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->lastversion());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void MasterOperator::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:paxos.MasterOperator)
  GOOGLE_DCHECK_NE(&from, this);
  const MasterOperator* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const MasterOperator>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:paxos.MasterOperator)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:paxos.MasterOperator)
    MergeFrom(*source);
  }
}

void MasterOperator::MergeFrom(const MasterOperator& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:paxos.MasterOperator)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 63u) {
    if (cached_has_bits & 0x00000001u) {
      nodeid_ = from.nodeid_;
    }
    if (cached_has_bits & 0x00000002u) {
      version_ = from.version_;
    }
    if (cached_has_bits & 0x00000004u) {
      timeout_ = from.timeout_;
    }
    if (cached_has_bits & 0x00000008u) {
      operator__ = from.operator__;
    }
    if (cached_has_bits & 0x00000010u) {
      lastversion_ = from.lastversion_;
    }
    if (cached_has_bits & 0x00000020u) {
      sid_ = from.sid_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void MasterOperator::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:paxos.MasterOperator)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void MasterOperator::CopyFrom(const MasterOperator& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:paxos.MasterOperator)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool MasterOperator::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000002f) != 0x0000002f) return false;
  return true;
}

void MasterOperator::Swap(MasterOperator* other) {
  if (other == this) return;
  InternalSwap(other);
}
void MasterOperator::InternalSwap(MasterOperator* other) {
  using std::swap;
  swap(nodeid_, other->nodeid_);
  swap(version_, other->version_);
  swap(timeout_, other->timeout_);
  swap(operator__, other->operator__);
  swap(lastversion_, other->lastversion_);
  swap(sid_, other->sid_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata MasterOperator::GetMetadata() const {
  protobuf_master_5fsm_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_master_5fsm_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// MasterOperator

// required uint64 nodeid = 1;
bool MasterOperator::has_nodeid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void MasterOperator::set_has_nodeid() {
  _has_bits_[0] |= 0x00000001u;
}
void MasterOperator::clear_has_nodeid() {
  _has_bits_[0] &= ~0x00000001u;
}
void MasterOperator::clear_nodeid() {
  nodeid_ = GOOGLE_ULONGLONG(0);
  clear_has_nodeid();
}
::google::protobuf::uint64 MasterOperator::nodeid() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.nodeid)
  return nodeid_;
}
void MasterOperator::set_nodeid(::google::protobuf::uint64 value) {
  set_has_nodeid();
  nodeid_ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.nodeid)
}

// required uint64 version = 2;
bool MasterOperator::has_version() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void MasterOperator::set_has_version() {
  _has_bits_[0] |= 0x00000002u;
}
void MasterOperator::clear_has_version() {
  _has_bits_[0] &= ~0x00000002u;
}
void MasterOperator::clear_version() {
  version_ = GOOGLE_ULONGLONG(0);
  clear_has_version();
}
::google::protobuf::uint64 MasterOperator::version() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.version)
  return version_;
}
void MasterOperator::set_version(::google::protobuf::uint64 value) {
  set_has_version();
  version_ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.version)
}

// required int32 timeout = 3;
bool MasterOperator::has_timeout() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
void MasterOperator::set_has_timeout() {
  _has_bits_[0] |= 0x00000004u;
}
void MasterOperator::clear_has_timeout() {
  _has_bits_[0] &= ~0x00000004u;
}
void MasterOperator::clear_timeout() {
  timeout_ = 0;
  clear_has_timeout();
}
::google::protobuf::int32 MasterOperator::timeout() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.timeout)
  return timeout_;
}
void MasterOperator::set_timeout(::google::protobuf::int32 value) {
  set_has_timeout();
  timeout_ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.timeout)
}

// required uint32 operator = 4;
bool MasterOperator::has_operator_() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
void MasterOperator::set_has_operator_() {
  _has_bits_[0] |= 0x00000008u;
}
void MasterOperator::clear_has_operator_() {
  _has_bits_[0] &= ~0x00000008u;
}
void MasterOperator::clear_operator_() {
  operator__ = 0u;
  clear_has_operator_();
}
::google::protobuf::uint32 MasterOperator::operator_() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.operator)
  return operator__;
}
void MasterOperator::set_operator_(::google::protobuf::uint32 value) {
  set_has_operator_();
  operator__ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.operator)
}

// required uint32 sid = 5;
bool MasterOperator::has_sid() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
void MasterOperator::set_has_sid() {
  _has_bits_[0] |= 0x00000020u;
}
void MasterOperator::clear_has_sid() {
  _has_bits_[0] &= ~0x00000020u;
}
void MasterOperator::clear_sid() {
  sid_ = 0u;
  clear_has_sid();
}
::google::protobuf::uint32 MasterOperator::sid() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.sid)
  return sid_;
}
void MasterOperator::set_sid(::google::protobuf::uint32 value) {
  set_has_sid();
  sid_ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.sid)
}

// optional uint64 lastversion = 6;
bool MasterOperator::has_lastversion() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
void MasterOperator::set_has_lastversion() {
  _has_bits_[0] |= 0x00000010u;
}
void MasterOperator::clear_has_lastversion() {
  _has_bits_[0] &= ~0x00000010u;
}
void MasterOperator::clear_lastversion() {
  lastversion_ = GOOGLE_ULONGLONG(0);
  clear_has_lastversion();
}
::google::protobuf::uint64 MasterOperator::lastversion() const {
  // @@protoc_insertion_point(field_get:paxos.MasterOperator.lastversion)
  return lastversion_;
}
void MasterOperator::set_lastversion(::google::protobuf::uint64 value) {
  set_has_lastversion();
  lastversion_ = value;
  // @@protoc_insertion_point(field_set:paxos.MasterOperator.lastversion)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace paxos

// @@protoc_insertion_point(global_scope)