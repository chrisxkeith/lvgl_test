#include "Arduino_H7_Video.h"

#include "lvgl.h"

#define WIDTH     800
#define HEIGHT    480
Arduino_H7_Video  Display(WIDTH, HEIGHT, GigaDisplayShield);

class OLEDWrapper {
  private:
    lv_obj_t*   leftCell = nullptr;
    lv_obj_t*   rightCell = nullptr;
    lv_obj_t*   screen = nullptr;
    const int   DEFAULT_FONT_SIZE = 24;
    int         currentColor = 0;
  public:
    void displayOff() {
      pinMode(74, OUTPUT);
      digitalWrite(74, LOW);
    }
    void displayOn() {
      pinMode(74, OUTPUT);
      digitalWrite(74, HIGH);
    }
    void startup() {
      delay(3000);
      Display.begin();
      screen = lv_obj_create(lv_scr_act());
      lv_obj_set_size(screen, Display.width(), Display.height());
      setupGrid();
    }
    void setupGrid() {
      static lv_coord_t col_dsc[] = { 300, 440, LV_GRID_TEMPLATE_LAST };
      static lv_coord_t row_dsc[] = { 430, 430, LV_GRID_TEMPLATE_LAST };

      lv_obj_t* grid = lv_obj_create(lv_scr_act());
      lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
      lv_obj_set_size(grid, Display.width(), Display.height());

      leftCell = lv_label_create(grid);
      lv_obj_set_grid_cell(leftCell, LV_GRID_ALIGN_STRETCH, 0, 1,  //column
                          LV_GRID_ALIGN_STRETCH, 0, 1);      //row
      lv_obj_set_style_text_font(leftCell, &lv_font_montserrat_28, 0);

      rightCell = lv_label_create(grid);
      lv_obj_set_grid_cell(rightCell, LV_GRID_ALIGN_STRETCH, 1, 1,  //column
                          LV_GRID_ALIGN_STRETCH, 0, 1);      //row
    }
    void display(String s, int textSize, uint8_t x, uint8_t y) {
      lv_label_set_text(leftCell, s.c_str());
    }
    void display(String s) {
      display(s, DEFAULT_FONT_SIZE, 10, 10);
    }
    void display(String s[], int nStrings) {
      for (int i = 0; i < nStrings; i++) {
        display(s[i], DEFAULT_FONT_SIZE, 10, 32 + (i * 32));
      }
    }
    void setDrawColor(int color) {
      currentColor = color;
    }
    void drawLine(int x0, int y0, int x1, int y1) {
    }
};
OLEDWrapper* oledWrapper = nullptr;

void setup() {
  oledWrapper = new OLEDWrapper();
  oledWrapper->startup();
}

int counter = 0;
unsigned long lastUpdateTime = 0;
void loop() {
  if (millis() - lastUpdateTime > 3000) {
    if (counter % 2 == 0) {
      oledWrapper->displayOff();
    } else {
      oledWrapper->displayOn();
    }
    lastUpdateTime = millis();
    String s("Counter: ");
    s.concat(counter++);
    oledWrapper->display(s);
  }
  lv_timer_handler();
  delay(5);
}