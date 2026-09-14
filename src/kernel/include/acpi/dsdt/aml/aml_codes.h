/*
 * File: aml_codes.h
 * File Created: 11 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <binary.h>

/*
https://github.com/open-acpica/acpica/blob/master/source/include/amlcode.h
*/

#define AML_ZERO_OP                 (uint16_t)0x00
#define AML_ONE_OP                  (uint16_t)0x01
#define AML_ALIAS_OP                (uint16_t)0x06
#define AML_NAME_OP                 (uint16_t)0x08
#define AML_BYTE_OP                 (uint16_t)0x0A
#define AML_WORD_OP                 (uint16_t)0x0B
#define AML_DWORD_OP                (uint16_t)0x0C
#define AML_STRING_OP               (uint16_t)0x0D
#define AML_QWORD_OP                (uint16_t)0x0E /* ACPI 2.0 */
#define AML_SCOPE_OP                (uint16_t)0x10
#define AML_BUFFER_OP               (uint16_t)0x11
#define AML_PACKAGE_OP              (uint16_t)0x12
#define AML_VARIABLE_PACKAGE_OP     (uint16_t)0x13 /* ACPI 2.0 */
#define AML_METHOD_OP               (uint16_t)0x14
#define AML_EXTERNAL_OP             (uint16_t)0x15 /* ACPI 6.0 */
#define AML_DUAL_NAME_PREFIX        (uint16_t)0x2E
#define AML_MULTI_NAME_PREFIX       (uint16_t)0x2F
#define AML_EXTENDED_PREFIX         (uint16_t)0x5B
#define AML_ROOT_PREFIX             (uint16_t)0x5C
#define AML_PARENT_PREFIX           (uint16_t)0x5E
#define AML_FIRST_LOCAL_OP          (uint16_t)0x60 /* Used for Local op # calculations */
#define AML_LOCAL0                  (uint16_t)0x60
#define AML_LOCAL1                  (uint16_t)0x61
#define AML_LOCAL2                  (uint16_t)0x62
#define AML_LOCAL3                  (uint16_t)0x63
#define AML_LOCAL4                  (uint16_t)0x64
#define AML_LOCAL5                  (uint16_t)0x65
#define AML_LOCAL6                  (uint16_t)0x66
#define AML_LOCAL7                  (uint16_t)0x67
#define AML_FIRST_ARG_OP            (uint16_t)0x68 /* Used for Arg op # calculations */
#define AML_ARG0                    (uint16_t)0x68
#define AML_ARG1                    (uint16_t)0x69
#define AML_ARG2                    (uint16_t)0x6A
#define AML_ARG3                    (uint16_t)0x6B
#define AML_ARG4                    (uint16_t)0x6C
#define AML_ARG5                    (uint16_t)0x6D
#define AML_ARG6                    (uint16_t)0x6E
#define AML_STORE_OP                (uint16_t)0x70
#define AML_REF_OF_OP               (uint16_t)0x71
#define AML_ADD_OP                  (uint16_t)0x72
#define AML_CONCATENATE_OP          (uint16_t)0x73
#define AML_SUBTRACT_OP             (uint16_t)0x74
#define AML_INCREMENT_OP            (uint16_t)0x75
#define AML_DECREMENT_OP            (uint16_t)0x76
#define AML_MULTIPLY_OP             (uint16_t)0x77
#define AML_DIVIDE_OP               (uint16_t)0x78
#define AML_SHIFT_LEFT_OP           (uint16_t)0x79
#define AML_SHIFT_RIGHT_OP          (uint16_t)0x7A
#define AML_BIT_AND_OP              (uint16_t)0x7B
#define AML_BIT_NAND_OP             (uint16_t)0x7C
#define AML_BIT_OR_OP               (uint16_t)0x7D
#define AML_BIT_NOR_OP              (uint16_t)0x7E
#define AML_BIT_XOR_OP              (uint16_t)0x7F
#define AML_BIT_NOT_OP              (uint16_t)0x80
#define AML_FIND_SET_LEFT_BIT_OP    (uint16_t)0x81
#define AML_FIND_SET_RIGHT_BIT_OP   (uint16_t)0x82
#define AML_DEREF_OF_OP             (uint16_t)0x83
#define AML_CONCATENATE_TEMPLATE_OP (uint16_t)0x84 /* ACPI 2.0 */
#define AML_MOD_OP                  (uint16_t)0x85 /* ACPI 2.0 */
#define AML_NOTIFY_OP               (uint16_t)0x86
#define AML_SIZE_OF_OP              (uint16_t)0x87
#define AML_INDEX_OP                (uint16_t)0x88
#define AML_MATCH_OP                (uint16_t)0x89
#define AML_CREATE_DWORD_FIELD_OP   (uint16_t)0x8A
#define AML_CREATE_WORD_FIELD_OP    (uint16_t)0x8B
#define AML_CREATE_BYTE_FIELD_OP    (uint16_t)0x8C
#define AML_CREATE_BIT_FIELD_OP     (uint16_t)0x8D
#define AML_OBJECT_TYPE_OP          (uint16_t)0x8E
#define AML_CREATE_QWORD_FIELD_OP   (uint16_t)0x8F /* ACPI 2.0 */
#define AML_LOGICAL_AND_OP          (uint16_t)0x90
#define AML_LOGICAL_OR_OP           (uint16_t)0x91
#define AML_LOGICAL_NOT_OP          (uint16_t)0x92
#define AML_LOGICAL_EQUAL_OP        (uint16_t)0x93
#define AML_LOGICAL_GREATER_OP      (uint16_t)0x94
#define AML_LOGICAL_LESS_OP         (uint16_t)0x95
#define AML_TO_BUFFER_OP            (uint16_t)0x96 /* ACPI 2.0 */
#define AML_TO_DECIMAL_STRING_OP    (uint16_t)0x97 /* ACPI 2.0 */
#define AML_TO_HEX_STRING_OP        (uint16_t)0x98 /* ACPI 2.0 */
#define AML_TO_INTEGER_OP           (uint16_t)0x99 /* ACPI 2.0 */
#define AML_TO_STRING_OP            (uint16_t)0x9C /* ACPI 2.0 */
#define AML_COPY_OBJECT_OP          (uint16_t)0x9D /* ACPI 2.0 */
#define AML_MID_OP                  (uint16_t)0x9E /* ACPI 2.0 */
#define AML_CONTINUE_OP             (uint16_t)0x9F /* ACPI 2.0 */
#define AML_IF_OP                   (uint16_t)0xA0
#define AML_ELSE_OP                 (uint16_t)0xA1
#define AML_WHILE_OP                (uint16_t)0xA2
#define AML_NOOP_OP                 (uint16_t)0xA3
#define AML_RETURN_OP               (uint16_t)0xA4
#define AML_BREAK_OP                (uint16_t)0xA5
#define AML_COMMENT_OP              (uint16_t)0xA9
#define AML_BREAKPOINT_OP           (uint16_t)0xCC
#define AML_ONES_OP                 (uint16_t)0xFF

