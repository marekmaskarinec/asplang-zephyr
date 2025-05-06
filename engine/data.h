/*
 * Asp engine data definitions.
 */

#ifndef ASP_DATA_H
#define ASP_DATA_H

#include "asp.h"
#include "bits.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Data types. */
typedef enum
{
    /* Objects. */
    DataType_None = 0x00,
    DataType_Ellipsis = 0x01,
    DataType_Boolean = 0x02,
    DataType_Integer = 0x03,
    DataType_Float = 0x04,
    /* DataType_Complex = 0x05, placeholder; not supported */
    DataType_Symbol = 0x06,
    DataType_Range = 0x07,
    DataType_String = 0x08,
    DataType_Tuple = 0x09,
    DataType_List = 0x0A,
    DataType_Set = 0x0B,
    DataType_Dictionary = 0x0D,
    DataType_Function = 0x0F,
    DataType_Module = 0x10,
    DataType_ReverseIterator = 0x15,
    DataType_ForwardIterator = 0x16,
    DataType_AppIntegerObject = 0x1A,
    DataType_AppPointerObject = 0x1B,
    DataType_Type = 0x1F,
    DataType_ObjectMask = 0x3F,

    /* Support types. */
    DataType_CodeAddress = 0x40,
    DataType_StackEntry = 0x50,
    DataType_Frame = 0x52,
    DataType_AppFrame = 0x54,
    DataType_Element = 0x62,
    DataType_StringFragment = 0x64,
    DataType_KeyValuePair = 0x66,
    DataType_Namespace = 0x70,
    DataType_SetNode = 0x74,
    DataType_DictionaryNode = 0x78,
    DataType_NamespaceNode = 0x7C,
    DataType_TreeLinksNode = 0x7D,
    DataType_Parameter = 0x80,
    DataType_ParameterList = 0x81,
    DataType_Argument = 0x82,
    DataType_ArgumentList = 0x83,
    DataType_AppIntegerObjectInfo = 0xAA,
    DataType_AppPointerObjectInfo = 0xAB,
    DataType_Free = 0xFF,
} DataType;

/* Determine whether pointers are wider than 32 bits. */
#if defined UINTPTR_MAX && defined UINT32_MAX && UINTPTR_MAX <= UINT32_MAX
#ifdef ASP_WIDE_PTR
#undef ASP_WIDE_PTR
#endif
#else
#define ASP_WIDE_PTR
#endif

/* Data entry. */
union AspDataEntry
{
    bool b;
    struct
    {
        uint8_t c;
        uint8_t s[14];
        uint8_t t;
    } s;
    struct
    {
        union
        {
            struct
            {
                uint32_t u0, u1;
            } u;
            void *p;
            void (*fi)(AspEngine *, int16_t, int32_t);
            void (*fp)(AspEngine *, int16_t, void *);
        } u;
        uint32_t u2;
        union
        {
            uint16_t h6;
            int16_t ih6;
        } h;
    } w;
    int32_t i;
    double d;
};

/* Word constants. */
#define AspWordBitSize 28U
#define AspWordMax ((1U << (AspWordBitSize)) - 1U)
#define AspSignedWordMin (-(1 << (AspWordBitSize) - 1))
#define AspSignedWordMax ((1 << (AspWordBitSize) - 1) - 1)

/* Low-level field access. */
#define AspDataSetWord0(eptr, value) \
    (AspBitSetField(&(eptr)->w.u.u.u0, 0, (AspWordBitSize), (value)))
#define AspDataGetWord0(eptr) \
    (AspBitGetField((eptr)->w.u.u.u0, 0, (AspWordBitSize)))
#define AspDataSetSignedWord0(eptr, value) \
    (AspBitSetSignedField(&(eptr)->w.u.u.u0, 0, (AspWordBitSize), (value)))
#define AspDataGetSignedWord0(eptr) \
    (AspBitGetSignedField((eptr)->w.u.u.u0, 0, (AspWordBitSize)))
#define AspDataSetWord1(eptr, value) \
    (AspBitSetField(&(eptr)->w.u.u.u1, 0, (AspWordBitSize), (value)))
