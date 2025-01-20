/*
 * Asp script function library implementation: iterator functions.
 */

#include "asp.h"
#include "iterator.h"

/* iter(iterable)
 * Return an iterator for the given iterable.
 */
ASP_LIB_API AspRunResult AspLib_iter
    (AspEngine *engine,
     AspDataEntry *iterable,
     AspDataEntry **returnValue)
{
    AspIteratorResult result = AspIteratorCreate(engine, iterable, false);
    if (result.result == AspRunResult_OK)
        *returnValue = result.value;
    return result.result;
}

/* reversed(iterator)
 * Return a reversed iterator for the given iterable.
 */
ASP_LIB_API AspRunResult AspLib_reversed
    (AspEngine *engine,
     AspDataEntry *iterable,
     AspDataEntry **returnValue)
{
    AspIteratorResult result = AspIteratorCreate(engine, iterable, true);
    if (result.result == AspRunResult_OK)
        *returnValue = result.value;
    return result.result;
}

/* at(iterator, end = None)
 * Return the current item at the iterator, returning the given end value
   if the iterator is at its end.
 */
ASP_LIB_API AspRunResult AspLib_at
    (AspEngine *engine,
     AspDataEntry *iterator, AspDataEntry *end,
     AspDataEntry **returnValue)
{
    /* Dereference the iterator. */
    AspIteratorResult dereferenceResult = AspIteratorDereference
        (engine, iterator);
    if (dereferenceResult.result == AspRunResult_IteratorAtEnd)
    {
        AspRef(engine, end);
        *returnValue = end;
        return AspRunResult_OK;
    }
    else if (dereferenceResult.result != AspRunResult_OK)
        return dereferenceResult.result;

    /* Store the dereferenced value. */
    *returnValue = dereferenceResult.value;

    return AspRunResult_OK;
}

/* next(iterator, end = None)
 * Return the next item from the iterator, returning the given end value
   if the iterator has reached its end.
 */
ASP_LIB_API AspRunResult AspLib_next
    (AspEngine *engine,
     AspDataEntry *iterator, AspDataEntry *end,
     AspDataEntry **returnValue)
{
    /* Dereference the iterator before advancing it. */
    AspIteratorResult dereferenceResult = AspIteratorDereference
        (engine, iterator);
    if (dereferenceResult.result == AspRunResult_IteratorAtEnd)
    {
        AspRef(engine, end);
        *returnValue = end;
        return AspRunResult_OK;
    }
    else if (dereferenceResult.result != AspRunResult_OK)
        return dereferenceResult.result;

    /* Store the dereferenced value. */
    *returnValue = dereferenceResult.value;

    /* Advance the iterator. */
    return AspIteratorNext(engine, iterator);
}

/* del_at(iterator)
 * Delete the item at the iterator, returning the removed item.
 */
ASP_LIB_API AspRunResult AspLib_del_at
    (AspEngine *engine,
     AspDataEntry *iterator,
     AspDataEntry **returnValue)
{
    /* Dereference the iterator. */
    AspIteratorResult dereferenceResult = AspIteratorDereference
        (engine, iterator);
    if (dereferenceResult.result != AspRunResult_OK)
        return dereferenceResult.result;

    /* Store the dereferenced value. */
    *returnValue = dereferenceResult.value;

    /* Remove the item. */
    return AspIteratorErase(engine, iterator);
}

/* iterable(iterator)
 * Return the iterable for the given iterator, or None if the parameter is not
 * an iterator.
 */
ASP_LIB_API AspRunResult AspLib_iterable
    (AspEngine *engine,
     AspDataEntry *iterator,
     AspDataEntry **returnValue)
{
    AspDataEntry *iterable = AspIterable(engine, iterator);
    if (iterable != 0)
        *returnValue = iterable;
    return AspRunResult_OK;
}
