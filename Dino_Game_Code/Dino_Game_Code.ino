/*
  --------------------------------------------
  |                                          |
  |             Subscribe Now!!!             |
  |                                          |
  | http://www.youtube.com/@ChethanaVishmika |
  |                                          |
  --------------------------------------------
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#define SCREEN_W   128
#define SCREEN_H   64
#define OLED_ADDR  0x3C      // try 0x3D if the screen stays blank
#define SDA_PIN    21
#define SCL_PIN    22
#define BTN_PIN    4

#define GROUND_Y   57        // ground line
#define DINO_X     8
#define DINO_W     16
#define DINO_H     16
#define FRAME_MS   40        // 25 fps

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
Preferences prefs;

// 16x14 dino body (legs are drawn separately for the run animation)
const uint8_t dinoBody[] PROGMEM = {
  0x03,0xF8, 0x07,0xFC, 0x07,0x7C, 0x07,0xFC,
  0x07,0xFC, 0x07,0xC0, 0x07,0xF0, 0x87,0x80,
  0x8F,0x80, 0xDF,0xC0, 0xFF,0xC0, 0xFF,0xC0,
  0x7F,0x80, 0x3F,0x00
};

enum State { WAITING, RUNNING, OVER };
State state = WAITING;

struct Obstacle { float x; uint8_t type; bool active; };
Obstacle obs[3];

// obstacle types: 0 = small, 1 = tall, 2 = double
const uint8_t OBS_W[] = { 7, 9, 16 };
const uint8_t OBS_H[] = { 14, 20, 14 };

float dinoY, vy;
bool onGround;
float speedPx;
float score;
uint32_t hiScore;
float distSinceSpawn, nextGap;
uint8_t frameCount;
uint32_t lastFrame, overTime;
int groundDots[8];
bool btnReleased = false;

bool pressed() { return digitalRead(BTN_PIN) == LOW; }

void resetGame() {
  dinoY = GROUND_Y - DINO_H;
  vy = 0;
  onGround = true;
  speedPx = 3.0;
  score = 0;
  distSinceSpawn = 0;
  nextGap = 60;
  for (auto &o : obs) o.active = false;
  for (int i = 0; i < 8; i++) groundDots[i] = random(0, SCREEN_W);
}

void spawnObstacle() {
  for (auto &o : obs) {
    if (!o.active) {
      o.active = true;
      o.x = SCREEN_W;
      o.type = random(0, 3);
      break;
    }
  }
  distSinceSpawn = 0;
  // gap scales with speed so every obstacle stays jumpable
  nextGap = speedPx * 22 + 30 + random(0, 60);
}

void drawCactus(int x, int w, int h) {
  int top = GROUND_Y - h;
  int cx = x + w / 2;
  display.fillRect(cx - 1, top, 3, h, WHITE);                 // stem
  display.fillRect(x, top + 5, 2, 5, WHITE);                  // left arm
  display.fillRect(x, top + 9, cx - x, 2, WHITE);
  display.fillRect(x + w - 2, top + 3, 2, 5, WHITE);          // right arm
  display.fillRect(cx, top + 7, x + w - cx, 2, WHITE);
}

void drawObstacle(const Obstacle &o) {
  int x = (int)o.x;
  if (o.type == 2) {
    drawCactus(x, 7, 14);
    drawCactus(x + 9, 7, 14);
  } else {
    drawCactus(x, OBS_W[o.type], OBS_H[o.type]);
  }
}

bool hitTest(const Obstacle &o) {
  // slightly shrunk hitboxes so it feels fair
  int dx1 = DINO_X + 3,  dx2 = DINO_X + DINO_W - 2;
  int dy1 = (int)dinoY + 2, dy2 = (int)dinoY + DINO_H - 1;
  int ox1 = (int)o.x + 1, ox2 = (int)o.x + OBS_W[o.type] - 1;
  int oy1 = GROUND_Y - OBS_H[o.type] + 1, oy2 = GROUND_Y;
  return dx1 < ox2 && dx2 > ox1 && dy1 < oy2 && dy2 > oy1;
}

void drawDino() {
  int y = (int)dinoY;
  display.drawBitmap(DINO_X, y, dinoBody, 16, 14, WHITE);
  // legs
  if (!onGround) {
    display.fillRect(DINO_X + 3, y + 14, 2, 2, WHITE);
    display.fillRect(DINO_X + 7, y + 14, 2, 2, WHITE);
  } else if ((frameCount / 3) % 2 == 0) {
    display.fillRect(DINO_X + 3, y + 14, 2, 2, WHITE);
    display.fillRect(DINO_X + 7, y + 14, 2, 1, WHITE);
  } else {
    display.fillRect(DINO_X + 3, y + 14, 2, 1, WHITE);
    display.fillRect(DINO_X + 7, y + 14, 2, 2, WHITE);
  }
}

void drawGround() {
  display.drawFastHLine(0, GROUND_Y, SCREEN_W, WHITE);
  for (int i = 0; i < 8; i++) {
    display.drawPixel(groundDots[i], GROUND_Y + 2 + (i % 3) * 2, WHITE);
  }
}

void drawScore() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  char buf[24];
  snprintf(buf, sizeof(buf), "HI %05u %05u", (unsigned)hiScore, (unsigned)score);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_W - w - 1, 0);
  display.print(buf);
}

void centerText(const char *s, int y) {
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds((char *)s, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_W - w) / 2, y);
  display.print(s);
}

void drawRestartIcon(int cx, int cy) {
  display.fillCircle(cx, cy, 8, BLACK);                 // clear background
  display.drawCircle(cx, cy, 6, WHITE);
  display.drawCircle(cx, cy, 5, WHITE);
  display.fillRect(cx, cy - 8, 8, 5, BLACK);            // gap in the ring
  display.fillTriangle(cx - 1, cy - 9, cx - 1, cy - 2, cx + 5, cy - 5, WHITE); // arrow head
}

void updateGame() {
  frameCount++;

  // jump (variable height: release early = shorter hop)
  if (onGround && pressed()) {
    vy = -6.2;
    onGround = false;
  }
  if (!onGround) {
    if (vy < -2.5 && !pressed()) vy += 0.6;
    vy += 0.55;
    dinoY += vy;
    if (dinoY >= GROUND_Y - DINO_H) {
      dinoY = GROUND_Y - DINO_H;
      vy = 0;
      onGround = true;
    }
  }

  // speed & score
  score += 0.15;
  speedPx = min(3.0f + score / 150.0f, 6.0f);

  // ground scroll
  for (int i = 0; i < 8; i++) {
    groundDots[i] -= (int)speedPx;
    if (groundDots[i] < 0) groundDots[i] = SCREEN_W + random(0, 20);
  }

  // obstacles
  distSinceSpawn += speedPx;
  if (distSinceSpawn >= nextGap) spawnObstacle();

  for (auto &o : obs) {
    if (!o.active) continue;
    o.x -= speedPx;
    if (o.x + OBS_W[o.type] < 0) { o.active = false; continue; }
    if (hitTest(o)) {
      state = OVER;
      overTime = millis();
      btnReleased = false;          // must release the button before restart
      if ((uint32_t)score > hiScore) {
        hiScore = (uint32_t)score;
        prefs.putUInt("hi", hiScore);
      }
      return;
    }
  }
}

void render() {
  display.clearDisplay();
  drawGround();
  for (auto &o : obs) if (o.active) drawObstacle(o);
  drawDino();
  drawScore();

  if (state == WAITING) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    centerText("Press to start", 24);
  } else if (state == OVER) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    centerText("G A M E  O V E R", 12);
    drawRestartIcon(64, 32);
  }
  display.display();
}

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for (;;) delay(1000);   // OLED not found - check wiring/address
  }
  prefs.begin("dino", false);
  hiScore = prefs.getUInt("hi", 0);
  randomSeed(esp_random());
  resetGame();
  render();
}

void loop() {
  if (millis() - lastFrame < FRAME_MS) return;
  lastFrame = millis();

  switch (state) {
    case WAITING:
      if (pressed()) { resetGame(); state = RUNNING; }
      break;
    case RUNNING:
      updateGame();
      break;
    case OVER:
      if (!pressed()) btnReleased = true;   // must let go of the button first
      if (btnReleased && millis() - overTime > 500 && pressed()) {
        resetGame();
        state = RUNNING;
      }
      break;
  }
  render();
}