/*
 * Combination opcodes (actually two one-byte opcodes)
 * Used by the disassembler and iASL compiler
 */

#define AML_LOGICAL_GREATER_EQUAL_OP (uint16_t)0x9295 /* LNot (LLess) */
#define AML_LOGICAL_LESS_EQUAL_OP    (uint16_t)0x9294 /* LNot (LGreater) */
#define AML_LOGICAL_NOT_EQUAL_OP     (uint16_t)0x9293 /* LNot (LEqual) */

/* Prefixed (2-byte) opcodes (with AML_EXTENDED_PREFIX) */

#define AML_EXTENDED_OPCODE       (uint16_t)0x5B00 /* Prefix for 2-byte opcodes */

#define AML_MUTEX_OP              (uint16_t)0x5B01
#define AML_EVENT_OP              (uint16_t)0x5B02
#define AML_SHIFT_RIGHT_BIT_OP    (uint16_t)0x5B10 /* Obsolete, not in ACPI spec */
#define AML_SHIFT_LEFT_BIT_OP     (uint16_t)0x5B11 /* Obsolete, not in ACPI spec */
#define AML_CONDITIONAL_REF_OF_OP (uint16_t)0x5B12
#define AML_CREATE_FIELD_OP       (uint16_t)0x5B13
#define AML_LOAD_TABLE_OP         (uint16_t)0x5B1F /* ACPI 2.0 */
#define AML_LOAD_OP               (uint16_t)0x5B20
#define AML_STALL_OP              (uint16_t)0x5B21
#define AML_SLEEP_OP              (uint16_t)0x5B22
#define AML_ACQUIRE_OP            (uint16_t)0x5B23
#define AML_SIGNAL_OP             (uint16_t)0x5B24
#define AML_WAIT_OP               (uint16_t)0x5B25
#define AML_RESET_OP              (uint16_t)0x5B26
#define AML_RELEASE_OP            (uint16_t)0x5B27
#define AML_FROM_BCD_OP           (uint16_t)0x5B28
#define AML_TO_BCD_OP             (uint16_t)0x5B29
#define AML_UNLOAD_OP             (uint16_t)0x5B2A
#define AML_REVISION_OP           (uint16_t)0x5B30
#define AML_DEBUG_OP              (uint16_t)0x5B31
#define AML_FATAL_OP              (uint16_t)0x5B32
#define AML_TIMER_OP              (uint16_t)0x5B33 /* ACPI 3.0 */
#define AML_REGION_OP             (uint16_t)0x5B80
#define AML_FIELD_OP              (uint16_t)0x5B81
#define AML_DEVICE_OP             (uint16_t)0x5B82
#define AML_PROCESSOR_OP          (uint16_t)0x5B83
#define AML_POWER_RESOURCE_OP     (uint16_t)0x5B84
#define AML_THERMAL_ZONE_OP       (uint16_t)0x5B85
#define AML_INDEX_FIELD_OP        (uint16_t)0x5B86
#define AML_BANK_FIELD_OP         (uint16_t)0x5B87
#define AML_DATA_REGION_OP        (uint16_t)0x5B88 /* ACPI 2.0 */

