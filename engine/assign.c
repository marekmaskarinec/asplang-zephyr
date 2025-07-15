/*
 * Asp engine assignment implementation.
 */

#include "assign.h"
#include "stack.h"
#include "sequence.h"

static AspRunResult CheckSequenceMatch
    (AspEngine *, const AspDataEntry *address, const AspDataEntry *newValue);

AspRunResult AspAssignSimple
    (AspEngine *engine,
     AspDataEntry *address, AspDataEntry *newValue)
{
    uint8_t addressType = AspDataGetType(address);
    AspRunResult assertResult = AspAssert
        (engine,
         addressType == DataType_Element ||
         addressType == DataType_DictionaryNode ||
         addressType == DataType_NamespaceNode);
    if (assertResult != AspRunResult_OK)
        return assertResult;

    AspRef(engine, newValue);
    uint32_t newValueIndex = AspIndex(engine, newValue);
    switch (addressType)
    {
        case DataType_Element:
        {
            AspDataEntry *oldValue = AspValueEntry
                (engine, AspDataGetElementValueIndex(address));
            if (AspIsObject(oldValue))
            {
                AspUnref(engine, oldValue);
                if (engine->runResult != AspRunResult_OK)
                    return engine->runResult;
            }
            AspDataSetElementValueIndex(address, newValueIndex);
            break;
        }

        case DataType_DictionaryNode:
        case DataType_NamespaceNode:
        {
            AspDataEntry *oldValue = AspValueEntry
                (engine, AspDataGetTreeNodeValueIndex(address));
            if (AspIsObject(oldValue))
            {
                AspUnref(engine, oldValue);
                if (engine->runResult != AspRunResult_OK)
                    return engine->runResult;
            }
            AspDataSetTreeNodeValueIndex(address, newValueIndex);
            break;
        }
    }

    return AspRunResult_OK;
}

AspRunResult AspAssignSequence
    (AspEngine *engine,
     AspDataEntry *address, AspDataEntry *newValue)
{
    DataType addressType = AspDataGetType(address);
    AspRunResult assertResult = AspAssert
        (engine,
         addressType == DataType_Tuple || addressType == DataType_List);
    if (assertResult != AspRunResult_OK)
        return assertResult;

    AspRunResult checkResult = CheckSequenceMatch
        (engine, address, newValue);
    if (checkResult != AspRunResult_OK)
        return checkResult;

    /* Avoid recursion by using the engine's stack. */
    const AspDataEntry *startStackTop = engine->stackTop;
    uint32_t iterationCount = 0;
    for (bool unrefNewValue = false;
         iterationCount < engine->cycleDetectionLimit;
         iterationCount++, unrefNewValue = true)
    {
        AspSequenceResult newValueIterResult = {AspRunResult_OK, 0, 0};
        uint32_t iterationCount = 0;
        for (AspSequenceResult addressIterResult = AspSequenceNext
                (engine, address, 0, true);
             iterationCount < engine->cycleDetectionLimit &&
             addressIterResult.element != 0;
             iterationCount++,
             addressIterResult = AspSequenceNext
                (engine, address, addressIterResult.element, true))
        {
            AspDataEntry *addressElement = addressIterResult.value;

            newValueIterResult = AspSequenceNext
                (engine, newValue, newValueIterResult.element, true);
            AspDataEntry *newValueElement = newValueIterResult.value;

            DataType addressElementType = AspDataGetType(addressElement);
            if (addressElementType == DataType_Tuple ||
                addressElementType == DataType_List)
            {
                AspRunResult checkResult = CheckSequenceMatch
                    (engine, addressElement, newValueElement);
                if (checkResult != AspRunResult_OK)
                    return checkResult;

                AspDataEntry *stackEntry = AspPush(engine, newValueElement);
                if (stackEntry == 0)
                    return AspRunResult_OutOfDataMemory;
                AspRef(engine, addressElement);
                AspDataSetStackEntryHasValue2(stackEntry, true);
                AspDataSetStackEntryValue2Index
                    (stackEntry, AspIndex(engine, addressElement));
            }
            else
            {
                AspRunResult assignResult = AspAssignSimple
                    (engine, addressElement, newValueElement);
                if (assignResult != AspRunResult_OK)
                    return assignResult;
            }
        }
        if (iterationCount >= engine->cycleDetectionLimit)
            return AspRunResult_CycleDetected;

        /* Make sure not to unreference the top-level value, as the
           type of instruction (SET or SETP) decides how it should be
           handled. */
        AspUnref(engine, address);
        if (unrefNewValue)
            AspUnref(engine, newValue);

        /* Check if there's more to do. */
        if (engine->stackTop == startStackTop ||
            engine->runResult != AspRunResult_OK)
            break;

        address = AspTopValue2(engine);
        assertResult = AspAssert(engine, address != 0);
        if (assertResult != AspRunResult_OK)
            return assertResult;
        newValue = AspTopValue(engine);
        if (newValue == 0)
            return AspRunResult_StackUnderflow;
        AspRef(engine, newValue);
        AspPop(engine);
    }
    if (iterationCount >= engine->cycleDetectionLimit)
        return AspRunResult_CycleDetected;

    return AspRunResult_OK;
}

static AspRunResult CheckSequenceMatch
    (AspEngine *engine,
     const AspDataEntry *address, const AspDataEntry *value)
{
    DataType addressType = AspDataGetType(address);
    AspRunResult assertResult = AspAssert
        (engine,
         addressType == DataType_Tuple || addressType == DataType_List);
    if (assertResult != AspRunResult_OK)
        return assertResult;

    DataType valueType = AspDataGetType(value);
    if (valueType != DataType_Tuple && valueType != DataType_List)
        return AspRunResult_UnexpectedType;
    uint32_t addressCount = AspDataGetSequenceCount(address);
    uint32_t valueCount = AspDataGetSequenceCount(value);
    return
        addressCount != valueCount ?
        AspRunResult_SequenceMismatch : AspRunResult_OK;
}
