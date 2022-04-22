#ifndef AUDIOERROR_H
#define AUDIOERROR_H

/**
 * Basic Audio Error return values
 */
typedef enum {
    AE_OK,
} AudioError;

/**
 *
 */
typedef enum {
    ADE_OK,
    ADE_Cannot_Connect,
    ADE_Not_Supported,
    ADE_Operation_Failed,
} AudioDriverError;

#endif // AUDIOERROR_H
