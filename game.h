char map[50 * 50] =
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                ###                               "
"                #C                 ## C           "
"              # ###                 ####          "
"                             C     ##             "
"                #            #                    "
"      C                      #       #            "
"    #####         C     #    #       #C           "
"      #           #     #             #           "
"                 ###    #                         "
"    ###                              #            "
"                              C      #            "
"                       ##     #      #            "
"     ##      ### #            #       #           "
"               # #           ####          C C    "
"C              # #           #  C          # #    "
"##  ###        # #           #             ###    "
"#        #  #   C        #   #C             #   C "
"##       #CC#   #       ######              #   # "
"#        ####                               #     "
"##                   #           C                "
"                                 #          ##    "
" #                       C                   #    "
"              ####                     C     #    "
"C                         C           ###         "
"#         #              C C         ##           "
" #       #                                        "
"  #  C  #                               C         "
"           ###       ###     #    #     #         "
"                             #    #     #     ### "
"                C C                           #   "
"               #####                          ### "
"                                              #   "
"                                              #   ";

#define MAX_RECTS 200
#define MAX_ROPE_POINTS 256
#define MAX_COINS 50
#define PLAYER_SPEED 200.0f
#define GRAVITY 800.0f
#define ROPE_GRAVITY 1200.0f
#define JUMP_FORCE -450.0f
#define ROPE_SEGMENT_LENGTH 10.0f
#define ROPE_ITERATIONS 20
#define ROPE_DAMPING 0.99f
#define ROPE_RADIUS 6.0f
#define COIN_RADIUS 10.0f

typedef struct {
    Vector2 position;
    Vector2 prevPosition;
    Vector2 velocity;
    float radius;
} Player;

typedef struct {
    Vector2 position;
    Vector2 prevPosition;
    Vector2 velocity;
} RopePoint;

typedef struct {
    Vector2 position;
    bool active;
} Coin;

Vector2 ropeAnchor;
int score = 0;
Coin coins[MAX_COINS];
int coinCount = 0;

Camera2D camera = { 0 };
Rectangle rects[MAX_RECTS] = { 0 };
int rectCount = 0;
Player player;
RopePoint ropePoints[MAX_ROPE_POINTS];

Sound jumpSound;
Sound coinSound;

void InitGame() {
    #if defined(PLATFORM_WEB)
    jumpSound = LoadSound("bin/resources/jump.wav");
    coinSound = LoadSound("bin/resources/coin.qoa");
    #else
    jumpSound = LoadSound("resources/jump.wav");
    coinSound = LoadSound("resources/coin.qoa");
    #endif

    SetSoundVolume(jumpSound, settings.volume / 100.0F);
    SetSoundVolume(coinSound, settings.volume / 100.0F);

    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + 50 };
    camera.rotation = 0.0f;
    camera.zoom = 0.75f;

    rects[0] = (Rectangle){ -4000, 0, 8000, 1000 };
    rectCount = 1;

    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            if(map[y * 50 + x] == (char)'#') {
                rects[rectCount] = (Rectangle){ -1000 + x * 40, (50 - y) * -40, 40, 40 };
                rectCount++;
            } else if(map[y * 50 + x] == (char)'C') {
                coins[coinCount].position = (Vector2){-1000 + x * 40 + 20, (50 - y) * -40 + 20};
                coins[coinCount].active = true;
                coinCount++;
            }
        }
    }

    player.position = (Vector2){ 0, -50 };
    player.velocity = (Vector2){ 0, 0 };
    player.radius = 15.0f;

    ropeAnchor = (Vector2){ 0, 0 };


    for (int i = 0; i < MAX_ROPE_POINTS; i++) {
        ropePoints[i].position = player.position;
        ropePoints[i].prevPosition = player.position;
        ropePoints[i].velocity = (Vector2){ 2, 0 };
    }


    score = 0;
}

float GetSDFDistance(Vector2 point) {
    float dist = 10000.0f;
    for (int i = 0; i < rectCount; i++) {
        Rectangle rect = rects[i];
        Vector2 rectCenter = (Vector2){ rect.x + rect.width / 2, rect.y + rect.height / 2 };

        if(i != 0 && Vector2LengthSqr(Vector2Subtract(point, rectCenter)) > 40 * 40 * 8) continue;

        Vector2 d = (Vector2){
            fabs(point.x - rectCenter.x) - rect.width / 2,
            fabs(point.y - rectCenter.y) - rect.height / 2
        };
        float outsideDist = Vector2Length((Vector2){ fmaxf(d.x, 0.0f), fmaxf(d.y, 0.0f) });
        float insideDist = fminf(fmaxf(d.x, d.y), 0.0f);
        float totalDist = outsideDist + insideDist;
        if(totalDist < dist) {
            dist = totalDist;
        }
    }

    return dist;
}

