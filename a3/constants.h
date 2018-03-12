// Constants.h

#if !defined(CONSTANTS_H)
#define CONSTANTS_H 1

// constatns that encode different message types
const int TERMINATE = 0;
const int LOC_REQUEST = 1;
const int LOC_SUCCESS= 2;
const int LOC_FAILURE = 3;
const int EXECUTE = 4;
const int EXECUTE_SUCCESS = 5;
const int EXECUTE_FAILURE = 6;
const int REGISTER = 7;
const int CACHE_LOC_REQUEST = 8;

// constatns that encode different data types
const int INTEGER = 0;
const int INT_ARRAY = 1;
const int STRING = 2;
const int ARG_ARRAY = 3;

#endif
