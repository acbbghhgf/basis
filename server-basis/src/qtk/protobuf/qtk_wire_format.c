#include "qtk_wire_format.h"

const static qtk_wire_type_t kWireTypeForFieldType[QPB_MAX_FIELD_TYPE + 1] = {
    -1,
    QPB_WIRETYPE_FIXED64,
    QPB_WIRETYPE_FIXED32,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_FIXED64,
    QPB_WIRETYPE_FIXED32,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_LENGTH_DELIMITED,
    QPB_WIRETYPE_START_GROUP,
    QPB_WIRETYPE_LENGTH_DELIMITED,
    QPB_WIRETYPE_LENGTH_DELIMITED,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_FIXED32,
    QPB_WIRETYPE_FIXED64,
    QPB_WIRETYPE_VARINT,
    QPB_WIRETYPE_VARINT,
};


const static qtk_ctype_t kFieldTypeToCTypeMap[QPB_MAX_FIELD_TYPE + 1] = {
    0,
    QPB_CTYPE_DOUBLE,
    QPB_CTYPE_FLOAT,
    QPB_CTYPE_INT64,
    QPB_CTYPE_UINT64,
    QPB_CTYPE_INT32,
    QPB_CTYPE_UINT64,
    QPB_CTYPE_UINT32,
    QPB_CTYPE_BOOL,
    QPB_CTYPE_STRING,
    QPB_CTYPE_MESSAGE,
    QPB_CTYPE_MESSAGE,
    QPB_CTYPE_STRING,
    QPB_CTYPE_UINT32,
    QPB_CTYPE_ENUM,
    QPB_CTYPE_INT32,
    QPB_CTYPE_INT64,
    QPB_CTYPE_INT32,
    QPB_CTYPE_INT64,
};


qtk_ctype_t qtk_fldtype2ctype(qtk_field_type_t fld_type) {
    return kFieldTypeToCTypeMap[fld_type];
}


qtk_wire_type_t qtk_wiretypeforfldtype(qtk_field_type_t fld_type) {
    return kWireTypeForFieldType[fld_type];
}