float GetSDFDistanceRope(Vector2 point) {
    float dist = 10000.0f;
    for (int i = 0; i < rectCount; i++) {
        Rectangle rect = rects[i];
        Vector2 rectCenter = (Vector2){ rect.x + rect.width / 2, rect.y + rect.height / 2 };

        if(i != 0 && Vector2LengthSqr(Vector2Subtract(point, rectCenter)) > 40 * 40 * 8) continue;

        Vector2 d = (Vector2){
            fabs(point.x - rectCenter.x) - (rect.width - 5) / 2,
            fabs(point.y - rectCenter.y) - (rect.height - 5) / 2
        };
        float outsideDist = Vector2Length((Vector2){ fmaxf(d.x, 0.0f), fmaxf(d.y, 0.0f) });
        float insideDist = fminf(fmaxf(d.x, d.y), 0.0f);
        float totalDist = outsideDist + insideDist - 5;
        if(totalDist < dist) {
            dist = totalDist;
        }
    }

    return dist;
}

Vector2 CalculateSDFNormal(Vector2 point) {
    float distX = GetSDFDistance((Vector2){ point.x + 0.01f, point.y});
    float distY = GetSDFDistance((Vector2){ point.x, point.y + 0.01f });
    float distCenter = GetSDFDistance((Vector2){ point.x, point.y});
    Vector2 normal = (Vector2){
        distX - distCenter,
        distY - distCenter
    };
    return Vector2Normalize(normal);
}

Vector2 CalculateSDFNormalRope(Vector2 point) {
    float distX = GetSDFDistanceRope((Vector2){ point.x + 0.01f, point.y});
    float distY = GetSDFDistanceRope((Vector2){ point.x, point.y + 0.01f });
    float distCenter = GetSDFDistanceRope((Vector2){ point.x, point.y});
    Vector2 normal = (Vector2){
        distX - distCenter,
        distY - distCenter
    };
    return Vector2Normalize(normal);
}

void CheckCollisions() {
    float dist = GetSDFDistance(player.position);
    if (dist < player.radius) {
        Vector2 normal = CalculateSDFNormal(player.position);
        player.position = Vector2Add(player.position, Vector2Scale(normal, player.radius - dist));
    }
}

bool IsGrounded() {
    float dist = GetSDFDistance((Vector2){player.position.x, player.position.y + 10});
    if (dist < player.radius) {
        return true;
    }
    return false;
}

void ApplyRopeConstraint(int i, int j, float lenght) {
    Vector2 delta = Vector2Subtract(ropePoints[j].position, ropePoints[i].position);
    float distance = Vector2Length(delta);

    if (distance > 0.0f) {
        float difference = (distance - lenght) / distance;
        Vector2 correction = Vector2Scale(delta, 0.5f * difference);

        if (i == 0) {
            ropePoints[j].position = Vector2Subtract(ropePoints[j].position, Vector2Scale(correction, 2.0f));
        } else {
            ropePoints[i].position = Vector2Add(ropePoints[i].position, correction);
            ropePoints[j].position = Vector2Subtract(ropePoints[j].position, correction);
        }
    }
}

void HandleRopeCollision(int i) {
    float dist = GetSDFDistanceRope(ropePoints[i].position);
    if (dist < ROPE_RADIUS) {
        Vector2 normal = CalculateSDFNormalRope(ropePoints[i].position);
        ropePoints[i].position = Vector2Add(ropePoints[i].position, Vector2Scale(normal, ROPE_RADIUS - dist));
    }
}

void UpdateRopePhysics(float dt) {
    ropePoints[0].position = player.position;
    ropePoints[0].velocity = player.velocity;

    ropePoints[MAX_ROPE_POINTS - 1].position = ropeAnchor;
    ropePoints[MAX_ROPE_POINTS - 1].velocity = (Vector2){0, 0};

    for (int i = 1; i < MAX_ROPE_POINTS - 1; i++) {
        ropePoints[i].prevPosition = ropePoints[i].position;
        ropePoints[i].velocity.y += ROPE_GRAVITY * dt;
        ropePoints[i].velocity = Vector2Scale(ropePoints[i].velocity, ROPE_DAMPING);
        ropePoints[i].position = Vector2Add(ropePoints[i].position, Vector2Scale(ropePoints[i].velocity, dt));
    }

    for (int iter = 0; iter < ROPE_ITERATIONS; iter++) {
        ropePoints[MAX_ROPE_POINTS - 1].position = ropeAnchor;
        ropePoints[MAX_ROPE_POINTS - 1].velocity = (Vector2){0, 0};

        for (int i = 0; i < MAX_ROPE_POINTS - 1; i++) {
            ApplyRopeConstraint(i, i + 1, ROPE_SEGMENT_LENGTH);
        }

        for (int i = 1; i < MAX_ROPE_POINTS - 1; i++) {
            HandleRopeCollision(i);
        }
    }

    for (int i = 1; i < MAX_ROPE_POINTS - 1; i++) {
        Vector2 positionDelta = Vector2Subtract(ropePoints[i].position, ropePoints[i].prevPosition);
        ropePoints[i].velocity = Vector2Scale(positionDelta, 1.0f / dt);
    }

    ropePoints[0].position = player.position;
    ropePoints[0].velocity = player.velocity;

    ropePoints[MAX_ROPE_POINTS - 1].position = ropeAnchor;
    ropePoints[MAX_ROPE_POINTS - 1].velocity = (Vector2){0, 0};
}

