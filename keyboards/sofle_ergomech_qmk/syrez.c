// Copyright 2021-2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifdef ACHORDION_ENABLE
#include "features/achordion.h"
#endif  // ACHORDION_ENABLE
#ifdef CUSTOM_SHIFT_KEYS_ENABLE
#include "features/custom_shift_keys.h"
#endif  // CUSTOM_SHIFT_KEYS_ENABLE
#ifdef LAYER_LOCK_ENABLE
#include "features/layer_lock.h"
#endif  // LAYER_LOCK_ENABLE
#ifdef ORBITAL_MOUSE_ENABLE
#include "features/orbital_mouse.h"
#endif  // ORBITAL_MOUSE_ENABLE
#ifdef SENTENCE_CASE_ENABLE
#include "features/sentence_case.h"
#endif  // SENTENCE_CASE_ENABLE


// enum layers {
//   BASE,
//   SYM,
//   NUM,
//   WIN,
//   FUN,
// };

// enum custom_keycodes {
//   ARROW = SAFE_RANGE,
//   DASH,
//   LLOCK,
//   RGB_DEF,
//   SELLINE,
//   SRCHSEL,
//   TMUXESC,
//   UPDIR,
//   USRNAME,
// };

// Short aliases for home row mods and other tap-hold keys.
#define HOME_A    LGUI_T(KC_A)
#define HOME_S    LALT_T(KC_S)
#define HOME_D    LSFT_T(KC_D)
#define HOME_F    LCTL_T(KC_F)

#define HOME_J    RCTL_T(KC_J)
#define HOME_K    RSFT_T(KC_K)
#define HOME_L    RALT_T(KC_L)
#define HOME_SCLN RGUI_T(KC_SCLN)

///////////////////////////////////////////////////////////////////////////////
// Combos (https://docs.qmk.fm/features/combo)
///////////////////////////////////////////////////////////////////////////////
// const uint16_t caps_combo[] PROGMEM = {KC_D, KC_K, COMBO_END};
// // clang-format off
// combo_t key_combos[1] = {
//     COMBO(caps_combo, CW_TOGG)      // J and , => activate Caps Word
// };


///////////////////////////////////////////////////////////////////////////////
// Custom shift keys (https://getreuer.info/posts/keyboards/custom-shift-keys)
///////////////////////////////////////////////////////////////////////////////
#ifdef CUSTOM_SHIFT_KEYS_ENABLE
const custom_shift_key_t custom_shift_keys[] = {
    {KC_DOT, KC_QUES},
    {KC_COMM, KC_EXLM},
    // {HOME_SC, KC_AT  },
    {KC_MPLY, KC_MNXT},
    {KC_EQL , KC_EQL },  // Don't shift =
    {KC_SLSH, KC_SLSH},  // Don't shift /
};
uint8_t NUM_CUSTOM_SHIFT_KEYS =
    sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);
#endif  // CUSTOM_SHIFT_KEYS_ENABLE

///////////////////////////////////////////////////////////////////////////////
// Tap-hold configuration (https://docs.qmk.fm/tap_hold)
///////////////////////////////////////////////////////////////////////////////
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {
    case HOME_A:
    case HOME_SCLN:
      return TAPPING_TERM + 50;
    case HOME_S:
    case HOME_L:
      return TAPPING_TERM + 25;
    default:
      return TAPPING_TERM;
  }
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t* record) {
  // If you quickly hold a tap-hold key after tapping it, the tap action is
  // repeated. Key repeating is useful e.g. for Vim navigation keys, but can
  // lead to missed triggers in fast typing. Here, returning 0 means we
  // instead want to "force hold" and disable key repeating.
  switch (keycode) {
    // case HOME_N:
    //   return QUICK_TAP_TERM;  // Enable key repeating.
    default:
      return 0;  // Otherwise, force hold and disable key repeating.
  }
}

///////////////////////////////////////////////////////////////////////////////
// Achordion (https://getreuer.info/posts/keyboards/achordion)
///////////////////////////////////////////////////////////////////////////////
#ifdef ACHORDION_ENABLE
bool achordion_chord(uint16_t tap_hold_keycode,
                     keyrecord_t* tap_hold_record,
                     uint16_t other_keycode,
                     keyrecord_t* other_record) {
  // Also allow same-hand holds when the other key is in the rows outside the
  // alphas. I need the `% (MATRIX_ROWS / 2)` because my keyboards are split.
  uint8_t row = other_record->event.key.row % (MATRIX_ROWS / 2);
  if (!(1 <= row && row <= 3)) { return true; }

  switch (tap_hold_keycode) {
    // Exceptionally allow symbol layer LTs + row 0 in same-hand chords.
    case HOME_A:
    case HOME_SCLN:
      if (row == 0) { return true; }
      break;
    // Exceptionally allow G + J as a same-hand chord.
    // case NUM_G:
    //   if (other_keycode == KC_J) { return true; }
    //   break;
  }

  return achordion_opposite_hands(tap_hold_record, other_record);
}