#define AspDataGetWord1(eptr) \
    (AspBitGetField((eptr)->w.u.u.u1, 0, (AspWordBitSize)))
#define AspDataSetSignedWord1(eptr, value) \
    (AspBitSetSignedField(&(eptr)->w.u.u.u1, 0, (AspWordBitSize), (value)))
#define AspDataGetSignedWord1(eptr) \
    (AspBitGetSignedField((eptr)->w.u.u.u1, 0, (AspWordBitSize)))
#define AspDataSetWord2(eptr, value) \
    (AspBitSetField(&(eptr)->w.u2, 0, (AspWordBitSize), (value)))
#define AspDataGetWord2(eptr) \
    (AspBitGetField((eptr)->w.u2, 0, (AspWordBitSize)))
#define AspDataSetSignedWord2(eptr, value) \
    (AspBitSetSignedField(&(eptr)->w.u2, 0, (AspWordBitSize), (value)))
#define AspDataGetSignedWord2(eptr) \
    (AspBitGetSignedField((eptr)->w.u2, 0, (AspWordBitSize)))
void AspDataSetWord3(AspDataEntry *, uint32_t value);
uint32_t AspDataGetWord3(const AspDataEntry *);
void AspDataSetSignedWord3(AspDataEntry *, int32_t value);
int32_t AspDataGetSignedWord3(const AspDataEntry *);
#define AspDataSetBit0(eptr, value) \
    (AspBitSet(&(eptr)->w.u.u.u1, (AspWordBitSize), (value)))
#define AspDataGetBit0(eptr) \
    (AspBitGet((eptr)->w.u.u.u1, (AspWordBitSize)))
#define AspDataSetBit1(eptr, value) \
    (AspBitSet(&(eptr)->w.u.u.u1, (AspWordBitSize) + 1, (value)))
#define AspDataGetBit1(eptr) \
    (AspBitGet((eptr)->w.u.u.u1, (AspWordBitSize) + 1))
#define AspDataSetBit2(eptr, value) \
    (AspBitSet(&(eptr)->w.u.u.u1, (AspWordBitSize) + 2, (value)))
#define AspDataGetBit2(eptr) \
    (AspBitGet((eptr)->w.u.u.u1, (AspWordBitSize) + 2))
#define AspDataSetBit3(eptr, value) \
    (AspBitSet(&(eptr)->w.u.u.u1, (AspWordBitSize) + 3, (value)))
#define AspDataGetBit3(eptr) \
    (AspBitGet((eptr)->w.u.u.u1, (AspWordBitSize) + 3))

/* Common field access. */
#define AspDataSetType(eptr, ty) \
    ((eptr)->s.t = (uint8_t)(ty))
#define AspDataGetType(eptr) \
    ((eptr)->s.t)
#define AspDataSetUseCount(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetUseCount(eptr) \
    (AspDataGetWord2((eptr)))

/* Boolean entry field access. */
#define AspDataSetBoolean(eptr, value) \
    ((eptr)->b = (value))
#define AspDataGetBoolean(eptr) \
    ((eptr)->b)

/* Integer entry field access. */
#define AspDataSetInteger(eptr, value) \
    ((eptr)->i = (value))
#define AspDataGetInteger(eptr) \
    ((eptr)->i)

/* Float entry field access. */
#define AspDataSetFloat(eptr, value) \
    ((eptr)->d = (value))
#define AspDataGetFloat(eptr) \
    ((eptr)->d)

/* Symbol entry field access. */
#define AspDataSetSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))

/* Range entry field access. */
#define AspDataSetRangeHasStart(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetRangeHasStart(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetRangeStartIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetRangeStartIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetRangeHasEnd(eptr, value) \
    (AspDataSetBit1((eptr), (unsigned)(value)))
#define AspDataGetRangeHasEnd(eptr) \
    ((bool)(AspDataGetBit1((eptr))))
#define AspDataSetRangeEndIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetRangeEndIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetRangeHasStep(eptr, value) \
    (AspDataSetBit2((eptr), (unsigned)(value)))
