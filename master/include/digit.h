#ifndef digit_h
#define digit_h

#include <Arduino.h>
#include "clock_state.h"

/**
 * Digits
 * structure: {
 * h0, m0,
 * h1, m1,
 * ...
 * h5, m5
 * }
*/

const t_digit digit_0 = {
  270, 0,
  270, 90,
  0, 90,
  270, 180,
  270, 90,
  180, 90
};

const t_digit digit_1 = {
  225, 225,
  225, 225,
  225, 225,
  270, 270,
  270, 90,
  90, 90
};

const t_digit digit_2 = {
  0, 0,
  270, 0,
  90, 0,
  180, 270,
  90, 180,
  180, 180
};

const t_digit digit_3 = {
  0, 0,
  0, 0,
  0, 0,
  180, 270,
  180, 90,
  180, 90
};

const t_digit digit_4 = {
  270, 270,
  90, 0,
  225, 225,
  270, 270,
  270, 90,
  90, 90
};

const t_digit digit_5 = {
  270, 0,
  90, 0,
  0, 0,
  180, 180,
  270, 180,
  90, 180
};

const t_digit digit_6 = {
  270, 0,
  270, 90,
  90, 0,
  180, 180,
  270, 180,
  90, 180
};

const t_digit digit_7 = {
  0, 0,
  225, 225,
  225, 225,
  270, 180,
  270, 90,
  90, 90
};

const t_digit digit_8 = {
  270, 0,
  90, 0,
  90, 0,
  270, 180,
  90, 180,
  90, 180
};

const t_digit digit_9 = {
  270, 0,
  0, 90,
  0, 0,
  270, 180,
  270, 90,
  90, 180
};

const t_digit digit_null = {
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270
};

const t_digit digit_I = {
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90
};

const t_digit digit_fun = {
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
};

const t_full_clock d_stop = {digit_null, digit_null, digit_null, digit_null};

const t_full_clock d_fun = {digit_fun, digit_fun, digit_fun, digit_fun};

const t_full_clock d_IIII = {digit_I, digit_I, digit_I, digit_I};

const t_digit digit_wave_a = {
  70, 255,
  85, 275,
  105, 290,
  60, 250,
  85, 275,
  110, 300
};

const t_digit digit_wave_b = {
  45, 245,
  80, 280,
  115, 315,
  15, 220,
  60, 300,
  140, 345
};

// Left-right mirrors (columns swapped, angles reflected: theta -> 180 - theta)
const t_digit digit_wave_a_mirror = {
  120, 290,
  95, 265,
  70, 240,
  110, 285,
  95, 265,
  75, 250
};

const t_digit digit_wave_b_mirror = {
  165, 320,
  120, 240,
  40, 195,
  135, 295,
  100, 260,
  65, 225
};

const t_full_clock d_WAVE = {digit_wave_a, digit_wave_b, digit_wave_b_mirror, digit_wave_a_mirror};

// All hands pointing down-left
const t_digit digit_JOINT = {
  225, 225,
  225, 225,
  225, 225,
  225, 225,
  225, 225,
  225, 225
};

const t_full_clock d_joint = {digit_JOINT, digit_JOINT, digit_JOINT, digit_JOINT};

const t_digit digit_cent_a = {
  345, 345,
  0, 0,
  15, 15,
  330, 330,
  0, 0,
  30, 30
};

const t_digit digit_cent_b = {
  315, 315,
  0, 0,
  45, 45,
  300, 300,
  0, 0,
  60, 60
};

// Left-right mirrors (columns swapped, angles reflected: theta -> 180 - theta)
const t_digit digit_cent_a_mirror = {
  210, 210,
  180, 180,
  150, 150,
  195, 195,
  180, 180,
  165, 165
};

const t_digit digit_cent_b_mirror = {
  240, 240,
  180, 180,
  120, 120,
  225, 225,
  180, 180,
  135, 135
};

const t_full_clock d_CENT = {digit_cent_a, digit_cent_b, digit_cent_b_mirror, digit_cent_a_mirror};

const t_digit digit_bubble = {
  270, 0,
  90, 0,
  270, 0,
  180, 270,
  180, 90,
  180, 270
};

const t_full_clock d_bubble = {digit_bubble, digit_bubble, digit_bubble, digit_bubble};

const t_digit digit_diag = {
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45
};

const t_full_clock d_diagonal = {digit_diag, digit_diag, digit_diag, digit_diag};

#endif