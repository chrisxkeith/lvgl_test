class Utils {
  private:
    static unsigned long lastPrintln;
  public:
    static void publish(String s) {
      if (millis() < lastPrintln + 1000) {
        delay(1000);
      }
      Serial.println(s);
    }
    static String msToString(unsigned long ms) {
      int totalSeconds = ms / 1000;
      int secs = totalSeconds % 60;
      int minutes = (totalSeconds / 60) % 60;
      int hours = (totalSeconds / 60) / 60;
    
      char buf[100];
      sprintf(buf, "%02u:%02u:%02u", hours, minutes, secs);
      return String(buf);
    }
};
unsigned long Utils::lastPrintln = 0;

#include "Arduino_H7_Video.h"

#include "lvgl.h"
#include "/home/ck/Arduino/libraries/lvgl/src/misc/lv_color.h"

#define WIDTH     800
#define HEIGHT    480
Arduino_H7_Video  Display(WIDTH, HEIGHT, GigaDisplayShield);
const int COLOR_WHITE = 0x65535;
const int COLOR_BLACK = 0x0;

class OLEDWrapper {
  private:
    lv_obj_t*   gridCell = nullptr;
    lv_obj_t*   screen = nullptr;
    const int   DEFAULT_FONT_SIZE = 24;
    lv_style_t    black;
    lv_style_t    white;
    int         currentColor;
  public:
    void startup() {
      delay(3000);
      Display.begin();
      screen = lv_obj_create(lv_scr_act());
      lv_obj_set_size(screen, Display.width(), Display.height());
      setupGrid();
      setupLineStyle(&black, 2, lv_color_black());
      setupLineStyle(&white, 4, lv_color_white());
    }
    void displayOff() {
      pinMode(74, OUTPUT);
      digitalWrite(74, LOW);
    }
    void displayOn() {
      pinMode(74, OUTPUT);
      digitalWrite(74, HIGH);
    }
    int getWidth() {
      return Display.width();
    }
    int getHeight() {
      return Display.height();
    }
    void fillRect(int x0, int y0, int x1, int y1, int color) {
      // no op
    }
    void setupLineStyle(lv_style_t *line_style, int width, lv_color_t color) {
      lv_style_init(line_style);
      lv_style_set_line_width(line_style, width);
      lv_style_set_line_color(line_style, color);
      lv_style_set_line_rounded(line_style, true);
    }
    void setupGrid() {
      static lv_coord_t col_dsc[] = { WIDTH - 50, LV_GRID_TEMPLATE_LAST };
      static lv_coord_t row_dsc[] = { HEIGHT - 50, LV_GRID_TEMPLATE_LAST };

      lv_obj_t* grid = lv_obj_create(lv_scr_act());
      lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
      lv_obj_set_size(grid, Display.width(), Display.height());

      gridCell = lv_label_create(grid);
      lv_obj_set_grid_cell(gridCell, LV_GRID_ALIGN_STRETCH, 0, 1,  //column
                          LV_GRID_ALIGN_STRETCH, 0, 1);      //row
      lv_obj_set_style_text_font(gridCell, &lv_font_montserrat_28, 0);
    }
    void display(String s, int textSize, uint8_t x, uint8_t y) {
      lv_label_set_text(gridCell, s.c_str());
    }
    void display(String s) {
      display(s, DEFAULT_FONT_SIZE, 10, 10);
    }
    void setDrawColor(int color) {
      currentColor = color;
    }
    void drawLines(lv_point_precise_t line_points[], int nPoints, lv_style_t *line_style) {
      /*Create a line and apply the new style*/
      lv_obj_t * line1;
      line1 = lv_line_create(lv_scr_act());
      lv_line_set_points(line1, line_points, nPoints);
      lv_obj_add_style(line1, line_style, 0);
      lv_obj_center(line1);
    }
    void drawLine(int x0, int y0, int x1, int y1) {
      static lv_point_precise_t line_points[] = { {x0, y0}, {x1, y1} };
      if (currentColor == COLOR_BLACK) {
        drawLines(line_points, 2, &black);
      } else {
        drawLines(line_points, 2, &white);
      }
    }
    void handleTimer() {
      lv_timer_handler();
      delay(5);
    }
};
OLEDWrapper* oledWrapper = nullptr;

