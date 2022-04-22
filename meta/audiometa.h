#ifndef AUDIOMETA_H
#define AUDIOMETA_H

/**
 * Created By WKB in 2022/4/21
 * Meta Information of Audio Use
 */

/**
 * Some Possible Track Types
 */
typedef enum {
    TRK_MONO = 0,
    TRK_STEREO,
    TRK_2_1,
    TRK_5_1,
    TRK_7_1,
    TRK_ATMOS_514,
    TRK_ATMOS_714,
} TrackType;

const int trackCnt[] = {
    1,
    2,
    3,
    6,
    8,
    10,
    12,
};

#endif // AUDIOMETA_H