/*
 * Opcodes for "Field" operators
 */
#define AML_FIELD_OFFSET_OP         (uint8_t) 0x00
#define AML_FIELD_ACCESS_OP         (uint8_t) 0x01
#define AML_FIELD_CONNECTION_OP     (uint8_t) 0x02        /* ACPI 5.0 */
#define AML_FIELD_EXT_ACCESS_OP     (uint8_t) 0x03        /* ACPI 5.0 */


/*
 * Internal opcodes
 * Use only "Unknown" AML opcodes, don't attempt to use
 * any valid ACPI ASCII values (A-Z, 0-9, '-')
 */
#define AML_INT_NAMEPATH_OP         (uint16_t) 0x002d
#define AML_INT_NAMEDFIELD_OP       (uint16_t) 0x0030
#define AML_INT_RESERVEDFIELD_OP    (uint16_t) 0x0031
#define AML_INT_ACCESSFIELD_OP      (uint16_t) 0x0032
#define AML_INT_BYTELIST_OP         (uint16_t) 0x0033
#define AML_INT_METHODCALL_OP       (uint16_t) 0x0035
#define AML_INT_RETURN_VALUE_OP     (uint16_t) 0x0036
#define AML_INT_EVAL_SUBTREE_OP     (uint16_t) 0x0037
#define AML_INT_CONNECTION_OP       (uint16_t) 0x0038
#define AML_INT_EXTACCESSFIELD_OP   (uint16_t) 0x0039

#define ARG_NONE                    0x0

/*
 * Argument types for the AML Parser
 * Each field in the ArgTypes UINT32 is 5 bits, allowing for a maximum of 6 arguments.
 * There can be up to 31 unique argument types
 * Zero is reserved as end-of-list indicator
 */
#define ARGP_BYTEDATA               0x01
#define ARGP_BYTELIST               0x02
#define ARGP_CHARLIST               0x03
#define ARGP_DATAOBJ                0x04
#define ARGP_DATAOBJLIST            0x05
#define ARGP_DWORDDATA              0x06
#define ARGP_FIELDLIST              0x07
#define ARGP_NAME                   0x08
#define ARGP_NAMESTRING             0x09
#define ARGP_OBJLIST                0x0A
#define ARGP_PKGLENGTH              0x0B
#define ARGP_SUPERNAME              0x0C
#define ARGP_TARGET                 0x0D
#define ARGP_TERMARG                0x0E
#define ARGP_TERMLIST               0x0F
#define ARGP_WORDDATA               0x10
#define ARGP_QWORDDATA              0x11
#define ARGP_SIMPLENAME             0x12 /* NameString | LocalTerm | ArgTerm */
#define ARGP_NAME_OR_REF            0x13 /* For ObjectType only */
#define ARGP_MAX                    0x13
#define ARGP_COMMENT                0x14

