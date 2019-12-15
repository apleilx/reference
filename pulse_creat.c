//脉冲、方波发生器
uint8_t pdata tick_ms_cnt;
uint8_t pdata powerup_time_cnt;
bit b_pulse_100ms, b_pulse_200ms, b_pulse_500ms, b_pulse_1s;
bit b_wave_100ms, b_wave_200ms, b_wave_500ms, b_wave_1s;
uint8_t pdata pulse_tick_old;
uint8_t pdata pulse_100ms_cnt;
uint8_t pdata pulse_200ms_cnt;
uint8_t pdata pulse_500ms_cnt;
uint8_t pdata pulse_1s_cnt;

        //时间脉冲发生，用于精确到0.1s等级的定时
        b_pulse_100ms = 0;
        b_pulse_200ms = 0;
        b_pulse_500ms = 0;
        b_pulse_1s = 0;
        isr_dis();
        u8t_cal = tick_ms_cnt - pulse_tick_old;
        pulse_tick_old = tick_ms_cnt;
        isr_en();
        pulse_100ms_cnt += u8t_cal;
        if(pulse_100ms_cnt >= 100)
        {
            pulse_100ms_cnt -= 100;
            b_pulse_100ms = 1;
            b_wave_100ms = !b_wave_100ms;
            pulse_200ms_cnt ++;
            pulse_500ms_cnt ++;
            pulse_1s_cnt ++;
            
            if(powerup_time_cnt < 255)
                powerup_time_cnt++;
            
            if(pulse_200ms_cnt >= 2)
            {
                pulse_200ms_cnt -= 2;
                b_pulse_200ms = 1;
                b_wave_200ms = !b_wave_200ms;
            }
            if(pulse_500ms_cnt >= 5)
            {
                pulse_500ms_cnt -= 5;
                b_pulse_500ms = 1;
                b_wave_500ms = !b_wave_500ms;
            }
            if(pulse_1s_cnt >= 10)
            {
                pulse_1s_cnt -= 10;
                b_pulse_1s = 1;
                b_wave_1s = !b_wave_1s;
            }
        }
