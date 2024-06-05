#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"

int oc(uint clk_interval) {
    // desired clock length in ns
    // working with 8ns, 4ns, 3.125ns, 2ns

    // default clock speed
    set_sys_clock_khz(125000, true);
    setup_default_uart();

    if (clk_interval == 8) {
        // default, gives you 8ns intervals
        set_sys_clock_khz(125000, true);
        printf("Running Pi at default 125MHz. One time interval will be 8ns.\n");
        setup_default_uart();
    }
    else if (clk_interval == 4) { 
        // 200% oc, gets you 4ns intervals
        vreg_set_voltage(VREG_VOLTAGE_1_05);
        printf("Overclocking Pi to 250MHz. One time interval will be 4ns.\n");
        set_sys_clock_khz(250000, true);
        setup_default_uart();
    } 
    else if (clk_interval == 3) {
        // 256% oc, gets you 3.125ns intervals
        vreg_set_voltage(VREG_VOLTAGE_1_20);
        printf("Overclocking Pi to 320MHz. One time interval will be 3.125ns.\n");
        set_sys_clock_khz(320000, true);
        setup_default_uart();
    }
    else if (clk_interval == 2) {
        // 320% oc, gets you 2.5ns intervals
        vreg_set_voltage(VREG_VOLTAGE_1_30);
        sleep_ms(250);
        printf("Overclocking Pi to 400MHz. One time interval will be 2.5ns.\n");
        set_sys_clock_khz(400000, true);
        sleep_ms(250);
        setup_default_uart();
        sleep_ms(250);
    }
}
