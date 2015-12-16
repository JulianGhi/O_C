
/*static*/ const char *note_names[12] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };

const char *note_name(int note) {
  return note_names[(note + 120) % 12];
}

void H1200_menu() {
   u8g.setFont(UI_DEFAULT_FONT);
   u8g.setColorIndex(1);
   u8g.setFontRefHeightText();
   u8g.setFontPosTop();

  const abstract_triad &current_chord = h1200_state.tonnetz_state.current_chord();

  static const uint8_t kStartX = 0;

  UI_DRAW_TITLE(kStartX);
  if (h1200_state.display_notes)
    u8g.print(note_name(h1200_state.tonnetz_state.root()));
  else
    u8g.print(h1200_state.tonnetz_state.root());
  u8g.print(mode_names[current_chord.mode()]);

  u8g.setPrintPos(64, kUiTitleTextY);
  if (h1200_state.display_notes) {
    for (size_t i=1; i < 4; ++i) {
      if (i > 1) u8g.print(' ');
      u8g.print(note_name(h1200_state.tonnetz_state.outputs(i)));
    }
  } else {
    for (size_t i=1; i < 4; ++i) {
      if (i > 1) u8g.print(' ');
      u8g.print(h1200_state.tonnetz_state.outputs(i));
    }
  }

  int first_visible = h1200_state.cursor_pos - kUiVisibleItems + 1;
  if (first_visible < 0)
    first_visible = 0;

  UI_START_MENU(kStartX);
  UI_BEGIN_ITEMS_LOOP(kStartX, first_visible, H1200_SETTING_LAST, h1200_state.cursor_pos, 0)
    const settings::value_attr &attr = H1200Settings::value_attr(current_item);
    UI_DRAW_SETTING(attr, h1200_settings.get_value(current_item), kUiWideMenuCol1X);
  UI_END_ITEMS_LOOP();
}

static const uint8_t note_circle_x = 32;
static const uint8_t note_circle_y = 32;
static const uint8_t note_circle_r = 28;

struct coords {
  uint8_t x, y;
} circle_pos_lut[12];

void init_circle_lut() {
  static const float pi = 3.14159265358979323846f;
  static const float semitone_radians = (2.f * pi / 12.f);

  for (int i = 0; i < 12; ++i) {
    float rads = ((i + 12 - 3) % 12) * semitone_radians;
    float x = note_circle_r * cos(rads);
    float y = note_circle_r * sin(rads);
    circle_pos_lut[i].x = note_circle_x + x;
    circle_pos_lut[i].y = note_circle_y + y;
  }
}
const uint8_t circle_disk_bitmap[] = {
  0, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0
};

void visualize_pitch_classes(uint8_t *normalized) {
  u8g.drawCircle(note_circle_x, note_circle_y, note_circle_r);

  coords last_pos = circle_pos_lut[normalized[0]];
  for (size_t i = 1; i < 3; ++i) {
    u8g.drawBitmap(last_pos.x - 3, last_pos.y - 3, 1, 8, circle_disk_bitmap);
    const coords &current_pos = circle_pos_lut[normalized[i]];
    u8g.drawLine(last_pos.x, last_pos.y, current_pos.x, current_pos.y);
    last_pos = current_pos;
  }
  u8g.drawLine(last_pos.x, last_pos.y, circle_pos_lut[normalized[0]].x, circle_pos_lut[normalized[0]].y);
  u8g.drawBitmap(last_pos.x - 3, last_pos.y - 3, 1, 8, circle_disk_bitmap);

}

void H1200_screensaver() {

  uint8_t y = 0;
  static const uint8_t x_col_0 = 66;
  static const uint8_t x_col_1 = 66 + 24;
  static const uint8_t x_col_2 = 66 + 38;
  static const uint8_t line_h = 16;

  //u8g.setFont(u8g_font_timB12); BBX 19x27
  u8g.setFont(u8g_font_10x20); // fixed-width makes positioning a bit easier
  u8g.setColorIndex(1);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  u8g.setDefaultForegroundColor();
 
  uint8_t normalized[3];
  y = 8;
  for (size_t i=0; i < 3; ++i, y += line_h) {
    int value = h1200_state.tonnetz_state.outputs(i + 1);

    u8g.setPrintPos(x_col_1, y);
    u8g.print(value / 12);

    value = (value + 120) % 12;
    u8g.setPrintPos(x_col_2, y);
    u8g.print(note_names[value]);
    normalized[i] = value;
  }
  y = 0;
  for (size_t i = 0; i < TonnetzState::HISTORY_LENGTH; ++i, y += line_h) {
    u8g.setPrintPos(x_col_0, y);
    u8g.print(h1200_state.tonnetz_state.history(i).str);
  }

  visualize_pitch_classes(normalized);
}
