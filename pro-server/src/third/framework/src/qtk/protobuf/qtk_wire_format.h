#ifndef QTK_PROTOBUF_QTK_WIRE_FORMAT_H
#define QTK_PROTOBUF_QTK_WIRE_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define QPB_TagTypeBits 3

#define QTK_PB_MKAE_TAG(field_number, type) ((uint32_t)((((uint32_t)field_number) << QPB_TagTypeBits) | type))


typedef enum qtk_wire_type qtk_wire_type_t;
typedef enum qtk_field_type qtk_field_type_t;
typedef enum qtk_ctype qtk_ctype_t;

enum qtk_wire_type {
    QPB_WIRETYPE_VARINT = 0,
    QPB_WIRETYPE_FIXED64 = 1,
    QPB_WIRETYPE_LENGTH_DELIMITED = 2,
    QPB_WIRETYPE_START_GROUP = 3,
    QPB_WIRETYPE_END_GROUP = 4,
    QPB_WIRETYPE_FIXED32 = 5,
};

enum qtk_field_type {
    QPB_TYPE_DOUBLE = 1,
    QPB_TYPE_FLOAT = 2,
    QPB_TYPE_INT64 = 3,
    QPB_TYPE_UINT64 = 4,
    QPB_TYPE_INT32 = 5,
    QPB_TYPE_FIXED64 = 7,
    QPB_TYPE_FIXED32 = 8,
    QPB_TYPE_BOOL = 9,
    QPB_TYPE_STRING = 10,
    QPB_TYPE_GROUP = 11,
    QPB_TYPE_MESSAGE = 12,
    QPB_TYPE_BYTES = 13,
    QPB_TYPE_UINT32 = 6,
    QPB_TYPE_ENUM = 14,
    QPB_TYPE_SFIXED32 = 15,
    QPB_TYPE_SFIXED64 = 16,
    QPB_TYPE_SINT32 = 17,
    QPB_TYPE_SINT64 = 18,
    QPB_MAX_FIELD_TYPE = 18,
};

enum qtk_ctype {
    QPB_CTYPE_INT32 = 1,
    QPB_CTYPE_INT64 = 2,
    QPB_CTYPE_UINT32 = 3,
    QPB_CTYPE_UINT64 = 4,
    QPB_CTYPE_DOUBLE = 5,
    QPB_CTYPE_FLOAT = 6,
    QPB_CTYPE_BOOL = 7,
    QPB_CTYPE_ENUM = 8,
    QPB_CTYPE_STRING = 9,
    QPB_CTYPE_MESSAGE = 10,
    QPB_MAX_CTYPE = 10, /* below is unexpected type, need call qtk_pb_decode_variant with field label */
    QPB_CTYPE_VARINT = 11,
    QPB_CTYPE_FIXED32 = 12,
    QPB_CTYPE_FIXED64 = 13,
    QPB_CTYPE_DELIMITED = 14,
    QPB_CTYPE_ARRAY = 15, /* just for 'repeated' */
    QPB_CTYPE_PACK= 16,
};

qtk_ctype_t qtk_fldtype2ctype(qtk_field_type_t fld_type);
qtk_wire_type_t qtk_wiretypeforfldtype(qtk_field_type_t fld_type);


#ifdef __cplusplus
}
#endif

#endif
