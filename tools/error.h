#ifndef ERROR_H
#define ERROR_H

#define ForeGround_Black     "\033[30m"
#define ForeGround_Red       "\033[31m"
#define ForeGround_Green     "\033[32m"
#define ForeGround_Yellow    "\033[33m"
#define ForeGround_Blue      "\033[34m"
#define ForeGround_Magenta   "\033[35m"
#define ForeGround_Cyan      "\033[36m"
#define ForeGround_White     "\033[37m"
#define ForeGround_Reset     "\033[0m"

#define BackGround_Black     "\033[40m"
#define BackGround_Red       "\033[40m"
#define BackGround_Green     "\033[40m"
#define BackGround_Yellow    "\033[40m"
#define BackGround_Blue      "\033[40m"
#define BackGround_Magenta   "\033[40m"
#define BackGround_Cyan      "\033[40m"
#define BackGround_White     "\033[40m"
#define BackGround_Reset     "\033[49m"


#define ERROR   ForeGround_Red    "[ERROR] "   ForeGround_Reset
#define INFO    ForeGround_Yellow "[INFO] "    ForeGround_Reset
#define ASRT    ForeGround_Cyan   "[ASSERT] "  ForeGround_Reset
#define TD      BackGround_Red ForeGround_White "[TODO]" ForeGround_Reset BackGround_Reset " "

#define THROW_ERROR(...) \
    do { \
        fprintf(stderr, ERROR); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(EXIT_FAILURE); \
    } while(0)

#define LOG_INFO(...) \
    do { \
        printf(INFO __VA_ARGS__); \
        printf("\n"); \
    } while(0)

#define ASSERT(condition, ...) \
    do { \
        if (condition) { break; } \
        fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, ASRT); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(EXIT_FAILURE); \
    } while(0)

#define TODO(...) \
    do { \
        fprintf(stdout, "%s:%d: ", __FILE__, __LINE__); \
        fprintf(stdout, TD); \
        fprintf(stdout, __VA_ARGS__); \
        fprintf(stdout, "\n"); \
        exit(EXIT_FAILURE); \
    } while(0)

#endif // ERROR_H