#define AspDataGetRangeHasStep(eptr) \
    ((bool)(AspDataGetBit2((eptr))))
#define AspDataSetRangeStepIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetRangeStepIndex(eptr) \
    (AspDataGetWord3((eptr)))

/* Sequence entry field access for String, Tuple, List, ParameterList, and
   ArgumentList. */
#define AspDataSetSequenceCount(eptr, value) \
    (AspDataSetSignedWord3((eptr), (value)))
#define AspDataGetSequenceCount(eptr) \
    (AspDataGetSignedWord3((eptr)))
#define AspDataSetSequenceHeadIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetSequenceHeadIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetSequenceTailIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetSequenceTailIndex(eptr) \
    (AspDataGetWord1((eptr)))

/* Common tree entry field access for Set, Dictionary, and Namespace. */
#define AspDataSetTreeCount(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetTreeCount(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetTreeRootIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetTreeRootIndex(eptr) \
    (AspDataGetWord1((eptr)))

/* Iterator entry field access. */
#define AspDataSetIteratorIterableIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetIteratorIterableIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetIteratorMemberIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetIteratorMemberIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetIteratorMemberNeedsCleanup(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetIteratorMemberNeedsCleanup(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetIteratorStringIndex(eptr, value) \
    ((eptr)->s.s[11] = (value))
#define AspDataGetIteratorStringIndex(eptr) \
    ((eptr)->s.s[11])
#define AspDataSetIteratorCollectionIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetIteratorCollectionIndex(eptr) \
    (AspDataGetWord3((eptr)))

/* Function entry field access. */
#define AspDataSetFunctionIsApp(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetFunctionIsApp(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetFunctionSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetFunctionSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetFunctionCodeAddress(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetFunctionCodeAddress(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetFunctionModuleIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetFunctionModuleIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetFunctionParametersIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetFunctionParametersIndex(eptr) \
    (AspDataGetWord3((eptr)))

/* Module entry field access. */
#define AspDataSetModuleIsApp(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetModuleIsApp(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetModuleSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetModuleSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetModuleCodeAddress(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetModuleCodeAddress(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetModuleNamespaceIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetModuleNamespaceIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetModuleIsLoaded(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetModuleIsLoaded(eptr) \
    ((bool)(AspDataGetBit0((eptr))))

/* AppIntegerObject and AppPointerObject entry field access. */
#define AspDataSetAppObjectType(eptr, value) \
    ((eptr)->w.h.ih6 = (value))
#define AspDataGetAppObjectType(eptr) \
    ((eptr)->w.h.ih6)
#define AspDataSetAppIntegerObjectValue(eptr, value) \
    (AspDataSetSignedWord1((eptr), (value)))
#define AspDataGetAppIntegerObjectValue(eptr) \
    (AspDataGetSignedWord1((eptr)))
#ifndef ASP_WIDE_PTR
#define AspDataSetAppPointerObjectValue(eptr, value) \
    (*(void **)&(eptr)->w.u.u.u1 = (value))
#define AspDataGetAppPointerObjectValue(eptr) \
    ((void *)(eptr)->w.u.u.u1)
#else
#define AspDataSetAppObjectInfoIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetAppObjectInfoIndex(eptr) \
    (AspDataGetWord3((eptr)))
#define AspDataSetAppPointerObjectValue(eptr, value) \
    ((eptr)->w.u.p = (value))
#define AspDataGetAppPointerObjectValue(eptr) \
    ((eptr)->w.u.p)
#endif
#define AspDataSetAppIntegerObjectDestructor(eptr, value) \
    ((eptr)->w.u.fi = (value))
#define AspDataGetAppIntegerObjectDestructor(eptr) \
    ((eptr)->w.u.fi)
#define AspDataSetAppPointerObjectDestructor(eptr, value) \
    ((eptr)->w.u.fp = (value))
#define AspDataGetAppPointerObjectDestructor(eptr) \
    ((eptr)->w.u.fp)

/* Type entry field access. */
#define AspDataSetTypeValue(eptr, value) \
    ((eptr)->s.c = (value))
