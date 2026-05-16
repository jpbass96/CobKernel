//TODO: Add GPL header here for code lifted from linux kernel
#include "bcm2712_temp.h"
#include "util.h"
#include "base.h"

u32 avs_read32(size_t offset) {
    return read32(AVS_TEMP_BASE + offset);
}

//Code taken from bcm2711_thermal.c from the linux library
int bcm2712_get_temp(int *temp)
{
    //From bcm2712-ds.dtsi : 
    //coefficients = <(-550) 450000>;
	int slope = -550;
	int offset = 450000;
	u32 val;

	val = avs_read32(AVS_RO_TEMP_STATUS);
	
	if (!(val & AVS_RO_TEMP_STATUS_VALID_MSK))
		return -1;

	val &= AVS_RO_TEMP_STATUS_DATA_MSK;

	/* Convert a HW code to a temperature reading (millidegree celsius) */
	*temp = slope * val + offset;

	return 0;
}