uint16_t achordion_timeout(uint16_t tap_hold_keycode) {
  switch (tap_hold_keycode) {
    default:
      return 800;  // Use a timeout of 800 ms.
  }
}

uint16_t achordion_streak_chord_timeout(
    uint16_t tap_hold_keycode, uint16_t next_keycode) {
  // Disable streak detection on LT keys.
  if (IS_QK_LAYER_TAP(tap_hold_keycode)) {
    return 0;
  }

  // // Exceptions so that certain hotkeys don't get blocked as streaks.
  // switch (tap_hold_keycode) {
  //   case HOME_N:
  //     if (next_keycode == KC_C || next_keycode == KC_V) {
  //       return 0;
  //     }
  //     break;
  //   case HOME_T:
  //     if (next_keycode == HOME_N) {
  //       return 0;
  //     }
  //     break;
  // }

  // Otherwise, tap_hold_keycode is a mod-tap key.
  const uint8_t mod = mod_config(QK_MOD_TAP_GET_MODS(tap_hold_keycode));
  if ((mod & MOD_LSFT) != 0) {
    return 100;  // A short streak timeout for Shift mod-tap keys.
  } else {
    return 220;  // A longer timeout otherwise.
  }
}
#endif  // ACHORDION_ENABLE

///////////////////////////////////////////////////////////////////////////////
// Autocorrect (https://docs.qmk.fm/features/autocorrect)
///////////////////////////////////////////////////////////////////////////////
#ifdef AUTOCORRECT_ENABLE
bool apply_autocorrect(uint8_t backspaces, const char* str,
                       char* typo, char* correct) {
  for (uint8_t i = 0; i < backspaces; ++i) {
    tap_code(KC_BSPC);
  }
  send_string_with_delay_P(str, TAP_CODE_DELAY);
  return false;
}
#endif  // AUTOCORRECT_ENABLE

///////////////////////////////////////////////////////////////////////////////
// Caps word (https://docs.qmk.fm/features/caps_word)
///////////////////////////////////////////////////////////////////////////////
#ifdef CAPS_WORD_ENABLE
bool caps_word_press_user(uint16_t keycode) {
  switch (keycode) {
    // Keycodes that continue Caps Word, with shift applied.
    case KC_A ... KC_Z:
      add_weak_mods(MOD_BIT(KC_LSFT));  // Apply shift to the next key.
      return true;

    // Keycodes that continue Caps Word, without shifting.
    case KC_1 ... KC_0:
    case KC_BSPC:
    case KC_DEL:
    case KC_UNDS:
      return true;

    default:
      return false;  // Deactivate Caps Word.
  }
}
#endif  // CAPS_WORD_ENABLE

///////////////////////////////////////////////////////////////////////////////
// Sentence case (https://getreuer.info/posts/keyboards/sentence-case)
///////////////////////////////////////////////////////////////////////////////
#ifdef SENTENCE_CASE_ENABLE
char sentence_case_press_user(uint16_t keycode, keyrecord_t* record,
                              uint8_t mods) {
  if ((mods & ~(MOD_MASK_SHIFT | MOD_BIT(KC_RALT))) == 0) {
    const bool shifted = mods & MOD_MASK_SHIFT;
    switch (keycode) {
      case KC_A ... KC_Z:
        return 'a';  // Letter key.

      case KC_DOT:  // Both . and Shift . (?) punctuate sentence endings.
      case KC_EXLM:
      case KC_QUES:
        return '.';

      case KC_COMM:
        return shifted ? '.' : '#';

      case KC_2 ... KC_0:  // 2 3 4 5 6 7 8 9 0
      case KC_AT ... KC_RPRN:  // @ # $ % ^ & * ( )
      case KC_MINS ... KC_SCLN:  // - = [ ] backslash ;
      case KC_UNDS ... KC_COLN:  // _ + { } | :
      case KC_GRV:
        return '#';  // Symbol key.

      case KC_SPC:
        return ' ';  // Space key.

      case KC_QUOT:
      case KC_DQUO:
        return '\'';  // Quote key.
    }
  }

  // Otherwise clear Sentence Case to initial state.
  sentence_case_clear();
  return '\0';
}
#endif  // SENTENCE_CASE_ENABLE

