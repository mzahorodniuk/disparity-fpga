#define SAD_WINDOW_SIZE 15

/* NO_OF_DISPARITIES must be greater than '0' and less than the image width */
#define NO_OF_DISPARITIES (16 * 2)

/* NO_OF_DISPARITIES must not be lesser than PARALLEL_UNITS and NO_OF_DISPARITIES/PARALLEL_UNITS must be a
 * non-fractional number */
#define PARALLEL_UNITS 16

#define XF_USE_URAM false

/* Filter window size*/
#define WINDOW_SIZE 3
