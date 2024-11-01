typedef struct {
    float volume;
    float music;
} Settings;

void DrawTextCustom(const char* text, int posX, int posY, int fontSize, Color color) {
    Font currentFont = font48x;
    if(fontSize == 96)
        currentFont = font96x;
    DrawTextEx(currentFont, text, (Vector2){ posX, posY }, fontSize, 2, color);
}

float MeasureTextCustom(const char* text, int fontSize) {
    Font currentFont = font48x;
    if(fontSize == 96)
        currentFont = font96x;

    Vector2 textSize = MeasureTextEx(currentFont, text, fontSize, 2);
    return textSize.x;
}

void DrawRoundedRectangleEx(Rectangle rec, float roundness, Color color) {
    DrawRectangleRounded(rec, roundness, 12, color);
}

bool GuiButton(Rectangle bounds, const char* text) {
    bool pressed = false;
    Vector2 mousePoint = GetMousePosition();
    Color color = F_BROWN;
    Color borderColor = F_RED;
    Color textColor = F_WHITE;

    if (CheckCollisionPointRec(mousePoint, bounds)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            color = F_BROWN;
        } else {
            color = F_RED;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            pressed = true;
        }
    }

    DrawRoundedRectangleEx(bounds, BUTTON_ROUND, color);

    #if defined(PLATFORM_WEB)
        DrawRectangleRoundedLines(bounds, BUTTON_ROUND, 12, 2, borderColor);
    #else
        DrawRectangleRoundedLinesEx(bounds, BUTTON_ROUND, 12, 2, borderColor);
    #endif


    float textWidth = MeasureTextCustom(text, FONT_SIZE_NORMAL);
    float textX = bounds.x + (bounds.width - textWidth) / 2;
    float textY = bounds.y + (bounds.height - FONT_SIZE_NORMAL) / 2;

    DrawTextCustom(text, textX, textY, FONT_SIZE_NORMAL, textColor);

    return pressed;
}

void SaveSettings(Settings settings) {
    FILE* file = fopen("settings.dat", "wb");
    if (file != NULL) {
        fwrite(&settings, sizeof(Settings), 1, file);
        fclose(file);
    }
}

Settings LoadSettings(void) {
    Settings settings = { 50.0f, 50.0f };
    FILE* file = fopen("settings.dat", "rb");
    if (file != NULL) {
        fread(&settings, sizeof(Settings), 1, file);
        fclose(file);
    }
    return settings;
}

float DrawSlider(Rectangle bounds, float value, const char* text) {
    DrawRoundedRectangleEx(bounds, BUTTON_ROUND/2, F_GRAY);

    float knobX = bounds.x + (value / 100.0f) * bounds.width;

    Rectangle filledPortion = {
        bounds.x,
        bounds.y,
        knobX - bounds.x,
        bounds.height
    };

    DrawRoundedRectangleEx(filledPortion, BUTTON_ROUND/2, F_WHITE);
    DrawTextCustom(text, bounds.x, bounds.y - 30, 24, F_WHITE);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Rectangle hitbox = {
            bounds.x - 20,
            bounds.y - 20,
            bounds.width + 40,
            bounds.height + 40
        };

        if (CheckCollisionPointRec(GetMousePosition(), hitbox)) {
            float mouseX = GetMouseX();
            float newValue = ((mouseX - bounds.x) / bounds.width) * 100.0f;
            value = Clamp(newValue, 0.0f, 100.0f);
        }
    }

    return value;
}