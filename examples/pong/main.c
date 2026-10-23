#include <stdint.h>

#include <libhng64/bios.h>
#include <libhng64/core.h>
#include <libhng64/iomcu.h>
#include <libhng64/irq.h>
#include <libhng64/sprite.h>
#include <libhng64/text.h>

#include "sprites.h"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 448

#define PADDLE_WIDTH 8
#define PADDLE_HALF_HEIGHT 16
#define PADDLE_HEIGHT (PADDLE_HALF_HEIGHT * 2)
#define PADDLE_SPEED 5
#define PADDLE_MARGIN 32
#define PADDLE_LEFT_X PADDLE_MARGIN
#define PADDLE_RIGHT_X (SCREEN_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH)

#define BALL_WIDTH 12
#define BALL_HEIGHT 12
#define BALL_SPEED_X 3
#define BALL_SPEED_Y 2
#define AI_SPEED 2

// Roadedge puts the wheel on AN0, 0 to 255 with the center at 0x80
#define WHEEL_PORT 0
#define WHEEL_CENTER 128
#define WHEEL_DEADZONE 16

#define SCORE_ROW 2
#define SCORE_LEFT_COL 24
#define SCORE_RIGHT_COL 38

#define MESSAGE_ROW 20
#define MESSAGE_COL 23
#define WAIT_MESSAGE "WAITING FOR IOMCU"

#define SPRITE_DEPTH 0x10

enum sprite_slot {
    SLOT_LEFT_TOP,
    SLOT_LEFT_BOTTOM,
    SLOT_RIGHT_TOP,
    SLOT_RIGHT_BOTTOM,
    SLOT_BALL,
    SLOT_COUNT,
};

struct pong {
    int32_t left_y;
    int32_t right_y;
    int32_t ball_x;
    int32_t ball_y;
    int32_t ball_vx;
    int32_t ball_vy;
    uint32_t score_left;
    uint32_t score_right;
    uint32_t score_dirty;
};

static volatile uint32_t frames;

static void on_vblank(void)
{
    frames++;
}

static int32_t clamp_y(int32_t y)
{
    if (y < 0) {
        return 0;
    }

    if (y > SCREEN_HEIGHT - PADDLE_HEIGHT) {
        return SCREEN_HEIGHT - PADDLE_HEIGHT;
    }

    return y;
}

// TODO: Generic?
static int32_t wheel_step(uint8_t wheel)
{
    if (wheel + WHEEL_DEADZONE < WHEEL_CENTER) {
        return -1;
    }

    if (wheel > WHEEL_CENTER + WHEEL_DEADZONE) {
        return 1;
    }

    return 0;
}

static void spawn_ball(struct pong *game, int32_t direction)
{
    uint32_t points = game->score_left + game->score_right;

    game->ball_x = (SCREEN_WIDTH - BALL_WIDTH) / 2;
    game->ball_y = (SCREEN_HEIGHT - BALL_HEIGHT) / 2;
    game->ball_vx = direction * BALL_SPEED_X;

    if (points & 1) {
        game->ball_vy = BALL_SPEED_Y;
        return;
    }

    game->ball_vy = -BALL_SPEED_Y;
}

static int32_t hits_paddle(int32_t ball_x, int32_t ball_y, int32_t paddle_x, int32_t paddle_y)
{
    int32_t ball_right = ball_x + BALL_WIDTH;
    int32_t ball_bottom = ball_y + BALL_HEIGHT;
    int32_t paddle_right = paddle_x + PADDLE_WIDTH;
    int32_t paddle_bottom = paddle_y + PADDLE_HEIGHT;

    return ball_x < paddle_right && ball_right > paddle_x &&
           ball_y < paddle_bottom && ball_bottom > paddle_y;
}

static void score_point(struct pong *game, uint32_t left)
{
    if (left) {
        game->score_left++;
    } else {
        game->score_right++;
    }

    game->score_dirty = 1;
}

static void move_ball(struct pong *game)
{
    game->ball_x += game->ball_vx;
    game->ball_y += game->ball_vy;

    if (game->ball_y < 0) {
        game->ball_y = 0;
        game->ball_vy = -game->ball_vy;
    }

    if (game->ball_y + BALL_HEIGHT > SCREEN_HEIGHT) {
        game->ball_y = SCREEN_HEIGHT - BALL_HEIGHT;
        game->ball_vy = -game->ball_vy;
    }

    if (game->ball_x + BALL_WIDTH < 0) {
        score_point(game, 0);
        spawn_ball(game, -1);
        return;
    }

    if (game->ball_x > SCREEN_WIDTH) {
        score_point(game, 1);
        spawn_ball(game, 1);
        return;
    }

    if (game->ball_vx < 0 &&
        hits_paddle(game->ball_x, game->ball_y, PADDLE_LEFT_X, game->left_y)) {
        game->ball_x = PADDLE_LEFT_X + PADDLE_WIDTH;
        game->ball_vx = BALL_SPEED_X;
    }

    if (game->ball_vx > 0 &&
        hits_paddle(game->ball_x, game->ball_y, PADDLE_RIGHT_X, game->right_y)) {
        game->ball_x = PADDLE_RIGHT_X - BALL_WIDTH;
        game->ball_vx = -BALL_SPEED_X;
    }
}