void UpdateCoins(float dt) {
    for (int i = 0; i < coinCount; i++) {
        if (!coins[i].active) continue;

        if (Vector2Distance(coins[i].position, player.position) < (player.radius + COIN_RADIUS)) {
            coins[i].active = false;
            score += 10;
            PlaySound(coinSound);
        }
    }
}

void DrawGame() {
    camera.target = player.position;
    BeginMode2D(camera);

    for (int i = 0; i < rectCount; i++) {
        DrawRectangleRec(rects[i], F_GRAY);
    }

    for (int i = 0; i < MAX_ROPE_POINTS - 1; i++) {
        DrawLineEx(ropePoints[i].position, ropePoints[i + 1].position, ROPE_RADIUS * 2, F_BROWN);
        DrawCircleV(ropePoints[i].position, ROPE_RADIUS, F_BROWN);
    }

    for (int i = 0; i < coinCount; i++) {
        if (!coins[i].active) continue;
        DrawCircleV(coins[i].position, COIN_RADIUS, F_YELLOW);
    }

    DrawCircleV(ropeAnchor, 10.0f, F_RED);
    DrawCircleV(player.position, player.radius, F_GREEN);

    EndMode2D();

    DrawTextCustom(TextFormat("SCORE: %d", score), 9, 10, 24, F_BLACK);
    DrawTextCustom(TextFormat("SCORE: %d", score), 10, 10, 24, F_ORANGE);
}

void UpdateGame() {
    float dt = 1.0f / 60.0f;

    player.prevPosition = player.position;
    player.velocity.x *= 0.90f;
    player.velocity.y += GRAVITY * dt;
    player.position = Vector2Add(player.position, Vector2Scale(player.velocity, dt));
    CheckCollisions();
    player.velocity = Vector2Scale(Vector2Subtract(player.position, player.prevPosition), 1 / dt);
    CheckCollisions();

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player.velocity.x = -PLAYER_SPEED;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player.velocity.x = PLAYER_SPEED;
    if ((IsKeyPressed(KEY_SPACE) || IsKeyDown(KEY_UP)) && IsGrounded()) {
        player.velocity.y = JUMP_FORCE;
        PlaySound(jumpSound);
    }

    if ((IsKeyPressed(KEY_Q)) ) {
        player.velocity.y = JUMP_FORCE;
        PlaySound(jumpSound);
    }

    if(IsKeyDown(KEY_E) || IsKeyDown(KEY_LEFT_CONTROL)){

        ropePoints[MAX_ROPE_POINTS - 1].position = ropeAnchor;
        ropePoints[MAX_ROPE_POINTS - 1].velocity = (Vector2){0, 0};

        ropePoints[0].position = player.position;
        ropePoints[0].velocity = player.velocity;

        for (int i = 1; i < MAX_ROPE_POINTS - 1; i++) {
            ropePoints[i].position = ropePoints[i + 1].position;
        }
        for (int i = 1; i < MAX_ROPE_POINTS - 1; i++) {
            HandleRopeCollision(i);
        }

        CheckCollisions();

        player.velocity = (Vector2){ 0, 0 };

    }
    else
    {
        UpdateRopePhysics(dt);
    }

    UpdateCoins(dt);

    Vector2 toFirstSegment = Vector2Subtract(ropePoints[1].position, player.position);
    float distanceToFirst = Vector2Length(toFirstSegment);

    if (distanceToFirst > ROPE_SEGMENT_LENGTH + 4) {
        Vector2 direction = Vector2Normalize(toFirstSegment);
        float stretch = distanceToFirst - ROPE_SEGMENT_LENGTH - 4;

        Vector2 ropeForce = Vector2Scale(direction, stretch);
        player.position = Vector2Add(player.position, ropeForce);
        CheckCollisions();
    }

    DrawGame();
}