/*
 * Resolved argument types for the AML Interpreter
 * Each field in the ArgTypes UINT32 is 5 bits, allowing for a maximum of 6 arguments.
 * There can be up to 31 unique argument types (0 is end-of-arg-list indicator)
 *
 * Note1: These values are completely independent from the ACPI_TYPEs
 *        i.e., ARGI_INTEGER != ACPI_TYPE_INTEGER
 *
 * Note2: If and when 5 bits becomes insufficient, it would probably be best
 * to convert to a 6-byte array of argument types, allowing 8 bits per argument.
 */

/* Single, simple types */

#define ARGI_ANYTYPE                0x01    /* Don't care */
#define ARGI_PACKAGE                0x02
#define ARGI_EVENT                  0x03
#define ARGI_MUTEX                  0x04
#define ARGI_DDBHANDLE              0x05

/* Interchangeable types (via implicit conversion) */

#define ARGI_INTEGER                0x06
#define ARGI_STRING                 0x07
#define ARGI_BUFFER                 0x08
#define ARGI_BUFFER_OR_STRING       0x09    /* Used by MID op only */
#define ARGI_COMPUTEDATA            0x0A    /* Buffer, String, or Integer */

/* Reference objects */

#define ARGI_INTEGER_REF            0x0B
#define ARGI_OBJECT_REF             0x0C
#define ARGI_DEVICE_REF             0x0D
#define ARGI_REFERENCE              0x0E
#define ARGI_TARGETREF              0x0F    /* Target, subject to implicit conversion */
#define ARGI_FIXED_TARGET           0x10    /* Target, no implicit conversion */
#define ARGI_SIMPLE_TARGET          0x11    /* Name, Local, Arg -- no implicit conversion */
#define ARGI_STORE_TARGET           0x12    /* Target for store is TARGETREF + package objects */

/* Multiple/complex types */

#define ARGI_DATAOBJECT             0x13    /* Buffer, String, package or reference to a Node - Used only by SizeOf operator*/
#define ARGI_COMPLEXOBJ             0x14    /* Buffer, String, or package (Used by INDEX op only) */
#define ARGI_REF_OR_STRING          0x15    /* Reference or String (Used by DEREFOF op only) */
#define ARGI_REGION_OR_BUFFER       0x16    /* Used by LOAD op only */
#define ARGI_DATAREFOBJ             0x17

/* Note: types above can expand to 0x1F maximum */

#define ARGI_INVALID_OPCODE         0xFFFFFFFF


/*
 * Some of the flags and types below are of the form:
 *
 * AML_FLAGS_EXEC_#A_#T,#R, or
 * AML_TYPE_EXEC_#A_#T,#R where:
 *
 *      #A is the number of required arguments
 *      #T is the number of target operands
 *      #R indicates whether there is a return value
 *
 * These types are used for the top-level dispatch of the AML
 * opcode. They group similar operators that can share common
 * front-end code before dispatch to the final code that implements
 * the operator.
 */

/*
 * Opcode information flags
 */
#define AML_LOGICAL                 BIT(0)
#define AML_LOGICAL_NUMERIC         BIT(1)
#define AML_MATH                    BIT(2)
#define AML_CREATE                  BIT(3)
#define AML_FIELD                   BIT(4)
#define AML_DEFER                   BIT(5)
#define AML_NAMED                   BIT(6)
#define AML_NSNODE                  BIT(7)
#define AML_NSOPCODE                BIT(8)
#define AML_NSOBJECT                BIT(9)
#define AML_HAS_RETVAL              BIT(10)
#define AML_HAS_TARGET              BIT(11)
#define AML_HAS_ARGS                BIT(12)
#define AML_CONSTANT                BIT(13)
#define AML_NO_OPERAND_RESOLVE      BIT(14)

/* Convenient flag groupings of the flags above */