static void move_ai(struct pong *game)
{
    int32_t target = game->ball_y + BALL_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    if (game->right_y < target) {
        game->right_y += AI_SPEED;
    } else if (game->right_y > target) {
        game->right_y -= AI_SPEED;
    }

    game->right_y = clamp_y(game->right_y);
}

static void clear_message(void)
{
    char blanks[sizeof(WAIT_MESSAGE)];
    uint32_t length = sizeof(WAIT_MESSAGE) - 1;

    for (uint32_t i = 0; i < length; i++) {
        blanks[i] = ' ';
    }

    blanks[length] = '\0';
    hng_text_print(blanks, MESSAGE_ROW, MESSAGE_COL, HNG_TEXT_PALETTE_WHITE);
}

static void draw_score(struct pong *game)
{
    char digit[2];

    if (!game->score_dirty) {
        return;
    }

    digit[0] = (char)('0' + game->score_left);
    digit[1] = '\0';
    hng_text_print(digit, SCORE_ROW, SCORE_LEFT_COL, HNG_TEXT_PALETTE_WHITE);

    digit[0] = (char)('0' + game->score_right);
    hng_text_print(digit, SCORE_ROW, SCORE_RIGHT_COL, HNG_TEXT_PALETTE_WHITE);

    game->score_dirty = 0;
}

static void draw_sprites(struct pong *game, struct hng_sprite *sprites)
{
    hng_sprite_set(&sprites[SLOT_LEFT_TOP], &sprite_paddle, PADDLE_LEFT_X, game->left_y, SPRITE_DEPTH);
    hng_sprite_set(&sprites[SLOT_LEFT_BOTTOM], &sprite_paddle, PADDLE_LEFT_X,
                   game->left_y + PADDLE_HALF_HEIGHT, SPRITE_DEPTH);
    hng_sprite_set(&sprites[SLOT_RIGHT_TOP], &sprite_paddle, PADDLE_RIGHT_X, game->right_y, SPRITE_DEPTH);
    hng_sprite_set(&sprites[SLOT_RIGHT_BOTTOM], &sprite_paddle, PADDLE_RIGHT_X,
                   game->right_y + PADDLE_HALF_HEIGHT, SPRITE_DEPTH);
    hng_sprite_set(&sprites[SLOT_BALL], &sprite_ball, game->ball_x, game->ball_y, SPRITE_DEPTH);
}

int main(void)
{
    struct hng_iomcu_inputs inputs = {0};
    struct hng_sprite sprites[SLOT_COUNT];
    struct pong game = {0};
    uint32_t seen = 0;
    uint32_t started = 0;

    hng_core_init();
    hng_bios_text_init();

    hng_sprite_init();
    hng_sprite_load_palette(&sprite_paddle);
    hng_sprite_load_palette(&sprite_ball);

    hng_irq_init();
    hng_irq_set_handler(HNG_IRQ_VBLANK, on_vblank);
    hng_irq_enable();

    hng_text_print("PONG", 0, 30, HNG_TEXT_PALETTE_RED);
    hng_text_print(WAIT_MESSAGE, MESSAGE_ROW, MESSAGE_COL, HNG_TEXT_PALETTE_WHITE);

    inputs.analog[WHEEL_PORT] = WHEEL_CENTER;
    game.left_y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    game.right_y = game.left_y;
    game.ball_x = (SCREEN_WIDTH - BALL_WIDTH) / 2;
    game.ball_y = (SCREEN_HEIGHT - BALL_HEIGHT) / 2;
    game.score_dirty = 1;

    draw_score(&game);
    draw_sprites(&game, sprites);
    hng_sprite_copy(sprites, 0, SLOT_COUNT);

    for (;;) {
        if (frames == seen) {
            continue;
        }

        seen = frames;

        if (hng_iomcu_poll(&inputs) == HNG_IOMCU_FRESH && !started) {
            started = 1;
            clear_message();
            spawn_ball(&game, 1);
        }

        game.left_y = clamp_y(game.left_y + wheel_step(inputs.analog[WHEEL_PORT]) * PADDLE_SPEED);

        if (started) {
            move_ai(&game);
            move_ball(&game);
        }

        draw_score(&game);
        draw_sprites(&game, sprites);
        hng_sprite_copy(sprites, 0, SLOT_COUNT);
    }
}
