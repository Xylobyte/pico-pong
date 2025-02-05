#include <cmath>
#include <ctime>

#include "libraries/pico_display_28/pico_display_28.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include "button.hpp"
#include "pico/rand.h"

using namespace pimoroni;
using namespace std;

ST7789 st7789(320, 240, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

RGBLED led(PicoDisplay28::LED_R, PicoDisplay28::LED_G, PicoDisplay28::LED_B);

Button button_a(PicoDisplay28::A);
Button button_b(PicoDisplay28::B);
Button button_x(PicoDisplay28::X);
Button button_y(PicoDisplay28::Y);

struct Vector2D {
    float x;
    float y;
};

struct VectorDir2D {
private:
    float angle;
    Vector2D dir;

    void normalize() {
        float length = sqrt(this->dir.x * this->dir.x + this->dir.y * this->dir.y);
        if (length > 0) {
            this->dir.x /= length;
            this->dir.y /= length;
        }
    }

public:
    void setAngle(float val) {
        this->angle = val;
        auto radAngle = val * (static_cast<float>(M_PI) / 180.0f);

        this->dir.x = cos(radAngle);
        this->dir.y = sin(radAngle);

        this->normalize();
    }

    float getAngle() { return this->angle; }

    void setDir(Vector2D val) {
        this->dir.x = val.x;
        this->dir.y = val.y;

        this->normalize();
    }

    Vector2D getDir() { return this->dir; }
};

// Ball variables
const float ballRadius = 4;
const auto screenW = (float) st7789.width;
const auto screenH = (float) st7789.height;
Vector2D ballPos(.0f, .0f);
VectorDir2D ballDir;
float ballSpeed = 1.2f;

// Rackets variables
const float racketWidth = 3;
const float racketSpeed = 2.5f;
const float hMargin = 8;
float racketHeight = 45;

// Players positions
Vector2D leftPlayerPos(hMargin, static_cast<float>(st7789.height) / 2 - racketHeight / 2);
Vector2D rightPlayerPos(static_cast<float>(st7789.width) - hMargin - racketWidth, static_cast<float>(st7789.height) / 2 - racketHeight / 2);

// Players scores
uint8_t leftPlayerScore = 0;
uint8_t rightPlayerScore = 0;

// Set colors pong
Pen bgColor = graphics.create_pen(0, 0, 0);
Pen ballColor = graphics.create_pen(255, 255, 255);
Pen leftPlayerColor = graphics.create_pen(255, 0, 0);
Pen rightPlayerColor = graphics.create_pen(0, 0, 255);
Pen otherColor = graphics.create_pen(100, 100, 100);

char winner = ' ';

float generateRandomAngle() {
    return (static_cast<float>(get_rand_32()) / static_cast<float>(UINT32_MAX)) * 360.0f;
}

void resetBall() {
    ballDir.setAngle(generateRandomAngle());

    ballSpeed = 1.2f;
    ballPos.x = screenW / 2;
    ballPos.y = screenH / 2;
}

bool gameUpdate() {
    // Players rackets update
    if (button_a.raw() && leftPlayerPos.y > 0) {
        leftPlayerPos.y -= racketSpeed;
    } else if (button_b.raw() && leftPlayerPos.y < static_cast<float>(st7789.height) - racketHeight) {
        leftPlayerPos.y += racketSpeed;
    }

    if (button_x.raw() && rightPlayerPos.y > 0) {
        rightPlayerPos.y -= racketSpeed;
    } else if (button_y.raw() && rightPlayerPos.y < static_cast<float>(st7789.height) - racketHeight) {
        rightPlayerPos.y += racketSpeed;
    }


    // Ball update
    Vector2D tmpBallPos(ballPos.x + ballDir.getDir().x * ballSpeed, ballPos.y + ballDir.getDir().y * ballSpeed);

    // Check if it hits the left or right paddle
    if ((tmpBallPos.x - ballRadius) < leftPlayerPos.x + racketWidth &&
        (tmpBallPos.y + ballRadius) > leftPlayerPos.y &&
        (tmpBallPos.y - ballRadius) < leftPlayerPos.y + racketHeight) {
        auto newDir = ballDir.getDir();
        newDir.x = -newDir.x;
        ballDir.setDir(newDir);
    } else if ((tmpBallPos.x + ballRadius) > rightPlayerPos.x &&
               (tmpBallPos.y + ballRadius) > rightPlayerPos.y &&
               (tmpBallPos.y - ballRadius) < rightPlayerPos.y + racketHeight) {
        auto newDir = ballDir.getDir();
        newDir.x = -newDir.x;
        ballDir.setDir(newDir);
    }

    // Ensure it does not go outside screen on the left or right side
    if ((tmpBallPos.x - ballRadius) < 0) {
        ++rightPlayerScore;
        resetBall();
        return false;
    }
    if ((tmpBallPos.x + ballRadius) > screenW) {
        ++leftPlayerScore;
        resetBall();
        return false;
    }

    // Ensure it does not go outside screen on the top or bottom side
    if ((tmpBallPos.y - ballRadius) < 0) {
        tmpBallPos.y = ballRadius;
        auto newDir = ballDir.getDir();
        newDir.y = -newDir.y;
        ballDir.setDir(newDir);
    }
    if ((tmpBallPos.y + ballRadius) > screenH) {
        tmpBallPos.y = screenH - ballRadius;
        auto newDir = ballDir.getDir();
        newDir.y = -newDir.y;
        ballDir.setDir(newDir);
    }

    ballPos = tmpBallPos;

    if (leftPlayerScore >= 6) {
        winner = 'l';
    } else if (rightPlayerScore >= 6) {
        winner = 'r';
    }

    return true;
}

void gameDraw() {
    // Background
    graphics.set_pen(bgColor);
    graphics.clear();

    // Draw players rackets and scores
    graphics.set_pen(leftPlayerColor);
    graphics.rectangle(Rect{
            static_cast<int32_t>(leftPlayerPos.x),
            static_cast<int32_t>(leftPlayerPos.y),
            static_cast<int32_t>(racketWidth),
            static_cast<int32_t>(racketHeight)
    });
    graphics.text(to_string(leftPlayerScore), Point(st7789.width / 2 - 20, 10), false);

    graphics.set_pen(rightPlayerColor);
    graphics.rectangle(Rect{
            static_cast<int32_t>(rightPlayerPos.x),
            static_cast<int32_t>(rightPlayerPos.y),
            static_cast<int32_t>(racketWidth),
            static_cast<int32_t>(racketHeight)
    });
    graphics.text(to_string(rightPlayerScore), Point(st7789.width / 2 + 10, 10), false);

    // Middle line
    graphics.set_pen(otherColor);
    graphics.rectangle(Rect{st7789.width / 2, 0, 1, st7789.height});

    // Draw ball
    graphics.set_pen(ballColor);
    graphics.circle(
            Point(static_cast<int32_t>(ballPos.x), static_cast<int32_t>(ballPos.y)),
            static_cast<int32_t>(ballRadius)
    );
}

[[noreturn]] int main() {
    st7789.set_backlight(200);
    led.set_rgb(0, 0, 0);

    resetBall();

    while (true) {
        if (gameUpdate()) {
            gameDraw();

            st7789.update(&graphics);
        }
    }
}