#define AspDataGetTypeValue(eptr) \
    ((eptr)->s.c)

/* CodeAddress entry field access. */
#define AspDataSetCodeAddress(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetCodeAddress(eptr) \
    (AspDataGetWord0((eptr)))

/* StackEntry entry field access. */
#define AspDataSetStackEntryPreviousIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetStackEntryPreviousIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetStackEntryValueIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetStackEntryValueIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetStackEntryHasValue2(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetStackEntryHasValue2(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetStackEntryValue2Index(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetStackEntryValue2Index(eptr) \
    (AspDataGetWord2((eptr)))
#define AspDataSetStackEntryFlag(eptr, value) \
    (AspDataSetBit1((eptr), (unsigned)(value)))
#define AspDataGetStackEntryFlag(eptr) \
    ((bool)(AspDataGetBit1((eptr))))

/* Frame entry field access. */
#define AspDataSetFrameReturnAddress(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetFrameReturnAddress(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetFrameModuleIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetFrameModuleIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetFrameLocalNamespaceIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetFrameLocalNamespaceIndex(eptr) \
    (AspDataGetWord2((eptr)))

/* Application function frame entry field access. */
#define AspDataSetAppFrameFunctionIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetAppFrameFunctionIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetAppFrameLocalNamespaceIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetAppFrameLocalNamespaceIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetAppFrameReturnValueDefined(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetAppFrameReturnValueDefined(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetAppFrameReturnValueIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetAppFrameReturnValueIndex(eptr) \
    (AspDataGetWord2((eptr)))

/* Common element entry field access. */
#define AspDataSetElementPreviousIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetElementPreviousIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetElementNextIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetElementNextIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetElementValueIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetElementValueIndex(eptr) \
    (AspDataGetWord2((eptr)))

/* StringFragment entry field access. */
#define AspDataGetStringFragmentMaxSize() \
    ((uint8_t)(offsetof(AspDataEntry, s.t) - offsetof(AspDataEntry, s.s)))
#define AspDataSetStringFragment(eptr, value, count) \
    ((eptr)->s.c = (count), memcpy((eptr)->s.s, (value), (count)))
#define AspDataSetStringFragmentData(eptr, index, value, count) \
    (memcpy((eptr)->s.s + (index), (value), (count)))
#define AspDataSetStringFragmentSize(eptr, count) \
    ((eptr)->s.c = (count))
#define AspDataGetStringFragmentSize(eptr) \
    ((eptr)->s.c)
#define AspDataGetStringFragmentData(eptr) \
    ((char *)(eptr)->s.s)

/* KeyValuePair entry field access. */
#define AspDataSetKeyValuePairKeyIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetKeyValuePairKeyIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetKeyValuePairValueIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetKeyValuePairValueIndex(eptr) \
    (AspDataGetWord1((eptr)))

/* Common tree node entry field access. */
#define AspDataSetTreeNodeKeyIndex(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetTreeNodeKeyIndex(eptr) \
    (AspDataGetWord0((eptr)))
#define AspDataSetTreeNodeParentIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetTreeNodeParentIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetTreeNodeIsBlack(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetTreeNodeIsBlack(eptr) \
    ((bool)(AspDataGetBit0((eptr))))

/* SetNode entry field access. */
#define AspDataSetSetNodeLeftIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetSetNodeLeftIndex(eptr) \
    (AspDataGetWord2((eptr)))
#define AspDataSetSetNodeRightIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetSetNodeRightIndex(eptr) \
    (AspDataGetWord3((eptr)))

/* DictionaryNode and NamespaceNode entry field access. */
#define AspDataSetTreeNodeLinksIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetTreeNodeLinksIndex(eptr) \
    (AspDataGetWord2((eptr)))
#define AspDataSetTreeNodeValueIndex(eptr, value) \
    (AspDataSetWord3((eptr), (value)))
#define AspDataGetTreeNodeValueIndex(eptr) \
    (AspDataGetWord3((eptr)))