#define AML_FLAGS_EXEC_0A_0T_1R                                     AML_HAS_RETVAL
#define AML_FLAGS_EXEC_1A_0T_0R     AML_HAS_ARGS                                   /* Monadic1  */
#define AML_FLAGS_EXEC_1A_0T_1R     AML_HAS_ARGS |                  AML_HAS_RETVAL /* Monadic2  */
#define AML_FLAGS_EXEC_1A_1T_0R     AML_HAS_ARGS | AML_HAS_TARGET
#define AML_FLAGS_EXEC_1A_1T_1R     AML_HAS_ARGS | AML_HAS_TARGET | AML_HAS_RETVAL /* Monadic2R */
#define AML_FLAGS_EXEC_2A_0T_0R     AML_HAS_ARGS                                   /* Dyadic1   */
#define AML_FLAGS_EXEC_2A_0T_1R     AML_HAS_ARGS |                  AML_HAS_RETVAL /* Dyadic2   */
#define AML_FLAGS_EXEC_2A_1T_1R     AML_HAS_ARGS | AML_HAS_TARGET | AML_HAS_RETVAL /* Dyadic2R  */
#define AML_FLAGS_EXEC_2A_2T_1R     AML_HAS_ARGS | AML_HAS_TARGET | AML_HAS_RETVAL
#define AML_FLAGS_EXEC_3A_0T_0R     AML_HAS_ARGS
#define AML_FLAGS_EXEC_3A_1T_1R     AML_HAS_ARGS | AML_HAS_TARGET | AML_HAS_RETVAL
#define AML_FLAGS_EXEC_6A_0T_1R     AML_HAS_ARGS |                  AML_HAS_RETVAL


/*
 * The opcode Type is used in a dispatch table, do not change
 * or add anything new without updating the table.
 */
#define AML_TYPE_EXEC_0A_0T_1R      0x00 /* 0 Args, 0 Target, 1 RetVal */
#define AML_TYPE_EXEC_1A_0T_0R      0x01 /* 1 Args, 0 Target, 0 RetVal */
#define AML_TYPE_EXEC_1A_0T_1R      0x02 /* 1 Args, 0 Target, 1 RetVal */
#define AML_TYPE_EXEC_1A_1T_0R      0x03 /* 1 Args, 1 Target, 0 RetVal */
#define AML_TYPE_EXEC_1A_1T_1R      0x04 /* 1 Args, 1 Target, 1 RetVal */
#define AML_TYPE_EXEC_2A_0T_0R      0x05 /* 2 Args, 0 Target, 0 RetVal */
#define AML_TYPE_EXEC_2A_0T_1R      0x06 /* 2 Args, 0 Target, 1 RetVal */
#define AML_TYPE_EXEC_2A_1T_1R      0x07 /* 2 Args, 1 Target, 1 RetVal */
#define AML_TYPE_EXEC_2A_2T_1R      0x08 /* 2 Args, 2 Target, 1 RetVal */
#define AML_TYPE_EXEC_3A_0T_0R      0x09 /* 3 Args, 0 Target, 0 RetVal */
#define AML_TYPE_EXEC_3A_1T_1R      0x0A /* 3 Args, 1 Target, 1 RetVal */
#define AML_TYPE_EXEC_6A_0T_1R      0x0B /* 6 Args, 0 Target, 1 RetVal */
/* End of types used in dispatch table */

#define AML_TYPE_LITERAL            0x0C
#define AML_TYPE_CONSTANT           0x0D
#define AML_TYPE_METHOD_ARGUMENT    0x0E
#define AML_TYPE_LOCAL_VARIABLE     0x0F
#define AML_TYPE_DATA_TERM          0x10

/* Generic for an op that returns a value */

#define AML_TYPE_METHOD_CALL        0x11

/* Miscellaneous types */

#define AML_TYPE_CREATE_FIELD       0x12
#define AML_TYPE_CREATE_OBJECT      0x13
#define AML_TYPE_CONTROL            0x14
#define AML_TYPE_NAMED_NO_OBJ       0x15
#define AML_TYPE_NAMED_FIELD        0x16
#define AML_TYPE_NAMED_SIMPLE       0x17
#define AML_TYPE_NAMED_COMPLEX      0x18
#define AML_TYPE_RETURN             0x19
#define AML_TYPE_UNDEFINED          0x1A
#define AML_TYPE_BOGUS              0x1B

/* AML Package Length encodings */