class Spinner {
  private:
    int middleX = oledWrapper->getWidth() / 2;
    int middleY = oledWrapper->getHeight() / 2;
    int lineLength = min(middleX, middleY);
    int color = COLOR_WHITE;
    int deg = 0;
    unsigned long msWhenOn = 0;
    unsigned long baseline = 0;
    unsigned long lastShift = 0;
    int incrementDegrees = 1;
    unsigned long lastDisplayTime = 0;
    const unsigned long DISPLAY_INTERVAL = 200; // milliseconds

    void drawElapsed() {
      unsigned long elapsed = millis() - msWhenOn;
      String s = Utils::msToString(elapsed);
      oledWrapper->fillRect(0, 0, 145, oledWrapper->getHeight(), COLOR_BLACK);
      oledWrapper->display(s, 3, 0, baseline);
      if (millis() - lastShift > 1000 * 10) {
        baseline += 3;
        if (baseline > oledWrapper->getHeight() - 20) {
          baseline = 0;
        }
        lastShift = millis();
      }
    }

  public:
    Spinner(int incrementDegrees) {
      this->incrementDegrees = incrementDegrees;
    }
    void reset() {
      deg = 0;
      color = COLOR_WHITE;
      msWhenOn = millis();
    }
    void display() {
      if (millis() - lastDisplayTime < DISPLAY_INTERVAL) {
        return;
      }
      int xEnd = lineLength * cos(deg * M_PI / 180.0);
      int yEnd = lineLength * sin(deg * M_PI / 180.0);

      oledWrapper->setDrawColor(color);
      oledWrapper->drawLine(middleX, middleY, middleX + xEnd, middleY + yEnd);
      drawElapsed();
      deg += incrementDegrees;
      if (deg >= 360) {
        deg = 0;
        if (color == COLOR_WHITE) {
          color = COLOR_BLACK;
        } else {
          color = COLOR_WHITE;
        }
      }
      lastDisplayTime = millis();
    }
    void dump() {
      String s("Spinner: middleX: ");
      s.concat(middleX);
      s.concat(", middleY: ");
      s.concat(middleY);
      s.concat(", lineLength: ");
      s.concat(lineLength);
      s.concat(", baseline: ");
      s.concat(baseline);
      Utils::publish(s);
    }
};
Spinner spinner(5);

lv_point_precise_t line_points1[] = { {0, 0}, {WIDTH, HEIGHT} };
lv_point_precise_t line_points2[] = { {WIDTH, 0}, {0, HEIGHT} };

class App {
  private:
    int           counter = 0;
    unsigned long lastUpdateTime = 0;
    lv_style_t    black;
    lv_style_t    white;
    bool          draw1Black = true;
    bool          draw2Black = true;

    void lineTest() {
      if (counter % 2 == 0) {
        if (draw1Black) {
          oledWrapper->drawLines(line_points1, 2, &black);
//          oledWrapper->setDrawColor(COLOR_BLACK);
//          oledWrapper->drawLine(0, 0, WIDTH, HEIGHT);
          draw1Black = false;
        } else {
          oledWrapper->drawLines(line_points1, 2, &white);
//          oledWrapper->setDrawColor(COLOR_WHITE);
//          oledWrapper->drawLine(0, 0, WIDTH, HEIGHT);
          draw1Black = true;
        }
      } else {
        if (draw2Black) {
          oledWrapper->drawLines(line_points2, 2, &black);
//          oledWrapper->setDrawColor(COLOR_BLACK);
//          oledWrapper->drawLine(WIDTH, 0, 0, HEIGHT);
          draw2Black = false;
        } else {
          oledWrapper->drawLines(line_points2, 2, &white);
//          oledWrapper->setDrawColor(COLOR_WHITE);
//          oledWrapper->drawLine(WIDTH, 0, 0, HEIGHT);
          draw2Black = true;
        }
      }
    }
  public:
    void loop() {
/*     if (millis() - lastUpdateTime > 3000) {
        if (counter % 2 == 0) {
          oledWrapper->displayOff();
        } else {
          oledWrapper->displayOn();
        }
      lastUpdateTime = millis();
        String s("Counter: ");
        s.concat(counter++);
        oledWrapper->display(s);
        // lineTest();
      }
*/      spinner.display();
      oledWrapper->handleTimer();
    }
    void setup() {
      oledWrapper = new OLEDWrapper();
      oledWrapper->startup();
      oledWrapper->setupLineStyle(&black, 2, lv_color_black());
      oledWrapper->setupLineStyle(&white, 4, lv_color_white());
    }
  };
App app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}