///////////////////////////////////////////////////////////////////////////////
// Repeat key (https://docs.qmk.fm/features/repeat_key)
///////////////////////////////////////////////////////////////////////////////
bool remember_last_key_user(uint16_t keycode, keyrecord_t* record,
                            uint8_t* remembered_mods) {
  // Unpack tapping keycode for tap-hold keys.
  switch (keycode) {
#ifndef NO_ACTION_TAPPING
    case QK_MOD_TAP ... QK_MOD_TAP_MAX:
      keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
      break;
#ifndef NO_ACTION_LAYER
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
      keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
      break;
#endif  // NO_ACTION_LAYER
#endif  // NO_ACTION_TAPPING
  }

  // Forget Shift on most letters when Shift or AltGr are the only mods. Some
  // letters are excluded, e.g. for "NN" and "ZZ" in Vim.
  switch (keycode) {
    case KC_A ... KC_H:
    case KC_K ... KC_M:
    case KC_O ... KC_U:
      if ((*remembered_mods & ~(MOD_MASK_SHIFT | MOD_BIT(KC_RALT))) == 0) {
        *remembered_mods &= ~MOD_MASK_SHIFT;
      }
      break;
  }

  return true;
}

// uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
//   if ((mods & MOD_MASK_CTRL)) {
//     switch (keycode) {
//       case HOME_F: return C(KC_C);  // Ctrl+A -> Ctrl+C
//       case KC_C: return C(KC_V);    // Ctrl+C -> Ctrl+V
//     }
//   } 
//   return KC_TRNS;
// }


bool process_record_user(uint16_t keycode, keyrecord_t* record) 
{
#ifdef ACHORDION_ENABLE
  if (!process_achordion(keycode, record)) { return false; }
#endif  // ACHORDION_ENABLE
#ifdef ORBITAL_MOUSE_ENABLE
  if (!process_orbital_mouse(keycode, record)) { return false; }
#endif  // ORBITAL_MOUSE_ENABLE
#ifdef LAYER_LOCK_ENABLE
  if (!process_layer_lock(keycode, record, LLOCK)) { return false; }
#endif  // LAYER_LOCK_ENABLE
#ifdef SENTENCE_CASE_ENABLE
  if (!process_sentence_case(keycode, record)) { return false; }
#endif  // SENTENCE_CASE_ENABLE
#ifdef CUSTOM_SHIFT_KEYS_ENABLE
  if (!process_custom_shift_keys(keycode, record)) { return false; }
#endif  // CUSTOM_SHIFT_KEYS_ENABLE


  const uint8_t mods = get_mods();
  const uint8_t all_mods = (mods | get_weak_mods()
#ifndef NO_ACTION_ONESHOT
                        | get_oneshot_mods()
#endif  // NO_ACTION_ONESHOT
  );

  const uint8_t shift_mods = all_mods & MOD_MASK_SHIFT;
  // const bool alt = all_mods & MOD_BIT(KC_LALT);

  // If alt repeating key A, E, I, O, U, Y with no mods other than Shift, set
  // the last key to KC_N. Above, alternate repeat of KC_N is defined to be
  // again KC_N. This way, either tapping alt repeat and then repeat (or
  // equivalently double tapping alt repeat) is useful to type certain patterns
  // without SFBs:
  //
  //   D <altrep> <rep> -> DYN (as in "dynamic")
  //   O <altrep> <rep> -> OAN (as in "loan")

  if (get_repeat_key_count() < 0 && !shift_mods &&
        (keycode == KC_A || keycode == KC_E || keycode == KC_I ||
       keycode == KC_O || keycode == KC_U || keycode == KC_Y)) 
  {
    set_last_keycode(KC_N);
    set_last_mods(0);
  }

  return true;
}

void matrix_scan_user(void) {
#ifdef ACHORDION_ENABLE
  achordion_task();
#endif  // ACHORDION_ENABLE
#ifdef LAYER_LOCK_ENABLE
  layer_lock_task();
#endif  // LAYER_LOCK_ENABLE
#ifdef ORBITAL_MOUSE_ENABLE
  orbital_mouse_task();
#endif  // ORBITAL_MOUSE_ENABLE
#ifdef SENTENCE_CASE_ENABLE
  sentence_case_task();
#endif  // SENTENCE_CASE_ENABLE
}