/* NamespaceNode entry field access. */
#define AspDataSetNamespaceNodeSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetNamespaceNodeSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetNamespaceNodeIsGlobal(eptr, value) \
    (AspDataSetBit1((eptr), (unsigned)(value)))
#define AspDataGetNamespaceNodeIsGlobal(eptr) \
    ((bool)(AspDataGetBit1((eptr))))
#define AspDataSetNamespaceNodeIsNotLocal(eptr, value) \
    (AspDataSetBit2((eptr), (unsigned)(value)))
#define AspDataGetNamespaceNodeIsNotLocal(eptr) \
    ((bool)(AspDataGetBit2((eptr))))

/* TreeLinksNode entry field access. */
#define AspDataSetTreeLinksNodeLeftIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetTreeLinksNodeLeftIndex(eptr) \
    (AspDataGetWord1((eptr)))
#define AspDataSetTreeLinksNodeRightIndex(eptr, value) \
    (AspDataSetWord2((eptr), (value)))
#define AspDataGetTreeLinksNodeRightIndex(eptr) \
    (AspDataGetWord2((eptr)))

/* Parameter entry field access. */
#define AspDataSetParameterSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetParameterSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetParameterHasDefault(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetParameterHasDefault(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetParameterIsTupleGroup(eptr, value) \
    (AspDataSetBit1((eptr), (unsigned)(value)))
#define AspDataGetParameterIsTupleGroup(eptr) \
    ((bool)(AspDataGetBit1((eptr))))
#define AspDataSetParameterIsDictionaryGroup(eptr, value) \
    (AspDataSetBit2((eptr), (unsigned)(value)))
#define AspDataGetParameterIsDictionaryGroup(eptr) \
    ((bool)(AspDataGetBit2((eptr))))
#define AspDataSetParameterDefaultIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetParameterDefaultIndex(eptr) \
    (AspDataGetWord1((eptr)))

/* Argument entry field access. */
#define AspDataSetArgumentSymbol(eptr, value) \
    (AspDataSetSignedWord0((eptr), (value)))
#define AspDataGetArgumentSymbol(eptr) \
    (AspDataGetSignedWord0((eptr)))
#define AspDataSetArgumentHasName(eptr, value) \
    (AspDataSetBit0((eptr), (unsigned)(value)))
#define AspDataGetArgumentHasName(eptr) \
    ((bool)(AspDataGetBit0((eptr))))
#define AspDataSetArgumentIsIterableGroup(eptr, value) \
    (AspDataSetBit1((eptr), (unsigned)(value)))
#define AspDataGetArgumentIsIterableGroup(eptr) \
    ((bool)(AspDataGetBit1((eptr))))
#define AspDataSetArgumentIsDictionaryGroup(eptr, value) \
    (AspDataSetBit2((eptr), (unsigned)(value)))
#define AspDataGetArgumentIsDictionaryGroup(eptr) \
    ((bool)(AspDataGetBit2((eptr))))
#define AspDataSetArgumentValueIndex(eptr, value) \
    (AspDataSetWord1((eptr), (value)))
#define AspDataGetArgumentValueIndex(eptr) \
    (AspDataGetWord1((eptr)))

/* Free entry field access. */
#define AspDataSetFreeNext(eptr, value) \
    (AspDataSetWord0((eptr), (value)))
#define AspDataGetFreeNext(eptr) \
    (AspDataGetWord0((eptr)))

/* Functions. */
void AspClearData(AspEngine *);
uint32_t AspAlloc(AspEngine *);
bool AspFree(AspEngine *, uint32_t index);
bool AspIsObject(const AspDataEntry *);
AspRunResult AspCheckIsImmutableObject
    (AspEngine *, const AspDataEntry *, bool *isImmutable);
AspDataEntry *AspAllocEntry(AspEngine *, DataType);
AspDataEntry *AspEntry(AspEngine *, uint32_t index);
AspDataEntry *AspValueEntry(AspEngine *, uint32_t index);
uint32_t AspIndex(const AspEngine *, const AspDataEntry *);
AspDataEntry *AspAppObjectInfoEntry(AspEngine *, AspDataEntry *);

#ifdef __cplusplus
}
#endif

#endif
