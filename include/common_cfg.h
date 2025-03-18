#ifndef B024A44A_D732_4702_AEEF_B6B69CD438C3
#define B024A44A_D732_4702_AEEF_B6B69CD438C3

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>

uint8_t set_f(void *value, float *val, uint8_t def);
uint8_t set_hhu(void *value, uint8_t *val, uint8_t def);
uint8_t set_u(void *value, uint16_t *val, uint8_t def);
uint8_t set_bit(void *value, uint8_t *val, int bit, uint8_t def);
uint8_t set_c(void *value, char *val, uint8_t def);

#ifdef __cplusplus
}
#endif

#endif /* B024A44A_D732_4702_AEEF_B6B69CD438C3 */
