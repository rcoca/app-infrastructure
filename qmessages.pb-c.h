/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_qmessages_2eproto__INCLUDED
#define PROTOBUF_C_qmessages_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _Qmessage Qmessage;
typedef struct _Tlv Tlv;
typedef struct _DataMessage DataMessage;


/* --- enums --- */

typedef enum _Qmessage__RequestType {
  QMESSAGE__REQUEST_TYPE__CREATE = 0,
  QMESSAGE__REQUEST_TYPE__DESTROY = 1,
  QMESSAGE__REQUEST_TYPE__EXIT = 3,
  QMESSAGE__REQUEST_TYPE__INVALID = 4
} Qmessage__RequestType;
typedef enum _Tlv__Type {
  TLV__TYPE__data = 0,
  TLV__TYPE__stream = 1
} Tlv__Type;

/* --- messages --- */

struct  _Qmessage
{
  ProtobufCMessage base;
  Qmessage__RequestType req;
  protobuf_c_boolean has_thread_main;
  uint64_t thread_main;
  protobuf_c_boolean has_thread_args;
  uint64_t thread_args;
  protobuf_c_boolean has_tid;
  uint64_t tid;
};
#define QMESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&qmessage__descriptor) \
    , 0, 0,0, 0,0, 0,0 }


struct  _Tlv
{
  ProtobufCMessage base;
  Tlv__Type type;
  uint32_t length;
};
#define TLV__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&tlv__descriptor) \
    , 0, 0 }


struct  _DataMessage
{
  ProtobufCMessage base;
  ProtobufCBinaryData data;
};
#define DATA_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&data_message__descriptor) \
    , {0,NULL} }


/* Qmessage methods */
void   qmessage__init
                     (Qmessage         *message);
size_t qmessage__get_packed_size
                     (const Qmessage   *message);
size_t qmessage__pack
                     (const Qmessage   *message,
                      uint8_t             *out);
size_t qmessage__pack_to_buffer
                     (const Qmessage   *message,
                      ProtobufCBuffer     *buffer);
Qmessage *
       qmessage__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qmessage__free_unpacked
                     (Qmessage *message,
                      ProtobufCAllocator *allocator);
/* Tlv methods */
void   tlv__init
                     (Tlv         *message);
size_t tlv__get_packed_size
                     (const Tlv   *message);
size_t tlv__pack
                     (const Tlv   *message,
                      uint8_t             *out);
size_t tlv__pack_to_buffer
                     (const Tlv   *message,
                      ProtobufCBuffer     *buffer);
Tlv *
       tlv__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   tlv__free_unpacked
                     (Tlv *message,
                      ProtobufCAllocator *allocator);
/* DataMessage methods */
void   data_message__init
                     (DataMessage         *message);
size_t data_message__get_packed_size
                     (const DataMessage   *message);
size_t data_message__pack
                     (const DataMessage   *message,
                      uint8_t             *out);
size_t data_message__pack_to_buffer
                     (const DataMessage   *message,
                      ProtobufCBuffer     *buffer);
DataMessage *
       data_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   data_message__free_unpacked
                     (DataMessage *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Qmessage_Closure)
                 (const Qmessage *message,
                  void *closure_data);
typedef void (*Tlv_Closure)
                 (const Tlv *message,
                  void *closure_data);
typedef void (*DataMessage_Closure)
                 (const DataMessage *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor qmessage__descriptor;
extern const ProtobufCEnumDescriptor    qmessage__request_type__descriptor;
extern const ProtobufCMessageDescriptor tlv__descriptor;
extern const ProtobufCEnumDescriptor    tlv__type__descriptor;
extern const ProtobufCMessageDescriptor data_message__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_qmessages_2eproto__INCLUDED */