#define ACPI_AML_PACKAGE_TYPE1      0x40
#define ACPI_AML_PACKAGE_TYPE2      0x4000
#define ACPI_AML_PACKAGE_TYPE3      0x400000
#define ACPI_AML_PACKAGE_TYPE4      0x40000000

/*
 * Opcode classes
 */
#define AML_CLASS_EXECUTE           0x00
#define AML_CLASS_CREATE            0x01
#define AML_CLASS_ARGUMENT          0x02
#define AML_CLASS_NAMED_OBJECT      0x03
#define AML_CLASS_CONTROL           0x04
#define AML_CLASS_ASCII             0x05
#define AML_CLASS_PREFIX            0x06
#define AML_CLASS_INTERNAL          0x07
#define AML_CLASS_RETURN_VALUE      0x08
#define AML_CLASS_METHOD_CALL       0x09
#define AML_CLASS_UNKNOWN           0x0A


/* Comparison operation codes for MatchOp operator */

typedef enum
{
    MATCH_MTR                       = 0,
    MATCH_MEQ                       = 1,
    MATCH_MLE                       = 2,
    MATCH_MLT                       = 3,
    MATCH_MGE                       = 4,
    MATCH_MGT                       = 5

} AML_MATCH_OPERATOR;

#define MAX_MATCH_OPERATOR          5


/*
 * FieldFlags
 *
 * This byte is extracted from the AML and includes three separate
 * pieces of information about the field:
 * 1) The field access type
 * 2) The field update rule
 * 3) The lock rule for the field
 *
 * Bits 00 - 03 : AccessType (AnyAcc, ByteAcc, etc.)
 *      04      : LockRule (1 == Lock)
 *      05 - 06 : UpdateRule
 */
#define AML_FIELD_ACCESS_TYPE_MASK  0x0F
#define AML_FIELD_LOCK_RULE_MASK    0x10
#define AML_FIELD_UPDATE_RULE_MASK  0x60


/* 1) Field Access Types */

typedef enum
{
    AML_FIELD_ACCESS_ANY            = 0x00,
    AML_FIELD_ACCESS_BYTE           = 0x01,
    AML_FIELD_ACCESS_WORD           = 0x02,
    AML_FIELD_ACCESS_DWORD          = 0x03,
    AML_FIELD_ACCESS_QWORD          = 0x04,    /* ACPI 2.0 */
    AML_FIELD_ACCESS_BUFFER         = 0x05     /* ACPI 2.0 */

} AML_ACCESS_TYPE;


/* 2) Field Lock Rules */

typedef enum
{
    AML_FIELD_LOCK_NEVER            = 0x00,
    AML_FIELD_LOCK_ALWAYS           = 0x10

} AML_LOCK_RULE;


/* 3) Field Update Rules */

typedef enum
{
    AML_FIELD_UPDATE_PRESERVE       = 0x00,
    AML_FIELD_UPDATE_WRITE_AS_ONES  = 0x20,
    AML_FIELD_UPDATE_WRITE_AS_ZEROS = 0x40

} AML_UPDATE_RULE;


/*
 * Field Access Attributes.
 * This byte is extracted from the AML via the
 * AccessAs keyword
 */
typedef enum
{
    AML_FIELD_ATTRIB_QUICK              = 0x02,
    AML_FIELD_ATTRIB_SEND_RECEIVE       = 0x04,
    AML_FIELD_ATTRIB_BYTE               = 0x06,
    AML_FIELD_ATTRIB_WORD               = 0x08,
    AML_FIELD_ATTRIB_BLOCK              = 0x0A,
    AML_FIELD_ATTRIB_BYTES              = 0x0B,
    AML_FIELD_ATTRIB_PROCESS_CALL       = 0x0C,
    AML_FIELD_ATTRIB_BLOCK_PROCESS_CALL = 0x0D,
    AML_FIELD_ATTRIB_RAW_BYTES          = 0x0E,
    AML_FIELD_ATTRIB_RAW_PROCESS_BYTES  = 0x0F

} AML_ACCESS_ATTRIBUTE;


/* Bit fields in the AML MethodFlags byte */

#define AML_METHOD_ARG_COUNT        0x07
#define AML_METHOD_SERIALIZED       0x08
#define AML_METHOD_SYNC_LEVEL       0xF0
