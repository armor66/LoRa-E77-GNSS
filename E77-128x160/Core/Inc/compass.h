#ifndef COMPASS_HEADER
#define COMPASS_HEADER

void init_compass(void);
void calibrate_compass(void);
uint8_t read_north(void);
uint8_t is_north_ready(void);
uint8_t reset_north(void);
//double get_north(void);

int32_t maxv(int32_t x, int32_t y);
uint32_t absv(int32_t value);
int32_t maxv(int32_t x, int32_t y);
int32_t minv(int32_t x, int32_t y);
int32_t limit_to(int32_t value, int32_t pos_lim, int32_t neg_lim);

#endif /*COMPASS_HEADER*/
